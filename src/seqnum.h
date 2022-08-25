/*
 * Copyright (c) 2006 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#ifndef SEQNUM_H
#define SEQNUM_H

#define SEQNUM_SIZE	16
/*
 * This function assumes that "key" has been declared to be
 * at least [SEQNUM_SIZE + 1] characters long.
 * 
 * char key[SEQNUM_SIZE + 1];
 */

int get_seqnum_key_old(char *key);
int get_seqnum_key(char *key);

#endif
