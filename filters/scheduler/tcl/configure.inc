#
# $Id$
#

. ../../../configure.inc

SCHEDCONF=scheduler.conf-freebsd

case $os in
    FreeBSD) 
        # use the defaults
	;;
    OpenBSD) 
	;;
    SunOS)
	SCHEDCONF=scheduler.conf-generic
	;;
    Linux)       
	SCHEDCONF=scheduler.conf-generic
	;;
    Darwin)       
	# use the default (freebsd) because it uses the same ``find''
	;;
esac
