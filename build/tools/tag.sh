#!/bin/sh

. ../../VERSION

masterrepo="nbsprepo"
#
project=$name
tag=tags/${name}-${version}
#
#masterhost="http://svn.1-loop.net"
masterhost="svn+ssh://jfnieves@svn.1-loop.net/home/jfnieves/svn"
#
## mastersite="svn+ssh://diablo/home/svn"
mastersite=${masterhost}/${masterrepo}

# npemwintclhttpd receives special treatment
tcllibs="tclmetar"
srclibs="libconnth libqdb libtclconf"
tclhttpd=${project}tclhttpd

[ $# -ne 0 ] && tag=tags/$1

cd ../../../
svn copy $project $mastersite/$project/$tag

cd $project

for p in $tcllibs
do
  svn copy $p $mastersite/$p/$tag
done
svn copy tclhttpd $mastersite/$tclhttpd/$tag

cd src
for p in $srclibs
do
  svn copy $p $mastersite/$p/$tag
done
