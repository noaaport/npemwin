/*
 * Copyright (c) 2004 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#ifndef INITE_H
#define INITE_H

void init_globals(void);
void cleanup(void);
int init_daemon(void);
int init_lock(void);
int init_server_list(void);
int init_directories(void);
int drop_privs(void);

#endif
