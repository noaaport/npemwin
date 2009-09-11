/*
 * Copyright (c) 2004-2008 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */

/*
 * This file has functions that are needed to support the silly things
 * of the BB protocol, and which some day should go away.
 */
#ifndef EMWIN_BB_H
#define EMWIN_BB_H

#include "servers.h"

void bb_xor(char *dst, char *src, int size);
void bb_send_clientid(struct emwin_server *es);
int bb_is_emwin_data_packet(char bbchar);
int bb_is_srvlist_packet(char bbchar);
int bb_readn(int fd, void *buf, size_t size, unsigned int secs, int retry);
int bb_read_srvlist(int fd, void *buf, size_t *size,
		    unsigned int secs, int retry);

#endif
