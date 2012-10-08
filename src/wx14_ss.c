/*
 * $Id$
 */
#include <stdio.h>
#include <time.h>
#include "wx14.h"
#include "wx14_private.h"

/*
 * Signal status
 */

/*
 * Application functions
 */
int wx14_signalstatus_log(char *logfile,
			  struct wx14_msg_st *wx14msg){
  /*
   * Returns:
   *
   *  0 => no error
   * -1 => write error
   */
  int status = 0;
  
  if(wx14msg->wx14ss.unixseconds_lastlog == wx14msg->wx14ss.unixseconds)
    return(0);

  status = wx14_signalstatus_fprintf(logfile, wx14msg, "a");
  if(status == 0)
    wx14msg->wx14ss.unixseconds_lastlog = wx14msg->wx14ss.unixseconds;

  return(status);
}

int wx14_signalstatus_write(char *statusfile,
			  struct wx14_msg_st *wx14msg){
  /*
   * Returns:
   *
   *  0 => no error
   * -1 => write error
   */
  int status = 0;

  if(wx14msg->wx14ss.unixseconds_lastlog == wx14msg->wx14ss.unixseconds)
    return(0);

  status = wx14_signalstatus_fprintf(statusfile, wx14msg, "w");
 
  return(status);
}

int wx14_signalstatus_fprintf(char *file,
			      struct wx14_msg_st *wx14msg,
			      const char *mode){
  /*
   * Returns:
   *
   *  0 => no error
   * -1 => write error
   */
  int status = 0;
  FILE *f;

  f = fopen(file, mode);
  if(f == NULL)
    return(-1);

  status = fprintf(f, "%ju %d %d %d %d %d %d %d %d %d %d\n",
		   (uintmax_t)wx14msg->wx14ss.unixseconds,
		   wx14msg->wx14ss.f_freq,
		   wx14msg->wx14ss.f_viterbi,
		   wx14msg->wx14ss.f_frame,
		   wx14msg->wx14ss.f_mode,
		   wx14msg->wx14ss.level,
		   wx14msg->wx14ss.data_quality,
		   wx14msg->wx14ss.gain,
		   wx14msg->wx14ss.signal_quality,
		   wx14msg->wx14ss.loss_frame,
		   wx14msg->wx14ss.freq_error);
  if(status < 0)
    status = -1;
  else
    status = 0;

  if(fclose(f) != 0)
    status = -1;

  return(status);
}

/*
 * Library functions
 */
void wx14_signalstatus_process(struct wx14_msg_st *wx14msg){
  /*
   * After the 5 byte header:
   *
   * byte5:
   *     bit1 - freq (locked or not)
   *     bit2 - viterbi (locked and processing or not)
   *     bit3 - frame (receiving frame information)
   *     bit4 - mode (1 = normal; 0 = alignment)
   * byte6 = level
   * byte7,8 = data quality
   * byte9 = gain
   * byte10 = signal quality
   * byte11 = loss frame
   * byte12 = freq error
   */
  unsigned char *data = wx14msg->data;

  /* assert(wx14msg->msg_type == WX14_MSG_SIGNAL_STATUS); */

  wx14msg->wx14ss.unixseconds = time(NULL);
  
  wx14msg->wx14ss.f_freq = ((data[0] & 1) != 0);
  wx14msg->wx14ss.f_viterbi = ((data[0] & 2) != 0);
  wx14msg->wx14ss.f_frame = ((data[0] & 4) != 0);
  wx14msg->wx14ss.f_mode = ((data[0] & 8) != 0);
  ++data;
  wx14msg->wx14ss.level = data[0];
  ++data;
  wx14msg->wx14ss.data_quality = (data[0] << 8) + data[1];
  ++data; ++data;
  wx14msg->wx14ss.gain = data[0];
  ++data;
  wx14msg->wx14ss.signal_quality = data[0];
  ++data;
  wx14msg->wx14ss.loss_frame = data[0];
  ++data;
  wx14msg->wx14ss.freq_error = data[0];
}
