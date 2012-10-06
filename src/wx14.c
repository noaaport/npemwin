/*
 * $Id$
 */
#include <string.h>
#include "wx14.h"
#include "wx14_private.h"

/*
 * lower level functions of the library
 */
int wx14_read_data_msg(int fd, unsigned int secs, int retry,
		       void *buf, size_t *size, int *type){
  /*
   * buf must have been allocated with the size given by *size;
   * Since the wx14 data size (dataN) is a 2-byte int, in principle
   * buf should be of size UINT16_MAX. In practice it can be just the
   * emwin packet size (in the case in which the msg_type is an emwin
   * message).
   *
   * unsigned char buf[EMWIN_BLOCK_SIZE];
   */
  int status = 0;
  struct wx14_msg_st wx14msg;

  status = wx14_read1_data_msg(fd, secs, retry, &wx14msg);
  if(status != 0)
    return(status);

  if(*size < wx14msg.dataN)
    return(WX14_ERROR_EMWIN_BUF);

  *type = wx14msg.msg_type;
  *size = wx14msg.dataN;
  memcpy(buf, wx14msg.data, wx14msg.dataN);

  return(status);
}

int wx14_read_data_msg_emwin(int fd, unsigned int secs, int retry,
			     void *buf, size_t *size){
  /*
   * Keep reading packets, ignoring any status signal packet(s),
   * until an emwin packet is received.
   */
  int status = 0;
  struct wx14_msg_st wx14msg;

  status = wx14_read1_data_msg_emwin(fd, secs, retry, &wx14msg);

  if(*size < wx14msg.dataN)
    return(WX14_ERROR_EMWIN_BUF);

  *size = wx14msg.dataN;
  memcpy(buf, wx14msg.data, wx14msg.dataN);

  return(status);
}

int wx14_is_emwin_msg(struct wx14_msg_st *wx14msg){

  if(wx14msg->msg_type == WX14_MSG_EMWIN)
    return(1);

  return(0);
}

int wx14_is_signalstatus_msg(struct wx14_msg_st *wx14msg){

  if(wx14msg->msg_type == WX14_MSG_SIGNAL_STATUS)
    return(1);

  return(0);
}
