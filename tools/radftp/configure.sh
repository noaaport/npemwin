#!/bin/sh

. ./configure.inc

sed -e "/@include@/s||$INCLUDE|" \
    -e "/@q@/s||$Q|g" \
    -e "/@INSTALL@/s||$INSTALL|" \
    -e "/@SHELL@/s||$SHELL|" \
    -e "/@GET_DATETIME@/s||$GET_DATETIME|" \
    Makefile.in > Makefile
