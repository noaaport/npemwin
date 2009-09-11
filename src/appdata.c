/*
 * Copyright (c) 2005-2007 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include "libconnth/libconn.h"
#include "appdata.h"

struct appdata_st {
  int protocol;
};

static int set_client_protocol(struct conn_table_st *ct, int i, int protocol);
static void free_appdata_st(void *p);

int ident_client_protocol(struct conn_table_st *ct, int i,
			  char *msg, size_t msg_size){
  /*
   * This function tries to identify the client's protocol from
   * the msg sent, using the default if it could not identify a particular one.
   * It also stores the protocol's id in the data portion of the client's
   * conn structure (libconn). If the protocol was already defined it
   * returns an error code (see below).
   *
   * Returns
   *	0 => protocol identified with no errors
   *   -1 => (memory) error setting storing the data 
   *    1 => the default protocol was used
   *    2 => the protocol was already defined
   *
   * The msg is a string (not '\0' terminated) of the form
   *
   *	ByteBlast Client|NM-info@wxmesg.com|V<n>
   *
   * Here we look only for the V1 or V2
   */
  char *s;	/* point to V<n> within msg */
  size_t p_len;
  int p = PROTOCOL_DEFAULT;
  int r = 1;  /* the default was used */
  int status = 0;

  if(get_client_protocol(ct, i) != PROTOCOL_UNKNOWN){
    /*
     * The protocol was already identified.
     */
    return(2);
  }

  /*
   * At the moment we have two to check. Perhaps
   * the best thing is organize them in an array.
   */

  p_len = strlen(PROTOCOL_EMWIN1_STR);
  if(msg_size >= p_len){
    s = &msg[msg_size - p_len];
    if(strncmp(s, PROTOCOL_EMWIN1_STR, p_len) == 0){
      p = PROTOCOL_EMWIN1;
      r = 0;
    }
  }

  if(r == 1){
    p_len = strlen(PROTOCOL_EMWIN2_STR);
    if(msg_size >= p_len){
      s = &msg[msg_size - p_len];
      if(strncmp(s, PROTOCOL_EMWIN2_STR, p_len) == 0){
	p = PROTOCOL_EMWIN2;
	r = 0;
      }
    }
  }

  status = set_client_protocol(ct, i, p);
  if(status != 0)
    r = -1;

  return(r);
}

int get_client_protocol(struct conn_table_st *ct, int i){

  int protocol = PROTOCOL_UNKNOWN;
  struct appdata_st *p;

  p = (struct appdata_st*)conn_table_get_element_appdata(ct, i);
  if(p == NULL){
    /*
     * This client never identified itself.
     */
    protocol = PROTOCOL_UNKNOWN;
  }else
    protocol = p->protocol;

  return(protocol);
}

static int set_client_protocol(struct conn_table_st *ct, int i, int protocol){

  struct appdata_st *p;
  size_t n;
  int status = 0;

  n = sizeof(struct appdata_st);
  p = malloc(n);
  if(p == NULL)
    return(-1);

  p->protocol = protocol;
  conn_table_set_element_appdata(ct, i, p, free_appdata_st);

  return(status);
}

int get_client_protocol_byce(struct conn_element_st *ce){

  int protocol = PROTOCOL_UNKNOWN;
  struct appdata_st *p;

  p = (struct appdata_st*)conn_element_get_appdata(ce);
  if(p != NULL)
    protocol = p->protocol;

  return(protocol);
}

static void free_appdata_st(void *p){

  if(p != NULL)
    free(p);
}
