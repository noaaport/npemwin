/*
 * $Id$
 */
#include <string.h>
#include "wx14.h"
#include "wx14_private.h"

static void start_emwin_block(struct wx14_msg_st *wx14msg);
static void end_emwin_block(struct wx14_msg_st *wx14msg);
static void append_emwin_block(struct wx14_msg_st *wx14msg);
static int find_emwin_block_start(struct wx14_msg_st *wx14msg);
static int find_emwin_block_end(struct wx14_msg_st *wx14msg);

/*
 * exposed by this file
 */
int wx14_init_emwin_block(int fd, unsigned int secs, int retry,
			  struct wx14_msg_st *wx14msg){
  /*
   * This function keeps reading wx14 messages until it detects one
   * that has the start of an emwin block.
   */
  int status = 0;

  wx14msg->emwin_block_index = 0;

  while(1){
    status = wx14_read1_data_msg_emwin(fd, secs, retry, wx14msg);
    if(status != 0)
      break;

    if(find_emwin_block_start(wx14msg)){
      start_emwin_block(wx14msg);
      break;
    }
  }

  return(status);
}

int wx14_read_emwin_block(int fd, unsigned int secs, int retry,
			  struct wx14_msg_st *wx14msg){
  int status = 0;
  
  while(1){
    status = wx14_read1_data_msg_emwin(fd, secs, retry, wx14msg);
    if(status != 0){
      break;
    }

    if(find_emwin_block_end(wx14msg)){
      end_emwin_block(wx14msg);
      if(find_emwin_block_start(wx14msg))
	start_emwin_block(wx14msg);
      else {
	status = wx14_init_emwin_block(fd, secs, retry, wx14msg);
      }
      break;
    } else if(find_emwin_block_start(wx14msg)) {
      /*
       * The start of a block has been received before completing
       * the current block. We could try to call "start_emwin_block()"
       * here and continue, but that would not report the error to
       * the application.
       */
      status = WX14_ERROR_EMWIN_INCOMPLETEBLOCK;
      break;
    } else
      append_emwin_block(wx14msg);
  }

  return(status);
}

void *wx14_get_emwin_block(struct wx14_msg_st *wx14msg){

  return(wx14msg->emwin_block);
}

int wx14_memcpy_emwin_block(void *buf, size_t *size,
			    struct wx14_msg_st *wx14msg){

  if(*size < wx14msg->emwin_block_size)
    return(WX14_ERROR_EMWIN_BUF);

  *size = wx14msg->emwin_block_size;
  memcpy(buf, wx14msg->emwin_block, wx14msg->emwin_block_size);

  return(0);
}
  
/*
 * private to this file
 */
static void start_emwin_block(struct wx14_msg_st *wx14msg){
  /*
   * Call after find_emwin_block_start().
   *
   * Copy six nulls to emwin_block_part[], and append the
   * contents of data[index]-data[dataN]
   */
  int index = wx14msg->emwin_block_index;
  unsigned char *data = wx14msg->data;
  size_t n;

  memset(wx14msg->emwin_block_part, 0, EMWIN_BLOCK_SIZE);
  n = wx14msg->dataN - index;
  memcpy(&wx14msg->emwin_block_part[6], &data[index], n);
  wx14msg->emwin_block_part_size = n + 6;
}

static void end_emwin_block(struct wx14_msg_st *wx14msg){

  int index = wx14msg->emwin_block_index;
  size_t n, start;
  
  /*
   * Append the contents in data[] to the start of the new block,
   * given by data[index].
   */
  start = wx14msg->emwin_block_part_size;
  n = index;
  memcpy(&wx14msg->emwin_block_part[start], wx14msg->data, n);

  /* Update packet_size */
  wx14msg->emwin_block_part_size += index;
  
  /*
   * Copy the finished emwin block that will be returned to the application,
   * and restart the next emwin block part
   */
  memcpy(wx14msg->emwin_block, wx14msg->emwin_block_part,
	 wx14msg->emwin_block_part_size);
  wx14msg->emwin_block_size = wx14msg->emwin_block_part_size;

  if(find_emwin_block_start(wx14msg))    
    start_emwin_block(wx14msg);
}

static void append_emwin_block(struct wx14_msg_st *wx14msg){

  size_t start = wx14msg->emwin_block_part_size;

  /*
  assert(wx14msg->emwin_block_part_size + wx14msg->dataN <=
	 EMWIN_BLOCK_SIZE);
  */

  memcpy(&wx14msg->emwin_block_part[start], wx14msg->data, wx14msg->dataN);
  wx14msg->emwin_block_part_size += wx14msg->dataN;
}

static int find_emwin_block_start(struct wx14_msg_st *wx14msg){
  /*
   * A new block is signaled by
   *
   * /PF
   *
   * or
   *
   * 000000/
   */
  int i;
  int found = 0;
  int index = 0;
  unsigned char *data = wx14msg->data;
  int size;

  /* assert(wx14msg->dataN <= INTMAX); */
  size = (int)wx14msg->dataN;

  for(i = 0; i < size - 2; ++i){
    if((data[0] == '/') &&
       (data[1] == 'P') &&
       (data[2] == 'F')){
      found = 1;
      break;
    }
    ++index;
    ++data;
  }

  if(found){
    wx14msg->emwin_block_index = index;
    return(found);
  }

  found = 0;
  index = 6;
  data = wx14msg->data;
  for(i = 0; i < size - 6; ++i){
    if((data[0] == '\0') &&
       (data[1] == '\0') &&
       (data[2] == '\0') &&
       (data[3] == '\0') &&
       (data[4] == '\0') &&
       (data[5] == '\0') &&
       (data[6] == '/')){
      found = 1;
      break;
    }
    ++index;
    ++data;
  }

  if(found)
    wx14msg->emwin_block_index = index;

  return(found);
}

static int find_emwin_block_end(struct wx14_msg_st *wx14msg){

  size_t n;

  if(wx14msg->emwin_block_part_size + wx14msg->dataN < EMWIN_BLOCK_SIZE)
    return(0);

  n = EMWIN_BLOCK_SIZE - wx14msg->emwin_block_part_size;
  wx14msg->emwin_block_index = n;
    
  return(1);
}
