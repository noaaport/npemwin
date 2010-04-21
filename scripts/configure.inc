#
# $Id$
#
. ../configure.inc

NPEMWINDSH=npemwind.sh-freebsd
NPEMWINDSHCONF=
HOURLYCONF=hourly-cleanup.conf-freebsd
STARTCLEANCONF=start-cleanup.conf-freebsd
STARTSTOPRC=startstop.rc-freebsd
PATH="/bin:/sbin:/usr/bin:/usr/sbin:/usr/local/bin:/usr/local/sbin"

case $os in
    FreeBSD) 
    	;;
    OpenBSD) 
	NPEMWINDSH=npemwind.sh-generic
    	;;
    SunOS)
	NPEMWINDSH=npemwind.sh-generic
	HOURLYCONF=hourly-cleanup.conf-generic
	STARTCLEANCONF=start-cleanup.conf-generic
    	;;
    Linux)
	NPEMWINDSH=npemwind.sh-${flavor}
	HOURLYCONF=hourly-cleanup.conf-generic
	STARTCLEANCONF=start-cleanup.conf-generic
	[ $flavor = debian ] && STARTSTOPRC=startstop.rc-debian
	[ $flavor = debian ] && NPEMWINDSHCONF=npemwind.sh.conf-debian
    	;;
    Darwin)
	NPEMWINDSH=npemwind.sh-macosx
	STARTSTOPRC=startstop.rc-macosx
	PATH="$PATH:/sw/bin:/sw/sbin"
	;;
esac
