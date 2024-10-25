#!/bin/sh

rm -rf emftp
mkdir emftp
cd emftp
for d in inputs inv lock spool tmp queue
do
    mkdir $d
done
