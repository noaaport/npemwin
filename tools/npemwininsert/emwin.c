/*
 * Copyright (c) 2022 Jose F. Nieves <nieves@ltp.uprrp.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#include <assert.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <time.h>
#include <err.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <libgen.h>
#include <ctype.h>
#include "file.h"
#include "util.h"
#include "emwin.h"

static off_t get_parts_total(off_t fsize);
static int get_last_part_size(off_t fsize);
static int build_header_date(struct emwin_packet_st *ep);
static int build_emwin_header(struct emwin_packet_st *ep);

int init_emwin_packet_st(struct emwin_packet_st *ep,
			 char *fpath, char *emwinfname){
  /*
   * emwinfname is the name that will be used in the header, while
   * path is the full path of the file. 
   */
  off_t fsize;
  int status = 0;

  ep->packet_size = EMWIN_PACKET_SIZE;
  ep->fd = -1;

  strncpy(ep->fname, emwinfname, EMWIN_HEADER_FNAMESIZE);
  ep->fname[EMWIN_HEADER_FNAMESIZE] = '\0';

  status = get_file_size(fpath, &fsize);
  if(status != 0)
    return(status);

  ep->parts_total = get_parts_total(fsize);
  ep->last_part_size = get_last_part_size(fsize);
  ep->part_number = 0;
  
  status = build_header_date(ep);
  if(status != 0){
    return(status);
  }

  /*
   * debug: log_info(ep->fname);
   */

  if(status == 0){
    ep->fd = open(fpath, O_RDONLY);
    if(ep->fd == -1)
      status = -1;
  }

  return(status);
}

void clean_emwin_packet_st(struct emwin_packet_st *ep){

  if(ep->fd != -1)
    close(ep->fd);

  ep->fd = -1;
}

int build_emwin_packet(struct emwin_packet_st *ep){

  int status = 0;
  char *p;
  int data_size;
  int n;

  ++ep->part_number;
  data_size = EMWIN_DATA_SIZE;
  if(ep->part_number == ep->parts_total)
    data_size = ep->last_part_size;

  memset(ep->packet, '\0', ep->packet_size);
  p = &(ep->packet[0]);
  p += EMWIN_NULL_SIZE + EMWIN_HEADER_SIZE;
  n = read(ep->fd, p, data_size);
  if(n != data_size)
    return(-1);

  /*
   * The checksum is calculated from the data part and p now points to it.
   */
  ep->checksum = calc_checksum(p, data_size);
  status = build_emwin_header(ep);
  if(status != 0)
    return(status);

  /*
   * This will leave the first and last six bytes unencrypted.
   *
   * int header_and_data_size;
   * int i;
   *
   * header_and_data_size = ep->packet_size - 12;
   * p = &(ep->packet[6]);
   * for(i = 0; i < header_and_data_size; ++i){
   *  p[i] = p[i] ^ 0xff;
   * }
   *
   * We reverted to encrypting everything when we revised npemwin
   * to make it "bug for bug" BB compatible (Jan2008),
   *
   * p = &(ep->packet[0]);
   * for(i = 0; i < ep->packet_size; ++i){
   *   p[i] = p[i] ^ 0xff;
   * }
   *
   * But here we do not imitate the old BB servers. We leave the data
   * as it used to be transmitted (and received ny Npemwin) from a
   * serial wx12 device. Moreover, we do not fill the data portion of the
   * last frame with NULLS to complete 1024 bytes if it is smaller than that.
   */

  return(status);
}

int send_emwin_packet(int fd, struct emwin_packet_st *ep){
  /*
   * fd is the client socket.
   * 
   * Returns:
   * -1 => write error
   *  0 => no errors
   *  1 => could not write all
   */
  int status = 0;
  ssize_t n = 0;

  n = write(fd, ep->packet, ep->packet_size);

  if(n == -1)
    status = -1;
  else if(n != ep->packet_size)
    status = 1;

  /*
   * debug: log_info("f:%d", status);
   */

  return(status);
}

static off_t get_parts_total(off_t fsize){
  
  off_t pt;

  pt = fsize/EMWIN_DATA_SIZE;
  if((fsize % EMWIN_DATA_SIZE) != 0)
    ++pt;

  return(pt);
}

static int get_last_part_size(off_t fsize){

  int size;
  
  size = fsize % EMWIN_DATA_SIZE;
  if(size == 0)
    size = EMWIN_DATA_SIZE;

  return(size);
}

static int build_header_date(struct emwin_packet_st *ep){

  char *fmt_date = "%m/%d/%Y %r";
  struct tm tm;
  time_t now;
  int n;

  now = time(NULL);
  localtime_r(&now, &tm);

  n = strftime(ep->header_date, EMWIN_HEADER_DATESIZE + 1, fmt_date, &tm);

  assert(n == EMWIN_HEADER_DATESIZE);
  if(n != EMWIN_HEADER_DATESIZE)
    return(1);

  return(0);
}

static int build_emwin_header(struct emwin_packet_st *ep){ 

  char *pad = "          \r\n";  /* 10 spaces and CR-LF */
  char *fmt_header = "/PF%-12s/PN %-5d/PT %-5d/CS %-6d/FD%s%s";
  /* char *fmt_header = "/PF%-12s/PN %-5d/PT %-5d/CS %-6d/FD%s"; */
  int n;
  int i;
  char header[EMWIN_HEADER_SIZE + 1];
  char fname_caps[EMWIN_HEADER_FNAMESIZE + 1];

  /*
   * To be sure that anything not explicitly written to fill the 80
   * bytes is filled with spaces, and terminate with \r\n
   *
   * memset(header, 32, EMWIN_HEADER_SIZE);
   * header[78] = '\r\;
   * header[79] = '\n';
   */

  for(i = 0; i < EMWIN_HEADER_FNAMESIZE; ++i){
    fname_caps[i] = toupper(ep->fname[i]);
  }
  fname_caps[EMWIN_HEADER_FNAMESIZE] = '\0';

  n = snprintf(header, EMWIN_HEADER_SIZE + 1, fmt_header,
	       fname_caps, ep->part_number, ep->parts_total, 
	       ep->checksum, ep->header_date, pad);

  /*
  n = snprintf(header, EMWIN_HEADER_SIZE + 1, fmt_header,
	       fname_caps, ep->part_number, ep->parts_total, 
	       ep->checksum, ep->header_date);
  */
  
  assert(n == EMWIN_HEADER_SIZE);
  if(n != EMWIN_HEADER_SIZE)
    return(1);

  memcpy(&ep->packet[EMWIN_NULL_SIZE], header, EMWIN_HEADER_SIZE);

  return(0);
}
