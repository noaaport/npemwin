#!/bin/sh

subdirs="emftp npemwininsert"

for d in $subdirs
do
    cd $d
    ./configure.sh
    cd ..
done
