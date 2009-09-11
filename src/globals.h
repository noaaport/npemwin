/*
 * Copyright (c) 2004 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#ifndef GLOBALS_H
#define GLOBALS_H

#include <sys/types.h>
#include "stats.h"

struct npemwin_globals {
  char *defconfigfile;	/* the default */
  char *configfile;	/* user-specified (via -c) */
  int   option_C;	/* just print configuration and exit */
  int   option_F;	/* foreground; do not call daemon(); for MacOSX */ 
  /*
   * runtime-configurable options 
   */
  char *user;
  char *group;
  mode_t umask;
  char *bbclientid;	/* the junk for the BB servers */
  int bbclientid_secs;  /* how often */
  char *spooldir;	/* where to put all data, tmp files, etc */
  char *datadir;	/* subdirectory of the spooldir */
  char *lockdir;	/* subdirectory of the spooldir */
  char *tmpdir;		/* subdirectory of the spooldir */
  char *qfilesdir;	/* subdirectory of the spooldir */
  char *pidfile;
  char *statusfile;	/* server's own statistics file */
  time_t nbspstats_period; /* report the server's statistics */
  char *serverlist;	/* path to file that contains the list */
  char *activeserverlist; /* server list received from the servers */
  char *activeserverlist_raw;
  char *emwinstatusfile;	/* emwin servers status file */
  /* mininum number of consecutive packets received to avoid a bad mark */
  int  min_consecutive_packets;
  /* max number of consecutive bad marks before switching to a new server */
  int  max_bad_packet_count;
  int  retrysleep;	 /* sleep secs before retrying the server list */
  char *serverstatefile;	/* server client connections (log) */
  char *serveractivefile;	/* server active client connections */
  char *serverthreadsfile;	/* server threads stats */
  int  connecttimeout_s; /* to server, in secs (the default is ~ 75 s) */
  int  readtimeout_s;	 /* reading a packet from server */
  int  readtimeout_retry;
  int  writetimeout_s;	 /* writing a packet to clients  */
  int  writetimeout_retry;
  char *qrunner;	/* path the queue processing program */
  int  qrunperiod;	 /* when to process the queue */
  int  qrunreportperiod; /* when to inform that the queue has been processed */
  char *prefilter;	/* external program to pre-filter what is received */
  char *startscript;	/* just before becoming a daemon */
  char *stopscript;	/* just before exiting */
  char *scheduler;
  int httpdenable;
  char *tclhttpd;		/* tclhttpd script */
  char *tclhttpdfifo;		/* fifo for nbsp -> httpd communicatiobn */
  char *servername;	/* null => gethostname */
  char *serverport;	/* listening port */
  int  listen_backlog;
  int  maxnetclients;
  int serverprotocol;
  int client_id_wait_secs; /* how long to wait for client to send (first) id */
  int min_compress_ratio;  /* percentage wise */
  /* libconnth queue db related variables */
  int dbcache_mb;		/* memory size for each client db queue */
  int client_queue_maxsize_soft;
  int client_queue_maxsize_hard;
  int client_queue_timeout_msecs;
  int serverstate_log_period;	/* in seconds */
  int serverthreads_log_freq;	/* in terms of number of packets */
  /*
   * internal variables
   */
  int  packet_count;	/* # of packets received  - determines when to runq */
  int  qrun_count;	/* number of times the que has been processed. */
  pid_t qrunner_pid;   /* pid of last qrunner executed */
  struct tclhttpd_st *httpd;	/* http server */
  struct nbsp_stats_st nbspstats;
  int  server_fd;	/* listening socket for client connections */
  struct conn_table_st *ct;     /* libconnth table */
  struct emwin_qfiles_st *qfiles;
  int  f_lock;
  int  f_debug;
  int  f_ndaemon;	/* do not become daemon */
  int  f_verbose;
  int  f_server_enabled;  /* set to 1 if protocol was set to other than NONE */
} g;

#endif
