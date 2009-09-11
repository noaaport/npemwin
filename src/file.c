/*
 * Copyright (c) 2004-2006 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include "file.h"

int file_exists(char *fname){
  /* 
   * Returns:
   * 0 == > ok. file exists
   * 1 ==> file does not exist
   * -1 ==> error from systat different from ENOENT
   */
  struct stat stbuf;
  int status = 0;

  if(stat(fname, &stbuf) == -1){
    if(errno == ENOENT)
      status = 1;
    else
      status = -1;
  }

  return(status);
}
