#
# $Id$
#

#
## Direct_Url /npemwin/status npemwin/status
#

proc npemwin/status/stats {} {
#
# Daily Statistics summary of products and frames received and missed
# and minute summary for the last ten minutes.
#
    global Config

    set npemwin_status_file $Config(npemwinstatusfile)
    set npemwin_stats_awk $Config(npemwinlibdir)/npemwin_stats.awk 

    if {[file_hasdata $npemwin_status_file] == 0} {
	return "$npemwin_status_file not found or empty."
    }

    append result [exec awk -f $npemwin_stats_awk $npemwin_status_file]
    append result [npemwin_status $npemwin_status_file]

    return $result
}

proc npemwin/status/connections {} {
#
# Active connections
#
    global Config

    set npemwin_active_file $Config(npemwinserveractivefile)

    if {[file_hasdata $npemwin_active_file] == 0} {
	return "$npemwin_active_file not found or empty."
    }

    return [npemwin_connections $npemwin_active_file]
}

proc npemwin/status/serverlist {} {
#
# Active servers
#
    global Config

    set npemwin_serverlist_file $Config(npemwinactiveserverlist)

    if {[file_hasdata $npemwin_serverlist_file] == 0} {
	return "$npemwin_serverlist_file not found or empty."
    }

    return [npemwin_serverlist $npemwin_serverlist_file]
}

proc npemwin/status/printconf {} {
#
# Print current npemwind configuration.
#

    set result "<h1>Configuration parameters</h1>\n";

    append result [display_config];

    return $result;
}

proc npemwin/status/received_last_minute {} {
#
# List of products received in the last minute.
#
    global Config

    set now [clock seconds]
    set last [clock format [expr $now - 65] -format "%H%M" -gmt true]
    
    set npemwin_received_file \
	[file join $Config(npemwininvdir) ${last}$Config(npemwininvfext)]
    
    if {[file_hasdata $npemwin_received_file] == 0} {
	return "$npemwin_received_file not found or empty."
    }

    return [npemwin_received $npemwin_received_file]
}

proc npemwin/status/received_minute {hhmm} {
#
# List of products received in a given minute.
#
    global Config

    set npemwin_received_file \
	$Config(npemwininvdir)/${hhmm}$Config(npemwininvfext)
    
    if {[file_hasdata $npemwin_received_file] == 0} {
	return "$npemwin_received_file not found or empty."
    }

    return [npemwin_received $npemwin_received_file]
}

proc npemwin/status/received_past_hour {received_minute_tml} {
#
# List of products received in the past hour
#
    set now [clock seconds]
    set t [expr $now - 3600]
    set hh [clock format $t -format "%H" -gmt true]

    return [npemwin_received_hour $received_minute_tml $hh]
}

proc npemwin/status/received_last_hour {received_minute_tml} {
#
# List of products received within the last hour
#
    set now [clock seconds]
    set hh [clock format $now -format "%H" -gmt true]
    set mm [clock format $now -format "%M" -gmt true]

    return [npemwin_received_hour $received_minute_tml $hh $mm]
}

proc npemwin/status/received_last_24hours {received_minute_tml} {
#
# List of products received in the last 24 hours.
#
    set now [clock seconds]
    set hh_now [clock format $now -gmt true -format "%H"]

    # Current hour
    set hh [clock format $now -format "%H" -gmt true]
    set mm [clock format $now -format "%M" -gmt true]
    set result [npemwin_received_hour $hh $mm]   

    set t $now;
    set done 0;
    while {$done == 0} {
	incr t -3600;
	set hh [clock format $t -gmt true -format "%H"]; 
	if {$hh eq $hh_now} {
	    set done 1;
	} else {
	    append result [npemwin_received_hour $received_minute_tml $hh]
	}
    }

    return $result;
}

proc npemwin/status/received_by_station {station ftype} {

     global Config;

    return [npemwin_received_by_station \
		$station $ftype $Config(npemwinfilesdir)];
}

proc npemwin/status/station_catalog_txt {} {

    return [npemwin_station_catalog_txt];
}

proc npemwin/status/received_by_station_rss {station ftype} {

     global Config;

    return [npemwin_received_by_station_rss \
		$station $Config(npemwinrssdir) $Config(npemwinrssfext)];
}
