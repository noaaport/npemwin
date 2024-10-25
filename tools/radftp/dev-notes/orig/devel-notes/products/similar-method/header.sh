#!/bin/sh

#p="173u1"
#wmo=`head -n 1 ${p}.nids | cut -d " " -f 1`
#awips=`head -n 2 ${p}.nids | tail -n 1 | cut -b 1-3`
#echo "$awips,$wmo,$p"

for p in `cat product-list.txt`
do
    [ ! -f "nids/${p}.nids" ] && continue
    wmo=`head -n 1 nids/${p}.nids | cut -d " " -f 1`
    awips=`head -n 2 nids/${p}.nids | tail -n 1 | cut -b 1-3`
    echo "$awips,$wmo,$p"
done
