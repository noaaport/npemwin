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

#define SEQNUMKEY_FIRST 123456
#define SEQNUMKEY_LAST 923456

static int g_seqnum_key = SEQNUMKEY_FIRST;

int get_seqnum_key(char *key){
  /*
   * This function assumes that "key" has been declared to be
   * at least [SEQNUM_SIZE + 1] characters long.
   * 
   * char key[SEQNUM_SIZE + 1];
   */
  int n;

  if(g_seqnum_key == SEQNUMKEY_LAST)
    g_seqnum_key = SEQNUMKEY_FIRST;
  else
    ++g_seqnum_key;

  n = snprintf(key, SEQNUM_SIZE + 1, "%d", g_seqnum_key);
  assert(n <= SEQNUM_SIZE);
  if(n > SEQNUM_SIZE)
    return(1);	

  return(0);
}

/*
 * This the old version. When we used it with emftp, the problem
 * was that the the files are received "at the same time" and the
 * microsends resolution is not enough. So we now use a simpler
 * scheme.
 */
int get_seqnum_key_old(char *key){
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
