/*
 * $Id$
 */
#include "readn.h"
#include "wx14_private.h"
#include "wx14.h"

static int wx14_readn(int fd, unsigned int secs, int retry,
		      void* buf, size_t size);
static int wx14_read_msg_header(int fd, unsigned int secs, int retry,
				struct wx14_msg_st *wx14msg);
static int wx14_read_msg_data(int fd, unsigned int secs, int retry,
			      struct wx14_msg_st *wx14msg);

/*
 * exposed functions of this file
 */
int wx14_read_msg(int fd, unsigned int secs, int retry,
		  struct wx14_msg_st *wx14msg){

  int status = 0;

  status = wx14_read_msg_header(fd, secs, retry, wx14msg);
  if(status == 0)
    status = wx14_read_msg_data(fd, secs, retry, wx14msg);

  return(status);
}

void *wx14_msg_data(struct wx14_msg_st *wx14msg){

  return(wx14msg->data);
}

int wx14_msg_data_size(struct wx14_msg_st *wx14msg){

  return(wx14msg->dataN);
}

int wx14_msg_cmdtype(struct wx14_msg_st *wx14msg){

  return(wx14msg->cmd_type);
}

/*
 * private to this file
 */
static int wx14_readn(int fd, unsigned int secs, int retry,
		      void* buf, size_t size){
  int status = 0;
  ssize_t n;

  n = readn(fd, buf, size, secs, retry);

  if(n == -1)
    status = -1;
  else if(n == -2)
    status = WX14_ERROR_READ_TIMEOUT; /* poll timeout before reading anything */
  else if(n == 0)
    status = WX14_ERROR_READ_EOF;     /* eof (server disconnected) */
  else if((size_t)n != size){
    /*
     * short read (timedout or disconnect) while reading
     */
    status = WX14_ERROR_READ_SHORT; 
  }

  return(status);
}

static int wx14_read_msg_header(int fd, unsigned int secs, int retry,
				struct wx14_msg_st *wx14msg){

  int status = 0;

  status = wx14_readn(fd, secs, retry,
		      &wx14msg->message[0], WX14_HEADER_SIZE);

  if(status == 0){
    if((wx14msg->message[0] != WX14_HEADER_BYTE0) ||
       (wx14msg->message[1] != WX14_HEADER_BYTE1)){
      
      status = WX14_ERROR_INVALID_HEADER;
    }
  }

  if(status == 0){
    wx14msg->cmd_type = wx14msg->message[4];
    wx14msg->dataN = (wx14msg->message[2] << 8) + wx14msg->message[3];
  }

  return(status);
}

static int wx14_read_msg_data(int fd, unsigned int secs, int retry,
			      struct wx14_msg_st *wx14msg){
  int status = 0;

  wx14msg->data = NULL;
  status = wx14_readn(fd, secs, retry,
		      &wx14msg->message[5], wx14msg->dataN);
  if(status == 0)
    wx14msg->data = &wx14msg->message[5];

  return(status);
}
