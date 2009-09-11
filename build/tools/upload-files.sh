#!/bin/sh

. upload.conf

cp ../RELEASE_NOTES ../conf/*.README .
upload_files="RELEASE_NOTES *.README"

readme_file=RELEASE_NOTES
dt=`date +%d%b%G`

ftp -n -v $uploadhost <<EOF
user $uploaduser
prompt
mkdir $uploaddir
cd $uploaddir
mkdir Docs
cd Docs
mdelete *
mput $upload_files
rename $readme_file $readme_file-$dt
quit
EOF

rm $upload_files
