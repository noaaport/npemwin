#!/bin/sh

subdirs="emftp radftp rad2ftp npemwininsert"

for d in $subdirs
do
    cd $d
    ./configure.sh
    cd ..
done
