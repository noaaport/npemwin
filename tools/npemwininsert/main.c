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
#include "err.h"
#include "fifo.h"
#include "io.h"
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
static int open_output(void);
static void close_output(void);
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
  
  if (g.opt_C == 1) {
    check();
    return(0);
  }

  atexit(cleanup);
  
  status = open_output();
  
  if(status == 0)
    status = process_file();

  close_output();

  return(status != 0 ? 1 : 0);
}

static void cleanup(void) {

  if(g.ep != NULL) {
    clean_emwin_packet_st(g.ep);
    g.ep = NULL;
  }

  close_output();
}

static void check(void){

  fprintf(stdout, "opt_input_emwinfname: %s\n", g.opt_input_emwinfname);
  fprintf(stdout, "opt_input_fpath: %s\n", g.opt_input_fpath);
  fprintf(stdout, "opt_fifo_fpath: %s\n", g.opt_fifo_fpath);

  fprintf(stdout, "opt_C: %d\n", g.opt_C);
  fprintf(stdout, "opt_background: %d\n", g.opt_background);
}

static int open_output(void) {

  int fd = -1;
  int status = 0;

  /* These two functions log the errors themselves */
  
  status = check_fifo(g.opt_fifo_fpath);  
  if(status != 0)
    return(status);

  fd = open_output_fifo(g.opt_fifo_fpath);
  if(fd == -1)
    return(-1);
  
  g.output_fifo_fd = fd;

  return(0);
}

static void close_output(void) {

  if(g.output_fifo_fd == -1)
    return;

  close_output_fifo(g.output_fifo_fd);
  g.output_fifo_fd = -1;
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
      log_err(0, "Could not send the entire emwin packet %s", gep.fname);

    if(status != 0)
      break;
  }

  clean_emwin_packet_st(&gep);

  return(status);
}
