#!/bin/sh

FULLPATH=`pwd`

sed -e "/@FULLPATH@/s##$FULLPATH#" Makefile.in > Makefile




