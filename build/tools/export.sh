#!/bin/sh

project=npemwin

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

svn export file:///home/svn/$project/trunk $project-$version
cd $project-$version
rm -r $exclude
for p in $tcllibs
do
  svn export file:///home/svn/$p/trunk $p
done
svn export file:///home/svn/$tclhttpd/trunk tclhttpd

cd src
for p in $srclibs
do
  svn export file:///home/svn/$p/trunk $p
done

cd ../.. 
tar -czf ../${name}-${version}.tgz ${name}-${version}

cd ..
rm -r $tmpdir
