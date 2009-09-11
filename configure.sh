#!/bin/sh

. configure.inc

config_dirs="conf doc filters scripts src tclhttpd tclmetar/dc/npemwin"

configure_default
configure_default Makefile.inc

savedir=`pwd`
for d in $config_dirs
do
    cd $d
    ./configure.sh
    cd $savedir
done
