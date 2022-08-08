/*
 * Copyright (c) 2022 Jose F. Nieves <nieves@ltp.uprrp.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#ifndef INFEED_FIFO_H

int infeed_open_fifo(char *fpath, char *mode_str);
int infeed_close_fifo(int fd, char *fpath);

#endif
