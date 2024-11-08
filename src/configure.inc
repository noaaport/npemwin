#
# $Id$
#
. ../configure.inc

COMMON_LIBDIRS="-L libconnth -L libqdb"
COMMON_LIBS="-lconn -lqdb"

# defaults (FreeBSD)
INCDIRS="$TCL_INCDIR $DB_INCDIR"
LIBDIRS="$COMMON_LIBDIRS $LOCAL_LIBDIR"
OSLIBS="-lz -lpthread -lwrap -lutil"

case $os in
    FreeBSD) 
        # use the defaults
	;;
    NetBSD)
	# use the deualts
	;;
    OpenBSD)
	OSLIBS="-lutil"
	;;
    SunOS)
	INSTALL=ginstall
	CC=gcc
	INCDIRS="-I /opt/sfw/include -I /usr/sfw/include"
	LIBDIRS="$COMMON_LIBDIRS -L /opt/sfw/lib -L /usr/sfw/lib"
	OSLIBS="-lz -lpthread -lwrap -lsocket -lnsl -lresolv"
	;;
    Linux)
	[ $flavor = redhat ] && OSLIBS="-lz -lpthread -lrt"
	[ $flavor = debian ] && OSLIBS="-lz -lpthread -lwrap"
	#
	# If we have to use a local db:
	#
	## if [ $flavor = centos ]
	## then
	##     _BDBDIR="/usr/local/BerkeleyDB.5.1"
	##     INCDIRS="$TCL_INCDIR -I ${_BDBDIR}/include"
	##     LIBDIRS="$COMMON_LIBDIRS -L ${_BDBDIR}/lib"
	## fi
	##
	;;
    Darwin)
	INCDIRS="$DB_INCDIR" 
	COMMON_LIBDIRS="-L./libconnth -L./libqdb"
	LIBDIRS="$COMMON_LIBDIRS $LOCAL_LIBDIR"
	OSLIBS="-lpthread"
	;;	    
esac

LIBS="$COMMON_LIBS -l${TCL_LIB} -l${DB_LIB} $OSLIBS"
