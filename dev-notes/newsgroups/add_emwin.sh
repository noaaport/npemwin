#!/bin/sh

# The collective emwin groups are
#
#	emwin.{urgent,warnings,data,forecast,summary}
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

subgrouplist="urgent
warnings
data
forecast
summary"

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
