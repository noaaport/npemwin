#!/bin/sh

sitedir="/usr/local/etc/npemwin/site"

for f in \
    radftp.conf \
    radftp.slist \
    radftp.plist
do
    install -m 0644 $f $sitedir
done

echo "Check ${sitedir}/scheduler.conf to enable radftp"
