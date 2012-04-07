#!/bin/sh

config_dirs="exporter iemwin"

for d in $config_dirs
do
    cd $d
    ./configure.sh
    cd ..
done
