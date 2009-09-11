/*
 * Copyright (c) 2005 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#include <stdio.h>
#include "err.h"
#include "stats.h"
#include "globals.h"
#include "defaults.h"

static void init_counters(void);
static void update_stats_goodframes(void);
static void update_stats_badframes(void);

void nbspstats_init(void){

  init_counters();
  g.nbspstats.time = time(NULL);
}
  
static void init_counters(void){

  g.nbspstats.frames_processed = 0;
  g.nbspstats.frames_bad = 0;
  g.nbspstats.frames_received = 0;
  g.nbspstats.frames_data_size = 0;
};

void nbspstats_update(void){

  time_t now;
  
  now = time(NULL);

  /*
   * This function is now called from the periodic module, and therefore
   * this should not be used.
   *
   *  if(difftime(now, g.nbspstats.time) < (double)g.nbspstats_logperiod_s)
   *    return;
   */

  g.nbspstats.time = now;
  nbspstats_report(g.statusfile);
  init_counters();  /* does not clear rtx_index */
}

void nbspstats_report(char *fname){

  FILE *f;

  f = fopen(fname, "a");
  if(f == NULL){
    log_err2("Could not open", fname);
    return;
  }

  fprintf(f, "%u %u %u %u %u\n",
	  (unsigned int)g.nbspstats.time,
	  g.nbspstats.frames_received,
	  g.nbspstats.frames_processed,
	  g.nbspstats.frames_bad,
	  g.nbspstats.frames_data_size);

  fclose(f);
}

void update_stats_frames_received(unsigned int frame_data_size){

  ++g.nbspstats.frames_received;
  g.nbspstats.frames_data_size += frame_data_size;
}

void update_stats_frames(int status){
  /*
   * Note: For updating missing frames the function
   * update_stats_frames_jumps() must be called separately.
   */
  if(status == 0)
    update_stats_goodframes();
  else
    update_stats_badframes();
}

static void update_stats_goodframes(void){

  ++g.nbspstats.frames_processed;
}

static void update_stats_badframes(void){

  ++g.nbspstats.frames_bad;
}
