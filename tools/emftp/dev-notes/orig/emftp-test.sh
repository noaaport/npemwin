#!/bin/sh
#
# The original names are of the form
#
# A_SAUS70KWBC101800_C_KWIN_20200310180006_331524-2-SAHOURLY.TXT";
#
# and they are converted to
#
# sahourly.331524.txt";
#

#
# configuration
#
url="https://tgftp.nws.noaa.gov/SL.us008001/CU.EMWIN/DF.xt/DC.gsatR/OPS/"
file="txtmin02.zip"
#
# options
#
gworkdir="."
datadir="data"
tmpdir="tmp"
md5keyfile="md5.txt"

#
# main
#
curl -s -O -m 20 ${url}/${file}
[ $? -ne 0 ] && { echo "Error from curl"; exit 1; }

oldmd5key=
md5key=`openssl md5 $file | cut -f 2 -d ' '` 
[ -f "md5.txt" ] && { oldmd5key=`cat "md5.txt"`; }

if [ $md5key = "$oldmd5key" ]
then
    echo "Not a new file"
    rm $file
    return 1;
fi

unzip -d tmp $file > /dev/null 2>&1
[ $? -ne 0 ] && { echo "Error from unzip"; exit 1; }

cd tmp
for _f in *.ZIP
do
    unzip ${_f} > /dev/null 2>&1
    rm ${_f}
done
cd ..

[ ! -d "data" ] && mkdir data

for f in tmp/*
do
    name=`basename $f`
    last_component=`echo $name | cut -d "_" -f 6 | tr A-Z a-z`
    seqnum=`echo $last_component | cut -c 1-6`
    name=`echo $last_component | cut -c 10-17`
    ext=`echo $last_component | cut -c 19-21`
    fbasename=${name}.${seqnum}.${ext}
    mv $f data/$fbasename
done

echo $md5key > "md5.txt"
mv $file ${file}-`date "+%s"`
