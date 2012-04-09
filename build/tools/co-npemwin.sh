#!/bin/sh

project=npemwin
#
host="bzr+ssh://repo.1-loop.net"
#
repo="home/repo/bzr/noaaport"
tag=trunk
#
site=${host}/${repo}

# npemwintclhttpd receives special treatment
tcllibs="tclmetar"
srclibs="libconnth libqdb libtclconf"
tclhttpd=${project}tclhttpd

bzr branch $site/$project/$tag $project
cd $project
for p in $tcllibs
do
  bzr branch $site/$p/$tag $p
done
bzr branch $site/$tclhttpd/$tag tclhttpd

cd src
for p in $srclibs
do
  bzr branch $site/$p/$tag $p
done
