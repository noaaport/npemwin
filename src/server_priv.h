/*
 * Copyright (c) 2005-2007 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#ifndef SERVER_PRIV_H
#define SERVER_PRIV_H

#include "emwin.h"

/*
 * This function is defined in serverc.c and used by the main
 * thread (serverm.c).
 */
void *client_thread_main(void *arg);

/* Defined in serverm.c and used in npemwin.c */
int server_send_client_queues(struct emwin_packet *ep);

/* Defined in serverm.c and used in per.c */
void set_report_client_connections_flag(void);

/* defined in loop.c, used in per.c */
void set_reload_servers_list_flag(void);
void set_send_bbclientid_flag(void);

/* Defined in loop.c, used in per.c */
int wait_qrunner(void);
void runq(void);

#endif
