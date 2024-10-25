#!/usr/bin/tclsh

set url "https://tgftp.nws.noaa.gov/SL.us008001/CU.EMWIN/DF.xt/DC.gsatR/OPS/";
set file "txtmin02.zip";

set status [catch {
    exec curl -s -O -m 20 ${url}/${file}
} errmsg];

if {$status != 0} {
    puts $errmsg;
    return 1;
}

set md5result [exec openssl dgst -md5 $file];
foreach {dummy md5} [split $md5result " "] {};
set md5 [string trim $md5];

set oldmd5 "";
if [file exists "md5.txt"] {
    set oldmd5 [exec cat "md5.txt"];
}

if {$md5 eq $oldmd5} {
    puts "Not a new file";
    file delete $file;
    return 0;
}

set status [catch {
    exec unzip -d tmp $file;
} errmsg];

if {$status != 0} {
    puts $errmsg;
    return 1;
}

# set oname "A_SAUS70KWBC101800_C_KWIN_20200310180006_331524-2-SAHOURLY.TXT";
# set nname [string tolower [string range $oname end-20 end]];

foreach oname [glob -directory "tmp" -tails *] {
    set nname [string tolower [string range $oname end-20 end]];
    file rename [file join "tmp" $oname] [file join "data" $nname];
}

file rename -force $file "$file-[clock seconds]";

cd data;
set ziplist [glob -nocomplain *.zip];
if {[llength $ziplist] != 0} {
    foreach f $ziplist {
	exec unzip $f;
	file delete $f;
    }
}
cd ..;

exec echo $md5 > "md5.txt";
