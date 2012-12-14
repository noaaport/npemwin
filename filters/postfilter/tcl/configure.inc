#
# $Id$
#
. ../../../configure.inc

# defaults (FreeBSD)
PATH="/bin:/sbin:/usr/bin:/usr/sbin";

case $os in
    FreeBSD) 
        # use the defaults
	;;
    NetBSD)
	;;
    OpenBSD) 
	;;
    SunOS)
	;;
    Linux)       
	;;
    Darwin)       
	PATH="/bin:/sbin:/usr/bin:/usr/sbin:/sw/bin:/sw/sbin";
	;;
esac
