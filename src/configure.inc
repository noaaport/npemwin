#
# $Id$
#
. ../configure.inc

COMMON_LIBDIRS="-L libconnth -L libqdb"
COMMON_LIBS="-lconn -lqdb -lwrap -lz"

INCDIRS="$TCL_INCDIR $DB_INCDIR"
LIBDIRS="$COMMON_LIBDIRS $LOCAL_LIBDIR"
# defaults (FreeBSD)
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
	LIBS="$COMMON_LIBS -l${TCL_LIB} -l${DB_LIB} -lpthread -lrt"
	[ $flavor = debian ] && \
		LIBS="$COMMON_LIBS -l${TCL_LIB} -l${DB_LIB} -lpthread"

	# In Ubuntu-12.04-i386 the default db-5.1 library apparently
	# cannot be used in a threaded application. We will use the
	# locally (static) compiled version, for both amd and i386.
	#
	if [ $ubuntu = "12.04" ]
	then
	    _BDBDIR="/usr/local/BerkeleyDB.5.1"
	    INCDIRS="$TCL_INCDIR -I ${_BDBDIR}/include"
	    LIBDIRS="$COMMON_LIBDIRS -L ${_BDBDIR}/lib"
	fi
	;;
    Darwin)
	INCDIRS="$DB_INCDIR" 
	COMMON_LIBDIRS="-L./libconnth -L./libqdb"
	LIBDIRS="$COMMON_LIBDIRS $LOCAL_LIBDIR"
	LIBS="$COMMON_LIBS -l${TCL_LIB} -l${DB_LIB} -lpthread"
	;;	    
esac
