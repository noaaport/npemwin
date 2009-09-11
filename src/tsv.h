/*
 * $Id$
 */
#ifndef TSV_H
#define TSV_H

#include <stdio.h>

int tsverror(void);
void tsvrelease(void);
char *tsvgetline(FILE *fp);
int tsvargc(void);
char *tsvargv(int n);	/* elements of the argv array */
char **tsvargvp(void);	/* pointer to the argv array */

#endif
