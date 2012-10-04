/*
 * Copyright (c) 2004-2006 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#ifndef EMWIN_H
#define EMWIN_H

#include <sys/types.h>
#include "seqnum.h"

#define EMWIN_PACKET_SIZE	1116
#define EMWIN_HEADER_SIZE	80
#define EMWIN_DATABLOCK_SIZE	1024
#define EMWIN_NULLPAD_SIZE	6
#define EMWIN_FNAME_LEN		8	/* without the extension */
#define EMWIN_TOTAL_FNAME_SIZE	12	/* 8 + period + ext */
#define EMWIN_TOTAL_FNAME_LEN	13	/* 8 + period + ext + NULL */
#define EMWIN_TIMESTAMP_SIZE	22
#define EMWIN_TIMESTAMP_LEN	23	/* 22 + NULL */
#define SEQNUM_LEN		(SEQNUM_SIZE + 1)

#define EMWIN_QUEUE_ENVELOPE_SIZE 4	/* uint32_t */
#define QUEUE_PACKET_SIZE	(EMWIN_PACKET_SIZE + EMWIN_QUEUE_ENVELOPE_SIZE)

/*
 * Note: The sizes of the file name and time stamp, determine
 * the corresponding widths of the format string in 
 * fill_packet_struct() [emwin.c]
 */
struct emwin_packet_header {
  char filename[EMWIN_TOTAL_FNAME_LEN];	
  int  blockno;
  int  numblocks;
  int  checksum;
  char dtstamp[EMWIN_TIMESTAMP_LEN];
  char seqnumkey[SEQNUM_LEN];
};

struct emwin_file_qinfo {
  char filename[EMWIN_TOTAL_FNAME_LEN];
  char seqnumkey[SEQNUM_LEN];
  int  numblocks;
  int  rcvblocks;	/* number of blocks received so far */
};

struct emwin_packet {
  struct emwin_packet_header header;
  char *data;	/* just the data as we will save it */
  int  data_size;
  char rawdata[EMWIN_PACKET_SIZE]; /* unxored header + data, or server list */
  int  rawdata_size;
  char bbdata[EMWIN_PACKET_SIZE];	/* the raw mess */
  int  bbdata_size;			/* actual size contained in bbdata */
#define BB_PACKET_TYPE_UNKNOWN	0
#define BB_PACKET_TYPE_DATA	1
#define BB_PACKET_TYPE_SRVLIST	2
  int  bbtype;				/* data packet or server list */
  char queue_data_v1[QUEUE_PACKET_SIZE]; /* <size><bbdata> */
  char queue_data_v2[QUEUE_PACKET_SIZE]; /* <size><bbdata>|<size><zbbdata> */
  int  queue_data_v1_size;		/* it is actually fixed */
  int  queue_data_v2_size;		/* variable */
  int  compress_ratio;			/* percentage wise (only for V2) */
};

struct emwin_qfiles_st {
  char *data_filename;
  char *tmp_filename;
  char *q_filename;
  int datafn_size;
  int tmpfn_size;
  int qfn_size;
};

int init_emwin_qfiles(void);
void destroy_emwin_qfiles(void);

int open_emwin_server_network(int type, char *ip_or_host_str, char *port,
			      int *gai_code);
int open_emwin_server_serial(char *device, char *settings_str);
int get_emwin_packet_bb(int f, struct emwin_packet *ep);
int get_emwin_packet_wx14(int f, struct emwin_packet *ep);
int get_emwin_packet_serial(int f, struct emwin_packet *ep);
int save_emwin_packet(struct emwin_packet *ep);
int save_server_list(struct emwin_packet *ep);

/* v2.c */
int build_queue_data_v2(struct emwin_packet *ep);

#endif
