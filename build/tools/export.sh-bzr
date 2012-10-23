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

exclude="build dev-notes"
tmpdir=tmp

# read name and version
. ../../VERSION

rm -r -f $tmpdir
mkdir $tmpdir
cd $tmpdir

bzr export ${name}-${version} $site/$name/$tag
cd ${name}-${version}
rm -r $exclude

for p in $tcllibs
do
  bzr export $p $site/$p/$tag
done
bzr export tclhttpd $site/$tclhttpd/trunk

cd src
for p in $srclibs
do
  bzr export $p $site/$p/trunk
done

cd ../.. 
tar -czf ../${name}-${version}.tgz ${name}-${version}

cd ..
rm -r $tmpdir
