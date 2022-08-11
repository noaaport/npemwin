/*
 * Copyright (c) 2005-2007 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#include <sys/file.h>
#include <unistd.h>
#include <fcntl.h>
#include "err.h"
#include "io.h"

static char *s_input_filename = NULL;
static char *s_output_filename = NULL;

/* local to this file */
static ssize_t writen(int fd, void *buf, size_t size);

int open_input(char *fpath){

  int fd;

  fd = open(fpath, O_RDONLY);
  if(fd == -1){
    log_errn_open(fpath);
    return(-1);
  }

  s_input_filename = fpath;

  return(fd);
}

int open_output(char *fpath){
  /*
   * In FreeBSD we cannot lock a fifo, so we don't use locking in the fifo
   * This assumes that only on instance of npemwininsert is running, or
   * some other mechanism must be used to control the various
   * instances of this program.
   */

  int fd;

  /*
   * We try to open nonblock, so that if it is _not_ opened for reading,
   * we get an error instead of blocking.
   */
  fd = open(fpath, O_WRONLY | O_NONBLOCK);

  /*
  if(fd != -1){
    if(flock(fd, LOCK_EX) == -1){
      (void)close(fd);
      fd = -1;
    }
  }
  */

  if(fd == -1){
    log_errn_open(fpath);
    return(-1);
  }

  s_output_filename = fpath;

  return(fd);
}

int read_page(int fd, void *page, size_t page_size){

  ssize_t n;

  n = read(fd, page, page_size);
  if(n == -1){
      log_errn_read(s_input_filename);
  }

  return(n);
}

int write_page(int fd, void *page, size_t page_size){

  ssize_t n;

  n = writen(fd, page, page_size);

  if ((n >= 0) && ((size_t)n < page_size)) {
    log_errn_write(s_output_filename);
    n = -1;
  }
    
  return(n);
}

static ssize_t writen(int fd, void *buf, size_t size) {
  /*
   * To be used when output is a fifo
   */
  ssize_t n;		/* number of bytes in one write */
  size_t i = 0;		/* accumulated number of bytes writen */
  char *p = (char*)buf;

  while(i < size){
    if((n = write(fd, &p[i], size - i)) == -1)
      return(-1);
    else if(n == 0)
      break;
    else
      i += n;
  }

  return(i);
}
