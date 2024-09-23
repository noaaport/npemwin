#
# $Id$
#

# defaults (FreeBSD)
INCLUDE=".include"
Q='"'
PREFIX=usr/local

CC=cc
CCWFLAGS="-Wall -Wextra -Wformat-security -Werror"
INSTALL=install
SUFFIXRULES=".c.o:"

# excude options to tar
EXCLUDE="--exclude "

TCL_INCDIR="-I /usr/local/include/tcl8.6"
TCL_LIB="tcl86"
TCLSH=/usr/local/bin/tclsh8.6

DB_INCDIR="-I /usr/local/include/db5"
DB_LIB="db-5"

LOCAL_INCDIR="-I /usr/local/include"
LOCAL_LIBDIR="-L /usr/local/lib"

# scripts dir
SHELL="/bin/sh"
FIND=find
UNZIP=unzip

os=`uname`
[ $os = Linux ] && flavor=unknown
if [ -f /etc/fedora-release ]
then
    flavor=fedoracore
elif [ -f /etc/SuSE-release ] 
then
   flavor=opensuse
elif [ -f /etc/redhat-release ] 
then
   flavor=redhat
elif [ -f /etc/debian_version ] 
then
   flavor=debian
   debian="0"
   ubuntu="0"
   if [ -f /etc/os-release ]
   then
	. /etc/os-release
	[ $ID = "debian" -o $ID="raspbian" ] && debian=$VERSION_ID
        [ $ID = "ubuntu" ] && ubuntu=$VERSION_ID
   fi
fi

case $os in
    FreeBSD) 
	;;
    NetBSD)
	CCWFLAGS="-Wall -Wno-char-subscripts -fno-strict-aliasing"
	# add the next ones to avoid spurious warnings about strlen
	CCWFLAGS="${CCWFLAGS} -Wno-stringop-overflow -Wno-stringop-truncation"
	#PREFIX=usr/pkg
        TCL_INCDIR="-I /usr/pkg/include"
        TCL_LIB="tcl86"
        TCLSH=/usr/pkg/bin/tclsh
        DB_INCDIR="-I /usr/pkg/include/db5"
        DB_LIB="db5"
	# See the note in build/netbsd about this setting
	DB_BINDIR=/usr/pkg/bin/db5
        LOCAL_LIBDIR="-L /usr/pkg/lib"
	LOCAL_INCDIR="-I /usr/pkg/include"
	;;
    OpenBSD) 
	;;
    Linux)
	INCLUDE="include"
	Q=	
	SUFFIXRULES="%.o: %.c"
	EXCLUDE="--exclude "
	TCL_INCDIR=
	TCL_LIB="tcl8.6"
	TCLSH=/usr/bin/tclsh8.6
	DB_INCDIR=
	DB_LIB="db-5.3"
	LOCAL_INCDIR=
	LOCAL_LIBDIR=

	if [ $flavor = redhat ]
	then
	   # alma linux 8
	   TCL_INCDIR=
           TCL_LIB="tcl8.6"
           TCLSH=/usr/bin/tclsh8.6
           DB_INCDIR=
           DB_LIB="db-5.3"
	elif [ $flavor = debian ]
	then
	   # debian 10, 11 or 12
	   if [ $debian = "10" -o $debian = "11" -o $debian = "12" ]
	   then
		TCL_INCDIR="-I /usr/include/tcl8.6"
	   	TCL_LIB="tcl8.6"
	   	TCLSH=/usr/bin/tclsh8.6
	   	DB_INCDIR=
	   	DB_LIB="db-5.3"
	   elif [ $ubuntu = "18.04" -o $ubuntu = "20.04" ]
	   then
 	   	TCL_INCDIR="-I /usr/include/tcl8.6"
	   	TCL_LIB="tcl8.6"
	   	TCLSH=/usr/bin/tclsh8.6
	   	DB_INCDIR=
	   	DB_LIB="db-5.3"
	   fi
    	fi
	;;
    Darwin)
	INCLUDE="include"
	Q=
	SUFFIXRULES="%.o: %.c"

	TCL_INCDIR=
	TCL_LIB="tcl8.4"
	TCLSH=/usr/bin/tclsh
	DB_INCDIR="-I /usr/local/include/db46"
	DB_LIB="db-4.6"
     	;;
esac

configure_default () {

  makefile_out=Makefile
  [ $# -eq 1 ] && makefile_out=$1
  makefile_in=${makefile_out}.in

  sed \
      -e "/@include@/ s||$INCLUDE|" \
      -e "/@q@/ s||$Q|g" \
      -e "/@INSTALL@/ s||$INSTALL|" \
      -e "/@PREFIX@/ s||$PREFIX|" \
      -e "/@CC@/ s||$CC|" \
      -e "/@CCWFLAGS@/ s||$CCWFLAGS|" \
      -e "/@SUFFIXRULES@/ s||$SUFFIXRULES|" \
      -e "/@EXCLUDE@/ s||$EXCLUDE|" \
      -e "/@TCL_INCDIR@/ s||$TCL_INCDIR|" \
      -e "/@TCL_LIB@/ s||$TCL_LIB|" \
      -e "/@TCLSH@/ s||$TCLSH|" \
      -e "/@DB_INCDIR@/ s||$DB_INCDIR|" \
      -e "/@DB_LIB@/ s||$DB_LIB|" \
      $makefile_in > $makefile_out
}
