#!/bin/sh
#
# $Id$
#

tmpdir=tmp
exclude="tools dev-notes bsd rpm macosx debian solaris"
libs="libconnth libqdb libtclconf"

# read name and version
. ../VERSION

rm -r -f $tmpdir
mkdir $tmpdir

cd $tmpdir
cvs export -D now -d ${name}-${version} ${name}

cd ${name}-${version}
rm -r $exclude
cvs export -D now -d tclhttpd npemwintclhttpd
#cvs export -D now tclmetarlib
cvs -d :ext:nieves@tclmetar.cvs.sourceforge.net:/cvsroot/tclmetar \
    export -D now tclmetar

cd src
for l in $libs
do
  cvs export -D now ${l}
done

cd ../.. 
tar -czf ../${name}-${version}.tgz ${name}-${version}

cd ..
rm -r $tmpdir
