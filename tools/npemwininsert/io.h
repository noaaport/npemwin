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
int open_output(char *path);
int read_page(int fd, void *page, size_t page_size);
int write_page(int fd, void *page, size_t page_size);

#endif
