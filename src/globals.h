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
#include <stdio.h>
#include "stats.h"
#include "wx14.h"	/* wx14msg struct */

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
  char *home;
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
  char *wx14_signal_statusfile;  /* wx14 last signal status */
  char *wx14_signal_logfile;     /* wx14 log (append) signal status */
  char *serverslistfile; /* file that contains our bb master server(s) */
  char *emwinstatusfile;	/* status file of our b master(s) */
  /* mininum number of consecutive packets received to avoid a bad mark */
  int  min_consecutive_packets;
  /* max number of consecutive bad marks before switching to a new server */
  int  max_bad_packet_count;
  int  retrysleep;	 /* sleep secs before retrying the server list */
  char *serverstatefile;	/* server client connections (log) */
  char *serveractivefile;	/* server client connections (active) */
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
  int httpd_enable;
  char *httpd;		/* tclhttpd script */
  int bbserver_enable;  /* advertise npemwind to NWS master host */
  char *bbserver;	/* bbserver script (for the NWS master host) */
  /* The various files used by the bbserver script */
  char *mserverlist;	/* ServerList as received from the master */
  char *mserverlist_raw;
  char *mserverpublist;	/* PublicList received from the master */
  char *mserverpublist_raw;
  char *mserverdirlist;	/* DdirectList received from the master */
  char *mserverdirlist_raw;
  char *mserversatlist;	/* SatList received built by bbserver script */
  char *mserversatlist_raw;
  char *bbserverlist;  /* ServerList received from our bb servers */
  char *bbserverlist_raw;
  char *bbserversatlist;  /* SatList received from our bb servers */
  char *bbserversatlist_raw;
  char *servername;	/* null => gethostname */
  char *serverport;	/* listening port */
  int  listen_backlog;
  int  maxnetclients;
  int serverprotocol;
  int client_id_wait_secs; /* how long to wait for client to send (first) id */
  int min_compress_ratio;  /* percentage wise */
  /* libconnth queue db related variables */
  int client_queue_dbcache_mb;	  /* size for each client db queue */
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
  FILE *httpdfp;
  FILE *bbserverfp;    /* the script that communicates with the host master */
  struct nbsp_stats_st nbspstats;
  int  server_fd;	/* listening socket for client connections */
  struct conn_table_st *ct;     /* libconnth table */
  struct emwin_qfiles_st *qfiles;
  struct wx14_msg_st wx14msg;
  int  f_lock;
  int  f_debug;
  int  f_ndaemon;	/* do not become daemon */
  int  f_verbose;
  int  f_server_enabled;  /* set to 1 if protocol was set to other than NONE */
} g;

#endif
