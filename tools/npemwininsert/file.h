/*
 * Copyright (c) 2005 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#ifndef FILE_H
#define FILE_H

#include <fcntl.h>

int file_exists(char *fname);
int get_file_size(char *fname, off_t *fsize);

#endif
