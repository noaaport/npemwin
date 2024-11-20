/*
 * Copyright (c) 2005-2007 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#ifndef IO_H
#define IO_H

int open_input(char *fpath);
int open_output_fifo(char *path);	/* the npemwin fifo */
void close_output_fifo(int fd);
ssize_t writef(int fd, void *buf, size_t size); /* to the npemwin fifo */

#endif
