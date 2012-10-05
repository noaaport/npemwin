/*
 * $Id$
 */
#ifndef WX14_PRIVATE_H
#define WX14_PRIVATE_H

#include <stddef.h>
#include <stdint.h>

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
#define WX14_HEADER_SIZE 5
#define WX14_MESSAGE_MAXSIZE (WX14_HEADER_SIZE + UINT16_MAX)

/* command (message) types */
#define WX14_CMD_HELLO_MSG 0x00
#define WX14_MSG_EMWIN_PACKET 0x0b
#define WX14_CMD_GET_VCDU_STATUS 0x0d
#define WX14_CMD_CLEAR_VCDU_STATUS 0x0e
#define WX14_CMD_GET_NET_CONFIG 0x15
#define WX14_CMD_SET_NET_CONFIG 0x16
#define WX14_CMD_GET_OUTPUT_TYPES 0x17
#define WX14_CMD_SET_OUTPUT_TYPES 0x18
#define WX14_MSG_SIGNAL_STATUS 0x1b
#define WX14_CMD_RESET 0x1f
#define WX14_CMD_SET_FRONT_END 0x21

struct wx14_msg_st {
  int cmd_type;
  size_t dataN;	/* size of data that starts at message[5] */
  void *data;	/* data = &message[5] */
  unsigned char message[WX14_MESSAGE_MAXSIZE];
};

int wx14_read_msg(int fd, unsigned int secs, int retry,
		  struct wx14_msg_st *wx14msg);
void *wx14_msg_data(struct wx14_msg_st *wx14msg);
int wx14_msg_data_size(struct wx14_msg_st *wx14msg);
int wx14_msg_cmdtype(struct wx14_msg_st *wx14msg);

#endif
