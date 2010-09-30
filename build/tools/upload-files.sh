#!/bin/sh
#
# $Id$
#
. ./upload.conf

cp ../../RELEASE_NOTES ../../UPGRADING ../../READMEs/*.README .
upload_files="RELEASE_NOTES UPGRADING *.README"

release_file=RELEASE_NOTES
dt=`date +%d%b%G`

lftp -c "\
$lftpoptions;
open -u $uploaduser $uploadhost;
cd $uploadbasedir;
mkdir -p $filesdir;
cd $filesdir;
mrm *
mput $upload_files
mv $release_file $release_file-$dt
quit"

rm $upload_files
