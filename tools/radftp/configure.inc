#
# $Id$
#

# defaults (FreeBSD)
GET_DATETIME="radftp_get_datetime.bsd"

. ../../configure.inc

case $os in
    *BSD) 
	;;
    Linux)
	GET_DATETIME="radftp_get_datetime.linux"
    	;;
esac
