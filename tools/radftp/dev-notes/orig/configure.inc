#
# $Id$
#

# defaults (FreeBSD)
GET_DATETIME="get_datetime.bsd"

os=`uname`
case $os in
    *BSD) 
	;;
    Linux)
	GET_DATETIME="get_datetime.linux"
    	;;
esac

configure_default () {

  makefile_out=Makefile
  [ $# -eq 1 ] && makefile_out=$1
  makefile_in=${makefile_out}.in

  sed \
      -e "/@GET_DATETIME@/s||$GET_DATETIME|" \
      $makefile_in > $makefile_out
}
