/*
 * Copyright (c) 2006 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#include <assert.h>
#include <stdio.h>
#include <assert.h>
#include <sys/time.h>
#include <err.h>
#include <inttypes.h>
#include "seqnum.h"

#define SEQNUM_SIZE	16

int get_seqnum_key(char *key){
  /*
   * This function assumes that "key" has been declared to be
   * at least [SEQNUM_SIZE + 1] characters long.
   * 
   * char key[SEQNUM_SIZE + 1];
   */
  int status = 0;
  struct timeval tv;
  uint32_t s;
  uint32_t u;
  int n;  

  status = gettimeofday(&tv, NULL);
  if(status != 0)
    return(-1);

  s = (uint32_t)tv.tv_sec;
  u = (uint32_t)tv.tv_usec;

  n = snprintf(key, SEQNUM_SIZE + 1, "%" PRIu32 "%06" PRIu32, s, u);
  assert(n <= SEQNUM_SIZE);
  if(n > SEQNUM_SIZE)
    return(1);	

  return(0);
}

/*
int main(void){

  int status;
  char key[SEQNUM_SIZE + 1];


  status = get_seqnum_key(key);
  if(status == 0)
    fprintf(stdout, "%s\n", key);
  else if(status == -1)
    err(1, "get");
  else
    errx(1, "get");
    
    
  return(0);
}
*/
