#
# Copyright (c) 2005 2006 Jose F. Nieves <nieves@ltp.upr.clu.edu>
#
# See LICENSE
#
# $Id$
#

SUBDIRS = postfilter nntpfilter rssfilter stdfilter npemwinfilter prefilter \
	panfilter metarfilter gpfilter dafilter ldmfilter spoolfilter \
	msgfilter trackfilter arcfilter bbserver scheduler

all:
	for dir in ${SUBDIRS}; do \
	(cd $$dir; ${MAKE}) \
	done

install:
	for dir in ${SUBDIRS}; do \
	(cd $$dir; ${MAKE} install) \
	done

clean:
	rm -f *~
	for dir in ${SUBDIRS}; do \
	(cd $$dir; ${MAKE} clean) \
	done

