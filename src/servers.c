/*
 * Copyright (c) 2004-2022 Jose F. Nieves <nieves@ltp.uprrp.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include "globals.h"
#include "defaults.h"
#include "tsv.h"
#include "util.h"
#include "emwin.h"
#include "ser.h"
#include "infeed.h"
#include "err.h"  /* perhaps the functions that log should be in another file */
#include "servers.h"

#define EMWIN_SERVERS_TABLE_GROW	5

struct emwin_server_list {
  struct emwin_server *server;	/* dynamically allocated */
  int allocated;		/* current allocated size */
  int max;			/* highest occupied index */
  int current;			/* index of current server */
  int refused_connections;	/* number of consecutive refused connections */
};

static struct emwin_server_list es_list = {NULL, 0, -1, 0, 0};

static void init_server(struct emwin_server *es);
static void init_server_stats(struct emwin_server *es);
static void close_server(struct emwin_server *es);
static int grow_server_table(void);
static int read_next_server(struct emwin_server *server);
static int switch_server(struct emwin_server *server);

/*
 * functions
 */
static int grow_server_table(void){

  size_t newsize;
  struct emwin_server *e;
  int allocated = es_list.allocated;
  int i;
  int status = 0;

  allocated += EMWIN_SERVERS_TABLE_GROW;
  newsize = allocated * sizeof(struct emwin_server);

  e = realloc(es_list.server, newsize); 
  if(e != NULL){
    /*
     * Initialize the new elements.
     */
    for(i = es_list.allocated; i < allocated; ++i){
      init_server(&e[i]);
    }
    es_list.server = e;
    es_list.allocated = allocated;
  }else{
    /*
     * es_list.server will be cleaned on exit
     */
    status = -1;
  }

  return(status);
}

struct emwin_server *get_current_server(void){

  struct emwin_server *es = &es_list.server[es_list.current];

  return(es);
}

struct emwin_server *get_next_server(void){

  time_t now = time(NULL);
  struct emwin_server *es = &es_list.server[es_list.current];

  if(es->stats.connect == 0){
    /*
     * the first time the function is called for this server.
     */
    es->stats.connect = now;
  }else{
    /*
     * always close the connection
     */
    close_server(es);

    if(switch_server(es) != 0){

      es->stats.disconnect = now;
      
      ++es_list.current;
      if(es_list.current > es_list.max)
	es_list.current = 0;
      
      es = &es_list.server[es_list.current];
      init_server_stats(es);
      es->stats.connect = now;
    }else{
      /*
       * Don't switch server but restart this counter
       */
      es->stats.consecutive_packets = 0;
    }
  }    
    
  if(server_type_serial_device(es))
    es->fd = open_emwin_server_serial(es->ip, es->port);
  else if(server_type_infeed(es))
    es->fd = open_emwin_server_infeed(es->ip, es->port);
  else if(server_type_wx14_msg_device(es) || server_type_wx14_raw_device(es))
      es->fd = open_emwin_server_network(es->type,
					 es->ip, es->port, &es->gai_code);
  else
    es->fd = open_emwin_server_network(EMWIN_SERVER_TYPE_BB,
					 es->ip, es->port, &es->gai_code);

  if(es->fd < 0){
    ++es->stats.bad_packet_count;
    es->stats.error = -1;
    ++es_list.refused_connections;
  } else {
    es_list.refused_connections = 0;	/* reset counter */
    ++es->stats.connections;
    es->f_up = 1;
  }

  if(es_list.refused_connections > es_list.max + 1){
    /*
     * This means that we have cycled through the entire list (and 
     * retried the starting point) and all the servers are down.
     * Reinitialize the list, and return NULL.
     */
    es_list.refused_connections = 0;	/* reset counter */
    es = NULL;
  }

  return(es);
}

static void close_server(struct emwin_server *es){

  int status = 0;

  if(es->fd != -1) {
    if(server_type_serial_device(es))
      status = ser_close_port(es->fd);
    else if(server_type_infeed(es))
      status = infeed_close_fifo(es->fd, es->ip);
    else
      status = close(es->fd);

    es->fd = -1;
  }

  es->f_up = 0;	/* mark it as down */

  if(status != 0)
    log_err2("Error closing %s: ", es->ip);
}

static int switch_server(struct emwin_server *es){
  /*
   * The server is switched if its number of consecutive bad marks exceeeds the
   * limit (a server receives a bad mark when the number of consecutive
   * packets is less than the minimum), or if the connection was refused.
   */
  int r = 0;

  if(es_list.refused_connections > 0)
    return(1);

  if((g.max_bad_packet_count > 0) &&
     (es->stats.bad_packet_count == g.max_bad_packet_count)){
    /*
     * Switch servers. Switching servers can be disabled by setting
     * g.max_bad_packet_count to zero in the config file.
     */  
    r = 1;
  }

  return(r);
}

int get_server_list(char *fname){

  int i;
  FILE *f;
  char *s;
  int status = 0;

  es_list.allocated = 0;
  es_list.current = 0;
  es_list.max = -1;
  es_list.refused_connections = 0;
  
  f = fopen(fname, "r");
  if(f == NULL)
    return (-1);

  i = 0;
  while(status == 0){
    if(i == es_list.allocated){
      /*
       * grow table
       */
      status = grow_server_table();
      if(status != 0)
	break;
    }
      
    s = tsvgetline(f);
    if(s == NULL){
      /*
       * Check to see if it was an error or just eof
       */
      if(tsverror() != 0){
	/* 
	 * tsverror() returns 1 if it is memory error or 2 if it is a
	 * file error; but we don't care about which one it was.
	 */
	status = -1;
      }

      break;

    } else if((tsvargc() != 0) && (tsvargv(0)[0] != '#')){
      /*
       * The if() above, first checks that the line is non-blank,
       * and then that it does not start with a '#'. Do not invert
       * the order.
       */

      status = read_next_server(&es_list.server[i]);
      /*
       * This function returns -1 if there is a memory error,
       * 1, if the format of the file is wrong, or 0.
       */
      if(status == 0){
	++es_list.max;
	++i;
      }
    }
  }

  fclose(f);
  tsvrelease();

  if((status == 0) && (es_list.max == -1))
    status = 2;			/* empty file */

  /*
  if(status == -1)
    err(1, "Error reading file.");
  else if(status == 1)
    errx(1, "Error reading file. Wrong format.");
  else if(status == 2)
    errx(1, "Empty file.");

  for(i = 0; i <= es_list.max; ++i){
    fprintf(stdout, "%s @ %s\n", 
	    es_list.server[i].ip, es_list.server[i].port);
  }
  exit(0);
  */

  return(status);
}

static int read_next_server(struct emwin_server *server){

  size_t size;
  char *argv0;

  if(tsvargc() != 2)
    return(1);

  argv0 = tsvargv(0);

  if(argv0[0] == '/') {
    if(argv0[1] == '/') {
	server->type = EMWIN_SERVER_TYPE_INFEED;
	++argv0;
    } else {
      server->type = EMWIN_SERVER_TYPE_SERIAL;
    }
  } else if(argv0[0] == '@'){
      if(argv0[1] == '@'){
	server->type = EMWIN_SERVER_TYPE_WX14_RAW;
	++argv0; ++argv0;
      } else {
	server->type = EMWIN_SERVER_TYPE_WX14_MSG;
	++argv0;
      }
  } else {
    server->type = EMWIN_SERVER_TYPE_BB;
  }

  size = strlen(argv0) + 1;
  server->ip = malloc(size);
  if(server->ip == NULL)
    return(-1);

  strncpy(server->ip, argv0, size);

  size = strlen(tsvargv(1)) + 1;
  server->port = malloc(size);
  if(server->port == NULL){
    free(server->ip);
    return(-1);
  }
  
  strncpy(server->port, tsvargv(1), size);

  return(0);
}

static void init_server(struct emwin_server *es){

  es->ip = NULL;
  es->port = NULL;
  es->fd = -1;
  es->f_up = 0;
  es->gai_code = 0;
  init_server_stats(es);
}

static void init_server_stats(struct emwin_server *es){

  es->stats.connect = 0;
  es->stats.disconnect = 0;
  es->stats.consecutive_packets = 0;
  es->stats.total_packets = 0;
  es->stats.max_packets = 0;
  es->stats.bad_packet_count = 0;
  es->stats.connections = 0;
  es->stats.error = 0;
  es->stats.serverread_errors = 0;
  es->stats.serverclosed_errors = 0;
  es->stats.header_errors = 0;
  es->stats.checksum_errors = 0;
  es->stats.filename_errors = 0;
  es->stats.unknown_errors = 0;
}

void release_server_list(void){

  int i;

  if(es_list.server == NULL)
    return;

  for(i = 0; i < es_list.allocated; ++i){
    if(es_list.server[i].fd != -1){
      close_server(&es_list.server[i]);
    }
    
    if(es_list.server[i].ip != NULL){      
      free(es_list.server[i].ip);
    }

    if(es_list.server[i].port != NULL){      
      free(es_list.server[i].port);
    }

    init_server(&es_list.server[i]);
  }

  free(es_list.server);

  es_list.server = NULL;
  es_list.allocated = 0;
}

void update_emwin_server_stats(int status){

  struct emwin_server *es = &es_list.server[es_list.current];

  if(status == 0){

    ++es->stats.consecutive_packets;
    ++es->stats.total_packets;

    if(es->stats.consecutive_packets >= g.min_consecutive_packets)
      es->stats.bad_packet_count = 0;

    if(es->stats.consecutive_packets > es->stats.max_packets)
      es->stats.max_packets = es->stats.consecutive_packets;
  }else{
    es->stats.error = status;
    if(es->stats.consecutive_packets < g.min_consecutive_packets)
      ++es->stats.bad_packet_count;

    if(status < 0)
      ++es->stats.serverread_errors;
    else if(status == 1)
      ++es->stats.serverclosed_errors;
    else if(status == 2)
      ++es->stats.header_errors;
    else if(status == 3)
      ++es->stats.checksum_errors;
    else if(status == 4) 
      ++es->stats.filename_errors;
    else
      ++es->stats.unknown_errors;
  }
}

int write_emwin_server_stats(char *file){

  int i;
  int n;
  struct emwin_server *es;
  FILE *f;
  int status = 0;

  if((f = fopen(file, "w")) == NULL)
    return(-1);

  /*
   * Remember: es_list.max is the highest index (not the number of elements)
   */
  for(i = 0; (i <= es_list.max) && (status == 0); ++i){
    es = &es_list.server[i];
    if(es->stats.connect != 0){
      n = fprintf(f, "%s %s"  " %" PRIdMAX " %" PRIdMAX  
		  " %d %d %d %d %d %d %d %d %d %d %d %d\n",
		  es->ip, es->port, 
		  (intmax_t)es->stats.connect, 
		  (intmax_t)es->stats.disconnect, 
		  es->stats.consecutive_packets, 
		  es->stats.max_packets,
		  es->stats.total_packets, 
		  es->stats.bad_packet_count, 
		  es->stats.connections, 
		  es->stats.error,
		  es->stats.serverread_errors,
		  es->stats.serverclosed_errors,
		  es->stats.header_errors,
		  es->stats.checksum_errors,
		  es->stats.filename_errors,
		  es->stats.unknown_errors);
      if(n <= 0)
	status = -1;
    }
  }

  fclose(f);
   
  return(status);
}

int server_type_bbserver(struct emwin_server *server){

  if(server->type == EMWIN_SERVER_TYPE_BB)
    return(1);

  return(0);
}

int server_type_serial_device(struct emwin_server *server){
  /*
   * An entry that starts with a "/" character in the serverslist file
   * is assumed to be the name of a serial device. The corresponding
   * "port" entry is then the serial settings string (e.g., 9600,n,8,1).
   */

  if(server->type == EMWIN_SERVER_TYPE_SERIAL)
    return(1);

  return(0);
}

int server_type_wx14_msg_device(struct emwin_server *server){
  /*
   * An entry that starts with the "@" character in the serverslist file
   * is assumed to be the name of a wx14 device (message port).
   */

  if(server->type == EMWIN_SERVER_TYPE_WX14_MSG)
    return(1);

  return(0);
}

int server_type_wx14_raw_device(struct emwin_server *server){
  /*
   * An entry that starts with two "@@" characters in the serverslist file
   * is assumed to be the name of a wx14 device (raw data port).
   */

  if(server->type == EMWIN_SERVER_TYPE_WX14_RAW)
    return(1);

  return(0);
}

int server_type_infeed(struct emwin_server *server){
  /*
   * An entry that starts with two "//" characters in the serverslist file
   * is assumed to be the path to a fifo file . The corresponding
   * "port" entry is then the creation mode string (e.g., 0664).
   */

  if(server->type == EMWIN_SERVER_TYPE_INFEED)
    return(1);

  return(0);
}

/**
int main(void){

  char *filename = "servers.conf";
  int status;
  int i;
  struct emwin_server *es;

  status = get_server_list(filename);
  if(status != 0)
    errx(1, "Error from get_server_list: %d", status);

  for(i = 0; i <= 29; ++i){
    es = get_next_server();
    fprintf(stdout, "%s -- %s\n", es->ip, es->port);
  }

  return 0;
}
**/
