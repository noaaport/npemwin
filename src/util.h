/*
 * Copyright (c) 2004 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#ifndef NPEMWIN_UTIL_H
#define NPEMWIN_UTIL_H

#include <sys/types.h>
#include <stdint.h>

char *get_month_name(int month);
int get_month(char *s);
int valid_month(int month);

char *get_wday_name(int wday);
int valid_wday(int wday);

int get_todays_month(void);
int get_todays_day(void);

int strto_int(char *s, int *val);
int strto_uint(char *s, unsigned int *val);
int strto_double(char *s, double *val);
int strto_u16(char *s, uint16_t *val);
int valid_str(char *s);

uint32_t unpack_uint32(void *p, size_t start);
void pack_uint32(void *p, uint32_t u, size_t start);
uint16_t unpack_uint16(void *p, size_t start);
void pack_uint16(void *p, uint16_t u, size_t start);

#endif
