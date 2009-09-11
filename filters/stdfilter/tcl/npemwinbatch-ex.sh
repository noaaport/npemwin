#!/bin/sh

hostport="noaaport.uprrp.edu:8016"
basedir=/var/npemwin/spool

npemwinbatch update -b $basedir $hostport
echo "Executing postfilter"
npemwinbatch filter -b $basedir -p postfilter -a
