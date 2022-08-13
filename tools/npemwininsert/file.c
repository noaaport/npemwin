/*
 * Copyright (c) 2005 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <libgen.h>
#include <sys/stat.h>
#include <errno.h>
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

int get_file_size(char *fname, off_t *fsize){

  struct stat sb;
  int status = 0;
  
  status = stat(fname, &sb);
  if(status == 0)
    *fsize = sb.st_size;

  return(status);
}
