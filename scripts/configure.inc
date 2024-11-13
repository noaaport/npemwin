#
# $Id$
#
. ../configure.inc

RCINIT=npemwind.sh-freebsd
RCFPATH=/usr/local/etc/rc.d/npemwind
RCCONF=
HOURLYCONF=hourly-cleanup.conf-freebsd
STARTCLEANCONF=start-cleanup.conf-freebsd
STARTSTOPRC=startstop.rc-freebsd
SYSTEMDCONF=
PATH="/bin:/sbin:/usr/bin:/usr/sbin:/usr/local/bin:/usr/local/sbin"

case $os in
    FreeBSD) 
    	;;
    NetBSD)
	RCINIT=npemwind.sh-netbsd
	;;
    OpenBSD) 
	RCINIT=npemwind.sh-generic
    	;;
    SunOS)
	RCINIT=npemwind.sh-generic
	HOURLYCONF=hourly-cleanup.conf-generic
	STARTCLEANCONF=start-cleanup.conf-generic
    	;;
    Linux)
	RCINIT=npemwind.sh-${flavor}
	STARTCLEANCONF=start-cleanup.conf-generic
	RCFPATH=/etc/init.d/npemwind
	SYSTEMDCONF=npemwin.service-generic

	[ $flavor = debian ] && STARTSTOPRC=startstop.rc-debian
	[ $flavor = debian ] && RCCONF=npemwind.sh.conf-debian
	[ $flavor = redhat ] && SYSTEMDCONF=npemwin.service-redhat
    	;;
    Darwin)
	RCINIT=npemwind.sh-macosx
	STARTSTOPRC=startstop.rc-macosx
	PATH="$PATH:/sw/bin:/sw/sbin"
	;;
esac
