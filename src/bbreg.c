/*
 * Copyright (c) 2005-2007 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include "err.h"
#include "util.h"
#include "globals.h"

static int bbserver_open(void);
static void bbserver_close(void);

int spawn_bbregistrar(void){

  return(bbserver_open());
}

void kill_bbregistrar(void){

  bbserver_close();
}

static int bbserver_open(void){

  if(valid_str(g.bbserver) == 0){
    log_errx("No bbserver registrar defined.");
    return(1);
  }

  g.bbserverfp = popen(g.bbserver, "w");
  if(g.bbserverfp != NULL){
    if(fprintf(g.bbserverfp, "%s\n", "init") < 0){
      (void)pclose(g.bbserverfp);
      g.bbserverfp = NULL;
    }else
      fflush(g.bbserverfp);
  }

  if(g.bbserverfp == NULL){
    log_err("Could not start bbserver registrar.");
    return(-1);
  }else
    log_info("Started bbserver registrar.");

  return(0);
}

static void bbserver_close(void){

  if(g.bbserverfp == NULL)
    return;

  if(pclose(g.bbserverfp) == -1)
    log_err("Error closing bbserver registrat.");
  else
    log_info("Stoped bbserver registrar.");

  g.bbserverfp = NULL;
}
