#!/bin/sh

# defaults
url="ftp://tgftp.nws.noaa.gov"
station="tjua"

#
# main
#
[ $# -ne 0 ] && { station=$1; }

rm -rf lftp.rc
mkdir -p $station

echo "open $url" > lftp.rc

while read p wmo awips
do
    mkdir -p ${station}/${awips}
    path="SL.us008001/DF.of/DC.radar/DS.${p}/SI.${station}/sn.last"
    echo "get $path -o ${station}/${awips}/last.nids" >> lftp.rc
done <  "list.txt"

echo "exit" >> lftp.rc

lftp -f lftp.rc
rm lftp.rc

# rename
station3=${station#?}		# remove first letter from station name

while read p wmo awips
do
    f="${station}/${awips}/last.nids"
    seconds=`nbspradinfo -t ${f}`
    if [ $? -eq 0 ]
    then
	#datetime=`date -d "@${seconds}" -u "+%Y%m%d_%H%M"`	# linux
	datetime=`date -r ${seconds} -u "+%Y%m%d_%H%M"`		# bsd
	newname="${awips}${station3}_${datetime}.nids"
	mv ${f} ${station}/${awips}/${newname}
    else
	rm $f
    fi   
done <  "list.txt"
