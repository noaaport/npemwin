#!/bin/sh

# The collective emwin groups are
#
#	emwin.misc.adm
#	emwin.img
#
# The individual station groups are created by add_station1.sh,
# which is called by add_stations.sh.

#base="npemwin"
base="emwin"

file=
if [ $# -eq 1 ]
then
    file=$1
fi

subgrouplist="misc.adm
img"

if [ -z "$file" ]
then
    cd
    for g in $subgrouplist
    do
      echo -n "${base}.$g: "
      bin/ctlinnd newgroup ${base}.$g y
    done
else
    for g in $subgrouplist
    do
      echo ${base}.$g >> $file
    done
fi
