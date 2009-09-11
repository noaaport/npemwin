/*
 * Copyright (c) 2008 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#include <string.h>
#include <zlib.h>
#include <stdio.h>
#include <stdlib.h>
#include "util.h"
#include "emwin.h"

/* Start of DL stag within header (0 based count) */
#define V2_DL_TAG_START 69

/* Static buffer so that malloc() is called only once */
static char *gzlib_buffer = NULL;
static uLong gzlib_buffer_size = 0;

static int raw1_to_raw2(char *rawdata2, int *rawdata2_size,
			char *rawdata1, int rawdata1_size);

int build_queue_data_v2(struct emwin_packet *ep){

  char *rawdata2;	/* header + compressed block */
  int rawdata2_size;
  int rawdata2_start;
  char *bbdata2;	/* 6 NULL's + rawdata2 */
  int bbdata2_size;
  int status;
  int i;

  rawdata2_start = EMWIN_QUEUE_ENVELOPE_SIZE + EMWIN_NULLPAD_SIZE;
  rawdata2 = &ep->queue_data_v2[rawdata2_start];
  bbdata2 = &ep->queue_data_v2[EMWIN_QUEUE_ENVELOPE_SIZE];
  rawdata2_size = QUEUE_PACKET_SIZE - rawdata2_start;

  memset(ep->queue_data_v2, '\0', rawdata2_start);

  /*
   * This returns an updated rawdata2_size.
   */
  status = raw1_to_raw2(rawdata2, &rawdata2_size,
			ep->rawdata, ep->rawdata_size);
  if(status != 0)
    return(status);

  bbdata2_size = rawdata2_size + EMWIN_NULLPAD_SIZE;
  pack_uint32(ep->queue_data_v2, (uint32_t)bbdata2_size, 0);
  for(i = 0; i < bbdata2_size; ++i){
    bbdata2[i] ^= 0xff;
  }
  ep->queue_data_v2_size = bbdata2_size + EMWIN_QUEUE_ENVELOPE_SIZE;

  ep->compress_ratio = (100 * rawdata2_size)/ep->rawdata_size;

  return(0);
}

static int raw1_to_raw2(char *rawdata2, int *rawdata2_size,
		 char *rawdata1, int rawdata1_size){
  /*
   * rawdata1 is the ``raw data'' (i.e. unxored header + data block) of a
   * bbdata packet (of size EMWIN_PACKET_SIZE - 12). This function
   * constructs a V2 packet and stores it in rawdata2.
   *
   * The function returns
   *
   * 0 => no error
   * -1 => system error
   * 1 => Error from zlib.
   * 1 => The compressed size would be larger than rawdata2_size. 
   * 2 => Error formating error
   */
  char *block1, *block2;
  uLong block1_size, block2_size;
  char *p;
  int p_size;
  uLong compress_bound;
  uLong compress_size;
  int n;
  int status;

  compress_bound = compressBound((uLong)rawdata1_size);
  if(compress_bound > gzlib_buffer_size){
    if(gzlib_buffer != NULL){
      free(gzlib_buffer);
      gzlib_buffer_size = 0;
    }

    gzlib_buffer = malloc(compress_bound);
    if(gzlib_buffer == NULL)
      return(-1);

    gzlib_buffer_size = compress_bound;
  }

  block1 = &rawdata1[EMWIN_HEADER_SIZE];
  block1_size = (uLong)(rawdata1_size - EMWIN_HEADER_SIZE);

  block2 = &rawdata2[EMWIN_HEADER_SIZE];
  block2_size = (uLong)(*rawdata2_size - EMWIN_HEADER_SIZE);

  compress_size = gzlib_buffer_size;
  status = compress((Bytef*)gzlib_buffer, &compress_size,
		    (const Bytef*)block1, block1_size);

  if(status != 0)
    return(1);

  if(compress_size > block2_size)
    return(1);

  memcpy(block2, gzlib_buffer, compress_size);
  block2_size = compress_size;

  /*
   * Build the packet. Insert the /DL<size> in the header.
   *
   * Position 70, literal /DL followed by the number of bytes in the
   * data block. This ranges from 0 to 1024 bytes.
   * If this field is not present, the packet is version 1.0.
   */
  *rawdata2_size = block2_size + EMWIN_HEADER_SIZE;
  memcpy(rawdata2, rawdata1, EMWIN_HEADER_SIZE);
  p = &rawdata2[V2_DL_TAG_START];
  p_size = EMWIN_HEADER_SIZE - V2_DL_TAG_START;
  n = snprintf(p, p_size, "/DL%d", (int)block2_size);
  if(n >= p_size)
    return(2);

  /* Replace the terminating '\0' inserted by snprintf, by a blank */
  p[n] = ' ';

  return(0);
}
