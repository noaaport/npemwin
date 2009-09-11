/*
 * Copyright (c) 2004 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#include <assert.h>
#include <time.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <sys/types.h>
#include <errno.h>
#include "util.h"

static  char *months[12] = {"Jan","Feb","Mar","Apr","May","Jun","Jul",
		      "Aug","Sep","Oct","Nov","Dec"};

static char *wdays[7] = {"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};

char *get_month_name(int month){

  if(valid_month(month))
    return(months[month]);

  return(NULL);
}

int get_month(char *s){

  int i;

  assert(s != NULL);

  for(i = 0; i <= 11; ++i){
    if(strcasecmp(months[i],s) == 0)
      return(i);
  }

  return(-1);
}

int valid_month(int month){

  if( (month >= 0) && (month <= 11) )
    return(1);

  return(0);
}

char *get_wday_name(int wday){

  if(valid_wday(wday))
    return(wdays[wday]);

  return(NULL);
}

int valid_wday(int wday){

  if( (wday >= 0) && (wday <= 6) )
    return(1);

  return(0);
}

int get_todays_month(void){

  time_t secs;
  struct tm *tp;

  secs = time(NULL);
  tp = localtime(&secs);
  
  return(tp->tm_mon);
}

int get_todays_day(void){

  time_t secs;
  struct tm *tp;

  secs = time(NULL);
  tp = localtime(&secs);
  
  return(tp->tm_mday);
}

int strto_int(char *s, int *val){

  char *end;
  int status = 0;
  int v;
  int save_errno;
 
  save_errno = errno;
  errno = 0;

  v = strtol(s, &end, 10);
  if((end == s) || (*end != '\0') || (errno != 0))
    status = 1;
  
  if(status == 0)
    *val = v;

  errno = save_errno;

  return(status);
}

int strto_uint(char *s, unsigned int *val){

  char *end;
  int status = 0;
  unsigned int v;
  int save_errno;
 
  save_errno = errno;
  errno = 0;

  v = strtoul(s, &end, 10);
  if((end == s) || (*end != '\0') || (errno != 0))
    status = 1;
  
  if(status == 0)
    *val = v;

  errno = save_errno;

  return(status);
}

int strto_double(char *s, double *val){

  char *end;
  int status = 0;
  double v;
  int save_errno;
 
  save_errno = errno;
  errno = 0;

  v = strtod(s, &end);
  if( (end == s) || (*end != '\0') )
    status = 1;
  
  if(status == 0)
    *val = v;

  errno = save_errno;

  return(status);
}

int strto_u16(char *s, uint16_t *val){

  unsigned int u;
  int status = 0;

  status = strto_uint(s, &u);
  if((status == 0) && (u <= USHRT_MAX))
      *val = (uint16_t)u;
  else
    status = 1;

  return(status);
}

int valid_str(char *s){

  if((s != NULL) && (s[0] != '\0'))
    return(1);

  return(0);
}

uint32_t unpack_uint32(void *p, size_t start){
  /*
   * The first byte is the most significant
   * one and the last byte is the least significant.
   */
  uint32_t u;
  unsigned char *uptr;

  uptr = &((unsigned char*)p)[start];

  u = (uptr[0] << 24) + (uptr[1] << 16) + (uptr[2] << 8) + uptr[3];

  return(u);
}

void pack_uint32(void *p, uint32_t u, size_t start){

  unsigned char *uptr;

  uptr = &((unsigned char*)p)[start];

  uptr[3] = u & 0xff;
  uptr[2] = (u >> 8) & 0xff;
  uptr[1] = (u >> 16) & 0xff;
  uptr[0] = (u >> 24) & 0xff;
}

uint16_t unpack_uint16(void *p, size_t start){
  /*
   * The first byte is the most significant
   * one and the last byte is the least significant.
   */
  uint16_t u;
  unsigned char *uptr;

  uptr = &((unsigned char*)p)[start];

  u = (uptr[0] << 8) + uptr[1];

  return(u);
}

void pack_uint16(void *p, uint16_t u, size_t start){

  unsigned char *uptr;

  uptr = &((unsigned char*)p)[start];

  uptr[1] = u & 0xff;
  uptr[0] = (u >> 8) & 0xff;
}
