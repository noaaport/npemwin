/*
 * Copyright (c) 2022 Jose F. Nieves <nieves@ltp.uprrp.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#include <libgen.h> /* basename */
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>	/* flock() */
#include <fcntl.h>
#include <errno.h>
#include "err.h"
#include "fifo.h"
#include "emwin.h"

/* defaults */
#define OUTPUT_FIFO_FPATH	"/var/run/npemwin/infeed.fifo"

static struct {
  char *opt_input_emwinfname;
  char *opt_input_fpath;
  char *opt_fifo_fpath; /* [-f] */
  int opt_C;            /* check and exit */
  int opt_background;
  /* variables */
  int output_fifo_fd;
  struct emwin_packet_st *ep;
} g = {NULL, NULL, NULL, 0, 0, -1, NULL};

static struct emwin_packet_st gep;

static void cleanup(void);
static void check(void);
static int open_output_fifo(void);
static void close_output_fifo(void);
static int process_file(void);

int main(int argc, char **argv){

  char *optstr = "bCf:";
  char *usage = "npemwininsert [-C] [-b] [-f <fifo>] <emwinfname> <fpath>";
  int c;
  int status = 0;
  
  set_progname(basename(argv[0]));

  /* defaults */
  g.opt_fifo_fpath = OUTPUT_FIFO_FPATH;

  while((status == 0) && ((c = getopt(argc, argv, optstr)) != -1)){
    switch(c){
    case 'b':
      g.opt_background = 1;
      break;
    case 'C':
      g.opt_C = 1;
      break;
    case 'f':
      g.opt_fifo_fpath = optarg;
      break;
    case 'h':
    default:
      status = 1;
      log_errx(1, "%s\n", usage);
      break;
    }
  }

  if(g.opt_background == 1)
    set_usesyslog();

  if(optind != argc - 2)
    log_errx(1, "%s\n", usage);

  g.opt_input_emwinfname = argv[optind++];
  g.opt_input_fpath = argv[optind++];

  atexit(cleanup);
  
  if (g.opt_C == 1) {
    check();
    return(0);
  }

  status = open_output_fifo();
  
  if(status == 0)
    status = process_file();

  close_output_fifo();

  return(status != 0 ? 1 : 0);
}

static void cleanup(void) {

  if(g.ep != NULL) {
    clean_emwin_packet_st(g.ep);
    g.ep = NULL;
  }

  close_output_fifo();
}

static void check(void){

  fprintf(stdout, "opt_input_emwinfname: %s\n", g.opt_input_emwinfname);
  fprintf(stdout, "opt_input_fpath: %s\n", g.opt_input_fpath);
  fprintf(stdout, "opt_fifo_fpath: %s\n", g.opt_fifo_fpath);

  fprintf(stdout, "opt_C: %d\n", g.opt_C);
  fprintf(stdout, "opt_background: %d\n", g.opt_background);
}

static int open_output_fifo(void) {

  int fd = -1;
  int status = 0;

  status = check_fifo(g.opt_fifo_fpath);

  if(status == -1)
    log_err(0, "Error from stat: %s", g.opt_fifo_fpath);
  else if(status != 0) {
    status = 1;
    log_errx(0, "Not a fifo: %s\n", g.opt_fifo_fpath); 
  }
  
  if(status != 0)
    return(status);

  /*
   * Open it in blocking mode so that the write() is blocked until npemwin
   * has read enough from the pipe to make space for the write(). Otherwise
   * we will get errors like "Resource tempoarily unvailable" 
   * and loss of packets when the pipe gets full.
   */
  fd = open(g.opt_fifo_fpath, O_WRONLY);
  if(fd == -1) {
    log_err(0, "Error from open: %s\n", g.opt_fifo_fpath);
    return(-1);
  }

  status = flock(fd, LOCK_EX);
  if(status == -1) {
    log_err(0, "Error from flock: %s\n", g.opt_fifo_fpath);

    if(close(fd) == -1)
      log_err(0, "Error from close: %s\n", g.opt_fifo_fpath);
    
    return(-1);
  }
  
  g.output_fifo_fd = fd;

  return(0);
}

static void close_output_fifo(void) {

  int status = 0;
  
  if(g.output_fifo_fd == -1)
    return;

  status = flock(g.output_fifo_fd, LOCK_UN);
  if(status == -1)
    log_err(0, "Error unlocking emwin fifo: %s\n", g.opt_fifo_fpath);
  
  status = close(g.output_fifo_fd);
  g.output_fifo_fd = -1;

  if(status != 0)
    log_err(0, "Error closing emwin fifo: %s\n", g.opt_fifo_fpath);
}

static int process_file(void) {

  int status = 0;
  int i;
  
  status = init_emwin_packet_st(&gep, g.opt_input_fpath,
				g.opt_input_emwinfname);

  if(status == -1)
    log_err(0, "%s: %s - %s\n", "Error from init_emwin_packet",
	    g.opt_input_fpath, g.opt_input_emwinfname);
  else if(status != 0)
    log_errx(0, "%s: %s - %s\n", "Error from init_emwin_packet",
	    g.opt_input_fpath, g.opt_input_emwinfname);

  if(status != 0)
    return(status);

  g.ep = &gep;
  
  for(i = 1; i <= gep.parts_total; ++i){
    status = build_emwin_packet(&gep);
    if(status == -1)
      log_err(0, "Could not build emwin packet %s", gep.fname); 
    else if(status != 0)
      log_errx(0, "Header error building emwin packet %s.", gep.fname); 

    if(status == 0)
      status = send_emwin_packet(g.output_fifo_fd, &gep);
    
    if(status == -1)
      log_err(0, "Could not send emwin packet %s", gep.fname);
    else if(status == 1)
      log_errx(0, "Could not send emwin packet %s", gep.fname);

    if(status != 0)
      break;
  }

  clean_emwin_packet_st(&gep);

  return(status);
}
