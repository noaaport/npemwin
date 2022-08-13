#!/bin/sh

. ./configure.inc

config_dirs="conf doc filters scripts src tools tclhttpd tclmetar/"

configure_default
configure_default Makefile.inc

savedir=`pwd`
for d in $config_dirs
do
    cd $d
    ./configure.sh
    cd $savedir
done
