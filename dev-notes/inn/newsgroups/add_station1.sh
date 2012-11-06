#!/bin/sh

# The station groups are
#
#	emwin.txt.kkkk
#	emwin.{urgent,warnings,data,forecast,summary}.kkkk

#base="npemwin"
base="emwin"

if [ $# -eq 1 ]
then
    station=$1
    file=
elif [ $# -eq 2 ]
then
    station=$1
    file=$2
else
    echo "One station as argument."
    exit 1
fi

subgrouplist="txt
urgent
warnings
data
forecast
summary"

if [ -z "$file" ]
then
    cd
    for g in $subgrouplist
    do
      echo -n "${base}.$g.$station: "
      bin/ctlinnd newgroup ${base}.$g.$station y
    done
else
    for g in $subgrouplist
    do
      echo ${base}.$g.$station >> $file
    done
fi
