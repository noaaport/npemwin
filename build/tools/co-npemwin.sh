#!/bin/sh

project=npemwin
#
#masterhost="http://svn.1-loop.net"
masterhost="svn+ssh://jfnieves@svn.1-loop.net/home/jfnieves/svn"
#
masterrepo="nbsprepo"
tag=trunk
#
## mastersite="svn+ssh://diablo/home/svn"
mastersite=${masterhost}/${masterrepo}

# npemwintclhttpd receives special treatment
tcllibs="tclmetar"
srclibs="libconnth libqdb libtclconf"
tclhttpd=${project}tclhttpd

# Override tag with the cmd line argument (e.g., "nbsp-2.1.1r")
[ $# -ne 0 ] && tag=tags/$1

svn co $mastersite/$project/$tag $project
cd $project
for p in $tcllibs
do
  svn co $mastersite/$p/$tag $p
done
svn co $mastersite/$tclhttpd/$tag tclhttpd

cd src
for p in $srclibs
do
  svn co $mastersite/$p/$tag $p
done
