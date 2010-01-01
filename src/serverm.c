/*
 * Copyright (c) 2004-2006 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */

/*
 * This file contains the functions to implement the server part
 * of the program.
 */
#include <assert.h>
#include <unistd.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <fcntl.h>
#include "libconnth/libconn.h"
#include "access.h"
#include "util.h"
#include "readn.h"
#include "globals.h"
#include "err.h"
#include "defaults.h"
#include "emwin.h"
#include "appdata.h"
#include "bb.h"
#include "servers.h"
#include "server.h"
#include "server_priv.h"

/*
 * This is not mutex protected since they are used only within
 * the server thread (the only thread in npemwin, apart from the
 * client threads.
 */
static int greport_client_connections_flag = 0;

static int handle_client(struct conn_table_st *ct, int i);
static void handle_client_hup(struct conn_table_st *ct, int i, int condition);
static void spawn_client_threads(void);
static int client_thread_create(struct conn_element_st *ce, pthread_t *t_id);
static int client_thread_kill(struct conn_element_st *ce);

/*
 * This portion is copied from nbsp.
 *
 * The process_connections() function is executed periodically,
 * but we feel there is no need to let the period be a run-time
 * configuration option. In case we later decide to do that, we leave
 * here some place holders for the flag functions that can be used in
 * per.c, similar to the other flags of the server.
 *
 * static int gprocess_connections = 0;
 * static pthread_mutex_t gprocess_connections_mutex =
 * PTHREAD_MUTEX_INITIALIZER;
 */
#define SERVER_PROCESS_CONNECTIONS_PERIOD_SECS 1
static time_t gprocess_connections_time = 0;	/* next time to process */
static int get_process_connections_flag(void);
static void process_connections(void);
static void process_unidentified_connections(void);
static void process_finished_connections(void);
static void print_client_connections(FILE *fp);

static int get_report_client_connections_flag(void);
static void report_client_connections(void);

int init_server(void){
  /*
   * Opens the socket for network clients (via the libconn library).
   */
  int status = 0;
  int gai_code;
  int backlog = g.listen_backlog;

  if(valid_str(g.serverport) == 0){
    log_errx("Server port not set.");
    return(1);
  }

  /*
   * Use the OS defaults for the so_sndbuf/so_rcvbuf.
   */
  g.server_fd = tcp_server_open_conn(g.servername, g.serverport, backlog,
				     -1, -1, NULL, &gai_code);
  if(g.server_fd == -1){
    if(gai_code != 0)
      log_errx("Cannot open listening port. %s", gai_strerror(gai_code));
    else
      log_err("Cannot open listening port.");

    return(-1);
  }

  if(fcntl(g.server_fd, F_SETFD, FD_CLOEXEC) == -1){
    log_err("Cannot set cloexec flag on server fd.");
    close(g.server_fd);
    g.server_fd = -1;
    return(-1);
  }

  g.ct = conn_table_create(g.maxnetclients,
			   handle_client,
			   handle_client_hup,
			   client_allow_nconn);
  if(g.ct == NULL){
    log_err("Cannot init server.");
    return(-1);
  }

  /* 
   * This server does not (yet) have a control socket. The server entry
   * is added with pid = 0, ip = NULL, name = NULL,
   * and the options (timeout_ms, timeout_retry, reconnect_sleep,
   * reconnect_retry) 0.
   */
  status = conn_table_add_element(g.ct, g.server_fd,
				  CONN_TYPE_SERVER_NET, 0, NULL, NULL,
				  0, 0, 0, 0);

  if(status != 0)
    log_err("Cannot init server.");
  else
    log_info("Server initialized.");

  return(status);
}

void terminate_server(void){
  /*
   * When each g.ct->ce element is released, the corresponding client thread
   * is also killed by the callback function ce->thkillproc (defined in
   * serverc.c).
   */

  if(g.ct != NULL){
    conn_table_destroy(g.ct);
    g.ct = NULL;
  }

  if(g.server_fd != -1){
    close(g.server_fd);
    g.server_fd = -1;
  }
}

void server_loop(void){

  if(get_process_connections_flag()){
    process_connections();
  }

  if(get_report_client_connections_flag())
    report_client_connections();
}

static int handle_client(struct conn_table_st *ct, int i){

  pid_t pid;
  int fd;
  size_t size = EMWIN_PACKET_SIZE;	/* How large can the login id be? */
  char buf[EMWIN_PACKET_SIZE];
  ssize_t n;
  char *ip;
  int protocol;
  int ident_status;

  pid = conn_table_get_element_pid(ct, i);
  assert(pid == -1);		/* this server only has network clients */
  ip = conn_table_get_element_ip(ct, i);
  fd = conn_table_get_element_fd(ct, i);

  /*
   * BB clients send a login id when they connect (and keep sending it),
   * of the form
   *          ByteBlast Client|NM-email@server.com|V1
   * The Wxmesg (Net) client sends something else, but unxored; e.g.
   *          C002CYGWIN-nieves3.2.2843
   * The 002 is configurable. In addition it expects the data to be unxored
   * as well.
   */
  memset(buf, '\0', size);
  n = read1(fd, buf, size - 1, 0, 0);
  if(n > 0){
    bb_xor(buf, buf, n);
    log_info("From client %s: %s", ip, buf);
    ident_status = ident_client_protocol(ct, i, buf, (size_t)n);
    protocol = get_client_protocol(ct, i);
    /*
     * A return code == 2 => client already identified, and we ignore it.
     */
    if(ident_status == 0)
      log_info("Client protocol: %d", protocol);
    else if(ident_status == 1)
      log_info("Client protocol default: %d", protocol);
    else if(ident_status == -1)
      log_err("Error from ident_client_protocol.");

    if((g.serverprotocol == PROTOCOL_EMWIN2) && (protocol != PROTOCOL_EMWIN2)){
      log_info("Serving only V2.");
      log_info("Rejecting %s.", ip);
      (void)poll_kill_client_connection(ct, i);
    }      
  } else {
    /*
     * Connection reset by peer, or disconnected. This should have been
     * caught by the libconn function poll_client_hangup()
     * [which calls the function below] so that these
     * two error messages should really not appear.
     */
    if(n < 0)
      log_err2("Unexpected app error from client", ip);
    else
      log_info("Unexpected app error. Client %s disconnected", ip);

    (void)poll_kill_client_connection(ct, i);
  }

  return(0);
}

static void handle_client_hup(struct conn_table_st *ct, int i, int condition){
  /*
   * This function is called by poll_client_hangup() in libconn2/poll.c
   * to notify the application when it has detected that a client has closed,
   * and it is about to delete the corresponding element from the table.
   * It is called if the descriptor has (POLLHUP | POLLERR | POLLNVAL) set;
   * or POLLIN in a condition that a read will return 0 or -1. 
   */
  char *ip;

  assert(condition != 0);

  ip = conn_table_get_element_ip(ct, i);
  if(condition > 0)
    log_info("Client %s disconnected", ip);
  else
    log_err2("Client", ip);
}

static void process_connections(void){
  /*
   * The function poll_loop_nowait() calls poll() with a zero
   * timeout value, so it does not block. With tye options it opens
   * any new client sockets to be non-blocking for read/write.
   * This function is called from the server's main loop() [in loop.c].
   */
  int status = 0;
  struct client_options_st clientopts;

  clientopts.nonblock = 1;
  clientopts.cloexec = 1;
  clientopts.write_timeout_ms = g.writetimeout_s * 1000; /* not used */
  clientopts.reconnect_wait_sleep_secs = 0;	/* not used */
  clientopts.reconnect_wait_sleep_retry = 0;	/* not used */
  /*
  clientopts.reconnect_wait_sleep_secs = g.client_reconnect_wait_sleep_secs;
  clientopts.reconnect_wait_sleep_retry = g.client_reconnect_wait_sleep_retry;
  */

  process_finished_connections();
  process_unidentified_connections();
  spawn_client_threads();

  status = poll_loop_nowait(g.ct, &clientopts);

  if(status == -1)
    log_err("Cannot listen for connections (poll()).");
  else if(status == -2){
    log_err("Cannot accept a new connection.");
  }
}

static void process_finished_connections(void){
  /*
   * Loop through the table and remove any entry for which the
   * thread has finished (exited) for some reason.
   */
  int i;
  int numentries;

  numentries = conn_table_get_numentries(g.ct);

  for(i = 0; i < numentries; ++i){
    if(conn_table_element_isnetclient_stoped(g.ct, i) == 0)
      continue;

    log_info("Removing finished client %d from table.", i);
    poll_kill_client_connection(g.ct, i);
    numentries = conn_table_get_numentries(g.ct);
  }
}

static void process_unidentified_connections(void){

  int i;
  int numentries;
  int protocol;
  char *nameorip;
  time_t ctime;		/* when the client connected */
  time_t now;

  now = time(NULL);
  numentries = conn_table_get_numentries(g.ct);

  for(i = 0; i < numentries; ++i){
    if(conn_table_element_isnetclient(g.ct, i) == 0)
      continue;
      
    protocol = get_client_protocol(g.ct, i);
    nameorip = conn_table_get_element_nameorip(g.ct, i);
    ctime = conn_table_get_element_ctime(g.ct, i);

    if((protocol == PROTOCOL_UNKNOWN) &&
       (now > ctime + g.client_id_wait_secs)){

      log_info("Client %d [%s] not identified within time limit.",
	       i, nameorip);
      log_info("Assuming %d [%s] is a V1 client.", i, nameorip);
      if(ident_client_protocol(g.ct, i, PROTOCOL_EMWIN1_STR,
			       strlen(PROTOCOL_EMWIN1_STR)) != 0){
	log_err2("Could not set protocol for", nameorip);
	poll_kill_client_connection(g.ct, i);
	numentries = conn_table_get_numentries(g.ct);
      }
    }
  }
}

int server_send_client_queues(struct emwin_packet *ep){
  /*
   * This function is called from process_packets() in loop.c.
   */
  int netclients;
  int numentries;
  char *nameorip;
  int f_thread_created;
  int protocol;
  int i;
  int status = 0;
  int dberror = 0;

  /*
   * Get total number of network clients and the total number of entries. 
   */
  netclients = conn_table_get_netclients(g.ct);   
  numentries = conn_table_get_numentries(g.ct);

  if(netclients == 0){
    /*
     * The table contains just the the listening socket.
     */
    return(0);
  }

  /*
   * There are some clients. We loop through the whole table.
   */

  for(i = 0; i < numentries; ++i){
    if(conn_table_element_isnetclient(g.ct, i) == 0)
      continue;

    f_thread_created = conn_table_get_element_fthread_created(g.ct, i);
    protocol = get_client_protocol(g.ct, i);
    nameorip = conn_table_get_element_nameorip(g.ct, i);

    if(conn_table_element_isnetclient_running(g.ct, i) == 0){
      if(f_thread_created == 0){
	log_verbose(1, "Client thread %d [%s] not ready.", i, nameorip);
      } else if (conn_table_element_isnetclient_stoped(g.ct, i)){
	log_warnx("Client thread %d [%s] stoped.", i, nameorip);
      }
      continue;
    }

    if(protocol == PROTOCOL_EMWIN1)
      status = connqueue_snd(g.ct->ce[i].cq,
			     ep->queue_data_v1, ep->queue_data_v1_size,
			     &dberror);
    else if(protocol == PROTOCOL_EMWIN2)
      status = connqueue_snd(g.ct->ce[i].cq,
			     ep->queue_data_v2, ep->queue_data_v2_size,
			     &dberror);
    else
      log_errx("Active client [%s] with unknown protocol.", nameorip);

    if(status == -1)
      log_err2_db("Error writing to client queue", nameorip, dberror);
    else if(status == 1)
      log_info("Soft limit reached for client thread %d [%s].", i, nameorip);
    else if(status == 2)
      log_errx("Hard limit reached for client thread %d [%s].", i, nameorip);
  }
  
  return(0);
}

static void report_client_connections(void){
  /*
   * Print the ip, and queue size for each connection. This function
   * is called from periodic().
   */
  FILE *fp = NULL;
  char *fname;

  fname = g.serverstatefile;
  if(valid_str(fname)){
    fp = fopen(fname, "a");
    if(fp == NULL){
      log_err_open(fname);
    } else {
      print_client_connections(fp);
      fclose(fp);
    }
  }

  fname = g.serveractivefile;
  if(valid_str(fname)){
    fp = fopen(fname, "w");
    if(fp == NULL){
      log_err_open(fname);
    } else {
      print_client_connections(fp);
      fclose(fp);
    }
  }
}

static void print_client_connections(FILE *fp){

  int netclients;	
  int numentries;
  char *nameorip;
  int protocol;
  uint32_t queue_size;
  time_t ctime;
  int i;
  time_t now;

  now = time(NULL);
  netclients = conn_table_get_netclients(g.ct);   
  numentries = conn_table_get_numentries(g.ct);			 

  fprintf(fp, "- %u %d %d\n", (unsigned int)now, numentries, netclients);

  if(netclients == 0)
    return;

  for(i = 0; i < numentries; ++i){
    /*
     * Only the clients that already have threads created have queues.
     */
    if(conn_table_element_isnetclient_running(g.ct, i) == 0)
      continue;

    nameorip = conn_element_get_nameorip(&(g.ct->ce[i]));
    protocol = get_client_protocol(g.ct, i);
    queue_size = connqueue_n(g.ct->ce[i].cq);
    ctime = conn_table_get_element_ctime(g.ct, i);

    fprintf(fp, "  %s %d" " %" PRIu32 " %" PRIuMAX "\n",
	    nameorip,
	    protocol,
	    queue_size,
	    (uintmax_t)ctime);
  }
}

static int get_report_client_connections_flag(void){

  int r;

  r = greport_client_connections_flag;
  greport_client_connections_flag = 0;

  return(r);
}

void set_report_client_connections_flag(void){

  greport_client_connections_flag = 1;
}

#if 0
/*
 * See comment in the top of this file about these two functions.
 */
static int get_process_connections_flag(void){

  int r = 0;

  if(gprocess_connections == 0)
    return(0);

  if(pthread_mutex_trylock(&gprocess_connections_mutex) == 0){
    r = gprocess_connections;
    gprocess_connections = 0;
    pthread_mutex_unlock(&gprocess_connections_mutex);
  }else
    log_info("Cannot lock mutex in get_process_connections_flag().");

  return(r);
}

void set_process_connections_flag(void){

  int status = 0;

  if((status = pthread_mutex_lock(&gprocess_connections_mutex)) == 0){
    gprocess_connections = 1;
    pthread_mutex_unlock(&gprocess_connections_mutex);
  }else
    log_errx("Error %d locking mutex in set_process_connections_flag().",
	     status);
}
#endif

static int get_process_connections_flag(void){

  time_t now;

  now = time(NULL);
  if(now < gprocess_connections_time)
    return(0);

  gprocess_connections_time = now + SERVER_PROCESS_CONNECTIONS_PERIOD_SECS;

  return(1);
}

/*
 * These are the functions to support the creation of the client
 * threads.
 */
static void spawn_client_threads(void){
  /*
   * Loop through the conn table and spawn the client thread for
   * each element whose protocol has been identified
   * and for which the thread has not been spawned yet. The
   * remaining initialization steps of each client are done here. The protocol
   * identification routine ends up calling conn_element_init2.
   * Here we setup the filter data, which calls init3, and then finally init4.
   */
  int numentries;
  int isnetclient;
  int f_thread_created;
  int protocol;
  struct connqueue_param_st cqparam;
  int status;
  int dberror = 0;
  char *nameorip;
  int i;

  assert(g.dbcache_mb >= 0);
  cqparam.cache_mb = (uint32_t)g.dbcache_mb;
  cqparam.reclen = QUEUE_PACKET_SIZE; /* EMWIN_PACKET_SIZE + envelope */
  /*
   * XXX cqparam.softlimit = g.client_queue_maxsize_soft;
   * XXX cqparam.hardlimit = g.client_queue_maxsize_hard;
   */
  cqparam.softlimit = 0;
  cqparam.hardlimit = 0;

  numentries = conn_table_get_numentries(g.ct);
  for(i = 0; i < numentries; ++i){
    f_thread_created = conn_table_get_element_fthread_created(g.ct, i);
    isnetclient = conn_element_isclient(&g.ct->ce[i]);
    protocol = get_client_protocol(g.ct, i);
    nameorip = conn_element_get_nameorip(&g.ct->ce[i]);

    if(f_thread_created || (isnetclient == 0) ||
       (protocol == PROTOCOL_UNKNOWN))
      continue;

    status = conn_element_init4(&g.ct->ce[i],
				client_thread_create,
				client_thread_kill,
				&cqparam,
				&dberror);
    if(status != 0){
      /*
       * The element must be removed from the table.
       */
      log_err2_db("Cannot spawn client thread", nameorip, dberror);
      (void)poll_kill_client_connection(g.ct, i);
      numentries = conn_table_get_numentries(g.ct);      
    } else {
      log_info("Spawned client thread for %s.", nameorip);
    }
  }
}

static int client_thread_create(struct conn_element_st *ce, pthread_t *t_id){
  /*
   * This function is what is passed to conn_element_init4().
   * At that time the ce table is locked, but subsequently
   * the table changes (when other elements are added or deleted).
   * This function must make a copy (used only as read-only)
   * of the ce and the client thread must work with that throught its lifetime.
   * The client thread's main routine client_thread_main() must free()
   * that copy when it finishes. The function client_thread_main()
   * is declared in server_priv.h and defined in serverc.c.
   */
  int status = 0;
  pthread_attr_t attr;
  void *arg;
  struct conn_element_st *ce_copy;
  char *nameorip;

  nameorip = conn_element_get_nameorip(ce);

  ce_copy = malloc(sizeof(struct conn_element_st));
  if(ce_copy == NULL){
    log_err2("Could not create client thread for", nameorip);
    return(-1);
  }

  memcpy(ce_copy, ce, sizeof(struct conn_element_st));
  arg = (void*)ce_copy;
  
  status = pthread_attr_init(&attr);
  if(status == 0)
    status = pthread_create(t_id, &attr, client_thread_main, arg);

  if(status != 0){
    log_errx("Error %d creating client thread for %s.", status, nameorip);
  } else {
    log_info("Created client thread for %s.", nameorip);
  }

  return(status);
}

static int client_thread_kill(struct conn_element_st *ce){
  /*
   * This function is what is passed to conn_element_init4().
   */
  int status = 0;
  void *pthread_status;
  char *nameorip;

  nameorip = conn_element_get_nameorip(ce);

  /*
   * If the thread loop function calls get_quit_flag() then it is not
   * necessary to call pthread_cancel for the final termination. But in cases
   * in which the server wants to terminate the thread early due to
   * errors this is the only way to notify the client thread.
   */
  status = pthread_cancel((ce->threadinfo)->thread_id);
  if((status != 0) && (status != ESRCH))
    log_errx("Error %d canceling client thread %s.", status, nameorip);

  status = pthread_join((ce->threadinfo)->thread_id, &pthread_status);
  if(status != 0)
    log_errx("Error %d joining client thread %s.", status, nameorip);
  else if(pthread_status == PTHREAD_CANCELED)
    log_info("Canceled client thread %s.", nameorip);
  else if(pthread_status == NULL)
    log_info("Finished client thread %s.", nameorip);

  return(0);
}
