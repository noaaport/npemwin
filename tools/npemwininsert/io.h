/*
 * Copyright (c) 2005-2007 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#ifndef IO_H
#define IO_H

/* These write a log msg in case of error */
int open_input(char *fpath);
int open_output_fifo(char *path);	/* open the npemwin fifo */
void close_output_fifo(int fd);
int write_output_fifo(int fd, void *buf, size_t size);

/* This does not write a log msg */
ssize_t writef(int fd, void *buf, size_t size); /* write to the npemwin fifo */

#endif
