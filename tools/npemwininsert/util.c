/*
 * Copyright (c) 2005-2008 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#include <stddef.h>
#include <string.h>
#include <ctype.h>
#include "util.h"

/*
uint32_t calc_checksum(void *data, size_t size){

  size_t i;
  unsigned char *p;
  uint32_t cs;

  p = (unsigned char*)data;
  cs = 0;
  for(i = 0; i < size; ++i)
    cs += p[i];

  return(cs);
}
*/

int calc_checksum(void *data, size_t size){

  size_t i;
  u_char *p;
  int c = 0;

  p = (u_char*)data;
  for(i = 0; i < size; ++i)
    c += p[i];

  return(c);
}

int valid_str(char *s){

  if((s != NULL) && (s[0] != '\0'))
    return(1);

  return(0);
}
