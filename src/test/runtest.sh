#!/bin/sh

testdir=`pwd`

./emwind -f $testdir/servers.conf -d $testdir/spool -r $testdir/qrunner.sh \
    -s $testdir/spool/status -P 49152

 
