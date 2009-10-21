#
# $Id$
#

# defaults (FreeBSD)
INCLUDE=".include"
Q='"'
PREFIX=usr/local

TCL_INCDIR="-I /usr/local/include/tcl8.5"
TCL_LIB="tcl85"
TCLSH=/usr/local/bin/tclsh8.5

DB_INCDIR="-I /usr/local/include/db46"
DB_LIB="db-4.6"

CC=cc
CCWFLAGS="-Wall -Wextra"
INSTALL=install
SUFFIXRULES=".c.o:"

# excude options to tar
EXCLUDE="-W exclude="

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
    SunOS)
	INCLUDE="include"
	Q=
	PREFIX=opt/local
	TCL_LIB="tcl8.3"
	INSTALL=ginstall
	SHELL="/bin/ksh"
	PATH="$PATH:/opt/sfw/bin"
	FIND=gfind
	;;
    Linux)
	# These defaults are for FC
	INCLUDE="include"
	Q=
	TCL_LIB="tcl8.5"
	TCL_INCDIR=
	DB_LIB="db-4.6"
	DB_INCDIR=
	TCLSH=/usr/bin/tclsh

	SUFFIXRULES="%.o: %.c"
	EXCLUDE="--exclude "

	# Override the defaults for flavor != fedoracore
    	if [ $flavor = opensuse ]
	then
	   DB_LIB="db-4.5"
	elif [ $flavor = centos ]
	then
	   TCL_LIB="tcl8.4"
	   DB_INCDIR="-I /usr/local/include/db46"
	   DB_LIB="db-4.6"
	elif [ $flavor = debian ]
	then
	   TCLSH=/usr/bin/tclsh8.5
	   TCL_INCDIR="-I /usr/include/tcl8.5"
	   TCL_LIB="tcl8.5"
	   DB_INCDIR="-I /usr/local/include/db46"
	   DB_LIB="db-4.6"
    	fi
	;;
    Darwin)
	INCLUDE="include"
	Q=
	TCL_LIB="tcl8.4"
	TCL_INCDIR=
	DB_LIB="db-4.6"
	DB_INCDIR="-I /usr/local/include/db46"
	TCLSH=/usr/bin/tclsh

	SUFFIXRULES="%.o: %.c"
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
      -e "/@TCL_INCDIR@/ s||$TCL_INCDIR|" \
      -e "/@TCL_LIB@/ s||$TCL_LIB|" \
      -e "/@TCLSH@/ s||$TCLSH|" \
      -e "/@DB_INCDIR@/ s||$DB_INCDIR|" \
      -e "/@DB_LIB@/ s||$DB_LIB|" \
      -e "/@CC@/ s||$CC|" \
      -e "/@CCWFLAGS@/ s||$CCWFLAGS|" \
      -e "/@SUFFIXRULES@/ s||$SUFFIXRULES|" \
      -e "/@EXCLUDE@/ s||$EXCLUDE|" \
      $makefile_in > $makefile_out
}
