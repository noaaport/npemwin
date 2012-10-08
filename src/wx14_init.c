/*
 * $Id$
 */
#include "wx14.h"

void wx14_init(struct wx14_msg_st *wx14msg){

  wx14msg->emwin_block_index = 0;
  wx14msg->wx14ss.unixseconds = 0;   /* wx14ss data is not yet valid */
  wx14msg->wx14ss.unixseconds_lastlog = 0;
}
