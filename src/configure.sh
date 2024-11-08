#!/bin/sh

. ./configure.inc

config_subdirs="libqdb libconnth libtclconf"

sed -e "/@include@/s||$INCLUDE|" \
    -e "/@q@/s||$Q|g" \
    -e "/@INSTALL@/s||$INSTALL|" \
    -e "/@INCDIRS@/s||$INCDIRS|" \
    -e "/@LIBDIRS@/s||$LIBDIRS|" \
    -e "/@LIBS@/s||$LIBS|" \
    -e "/@CC@/s||$CC|" \
    -e "/@CCWFLAGS@/s||$CCWFLAGS|" \
    -e "/@SUFFIXRULES@/ s||$SUFFIXRULES|" \
    Makefile.in > Makefile

# copy the config.h file appropriate for the OS
cd config.h.d
./configure.sh
cd ..

# the lib dirs
for d in $config_subdirs
do
  cd $d
  ./configure.sh
  cd ..
done
