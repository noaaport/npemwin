#!/bin/sh

. configure.inc

sed -e "/@include@/s||$INCLUDE|" \
    -e "/@q@/s||$Q|g" \
    -e "/@INSTALL@/s||$INSTALL|" \
    -e "/@PATH@/s||$PATH|" \
    -e "/@FIND@/s||$FIND|" \
    -e "/@UNZIP@/s||$UNZIP|" \
    -e "/@SHELL@/s||$SHELL|" \
    -e "/@TCLSH@/s||$TCLSH|" \
    -e "/@RCINIT@/s||$RCINIT|" \
    -e "/@RCFPATH@/s||$RCFPATH|" \
    -e "/@RCCONF@/s||$RCCONF|" \
    -e "/@HOURLYCONF@/s||$HOURLYCONF|" \
    -e "/@STARTCLEANCONF@/s||$STARTCLEANCONF|" \
    -e "/@STARTSTOPRC@/s||$STARTSTOPRC|" \
    Makefile.in > Makefile
