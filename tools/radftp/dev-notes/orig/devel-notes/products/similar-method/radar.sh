#!/bin/sh

# product="p94r0"
#ftp://tgftp.nws.noaa.gov:/SL.us008001/DF.of/DC.radar/DS.${product}/SI.tjua/sn.last

#p="p94r0"
#curl -o ${p}.nids "ftp://tgftp.nws.noaa.gov:/SL.us008001/DF.of/DC.radar/DS.${p}/SI.tjua/sn.last"


for p in `cat product-list.txt`
do
    url="ftp://tgftp.nws.noaa.gov:/SL.us008001/DF.of/DC.radar/DS.${p}/SI.tjua/sn.last"
    curl -o ${p}.nids $url
done
