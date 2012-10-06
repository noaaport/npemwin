/*
 * $Id$
 */
#include <string.h>
#include "wx14_private.h"
#include "wx14.h"

int wx14_read_data_packet(int fd, unsigned int secs, int retry,
			  void *buf, size_t *size, int *type){
  /*
   * buf must have been allocated with the size given by *size.
   * Since the wx14 data size (dataN) is a 2-byte int, in principle
   * buf should be of size UINT16_MAX. In practice it can be just the
   * emwin packet size (in the case in which the cmd_type is an emwin
   * packet message).
   */
  int status = 0;
  struct wx14_msg_st wx14msg;

  status = wx14_read_msg(fd, secs, retry, &wx14msg);
  if(status != 0)
    return(status);

  /*
   * Verify that it is an emwin or signal status packet
   */
  if((wx14msg.cmd_type != WX14_MSG_EMWIN_PACKET) &&
     (wx14msg.cmd_type != WX14_MSG_SIGNAL_STATUS))
    return(WX14_ERROR_UNEXPECTED_CMDTYPE);

  if(*size < wx14msg.dataN)
    return(WX14_ERROR_EMWIN_BUF);

  *type = wx14msg.cmd_type;
  *size = wx14msg.dataN;
  memcpy(buf, wx14msg.data, wx14msg.dataN);

  return(status);
}

int wx14_read_emwin_packet(int fd, unsigned int secs, int retry,
			   void *buf, size_t *size){
  /*
   * Keep reading packets, ignoring any status signal packet(s),
   * until an emwin packet is received.
   */
  int status = 0;
  int type = 0;
  int original_size = *size;

  while(status == 0){
    status = wx14_read_data_packet(fd, secs, retry, buf, size, &type);
    if((status == 0) && wx14_is_emwin_packet(type))
      break;
    else
      *size = original_size;
  }

  return(status);
}

int wx14_is_emwin_packet(int type){

  if(type == WX14_MSG_EMWIN_PACKET)
    return(1);

  return(0);
}

int wx14_is_signalstatus_packet(int type){

  if(type == WX14_MSG_SIGNAL_STATUS)
    return(1);

  return(0);
}
