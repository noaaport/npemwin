#!/bin/sh

. ./configure.inc

sed -e "/@include@/s||$INCLUDE|" \
    -e "/@q@/s||$Q|g" \
    -e "/@INSTALL@/s||$INSTALL|" \
    -e "/@SHELL@/s||$SHELL|" \
    Makefile.in > Makefile
