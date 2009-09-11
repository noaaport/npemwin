/*
 * Copyright (c) 2004 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#ifndef SERVERS_H
#define SERVERS_H

#include <sys/types.h>

struct emwin_server_stats_st {
  time_t connect;
  time_t disconnect;
  int    consecutive_packets;
  int	 total_packets;
  int    max_packets;	/* maximum consecutive packets received */
  int    bad_packet_count;
  int    connections;
  int    error;
  int    serverread_errors;
  int    serverclosed_errors;
  int    header_errors;
  int    checksum_errors;
  int    filename_errors;
  int    unknown_errors;
};
  
struct emwin_server {
  char *ip;		/* ip or hostname */
  char *port;
  int   fd;
  int   f_up;		/* 0 if not available (connection refused), or 1 */
  int   gai_code;	/* error code from getaddrinfo */
  struct emwin_server_stats_st stats;
};

int get_server_list(char *fname);
struct emwin_server *get_current_server(void);
struct emwin_server *get_next_server(void);
void update_emwin_server_stats(int status);
int write_emwin_server_stats(char *file);
void release_server_list(void);
int server_is_device(struct emwin_server *server);

#endif
