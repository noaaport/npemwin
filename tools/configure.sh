#!/bin/sh

subdirs="npemwininsert emftp radftp rad2ftp satftp"

for d in $subdirs
do
    cd $d
    ./configure.sh
    cd ..
done
