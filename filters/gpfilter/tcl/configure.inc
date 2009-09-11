#
# $Id$
#

. ../../../configure.inc

GPOSNAME=freebsd

case $os in
    FreeBSD) 
        # use the defaults
	;;
    NetBSD)
	GPOSNAME=netbsd
	;;
    OpenBSD) 
        GPOSNAME=openbsd
	;;
    SunOS)
	GPOSNAME=sunos
	;;
    Linux)
        GPOSNAME=linux
        [ `uname -m` = "x86_64" ] && GPOSNAME=linux64
	;;
    Darwin) 
        GPOSNAME=darwin
	;;
esac
