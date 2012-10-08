/*
 * $Id$
 */
#ifndef WX14_PRIVATE_H
#define WX14_PRIVATE_H

#include <stddef.h>
#include <stdint.h>

#ifndef WX14_H
#include "wx14.h"
#endif

/*
 * WX14 Message Protocol - Rev DR1
 *
 * The "message" has a 5 byte header
 *
 * 0 => 0xbb
 * 1 => 0xd2
 * 2,3 => N + 1  (but see note in wx14_private.c)
 * 4 => command type
 * 5-?? => N-byte data.
 */
#define WX14_HEADER_BYTE0 0xbb
#define WX14_HEADER_BYTE1 0xd2

/* command (message) types */
#define WX14_MSG_HELLO 0x00
#define WX14_MSG_EMWIN 0x0b
#define WX14_MSG_GET_VCDU_STATUS 0x0d
#define WX14_MSG_CLEAR_VCDU_STATUS 0x0e
#define WX14_MSG_GET_NET_CONFIG 0x15
#define WX14_MSG_SET_NET_CONFIG 0x16
#define WX14_MSG_GET_OUTPUT_TYPES 0x17
#define WX14_MSG_SET_OUTPUT_TYPES 0x18
#define WX14_MSG_SIGNAL_STATUS 0x1b
#define WX14_MSG_RESET 0x1f
#define WX14_MSG_SET_FRONT_END 0x21

/* wx14_private.c */

/* Read a wx14 "message" */
int wx14_read_msg(int fd, unsigned int secs, int retry,
		  struct wx14_msg_st *wx14msg);
void *wx14_get_msg_data(struct wx14_msg_st *wx14msg);
int wx14_get_msg_datasize(struct wx14_msg_st *wx14msg);
int wx14_get_msg_msgtype(struct wx14_msg_st *wx14msg);

/* Read wx14 data messages - emwin or signal status */
int wx14_read1_data_msg(int fd, unsigned int secs, int retry,
			struct wx14_msg_st *wx14msg);
int wx14_read1_data_msg_emwin(int fd, unsigned int secs, int retry,
			      struct wx14_msg_st *wx14msg);

/* wx14_ss.c */
void wx14_signalstatus_process(struct wx14_msg_st *wx14msg);

#endif
