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
   flavor=centos
elif [ -f /etc/debian_version ] 
then
   flavor=debian
   ubuntu=0
   ubuntu=0
   if [ -f /etc/os-release ]
   then
	. /etc/os-release
	ubuntu=$VERSION_ID
   fi
fi

case $os in
    FreeBSD) 
	;;
    OpenBSD) 
	;;
    NetBSD)
	PREFIX=usr/pkg
	DB_INCDIR="-I /usr/pkg/include/db46"
	CCWFLAGS="-Wall -Wno-char-subscripts -fno-strict-aliasing"
	;;
    Linux)
	INCLUDE="include"
	Q=	
	SUFFIXRULES="%.o: %.c"
	EXCLUDE="--exclude "

      	# opensuse/fedora
	TCL_INCDIR=
	TCL_LIB="tcl8.5"
	TCLSH=/usr/bin/tclsh8.5
	DB_INCDIR=
	DB_LIB="db-4.8"

	if [ $flavor = centos ]
	then
	   TCL_INCDIR=
           TCL_LIB="tcl8.5"
           TCLSH=/usr/bin/tclsh8.5
           DB_INCDIR=
           DB_LIB="db-4.7"
	elif [ $flavor = debian ]
	then
	   TCL_INCDIR="-I /usr/include/tcl8.5"
	   TCL_LIB="tcl8.5"
	   TCLSH=/usr/bin/tclsh8.5
	   DB_INCDIR=
	   DB_LIB="db-5.1"
	   [ $ubuntu != "0" ] && DB_LIB="db-5.1"
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
