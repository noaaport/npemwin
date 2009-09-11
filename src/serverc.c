/*
 * Copyright (c) 2005-2007 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */

/*
 * This file has the client thread functions. The main function here
 * is what the main server inserts in pthread_create when it calls
 * client_thread_create(). The main server thread is in serverm.c.
 */
#include <assert.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include "libconnth/libconn.h"
#include "util.h"
#include "readn.h"
#include "globals.h"
#include "err.h"
#include "signal.h"
#include "server_priv.h"

static void loop(struct conn_element_st *ce);
static void cleanup(void*);
static int send_emwin_client(struct conn_element_st *ce, void *data, 
			     uint32_t data_size);
static void periodic(struct conn_element_st *ce);

void *client_thread_main(void *arg){
  /*
   * The *arg is a private copy of the ce. This function must
   * free(ce) it when it is finished, but it must _not_ try to
   * delete anything else from the ce.
   * To avoid cancelling the thread while they hold a mutex locked,
   * the cancellation state is set to DISABLED and renabled in the loop.
   */
  int cancel_state;
  int status = 0;
  struct conn_element_st *ce = (struct conn_element_st*)arg;

  pthread_cleanup_push(cleanup, arg);

  while((get_quit_flag() == 0) && (conn_element_get_exit_flag(ce) == 0)){
    pthread_testcancel();
    status = pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &cancel_state);
    if(status == 0){
      periodic(ce);
      loop(ce);
      status = pthread_setcancelstate(cancel_state, &cancel_state);
    }
    assert(status == 0);
    if(status != 0)
      log_err("pthread_setcancelstate() returning error in serverc.c.");
  }

  pthread_cleanup_pop(1);

  return(NULL);
}

static void loop(struct conn_element_st *ce){
  /*
   * The only job of this function is to read from the ce->cq, and
   * write to the ce->fd client file descriptor. The *data returned
   * points to a reusable internal storage in the ce->cq, and
   * should not be free()'d explicitly. It is released when the
   * main thread deletes the client from the table.
   */
  int status;
  int dberror;
  void *data = NULL;
  uint32_t data_size;
  char *bbdata;
  uint32_t bbdata_size;
  int timeout_ms;
  
  timeout_ms = g.client_queue_timeout_msecs;

  status = connqueue_rcv(ce->cq, &data, &data_size, timeout_ms, &dberror);
  if(status == -1)
    log_err2_db("Error reading from queue for %s.",
		conn_element_get_nameorip(ce), dberror);
  else if(status == 1)
    log_errx("No data in queue for %s.", conn_element_get_nameorip(ce));

  if(status == 0){
    bbdata_size = unpack_uint32(data, 0);
    bbdata = (char*)data;
    bbdata += EMWIN_QUEUE_ENVELOPE_SIZE;
    status = send_emwin_client(ce, bbdata, bbdata_size);
  }

  if(status != 0)
    conn_stats_update_errors(&ce->cs);
}

static void cleanup(void *arg){
  /*
   * See note in client_thread_main().
   *
   * In principle, we should also call connqueue_rcv_cleanup() here to
   * unlock the libqdb mutex since it is posible that the thread was
   * canceled while waiting on the condition variable in connqueue_rcv().
   * But since we have disabled the cancellation while calling
   * connqueue_rcv() there is no need to do that.
   */
  int status;
  struct conn_element_st *ce = (struct conn_element_st*)arg;

  /*
   * status = connqueue_rcv_cleanup(ce->cq, &dberror);
   * if(status != 0)
   *    log_err("Error from connqueue_rcv_cleanup()");
   */

  /*
   * The thread_finished flag is set here so that if the thread is exiting
   * by itself (e.g., if it encountered an unrrecoverable error from write())
   * the main thread can check and remove the connection element
   * from the table.
   */

  status = conn_element_set_finished_flag(ce);
  if(status != 0){
    /*
     * This really should not happen; if it does it is a bug.
     */
    log_err("Error from conn_element_set_finished_flag().");
  }

  free(ce);
}

static void periodic(struct conn_element_st *ce){
  /*
   * When the number of packets given in the variable g.serverthreads_freq
   * have been processed, this function will write the stats summary since
   * the last time that it was called.
   *
   * Also check the status flag of the queue. The flags are defined
   * in libqdb/qdb.h, and they are set by the server via the libqdb snd()
   * function. If the hard or soft limit flags are set, we
   * let the thread continue to run to see if the situation normalizes.
   * If the error flag is set, then exit the thread to resync.
   */
  int status = 0;
  char *nameorip;

  if(connqueue_test_dberror_flag(ce->cq) != 0){
      nameorip = conn_element_get_nameorip(ce);
      log_errx("Server set qdb_status flag for %s. Finishing client thread.",
	       nameorip);
    conn_element_set_exit_flag(ce);
    return;
  }

  status = conn_element_report_cstats(ce, g.serverthreads_log_freq,
				      g.serverthreadsfile);
  if(status != 0)
    log_err_write(g.serverthreadsfile);
}

static int send_emwin_client(struct conn_element_st *ce, void *data, 
			     uint32_t data_size){  
  int fd;
  char *nameorip;
  int status = 0;
  ssize_t n = 0;

  fd = conn_element_get_fd(ce);
  nameorip = conn_element_get_nameorip(ce);

  /* We are not using ce->write_timeout_ms in npemwin */
  n = writen(fd, data, (size_t)data_size, (unsigned int)g.writetimeout_s,
	     g.writetimeout_retry);

  if(n < 0){
    status = -1;
    log_err2("Cannot transmit to client", nameorip);
  } else if((uint32_t)n != data_size){
    status = 1;
    log_errx("Cannot transmit to client %s. Timed out.", nameorip);
  }

  if(status != 0){
    /*
     * We will exit the thread to force the client
     * to reopen the connection since there is no other way to
     * resynchronize the server and client.
     */
    conn_element_set_exit_flag(ce);
  } else
    conn_stats_update_packets(&ce->cs, data_size);

  if(status == 0)
    log_verbose(1, "Transmitted packet to client %s", nameorip);

  return(status);
}
