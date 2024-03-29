#!/bin/sh

# The defaults are for debian-11
ID=debian
VERSION_ID="11"

if [ -f /etc/os-release ]
then
    . /etc/os-release
fi

[ ${ID} = "raspbian" ] && ID="debian"

cp control.${ID}-${VERSION_ID} control
