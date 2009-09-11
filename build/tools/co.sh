#!/bin/sh

project=npemwin

# nbsptclhttpd receives special treatment
tcllibs="tclmetar"
srclibs="libconnth libqdb libtclconf"
tclhttpd=${project}tclhttpd

svn co file:///home/svn/$project/trunk $project
cd $project
for p in $tcllibs
do
  svn co file:///home/svn/$p/trunk $p
done
svn co file:///home/svn/$tclhttpd/trunk tclhttpd

cd src
for p in $srclibs
do
  svn co file:///home/svn/$p/trunk $p
done
