#!/bin/sh
#
# nbsp name: kilm_fnus82-fwmilm.406176.txt
# npemwin name: fwmilmnc.406176.txt
#
# Test file: A_FNUS82KILM111849_C_KWIN_20200311184946_406176-3-FWMILMNC.TXT"
# Tiene:
#             FNUS82 KILM 111849
#             FWMILM
#
name="A_FNUS82KILM111849_C_KWIN_20200311184946_406176-3-FWMILMNC.TXT"

first_component=`echo $name | cut -d "_" -f 2 | tr A-Z a-z`
last_component=`echo $name | cut -d "_" -f 6 | tr A-Z a-z`

seqnum=`echo $last_component | cut -c 1-6`
emname=`echo $last_component | cut -c 10-17`
ext=`echo $last_component | cut -c 19-21`
emfbasename=${emname}.${seqnum}.${ext}

wmoid=`echo $first_component | cut -c 1-6`
wmostation=`echo $first_component | cut -c 7-10`
awips=`echo $emname | cut -c 1-6`
nbspfbasename=${wmostation}_${wmoid}-${awips}.${seqnum}.${ext}

echo $emfbasename
echo $nbspfbasename



