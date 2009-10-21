#!/bin/sh

project=npemwin
mastersite="svn+ssh://diablo/home/svn"

# nbsptclhttpd receives special treatment
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
