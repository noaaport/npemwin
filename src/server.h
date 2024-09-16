/*
 * Copyright (c) 2004 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */

#ifndef SERVER_H
#define SERVER_H

int init_network_server(void);
void terminate_network_server(void);
void network_server_loop(void);

#endif
