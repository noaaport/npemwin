#!/bin/sh

branchname=npemwin

#tgzfile=${branchname}.tgz
#rm -rf $branchname
#tar -xzf $tgzfile

cd ${branchname}/build/rpm
make clean
make package
