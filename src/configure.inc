#
# $Id$
#
. ../configure.inc

COMMON_LIBDIRS="-L libconnth -L libqdb"
COMMON_LIBS="-lconn -lqdb -lwrap -lz"

# defaults (FreeBSD)
INCDIRS="$TCL_INCDIR $DB_INCDIR"
LIBDIRS="$COMMON_LIBDIRS -L /usr/local/lib"
LIBS="$COMMON_LIBS -lutil -l${TCL_LIB} -l${DB_LIB} -lpthread"

case $os in
    FreeBSD) 
        # use the defaults
	;;
    OpenBSD) 
	LIBS="$COMMON_LIBS -lutil -l${TCL_LIB} -l${DB_LIB}"
	;;
    SunOS)
	INSTALL=ginstall
	CC=gcc
	INCDIRS="-I /opt/sfw/include -I /usr/sfw/include"
	LIBDIRS="$COMMON_LIBDIRS -L /opt/sfw/lib -L /usr/sfw/lib"
	LIBS="$COMMON_LIBS -l${TCL_LIB} -l${DB_LIB} \
			   -lpthread -lsocket -lnsl -lresolv"
	;;
    Linux)       
	LIBDIRS="$COMMON_LIBDIRS"
	LIBS="$COMMON_LIBS -l${TCL_LIB} -l${DB_LIB} -lpthread -lnsl -lresolv"
	[ $flavor = debian ] && \
		LIBS="$COMMON_LIBS -l${TCL_LIB} -l${DB_LIB} -lpthread"
	#
	# If we need to use local libs (e.g., bdb) use
	# [ $flavor = centos ] && LIBDIRS="$COMMON_LIBDIRS -L/usr/local/lib"
	#
	;;
    Darwin)
	INCDIRS="$DB_INCDIR" 
	COMMON_LIBDIRS="-L./libconnth -L./libqdb"
	LIBDIRS="$COMMON_LIBDIRS -L/usr/local/lib"
	LIBS="$COMMON_LIBS -l${TCL_LIB} -l${DB_LIB} -lpthread"
	;;	    
esac
