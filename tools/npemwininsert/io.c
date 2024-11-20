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

int open_output_fifo(char *fpath){
  /*
   * In the npemwin application, the output file is a fifo. We assume
   * here that this has altready been checked in the application.
   */
  int fd;
  int flags = 0;
  int status = 0;

  /*
   * We try to open nonblock, so that if it is _not_ opened for reading,
   * we get an error instead of blocking.
   */
  fd = open(fpath, O_WRONLY | O_NONBLOCK);
  if(fd == -1) {
    log_err(0, "Error from open: %s\n", fpath);
    return(-1);
  }
  
  status = flock(fd, LOCK_EX);
  if(status == -1) {
    log_err(0, "Error from flock: %s\n", fpath);

    if(close(fd) == -1)
      log_err(0, "Error from close: %s\n", fpath);
    
    return(-1);
  }

  /*
   * Now we block it so that the write() is blocked until npemwin
   * has read enough from the pipe to make space for the write(). Otherwise
   * we will get errors like "Resource temporarily unvailable" 
   * and loss of packets when the pipe gets full. If the other
   * end is closed while this function is blocked we should a SIGPIPE
   * and, if we are not catching signals, the program will exit (without
   * calling cleanup(), but there is not much harm since this program
   * does create any files).
   */
   flags = fcntl(fd, F_GETFL);
   flags &= ~O_NONBLOCK;
   status = fcntl(fd, F_SETFL, flags);
   if(status == -1) {
     if(close(fd) == -1)
       log_err(0, "Error from close: %s\n", fpath);

     return(-1);
   }

   s_output_filename = fpath;
   
   return(fd);
}

void close_output_fifo(int fd) {

  int status = 0;
  
  if(fd < 0)
    return;

  status = flock(fd, LOCK_UN);
  if(status == -1)
    log_err(0, "Error unlocking emwin fifo: %s\n", s_output_filename);
  
  status = close(fd);
  if(status == -1)
    log_err(0, "Error closing emwin fifo: %s\n", s_output_filename);
}

ssize_t writef(int fd, void *buf, size_t size) {
  /*
   * To be used when output is a (the npemwin) fifo
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
