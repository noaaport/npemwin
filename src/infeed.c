/*
 * Copyright (c) 2022 Jose F. Nieves <nieves@ltp.uprrp.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <err.h>
#include <stdio.h>
#include <unistd.h>
#include "infeed.h"

int infeed_open_fifo(char *fpath, char *mode_str) {
  /*
   * Returns:
   * fd
   * -1 => system error
   * -2 => mode_str does not represent an octal number
   */
  mode_t mode;
  int fd = -1;
  int status = 0;
  unsigned short r;

  /*
   * In FreeBSD mode_t is unsigned short but in linux it s unsigned int.
   * Thus in FreeBSD the compiler complains of this
   *
   * if(sscanf(mode_str, "%o", &mode) != 1)
   *   return(-2);
   *
   * and in Linux it complains if we use "%ho"
   */
  
  if(sscanf(mode_str, "%ho", &r) != 1)
    return(-2);

  mode = r;
  
  (void)unlink(fpath);
  status = mkfifo(fpath, mode);

  if(status == 0){
    /*
     * The mkfifo() sets the mode masked by the default umask. We need to call
     * chmod to set the absolute mask.
     */
    status = chmod(fpath, mode);
  }

  /*
   * If we open the fifo O_RDONLY, then when the last writer closes the fifo
   * it will cause readn() in recv_in_packet() [reader.c] to return eof (n== 0)
   * which will trigger a return error from recv_in_packet() in slavein_loop
   * and will force to close and reopen the fifo (and the associated "error"
   * message(s). Therefore we will open the fifo RW so that the number of
   * writers never drops to zero.
   */
  if(status == 0)
    fd = open(fpath, O_RDWR);

  return(fd);
}

int infeed_close_fifo(int fd, char *fpath) {

  int status = 0;

  status = close(fd);

  if((status == 0) && (fpath != NULL))
    (void)unlink(fpath);

  return(status);
}
