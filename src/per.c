/*
 * Copyright (c) 2005-2007 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#include <time.h>
#include "globals.h"
#include "signal.h"
#include "server_priv.h"
#include "exec.h"
#include "per.h"

/*
 * The periodic function is called from within the server loop. Any
 * function registered here should be limited to set a flag or some other
 * minor chore, and put the real working function as a separate thread
 * (like is done with the scheduler).
 */

#define PERIOD_HALF_MINUTE	30
#define PERIOD_MINUTE		60
#define PERIOD_5MINUTES		300
#define PERIOD_HOUR		3600

/*
 * These are periodic events for which it does not matter exactly when they
 * are called.
 */
struct periodic_event_st {
  time_t period;
  time_t next;
  void (*proc)(void);
};

/* Events for which we must guarantee that they are called each minute. */
struct minutely_event_st {
  int last_minute;
  void (*proc)(void);
};

#define EVENT_REPORT_SERVER_STATE	0
#define EVENT_SEND_BBCLIENTID           1
#define EVENT_UPDATE_NBSP_STATS		2
static struct periodic_event_st gevents [] = {
  {PERIOD_MINUTE, 0, set_report_client_connections_flag},
  {PERIOD_5MINUTES, 0, set_send_bbclientid_flag},
  {PERIOD_MINUTE, 0, nbspstats_update},
  {0, 0, NULL}
};

#define EVENT_SCHEDULER		0
static struct minutely_event_st gmevents [] = {
  {0, exec_scheduler},
  {0, NULL}
};

static int current_minute(time_t now);

void init_periodic(void){

  struct periodic_event_st *ev = &gevents[0];
  struct minutely_event_st *mev = &gmevents[0];
  time_t now;
  int minute_now;

  /*
   * If the default periods above are changed by the configuration file
   * we use that instead.
   */
  gevents[EVENT_REPORT_SERVER_STATE].period = g.serverstate_log_period;
  gevents[EVENT_SEND_BBCLIENTID].period = g.bbclientid_secs;
  gevents[EVENT_UPDATE_NBSP_STATS].period = g.nbspstats_period;

  now = time(NULL);
  minute_now = current_minute(now);
  while(ev->proc != NULL){
    ev->next = now + ev->period;
    ++ev;
  }

  while(mev->proc != NULL){
    mev->last_minute = minute_now;
    ++mev;
  }
}

void periodic(void){
  /*
   * This function is called from the main loop thread.
   *
   * Call first the functions that are scheduled to run at some
   * specified intervals. Then those that should be run each time
   * periodic() is called.
   */
  struct periodic_event_st *ev = &gevents[0];
  struct minutely_event_st *mev = &gmevents[0];
  time_t now;
  int minute_now;
  pid_t pid;

  now = time(NULL);
  minute_now = current_minute(now);
  while(ev->proc != NULL){
    if(now >= ev->next){
      ev->next += ev->period;
      ev->proc();
    }
    ++ev;
  }

  while(mev->proc != NULL){
    if(minute_now != mev->last_minute){
      mev->last_minute = minute_now;
      mev->proc();
    }
    ++mev;
  }

  /*
   * This is the mechanism used to reload the servers list. Just let
   * the server know it should reload the list.
   */
  if(get_hup_flag() != 0){
    set_reload_servers_list_flag();
  }

  /*
   * wait_qrunner can be called (from runq) every time the qrunner is
   * executed again, but then the zombie is left until the next time
   * runq() is called. There is no harm in that, but just to keep it
   * clean, wait() is called more often here. We don't call runq
   * (i.e., we wait another round) if wait_qrunner() returns
   * non-negative (i.e., if a qrunner is still running or has just
   * "waited" for.)
   */
  pid = wait_qrunner();

  /*
   * Execute the qrunner as specified.
   */
  if((g.packet_count >= g.qrunperiod) && (pid < 0)){
    g.packet_count = 0;
      runq();
  }
}

static int current_minute(time_t now){

  struct tm tm, *tmp;
  time_t secs;
  int minute;

  secs = now;
  tmp = localtime_r(&secs, &tm);
  minute = tmp->tm_min;

  return(minute);
}
