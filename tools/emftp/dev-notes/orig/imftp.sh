#!/bin/sh
#
# usage: emftp [-c] [-f userconffile]
#
# The [-c] option will create the working directories used by emftp,
# as defined in the configuration file, and exit.
#        
# The original names are of the form
#
# Z_QAEA00TJSJ041130_C_KWIN_20220804233035_529673-3-RADALLPR.GIF
# Z_EIPU00KWBC041130_C_KWIN_20220804233034_529667-4-IMGSJUPR.JPG
# Z_QGTA98KWNS041130_C_KWIN_20220804233034_529668-4-IMGWWAUS.PNG
#
# They are renamed as
#
# radallpr.20220804.233035.gif
# imgsjupr.20220804.233034.jpg
# imgwwaus.20220804.233034.png

#
# functions
#

log_msg(){

    if [ $gbackground -eq 1 ]
    then
	logger -t emftp "$1"
    else
	echo "$1"
    fi
}

log_msg_quit(){

    log_msg "$1"
    exit 0
}

log_err(){

    log_msg "$1"
    exit 1
}

cleanup(){

    log_msg "emftp script finished"
    rm -f $glockfpath
}

get_parts(){

    _name=`echo $1 | tr A-Z a-z`

    _first_component=`echo $_name | cut -d "_" -f 2`
    _second_component=`echo $_name | cut -d "_" -f 5`
    _last_component=`echo $_name | cut -d "_" -f 6`

    rc_seqnum=`echo $_last_component | cut -c 1-6`
    rc_emname=`echo $_last_component | cut -c 10-17`
    rc_ext=`echo $_last_component | cut -c 19-21`
    rc_awips=`echo $rc_emname | cut -c 1-6`
    
    rc_wmoid=`echo $_first_component | cut -c 1-6`
    rc_wmostation=`echo $_first_component | cut -c 7-10`
    rc_wmotime=`echo $_first_component | cut -c 11-16`

    rc_yyyymmdd=`echo $_second_component | cut -c 1-8`
    rc_hhmmss=`echo $_second_component | cut -c 9-14`

    if [ $rc_ext == "txt" ]
    then
	rc_ftype="txt"
    else
	rc_ftype="img"
    fi
}

get_ftype(){

    echo $rc_ftype
}
    
get_parentsubdir(){

    # must be used after calling get_parts()

    # CHANGED IMG
    echo ${rc_ftype}/${rc_wmostation} 
}

get_emfbasename(){
    
    # must be used after calling get_parts()

    # CHANGED IMG
    if [ ${rc_ftype} == "txt" ]
    then
	echo ${rc_emname}.${rc_seqnum}.${rc_ext}
    else
	echo ${rc_emname}.${rc_yyyymmdd}.${rc_hhmmss}.${rc_ext}
    fi
}

get_nbspfbasename(){

    # must be used after calling get_parts
 
    _nbspfrootname=${rc_wmostation}_${rc_wmoid}-${rc_awips}.${rc_seqnum}
    echo ${_nbspfrootname}.${rc_ext}
}

get_nbspfname(){

    # must be used after calling get_parts
 
    _nbspfname=${rc_wmostation}_${rc_wmoid}-${rc_awips}
    echo ${_nbspfname}
}

sanity_check(){

    #
    #[ ! -d $out_dirname ] && \
    #	{ logger -t qrunner "$out_dirname not found. Exiting."; exit 1; }
    #

    # checks
    for _d in inputs spool inv lock queue tmp
    do
	eval _dir=\$g${_d}dir
	[ ! -d ${_dir} ] && { log_err "${_dir} does not exist"; }
    done
}

#
# configuration file
#
gconffile="emftp.conf"
gconfdir="/usr/local/etc/emftp"
gconfoptdirs="${gconfdir}/defaults ${gconfdir}/site"

#
# input configuration
#
gurl="https://tgftp.nws.noaa.gov/SL.us008001/CU.EMWIN/DF.xt/DC.gsatR/OPS"
ginputfilename_txt="txtmin02.zip"
ginputfilename_img="imgmin15.zip"  # CHANGED IMG

#
# options
#

#
gbackground=0
gremove_emfpath=0
gwrite_inv=1
gwrite_queue=1
#
## gworkdir="/var/noaaport/emftp"
gworkdir="emftp"
ginputssubdir="inputs"
gspoolsubdir="spool"
ginvsubdir="inv"
glocksubdir="lock"
gqueuesubdir="queue"
gtmpsubdir="tmp"
gmd5keyfile="md5.txt"
ginvfilename="`date -u +%H`.txt"
glockfilename="emftp.lock"
gqueuefilename="`date -u +%H%M`.q"
gtime="`date -u +%H%M`"           # for entry in inv file

#
# Nbsp and Npemwin processing
#
npemwin_enable=0
gnpemwin_postfilter="/usr/local/libexec/npemwin/postfilter"
gnpemwin_filesdir="/var/npemwin/files"
gnpemwin_qdir="/var/npemwin/spool/lock"
gnpemwin_queuefilename="postfilter.q"
#
nbsp_enable=0
gnbsp_spooldir="/var/noaaport/nbsp/spool"
gnbsp_infifo="/var/run/nbsp/infeed.fifo"

#
# main
#

option_c=0
option_f=0
option_i=0
while getopts ":cf:i" option
do
    case $option in
	c) option_c=1;;
        f) userconffile=$OPTARG; option_f=1;;
	i) option_i=1;;
        \?) echo "Unsupported option $OPTARG"; exit 1;;
	:) echo "Missing value option $OPTARG"; exit 1;;
    esac
done
shift $((OPTIND-1))

if [ $option_f -eq 1 ]
then
    . $userconffile
else
    [ -f ${gconfdir}/${gconffile} ] && . ${gconfdir}/${gconffile}
fi

if [ $option_c -eq 1 ]
then
    rm -rf ${gworkdir}
    mkdir ${gworkdir}
    cd ${gworkdir}

    for d in $ginputssubdir $ginvsubdir $glocksubdir $gspoolsubdir \
	     $gtmpsubdir $gqueuesubdir
    do
	mkdir $d
    done

    return
fi

if [ $option_i -eq 1 ]
then
    ginputfilename=${ginputfilename_img}
else
    ginputfilename=${ginputfilename_txt} # default
fi

# variables
ginputsdir=${gworkdir}/${ginputssubdir}
gspooldir=${gworkdir}/${gspoolsubdir}
ginvdir=${gworkdir}/${ginvsubdir}
glockdir=${gworkdir}/${glocksubdir}
gqueuedir=${gworkdir}/${gqueuesubdir}
gtmpdir=${gworkdir}/${gtmpsubdir}
#
ginputfpath=${ginputsdir}/${ginputfilename}
gmd5keyfpath=${ginputsdir}/${gmd5keyfile}
ginvfpath=${ginvdir}/${ginvfilename}
glockfpath=${glockdir}/${glockfilename}
gqueuefpath=${gqueuedir}/${gqueuefilename}
#

gnpemwin_queuefpath=${gnpemwin_qdir}/${gnpemwin_queuefilename}

# to ensure that other callers to nbspinsert can write to the directories
# (e.g., directories within the nbsp/spool) that this script may create
umask 002

# checks
sanity_check

# run once
[ -f $glockfpath ] && { log_msg_quit "queue is locked"; }

trap cleanup HUP INT QUIT ABRT KILL ALRM TERM EXIT
touch $glockfpath

log_msg "emftp script starting"

curl -s -o ${ginputfpath} -m 20 ${gurl}/${ginputfilename}
[ $? -ne 0 ] && { log_err "Error from curl"; }

oldmd5key=
[ -f $gmd5keyfpath ] && { oldmd5key=`cat $gmd5keyfpath`; }
# [ -f $ginvfpath ] && { oldmd5key=`tail -n 1 ${ginvfpath} | cut -f 2 -d ' '`; }
md5key=`openssl md5 ${ginputfpath} | cut -f 2 -d ' '` 

if [ $md5key = "$oldmd5key" ]
then
    rm $ginputfpath
    log_msg_quit "Not a new file"
fi

unzip -d $gtmpdir $ginputfpath > /dev/null 2>&1
[ $? -ne 0 ] && { log_err "Error from unzip"; }

_savedir=`pwd`
cd $gtmpdir
_zipfilelist=`find . -name "*.ZIP"`
if [ -n "${_zipfilelist}" ]
then
    for _f in ${_zipfilelist}
    do
	#[ "${_f}" = "" ] && break
	unzip ${_f} > /dev/null 2>&1
	[ $? -ne 0 ] && { log_err "Error from unzip"; }
	rm ${_f}
    done
fi
cd ${_savedir}

for f in $gtmpdir/*
do
    # Define the rc_xxx variables
    get_parts `basename $f`

    # These functions need the rc_xxx variables
    emfbasename=`get_emfbasename`
    parentsubdir=`get_parentsubdir`
    ftype=`get_ftype`

    parentdir=${gspooldir}/${parentsubdir}
    emfpath=${parentdir}/${emfbasename}

    mkdir -p $parentdir
    mv $f $emfpath
    
    [ $gwrite_queue -eq 1 ] && { echo $rc_emname $emfpath >> $gqueuefpath; }

    #
    # Process npemwin
    #
    if [ $npemwin_enable -eq 1 -a $ftype == "txt" ]
    then
	npemwin_parentdir=${gnpemwin_filesdir}/${parentsubdir}
	npemwin_fpath=${npemwin_parentdir}/${emfbasename}
	mkdir -p $npemwin_parentdir
	cp $emfpath $npemwin_fpath
	[ -f $npemwin_fpath ] && \
	    echo $rc_emname $npemwin_fpath >> $gnpemwin_queuefpath
    fi

    if [ $nbsp_enable -eq 1 -a $ftype == "txt" ]
    then
	nbspfbasename=`get_nbspfbasename`
	nbspfname=`get_nbspfname`
	nbspparentdir=${gnbsp_spooldir}/${rc_wmostation}
	nbspfpath=${nbspparentdir}/${nbspfbasename}
	_finfo="$rc_seqnum 0 0 0 0 $nbspfname $nbspfpath"
	mkdir -p $nbspparentdir
	echo -n "000000000000000000000000" > $nbspfpath
	cat $emfpath >> $nbspfpath
	[ -f $nbspfpath ] && nbspinsert -f $gnbsp_infifo $_finfo
    fi

    #
    # Remove it here, or setup a schedule to remove them periodically.
    # In the default configuration they are under
    # /var/noaaport/emftp/emftp/spool/txt/<station>
    #
    if [ $gremove_emfpath -eq 1 -a $ftype == "txt" ]
    then
	rm -f $emfpath
    fi
done

echo $md5key > ${gmd5keyfpath}
[ $gwrite_inv -eq 1 ] && { echo "$gtime $md5key" >> $ginvfpath; }

rm $ginputfpath

# Execute the npemwin postfilter
# CHANGED img
if [ $npemwin_enable -eq 1 -a -f $gnpemwin_queuefpath -a $ftype == "txt" ]
then
    $gnpemwin_postfilter < $gnpemwin_queuefpath
    rm $gnpemwin_queuefpath
fi
