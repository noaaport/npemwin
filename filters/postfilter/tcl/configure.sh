#!/bin/sh

. ./configure.inc

sed \
    -e "/@include@/ s||$INCLUDE|" \
    -e "/@q@/ s||$Q|g" \
    -e "/@INSTALL@/ s||$INSTALL|" \
    -e "/@TCLSH@/ s||$TCLSH|" \
    -e "/@PATH@/s||$PATH|" \
    Makefile.in > Makefile
