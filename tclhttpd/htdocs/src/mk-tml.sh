#!/bin/sh

include_dir="include"
templates_dir="templates"

main_page="main.tml.in"

index_pages="index"

status_pages="connections \
    printconf \
    received_by_station \
    received_by_station_rss \
    received_last_24hours \
    received_last_hour \
    received_last_minute \
    received_minute \
    received_past_hour \
    serverlist \
    station_catalog_txt \
    stats"

metar_pages=

#
# main
#
[ $# -ne 1 ] && { echo "Needs build dir"; exit 1; }
build_dir=$1

index_dir=${build_dir}
status_dir=${build_dir}/npemwin/status
metar_dir=${build_dir}/npemwin/metar

rm -rf ${build_dir}
mkdir -p ${index_dir} ${status_dir} ${metar_dir}

for p in $status_pages
do
    sed -e "/@header@/r ${include_dir}/header.html" \
	-e "/@header@/d" \
	-e "/@topmenu@/r ${include_dir}/topmenu.html" \
	-e "/@topmenu@/d" \
	-e "/@leftmenu@/r ${include_dir}/leftmenu.html" \
	-e "/@leftmenu@/d" \
	-e "/@body@/r ${p}.tml.body" \
	-e "/@body@/d" \
	${templates_dir}/status/main.tml.in > ${status_dir}/${p}.tml
done

for p in $metar_pages
do
    sed -e "/@header@/r ${include_dir}/header.html" \
	-e "/@header@/d" \
	-e "/@topmenu@/r ${include_dir}/topmenu.html" \
	-e "/@topmenu@/d" \
	-e "/@leftmenu@/r ${include_dir}/leftmenu.html" \
	-e "/@leftmenu@/d" \
	-e "/@body@/r ${p}.tml.body" \
	-e "/@body@/d" \
	${templates_dir}/metar/main.tml.in > ${metar_dir}/${p}.tml
done

for p in $index_pages
do
    sed -e "/@header@/r ${include_dir}/header.html" \
	-e "/@header@/d" \
	-e "/@topmenu@/r ${include_dir}/topmenu.html" \
	-e "/@topmenu@/d" \
	-e "/@leftmenu@/r ${include_dir}/leftmenu.html" \
	-e "/@leftmenu@/d" \
	-e "/@body@/r ${p}.tml.body" \
	-e "/@body@/d" \
	${templates_dir}/index/main.tml.in > ${index_dir}/${p}.tml
done
