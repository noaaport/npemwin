#!/bin/sh

glockdir="lock"
gsleepsecs=5

lock_dir(){

    lockdir=$1
    sleepsecs=$2
    
    mkdir $lockdir > /dev/null 2>&1
    status=$?
    while [ $status -ne 0 ]
    do
	echo sleeping
	sleep $sleepsecs
	mkdir $lockdir > /dev/null 2>&1
	status=$?
    done
}

unlock_dir(){

    lockdir=$1
    
    [ -d $lockdir ] && rmdir $lockdir
}

lock_dir $glockdir $gsleepsecs

echo starting
sleep 10
echo end

unlock_dir $glockdir

