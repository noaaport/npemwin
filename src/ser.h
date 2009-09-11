/*
 * Copyright (c) 2005-2008 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#ifndef SERIAL_SETTINGS_H

int ser_open_port(char *device, char *settings_str);
int ser_close_port(int fd);

#endif
