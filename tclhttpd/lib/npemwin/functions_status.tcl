#
# $Id$
#

# Read the station definitions. This loads the arrays rssstation(), rssstate()
# in the global context, but they are used only by the functions in this file.
#
if {[file exists $Config(npemwinrsswfodef)]} {
    source $Config(npemwinrsswfodef);
}

proc proc_secstohm {secs} {

    return [clock format $secs -gmt true -format "%H:%M"]
}

proc proc_secstohms {secs} {

    return [clock format $secs -gmt true -format "%H:%M:%S"]
}

proc proc_fmtrow {n} {

    set fmt "<tr>"
    set i 1
    while {$i <= $n} {
	append fmt "<td>%s</td>"
	incr i
    }
    append fmt "</tr>\n"
    
    return $fmt
}

proc proc_stringtoarray {str} {

    set strlist [split $str]
    set n [llength $strlist]
    set i 0
    while {$i < $n} {
	set j [expr $i + 1]
	set a($j) [lindex $strlist $i]
	set i $j
    }
    set a(0) $str

    return [array get a]
}

proc file_hasdata {file} {

    if {[file exists $file] && ([file size $file] != 0)} {
	return 1;
    }

    return 0;
}

proc file_isfrom_date {file date} {
    #
    # date is in the format ddmmyyyy
    #
    if {[file exists $file]} {
	set fdate [clock format [file mtime $file] -format "%d%m%Y" -gmt true];
	if {$fdate eq $date} {
	    return 1;
	}
    }

    return 0;
}

proc get_date {seconds} {

    set r [clock format $seconds -format "%d%m%Y" -gmt true]

    return $r;
}
   
proc get_date_today {now} {
    #
    # The format of the return value is ddmmyyyy
    #
    set today [get_date $now]

    return $today;
}

proc get_date_yesterday {now} {
    #
    # The format of the return value is ddmmyyyy
    #
    incr now -86400

    set yesterday [get_date $now]

    return $yesterday;
}

proc npemwin_status {file} {

    set fmt [proc_fmtrow 6]

    set result "<h3>Ten Minute Summary for frames and bytes</h3>\n";
    append result "<table border>\n";
    append result [format $fmt "time" "frames<br>received" \
		       "frames<br>processed" "frames<br>errors" \
		       "bytes<br>received" "bytes/s"]

    set f [open "|tail $file" r]
    while {[gets $f finfo] > 0} {

	array set a [proc_stringtoarray $finfo]
	set hm [proc_secstohm $a(1)]
	set bps [expr int($a(5)/60)]

	append result [format $fmt $hm $a(2) $a(3) $a(4) $a(5) $bps]
    }
    close $f

    append result "</table>\n"

    return $result
}

proc npemwin_connections {file} {

    set fmt [proc_fmtrow 4]

    set table "<table border>\n";
    append table [format $fmt "ip" "type" "queue" "time"]

    set numconn 0;
    set f [open $file r]
    while {[gets $f finfo] > 0} {

	if {[regexp {^(-|\s*$)} $finfo]} {
	    continue;
	}

	array set a [proc_stringtoarray [string trimleft $finfo]]
	set time [clock format $a(4) -format "%Y%m%d %H:%M"]
	append table [format $fmt $a(1) $a(2) $a(3) $time]

	incr numconn
    }
    close $f

    append table "</table>\n"

    set result "<h3>Active connections - $numconn</h3>\n";
    if {$numconn != 0} {
        append result $table;
    }
    return $result
}

proc npemwin_serverlist {file} {

    set fmt [proc_fmtrow 2]

    set result "<h3>Active servers</h3>\n";
    append result "<table border>\n";
    append result [format $fmt "ip" "port"]

    set f [open $file r]
    while {[gets $f finfo] > 0} {
	array set a [proc_stringtoarray $finfo]
	append result [format $fmt $a(1) $a(2)]
    }
    close $f

    append result "</table>\n"

    return $result
}

proc display_config {} {

    set fmt [proc_fmtrow 2];

    append result "<table border>\n";

    append result [format $fmt "Package version" [exec npemwinversion]];
    foreach line [split [exec npemwind -C 2> /dev/null] "\n"] {
	set parts [split $line];
        set p [lindex $parts 0];
        set v [join [lreplace $parts 0 0] " "];
        append result [format $fmt $p $v];
    }

    append result "</table>\n";
}

proc npemwin_received {file} {

    set fmt [proc_fmtrow 6]

    set hhmm [file rootname [file tail $file]]

    set result "<h3>Products received at $hhmm</h3>\n";
    append result "<table border>\n";
    append result [format $fmt "time" "name" "wmoid" "wfo"  "wmotime" "path"]

    set f [open $file r]
    while {[gets $f finfo] > 0} {
	array set a [proc_stringtoarray $finfo]
	set hms [proc_secstohms $a(1)]
	#
	# The path will be output with a link to the file in the /spool
	# domain handler. The path is a(6) and the station is (a(4).
	#
	set fname [file tail $a(6)]
	set ftype [string range [file extension $fname] 1 end]
	if {$ftype eq "txt"} {
	    set path "<a href=\"/_get/spool/$a(4)/$fname\">$a(6)</a>";
	} else {
	    set path "<a href=\"/_get/spool/$fname\">$a(6)</a>";
	}
	append result [format $fmt $hms $a(2) $a(3) $a(4) $a(5) $path]
    }
    close $f

    append result "</table>\n"

    return $result
}

proc npemwin_received_hour {received_minute_tml ddmmyyyy hh {mm 60}} {
#
# Files received at the given hour on the given day.
# The mm argument, if given, determines the maximum minute to include. It is
# used when the function is called for the current hour. In that
# case it is set to the current minute. We could exclude that file from
# the list returned since that minute has not been completed,
# or return the list as "it is" at the moment of call, understanding
# that it is incomplete for the current minute.
#
    global Config;

    set fulllist [lsort [glob -tails -nocomplain \
			     -directory $Config(npemwininvdir) \
			     ${hh}*$Config(npemwininvfext)]];
    set today [clock format [clock seconds] -format "%D" -gmt true];
    
    if {$mm == 60} {
	set flist $fulllist;
    } else {
	set flist [list];
	set max_hhmm ${hh}${mm};
	foreach file $fulllist {
	    set hhmm [file rootname $file];
	    if {$hhmm > $max_hhmm} {
		break;
	    }
	    lappend flist $file;
	}
    }

    # Include only the files from the given day (ddmmyyyy)
    set fulllist $flist;
    set flist [list];
    foreach file $fulllist {
	set fpath [file join $Config(npemwininvdir) $file];
	if {[file_isfrom_date $fpath $ddmmyyyy] == 1} {
	    lappend flist $file;
	}
    }
	       
    if {[llength $flist] == 0} {
	set result "<h5>No products received at $hh.</h5>\n";
	return $result;
    }

    set result "<h3>Products received at $hh</h3>\n"; 
    foreach file $flist {
	set hhmm [file rootname $file];
	# set href "<a href=\"/npemwin/status/received_minute.tml?hhmm=$hhmm\">$hhmm</a>\n";
	set href \
	    "<a href=\"${received_minute_tml}?hhmm=${hhmm}\">${hhmm}</a>\n";
	append result $href;
    }

    return $result;
}

proc npemwin_received_by_station {station ftype basedir} {

    global rssstation;

    set dir [file join $basedir $ftype $station];

    set result "<h1>\n";
    append result \
	"<a href=\"/npemwin/status/received_by_station_rss.tml?station=$station\">";
    append result \
	"<img src=\"/images/16px-Feed-icon.svg.png\" border=0></a>\n";
    append result [string toupper $station] "\n</h1>";
    if {[info exists rssstation($station)]} {
	append result "<h3>" $rssstation($station) "</h3>\n";
    }

    set flist [glob -nocomplain -tails -directory $dir *.$ftype];
    if {[llength $flist] == 0} {
	set result "No files received.\n";

	return $result;
    }

    foreach f $flist {
	set awips1 [string range $f 0 2];
	set mtime [file mtime [file join $dir $f]];
	lappend plist($awips1) ${mtime}@${f};
    }

    foreach awips1 [lsort [array names plist]] {
	append result "<h3>[string toupper $awips1]</h3>\n";
	append result "<ul>\n";
	foreach F [lsort $plist($awips1)] {
	    set Fparts [split $F "@"];
	    set mtime [lindex $Fparts 0];
	    set f [lindex $Fparts 1];
	    set date [clock format $mtime -format "%Y-%m-%d-%H:%M"];
	    set href "<a href=\"/_get/spool/$station/$f\">$f</a>";
	    append result "<li>$date<br>$href</li>\n";
	}
	append result "</ul>\n";
    }

    return $result;
}

proc npemwin_station_catalog_txt {} {

    global Config;
    global rssstation rssstate;

    set ftype "txt";
    set basedir [file join $Config(npemwinfilesdir) $ftype];

    set result "";
    foreach entry [lsort [glob -nocomplain -directory $basedir *]] {
	if {[file isdirectory $entry]} {
	    set station [file tail $entry];
	    set state_name "";
	    if {[info exists rssstation($station)]} {
		set stinfo [split $rssstation($station) ","];
		set country_code [string trim [lindex $stinfo 0]];
		set state_code [string trim [lindex $stinfo 1]];
		if {[info exists rssstate($country_code,$state_code)]} {
		    set state_name $rssstate($country_code,$state_code);
		}
	    }
	    if {$state_name ne ""} {
		lappend href($state_name) \
		    "<a href=\"/npemwin/status/received_by_station.tml?station=$station&ftype=txt\">$station</a>";
	    } else {
		lappend href_other \
		    "<a href=\"/npemwin/status/received_by_station.tml?station=$station&ftype=txt\">$station</a>";
	    }
	}
    }
    
    foreach state [lsort [array names href]] {
	append result "<h3>$state</h3>\n";
	foreach h $href($state) {
	    append result "$h\n";
	}
    }
    append result "<h3>Other</h3>\n";
    foreach h $href_other {
	append result "$h\n";
    }

    return $result;
}

proc npemwin_received_by_station_rss {station rssdir rssfext} {

    set flist [glob -nocomplain -tails -directory $rssdir \
		   *${station}${rssfext}];
    if {[llength $flist] == 0} {
	set result "No files available.\n";

	return $result;
    }

    append result "<h3>Feeds from [string toupper $station]</h3>\n";
    append result "<ul>\n";    
    foreach f [lsort $flist] {
	set rsschannel [file rootname $f];
	set href "<a href=\"/_get/rss/$rsschannel\">$rsschannel</a>";
	append result "<li>$href</li>\n";
    }
    append result "</ul>\n";

    return $result;
}
