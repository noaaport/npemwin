#!/bin/sh

station="tjua"

url="ftp://tgftp.nws.noaa.gov/SL.us008001/DF.of/DC.radar"

for p in `cat list-all.txt`
do
    curl -o ${station}/$p.nids  ${url}/DS.${p}/SI.${station}/sn.last
done



