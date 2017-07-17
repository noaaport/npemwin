#!/bin/sh

DPKG_COLORS="none"
export DPKG_COLORS

name=npemwin

cd $name/build/debian
./mk.sh
