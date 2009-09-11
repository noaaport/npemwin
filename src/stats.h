/*
 * Copyright (c) 2005 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#ifndef STATS_H
#define STATS_H

#include <time.h>

struct nbsp_stats_st {
  unsigned int frames_processed;
  unsigned int frames_bad;
  unsigned int frames_received;
  unsigned int frames_data_size;
  int f_valid;
  time_t time;
};

void nbspstats_init(void);
void nbspstats_update(void);
void nbspstats_report(char *fname);

void update_stats_frames_received(unsigned int frame_data_size);
void update_stats_frames(int status);

#endif
