#!/bin/sh

sitedir="/usr/local/etc/npemwin/site"

for f in \
    emftp.conf-debug \
    npemwind.conf \
    scheduler.conf \
    servers.conf
do
    install -m 0644 $f $sitedir
done
