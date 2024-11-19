/*
 * Copyright (c) 2004-2008 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */

/*
 * This file has functions that are needed to support the silly things
 * of the BB protocol, and which some day should go away.
 */
#include <stdlib.h>
#include <string.h>
#include "globals.h"
#include "err.h"
#include "nreadn.h"
#include "util.h"
#include "bb.h"

static char *gbbclientid_xor = NULL;
static int gbbclientid_xor_size = 0;

void bb_xor(char *dst, char *src, int size){
  /*
   * dst and src can be the same.
   */
  int i;
  char c;
  
  for(i = 0; i < size; ++i){
    c = src[i] ^ 0xff;
    dst[i] = c;
  }
}

void bb_send_clientid(struct emwin_server *es){

  int fd = es->fd;
  int size;
  int n;

  if(server_type_bbserver(es) == 0)
    return;

  /*
   * The string can be left NULL or set to empty to disable the ping-pong
   * (e.g., if nbsp is the master server).
   */
  if(valid_str(g.bbclientid) == 0)
    return;

  /*
   * This will be done only the first time this function is called; the
   * memory stays allocated until the program exits.
   */
  if(gbbclientid_xor == NULL){
      size = strlen(g.bbclientid);
      gbbclientid_xor = malloc(size);
      if(gbbclientid_xor == NULL){
	n = -1;
	goto end;
      }else{
	gbbclientid_xor_size = size;
	bb_xor(gbbclientid_xor, g.bbclientid, size);
      }
  }

  /*
   * Use the same timeout that is used for reading.
   */
  n = writen(fd, gbbclientid_xor, gbbclientid_xor_size, g.readtimeout_s, 0);

 end:
  if(n < 0)
    log_err("Error sending id to server.");
  else if(n != gbbclientid_xor_size)
    log_errx("Error sending id to server.");
  else
    log_info("Sent id to server.");
}

int bb_is_emwin_data_packet(char bbchar){

  char c;

  c = bbchar ^ 0xff;
  if(c == '\0')
    return(1);

  return(0);
}

int bb_is_srvlist_packet(char bbchar){

  char c;

  c = bbchar ^ 0xff;
  if(c == '/')
    return(1);

  return(0);
}

int bb_readn(int fd, void *buf, size_t size, unsigned int secs, int retry){

  int status = 0;
  ssize_t n;

  n = readn(fd, buf, size, secs, retry);

  if(n == -1)
    status = -1;
  else if(n == -2)
    status = -2;	/* timed out (poll) before reading anything */
  else if(n == 0)
    status = -3;	/* eof (server disconnected) */
  else if((size_t)n != size)
    status = 1;	/* short read (timedout or disconnect) while reading */

  return(status);
}

int bb_read_srvlist(int fd, void *buf, size_t *size,
		    unsigned int secs, int retry){
  /*
   * The server list formatted something like
   *
   * /ServerList/<ip:port>|<ip:port>|...|\ServerList\/SatServers/\SatServers\
   *
   * i.e., an empty "SatServers". We will look for the fourth '\' character
   * as the packet delimiter.
   *
   * This function returns one of the errors from bb_readn, or 2
   * if the header does not contain a server list.
   */
  unsigned char *p;
  size_t i;
  int status = 0;
  int backslashcount = 0;

  p = (unsigned char*)buf;
  i = 0;
  while((backslashcount != 4) && (i < *size)){
    status = bb_readn(fd, p, sizeof(unsigned char), secs, retry);
    if(status != 0)
      return(status);

    if((*p ^ 0xff) == '\\')
      ++backslashcount;

    ++i;
    ++p;
  }

  if(backslashcount != 4)
    return(2);		/* Unknown packet type */

  *size = i;
  return(0);
}
