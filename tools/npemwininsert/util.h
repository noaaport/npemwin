/*
 * Copyright (c) 2005 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#ifndef UTIL_H
#define UTIL_H

#include <sys/types.h>
#include <stdint.h>

/* uint32_t calc_checksum(void *data, size_t size); */
int calc_checksum(void *data, size_t size);
int valid_str(char *s);

#endif
