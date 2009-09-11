#!/bin/sh

subdirs="postfilter
nntpfilter
rssfilter
stdfilter
npemwinfilter
prefilter
panfilter
metarfilter
gpfilter
dafilter
ldmfilter
spoolfilter
msgfilter
trackfilter
bbserver
scheduler"

for d in $subdirs
do
    cd $d
    ./configure.sh
    cd ..
done
