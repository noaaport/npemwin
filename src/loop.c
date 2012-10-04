/*
 * Copyright (c) 2004-2012 Jose F. Nieves <nieves@ltp.uprrp.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#include "config.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <pthread.h>
#include <errno.h>
#include <netdb.h> /* gai_strerror */
#include <string.h>
#include <syslog.h>
#include "signal.h"
#include "readn.h"
#include "util.h"
#include "emwin.h"
#include "servers.h"
#include "bb.h"
#include "err.h"
#include "conf.h"
#include "globals.h"
#include "init.h"	/* cleanup() */
#include "server_priv.h"
#include "per.h"
#include "loop.h"

/*
 * These are not mutex protected since they are used only within
 * the server thread (the only thread in npemwin, apart from the
 * client threads.
 */
static int greload_servers_list_flag = 0;
static int gsend_bbclientid_flag = 0;

static int get_reload_servers_list_flag(void);
static int get_send_bbclientid_flag(void);

static void reload_servers_list(void);

static int process_packets(struct emwin_server *emserver);
static void exec_qrunner(void);
static int exec_filter(char *filename);

int loop(void){

  int i;
  int status = 0;
  struct emwin_server *es;

  es = get_next_server();

  if(es == NULL){
    status = 1;
    log_errx("No servers available.");
    log_info("Waiting %d seconds.", g.retrysleep);
    for(i = 0; i <= g.retrysleep; ++i){
      sleep(1);
      if(get_quit_flag() != 0)
	break;
    }
    log_info("Trying server list again."); 
  }else if(es->fd == -1){
    if(es->gai_code != 0){
      log_errx("Cannot open connection to %s. %s", es->ip,
	       gai_strerror(es->gai_code));
    }else{
      log_err2("Cannot open connection to", es->ip);
    }
    status = 1;
  }else if(es->fd == -2){
    if(server_type_serial_device(es))
      log_errx("Cannot configure or synchronize %s:%s", es->ip, es->port);
    else
      log_errx("Could not get packet from %s", es->ip);

    status = 1;
  }else{    
    log_info("Connected to %s @ %s", es->ip, es->port);
  }

  while((status == 0) && (get_quit_flag() == 0)){
    if(g.f_server_enabled == 1)
      server_loop();

    status = process_packets(es);

    periodic();

    if(get_reload_servers_list_flag())
      reload_servers_list();

    if(get_send_bbclientid_flag())
      bb_send_clientid(es);
  }

  if(get_quit_flag() != 0)
    log_info("Closing processor.");

  return(status);
}

static int process_packets(struct emwin_server *emserver){

  int status;
  int skip = 0;
  struct emwin_packet ep;
  int server_fd = emserver->fd;

  if(server_type_wx14_device(emserver))
    status = get_emwin_packet_wx14(server_fd, &ep);
  else if(server_type_serial_device(emserver))
    status = get_emwin_packet_serial(server_fd, &ep);
  else
    status = get_emwin_packet_bb(server_fd, &ep);

  update_emwin_server_stats(status);
  if(write_emwin_server_stats(g.emwinstatusfile) != 0)
    log_err2("Error writing status file", g.emwinstatusfile);

  if(server_type_wx14_device(emserver)){
    if(status == -1)
      log_err2("Error reading packet from WX14 device:", emserver->ip);
    else if(status != 0)
      log_errx("Error [%d] reading packet from WX14 device:", emserver->ip);
  } else {
    if(status == -1)
      log_err2("Error reading packet from", emserver->ip);
    else if(status == -2)
      log_errx("Timedout trying to get packet from %s.", emserver->ip);
    else if(status == -3)
      log_info("Connection closed by %s.", emserver->ip);
    else if(status == 1)
      log_errx("Short read from %s.", emserver->ip);
    else if(status == 2)
      log_errx("Error in header format or unrecognized packet type.");
    else if(status == 3)
      log_errx("Checksum error.");
    else if(status == 4)
      log_errx("Check file name error: %s", ep.header.filename);
    else if(status != 0)
      log_errx("Cannot process packet: unknown error from get_emwin_packet()");
  }

  if(status != 0)    
    return(status);

  /*
   * Sends the packet to the queues for retransmission to the clients.
   */
  if(g.f_server_enabled == 1)
    (void)server_send_client_queues(&ep);

  if(ep.bbtype == BB_PACKET_TYPE_SRVLIST){
    status = save_server_list(&ep);
    if(status != 0)
      log_err("Error saving the server list.");
    else
      log_info("Received the server list.");

    return(0);
  }

  /*
   * Handle and save the data packet.
   */
  if(ep.header.blockno == 1){
    /*
     * Only the first block needs to be checked because, if the file
     * is rejected, it is not inserted in the queue and subsequent blocks
     * will be discarded.
     * "filename" is the product name and the extension; e.g.,
     * cfwsjupr.txt
     */
    skip = exec_filter(ep.header.filename);
    if(skip == 1){
      log_verbose(1, "Prefilter is rejecting %s.", ep.header.filename);
      return(0);
    }
  }

  log_verbose(1, "Received: %s (%d of %d)", 
	      ep.header.filename, ep.header.blockno, ep.header.numblocks);

  status = save_emwin_packet(&ep);

  if(status == -1)
    log_err2("Error saving", ep.header.filename);
  else
    log_verbose(1, "Saved: %s (%d of %d)", 
		ep.header.filename, ep.header.blockno, ep.header.numblocks);
  
  if(status == 0)
    ++g.packet_count;

  return(status);
}

void runq(void){

  if(g.qrunner_pid > 0){
    /*
     * This should not be seen, since runq() is called only if wait_qrunner
     * returns -1 (in per.c).
     */
    log_verbose(1, "A previous qrunner is runnning. Skiping.");
    return;
  }

  if((g.qrunreportperiod > 0) && (g.qrun_count >= g.qrunreportperiod)){
    log_info("Que has been processed %d times.", g.qrun_count);
    g.qrun_count = 0;
  }

  exec_qrunner();
}

static void exec_qrunner(void){

  int status = 0;
  pid_t pid;

  ++g.qrun_count;

  pid = fork();
  if(pid == -1)
    status = -1;
  else if(pid == 0){
    /*
     * child executes the qrunner
     */
    if(g.f_debug == 1)
      log_info("Processing que. Executing %s.", g.qrunner);

    status = execl(g.qrunner, g.qrunner, NULL);
    if(status == -1)
      log_err2("Cannot exec", g.qrunner);

    _exit(1);
  } else
    g.qrunner_pid = pid;

  if(status == -1){
    /*
     * system error from fork()
     */
    log_err2("Error executing", g.qrunner);
  }
}

int wait_qrunner(void){
/*
 * This function is called from periodic().
 *
 * Returns:
 *  0 => A qrunner is still running
 * -1 => No qrunner running.
 * > 0 => Just caught a qrunner exiting.
 */
  pid_t pid = 0;
  int wait_status = 0;
  int exit_status = 0;
  int signo = 0;

  if(g.qrunner_pid < 0)
    return(-1);

  pid = waitpid(g.qrunner_pid, &wait_status, WNOHANG);
  if(pid <= 0)
    return(pid);
  
  /*
   * log_info("A child was awaited.");
   */
  if(WIFEXITED(wait_status))
    exit_status = WEXITSTATUS(wait_status);
  else if(WIFSIGNALED(wait_status)){
    exit_status = 1;
    signo =  WTERMSIG(wait_status);	/* signal number */
  }

  if(exit_status != 0){
    if(signo != 0){
      if(g.qrunner_pid == pid)
	log_errx("%s exited abnormally with signal %d.", g.qrunner, signo);
      else
	log_errx("%u exited abnormally with signal %d.",
		 (unsigned int)pid, signo);
    }else{
      if(g.qrunner_pid == pid)
	log_errx("%s exited with error %d.", g.qrunner, exit_status);
      else
	log_errx("%u exited with error %d.", (unsigned int)pid, exit_status);
    }
  }

  if(pid == g.qrunner_pid)
    g.qrunner_pid = -1;

  return(pid);
}

static int exec_filter(char *filename __attribute__((unused))){
  /*
   * The filter should return:
   * 1 => skip this file
   * 0 => do not skip the file
   * -1 => error
   *
   * This should be implemented as a tcl script that is evaluated in its
   * own namespace (interp).
   */
  char *filter = g.prefilter;

  if(valid_str(filter) == 0)
    return(0);

  return(0);
}

static int get_reload_servers_list_flag(void){

  int r;

  r = greload_servers_list_flag;
  greload_servers_list_flag = 0;

  return(r);
}

static int get_send_bbclientid_flag(void){

  int r;

  r = gsend_bbclientid_flag;
  gsend_bbclientid_flag = 0;

  return(r);
}

void set_reload_servers_list_flag(void){

  greload_servers_list_flag = 1;
}

void set_send_bbclientid_flag(void){

  gsend_bbclientid_flag = 1;
}

static void reload_servers_list(void){

  int status = 0;

  release_server_list();
  status = get_server_list(g.serverslistfile);
  if(status == -1){
    log_err2("Error reading", g.serverslistfile);
    exit(1);
  }else if(status == 1){
    log_errx("Error reading %s.", g.serverslistfile);
    exit(1);
  }else{
    log_info("Reread server list.");
  }
}

#if 0
static void write_environ(char *fname){

  extern char **environ;
  int i;
  FILE *f;

  f = fopen(fname, "w");
  if(f == NULL){
    log_err_open(fname);
    return;
  }

  for(i = 0; environ[i] != NULL; ++i)
    fprintf(f, "%s\n", environ[i]);

  fclose(f);
}
#endif
