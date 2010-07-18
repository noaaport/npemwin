#!/bin/sh

. ../../VERSION

project=${name}
tag="tags/${name}-${version}"
#
masterhost="http://svn.1-loop.net"
masterrepo="nbsprepo"
#
## mastersite="svn+ssh://diablo/home/svn"
mastersite=${masterhost}/${masterrepo}

# npemwintclhttpd receives special treatment
tcllibs="tclmetar"
srclibs="libconnth libqdb libtclconf"
tclhttpd=${project}tclhttpd

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
