#!/bin/sh

project=npemwin
masterhost="http://svn.1-loop.net"
masterrepo="nbsprepo"
#
## mastersite="svn+ssh://diablo/home/svn"
mastersite=${masterhost}/${masterrepo}

# npemwintclhttpd receives special treatment
tcllibs="tclmetar"
srclibs="libconnth libqdb libtclconf"
tclhttpd=${project}tclhttpd

svn co $mastersite/$project/trunk $project
cd $project
for p in $tcllibs
do
  svn co $mastersite/$p/trunk $p
done
svn co $mastersite/$tclhttpd/trunk tclhttpd

cd src
for p in $srclibs
do
  svn co $mastersite/$p/trunk $p
done
