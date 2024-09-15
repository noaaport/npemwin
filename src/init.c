/*
 * Copyright (c) 2004-2006 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#include "config.h"
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#ifndef HAVE_DAEMON
#include "solaris.h"
#endif
#include "err.h"
#include "globals.h"
#include "defaults.h"
#include "emwin.h"
#include "servers.h"
#include "server.h"
#include "httpd.h"
#include "bbreg.h"
#include "stats.h"
#include "util.h"
#include "strsplit.h"
#include "pid.h"
#include "pw.h"
#include "conf.h"
#include "exec.h"
#include "init.h"

static int checkdir(char *dirname);
static int check_subdir(char *parentdir, char *subdir);

void init_globals(void){

  /*
   * defaults
   */
  g.defconfigfile = CONFIGFILE;
  g.configfile = NULL;
  g.option_C = 0;
  g.option_F = 0;

  g.user = EMWIN_USER;
  g.group = EMWIN_GROUP;
  g.home = EMWIN_HOME;
  g.umask = DEFAULT_UMASK;

  g.bbclientid = BBCLIENTID;
  g.bbclientid_secs = BBCLIENTID_SECS;

  g.spooldir = EMWIN_SPOOL_DIR;
  g.datadir = EMWIN_DATA_DIR;
  g.lockdir = EMWIN_LOCK_DIR;
  g.tmpdir = EMWIN_TMP_DIR;
  g.qfilesdir = EMWIN_QFILES_DIR;
  g.spoolfile_mode = EMWIN_SPOOLFILE_MODE;
  g.pidfile = EMWIN_PIDFILE;
  g.pidfile_mode = EMWIN_PIDFILE_MODE;
  g.statusfile = SERVERSTATUSFILE;
  g.nbspstats_period = NBSP_STATS_PERIOD;

  g.wx14_signal_statusfile = WX14_SIGNAL_STATUSFILE;
  g.wx14_signal_logfile = WX14_SIGNAL_LOGFILE;

  g.serverslistfile = EMWIN_SERVERS_LIST_FILE;
  g.emwinstatusfile = EMWIN_SERVERS_STATUS_FILE;
  g.min_consecutive_packets = MIN_CONSECUTIVE_PACKETS;
  g.max_bad_packet_count = MAX_BAD_PACKET_COUNT;
  g.retrysleep = SLEEP_SECS_BEFORE_RETRY;

  g.serverstatefile = SERVERSTATEFILE;
  g.serveractivefile = SERVERACTIVEFILE;
  g.serverthreadsfile = SERVERTHREADSFILE;
  g.connecttimeout_s = CONNECT_TIMEOUT_SECS;
  g.readtimeout_s = READ_TIMEOUT_SECS;
  g.readtimeout_retry = READ_TIMEOUT_RETRY;
  g.writetimeout_s = WRITE_TIMEOUT_SECS;
  g.writetimeout_retry = WRITE_TIMEOUT_RETRY;

  g.qrunner = QRUNNER_PROGRAM;
  g.qrunperiod = QRUN_PERIOD;
  g.qrunreportperiod = QRUN_REPORT_PERIOD;

  g.prefilter = PREFILTER_PROGRAM;
  g.startscript = START_SCRIPT;
  g.stopscript = STOP_SCRIPT;
  g.scheduler = SCHEDULER;

  g.httpd_enable = NBSP_HTTPD_ENABLE;
  g.httpd = NBSP_HTTPD;

  g.bbserver_enable = BBSERVER_ENABLE;
  g.bbserver = BBSERVER;

  g.mserverlist = BBSERVER_MSERVERLIST;
  g.mserverlist_raw = BBSERVER_MSERVERLIST_RAW;
  g.mserverpublist = BBSERVER_MSERVERPUBLIST;
  g.mserverpublist_raw = BBSERVER_MSERVERPUBLIST_RAW;
  g.mserverdirlist = BBSERVER_MSERVERDIRLIST;
  g.mserverdirlist_raw = BBSERVER_MSERVERDIRLIST_RAW;
  g.mserversatlist = BBSERVER_MSERVERSATLIST;
  g.mserversatlist_raw = BBSERVER_MSERVERSATLIST_RAW;
  g.bbserverlist = BBSERVER_BBSERVERLIST;
  g.bbserverlist_raw = BBSERVER_BBSERVERLIST_RAW;
  g.bbserversatlist = BBSERVER_BBSERVERSATLIST;
  g.bbserversatlist_raw = BBSERVER_BBSERVERSATLIST_RAW;

  g.servername = SERVER_NAME;
  g.serverport = SERVER_PORT;
  g.listen_backlog = LISTEN_BACKLOG;
  g.maxnetclients = MAX_NET_CLIENTS;
  g.serverprotocol = SERVER_PROTOCOL;
  g.client_id_wait_secs = CLIENT_ID_WAIT_SECS;
  g.min_compress_ratio = MIN_COMPRESS_RATIO;

  g.client_queue_dbcache_mb = CLIENT_QUEUE_DBCACHE_MB;
  g.client_queue_maxsize_soft = CLIENT_QUEUE_MAXSIZE_SOFT;
  g.client_queue_maxsize_hard = CLIENT_QUEUE_MAXSIZE_HARD;
  g.client_queue_timeout_msecs = CLIENT_QUEUE_TIMEOUT_MSECS;
  g.serverstate_log_period = SERVERSTATE_LOG_PERIOD;
  g.serverthreads_log_freq = SERVERTHREADS_LOG_FREQ;
  /*
   * internal variables
   */
  g.runq_last_time = 0;
  g.packet_count = 0;
  g.qrun_count = 0;
  g.qrunner_pid = -1;
  g.httpdfp = NULL;
  g.bbserverfp = NULL;
  nbspstats_init();
  g.server_fd = -1;
  g.ct = NULL;
  g.qfiles = NULL;
  /* wx14_init() is called in emwin.c */
  /* pthread_t npemwin_thread_id */
  g.f_npemwin_thread_created = 0;
  g.f_lock = 0;
  g.f_debug = 0;
  g.f_ndaemon = 0;
  g.f_verbose = 0;
  g.f_server_enabled = 0;
}
  
int init_daemon(void){

  int status = 0;
  int nochdir = 0;

  /*
   * If g.home is set, then drop_privs() will have already set
   * the root directory to the normal user's home directory.
   */
  if(valid_str(g.home))
    nochdir = 1;

  if(g.option_F == 0)
    status = daemon(nochdir, 0);

  if(status != 0)
    return(status);

  umask(g.umask);
  openlog(SYSLOG_IDENT, SYSLOG_OPTS, SYSLOG_FACILITY);
  set_log_daemon();
  
  return(status);
}

int init_lock(void){
  /*
   * This has to be done after daemon() so that the lock file contains the
   * daemon's pid, not the starting program's.
   */
  int status = 0;

  if(create_pidfile(g.pidfile, g.pidfile_mode) != 0){
    status = 1;
    log_err2("Could not create", g.pidfile);
  }else
    g.f_lock = 1;

  return(status);
}

void cleanup(void){

  /*
   * There are no shared queues, otherwise they would be cleared here.
   */

  release_confoptions();
  destroy_emwin_qfiles();

  kill_httpd_server();
  kill_bbregistrar();

  if(g.f_lock == 1){
    if(remove_pidfile(g.pidfile) != 0)
      log_err2("Could not remove", g.pidfile);

    g.f_lock = 0;
  }

  log_info("Stoped.");
  (void)exec_stopscript();
}

int drop_privs(void){

  struct strsplit_st *strp = NULL;
  int ngroups = 0;
  char **groups = NULL;
  char *user = NULL;
  char *home = NULL;
  int status = 0;

  if(valid_str(g.group)){
    strp = strsplit_create(g.group, ",", 0);
    if(strp == NULL){
      log_err("Error from strsplit_create() in drop_privs()");
      return(-1);
    }

    groups = strp->argv;
    ngroups = strp->argc;
  }

  if(valid_str(g.user))
    user = g.user;

  if(valid_str(g.home))
    home = g.home;

  status = change_privs(ngroups, groups, user, home);

  if(strp != NULL)
    strsplit_delete(strp);

  if(status == -1)
    log_err2("Cannot change user", g.user);
  else if(status == -2)
    log_err2("Cannot set group(s)", g.group);
  else if(status == -3)
    log_err2("Cannot chdir to", g.home);

  if(status != 0)
    return(-1);

  return(status);
}

int init_server_list(void){

  int status = 0;

  status = get_server_list(g.serverslistfile);
  if(status == -1)
    err(1, "Error reading %s.", g.serverslistfile);
  else if(status == 1)
    errx(1, "Error reading %s.", g.serverslistfile);
  else if(status == 2)
    errx(1, "No servers in %s.", g.serverslistfile);

  return(status);
}

int init_directories(void){

  int status = 0;

  status = checkdir(g.spooldir);
  if(status == 0)
    status = check_subdir(g.spooldir, g.datadir);

  if(status == 0)
    status = check_subdir(g.spooldir, g.lockdir);

  if(status == 0)
    status = check_subdir(g.spooldir, g.tmpdir);

  if(status == 0)
    status = check_subdir(g.spooldir, g.qfilesdir);

  return(status);
}

static int check_subdir(char *dir, char *subdir){

  char *fullname = NULL;
  int size;
  int n;

  size = strlen(dir) + strlen(subdir) + 2;
  if((fullname = malloc(size)) == NULL){
    err(1, "Error checking %s/%s.", dir, subdir);
    return(-1);
  }

  n = snprintf(fullname, size, "%s/%s", dir, subdir);
  assert(n == size - 1);

  n = checkdir(fullname);

  return(n);
}

static int checkdir(char *dirname){

  int status;
  struct stat sb;

  status = stat(dirname, &sb);
  if(status == 0){
    if(S_ISDIR(sb.st_mode) == 0)
      status = 1;
  }

  if(status == -1)
    err(1, "Error checking %s.", dirname);
  else if(status == 1)
    errx(1, "%s is not a directory", dirname);

  return(status);
}
