#!/bin/sh

subdirs="emftp radftp npemwininsert"

for d in $subdirs
do
    cd $d
    ./configure.sh
    cd ..
done
