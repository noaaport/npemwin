#!/bin/sh
#
# print the wmoid and awips from each file
#

for key in `cat "list.txt"`
do
    file="${key}.nids"
 
    echo -n "$key "
    if [ -f tjua/$file ]
    then	
	awk -f extract.awk tjua/$file
    else
	echo ""
    fi
done
