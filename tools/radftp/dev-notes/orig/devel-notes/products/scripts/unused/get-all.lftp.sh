#!/bin/sh

rm -f lftp.rc

url="ftp://tgftp.nws.noaa.gov"
station="tjua"

echo "open $url" > lftp.rc

for p in `cat list-all.txt`
do
    path="SL.us008001/DF.of/DC.radar/DS.${p}/SI.${station}/sn.last"
    echo "get $path -o ${station}/${p}.nids" >> lftp.rc
done

echo "exit" >> lftp.rc

lftp -f lftp.rc
rm lftp.rc
