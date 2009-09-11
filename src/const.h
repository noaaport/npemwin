/*
 * Copyright (c) 2005-2007 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#ifndef NPEMWIN_CONST_H
#define NPEMWIN_CONST_H

/*
 * The protocols supported.
 */
#define PROTOCOL_NONE		0	/* disable server */
#define PROTOCOL_EMWIN1		1
#define PROTOCOL_EMWIN2		2
#define PROTOCOL_ALL		3
#define PROTOCOL_UNKNOWN	4	/* a client that has not identified */
#define PROTOCOL_DEFAULT	PROTOCOL_EMWIN1

/* The key that the client must send in the id string */
#define PROTOCOL_EMWIN1_STR	"V1"
#define PROTOCOL_EMWIN2_STR	"V2"

#endif
