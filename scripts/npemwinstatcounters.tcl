#!%TCLSH%
#
# $Id$
#
# Usage: npemwinstatcounters [-f <fmt>] [-a <activefile>]
#                            [-e <emwinstatusfile>] [-s <statusfile>]
#
# npemwin counters in the last minute (ending at "unix_seconds").
#
# The default files are:
#
#   <activefile>  = npemwind.active
#   <emwinstatusfile> = servers.status
#   <statusfile>  = npemwind.status
#
# The last line is written to stdout; the order of the columns is given below.
# The <fmt> can be: std (default), stdh, xml, csv, csvk
# The motivation for the existence of this tool is to use it for extracting
# and feeding the data to rrdtool or similar programs
# (it is used by the _iemwin extension in the web interface).

package require cmdline;
set usage {npemwintatcounters [-f <fmt>] [-a <activefile>]
    [-e <emwinstatusfile>] [-s <statusfile>]};

set optlist {{a.arg ""} {e.arg ""} {f.arg "std"} {s.arg ""}};

proc err {s} {

    global argv0;

    puts "Error: $s";
    exit 1;
}

#
# auxiliary functions
#
proc _output_stats_connections {file} {
#
# This function returns a tcl list, in which each element is the data
# for each connected client. The code is copied from npemwin_connections{}
# in functions.tcl in the tclhttpd main lib.
# The data for each client is: "ip" "type" "queue" "time"
#
    set r [list];
    set f [open $file r];
    while {[gets $f finfo] > 0} {

        if {[regexp {^(-|\s*$)} $finfo]} {
            continue;
        }

        array set a [proc_stringtoarray [string trimleft $finfo]];
        set client_data [format "%s %s %s %s" $a(1) $a(2) $a(3) $a(4)];
	lappend r $client_data;

    }
    close $f;

    return $r;
}

proc _get_upstream_stats {file} {

    set upstream_servers_status_list [split [exec cat $file] "\n"];
    
    # The current server is the one with the disconnect time equal to 0
    foreach server $upstream_servers_status_list {
	set disconnect_time [lindex $server 3];
	if {$disconnect_time == 0} {
	    break;
	}
    }

    return $server;
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

## The common defaults
set _defaultsfile "/usr/local/etc/npemwin/filters.conf";
if {[file exists ${_defaultsfile}] == 0} {
    err "${_defaultsfile} not found.";
}
source ${_defaultsfile};
unset _defaultsfile;

# npemwind.init is needed for the stats files and variables (log periods)
set npemwind_init_file [file join $gf(filterdir) npemwind.init];
if {[file exists $npemwind_init_file] == 0} {
    err "$npemwind_init_file not found.";
}
source $npemwind_init_file;
unset npemwind_init_file;

array set option [::cmdline::getoptions argv $optlist $usage];
set argc [llength $argv];

# defaults
set fmt $option(f);
set statsperiod_secs $npemwind(statsperiod);    # seconds
set pidfile $npemwind(pidfile);
set activefile $npemwind(serveractivefile);
set statusfile $npemwind(statusfile);
set emwinstatusfile $npemwind(emwinstatusfile);
#
# not configurable
#
set data_format 2;

# Initialization
set npemwind_start_time "";                     # calculated from pid file
set status_values [list];
set estatus_values [list];
set num_clients "";                             # "" means undetermined
set client_table "";
 
if {$option(a) ne ""} {
    set activefile $option(a);
}

if {$option(e) ne ""} {
    set emwinstatusfile $option(e);
}

if {$option(s) ne ""} {
    set statusfile $option(s);
}

# The data is prepended by meta data (data_format, server start time,
#                                     log period)
if {[file exists $pidfile]} {
    set npemwind_start_time [file mtime $pidfile];
}

set keywords [list data_format npemwind_start_time statsperiod_secs];
set values [list $data_format $npemwind_start_time $statsperiod_secs];

#
# These are the fields of the npemwind.status file, followed by the 
# servers.status file.
#
lappend keywords \
    stats_time \
    frames_received \
    frames_processed \
    frames_bad \
    frames_data_size \
    es_ip \
    es_port \
    es_stats_connect \
    es_stats_disconnect \
    es_stats_consecutive_packets \
    es_stats_max_packets \
    es_stats_total_packets \
    es_stats_bad_packet_count \
    es_stats_connections \
    es_stats_error \
    es_stats_serverread_errors \
    es_stats_serverclosed_errors \
    es_stats_header_errors \
    es_stats_checksum_errors \
    es_stats_filename_errors \
    es_stats_unknown_errors;

# Include the upstream conection info and the client table
lappend keywords num_clients client_table;

if {[file exists $statusfile]} {
    set status_values [split [exec tail -n 1 $statusfile]];
}

if {[file exists $emwinstatusfile]} {
    set estatus_values [split [_get_upstream_stats $emwinstatusfile]];
}

# The client table
if {[file exists $activefile]} {
    set _client_data_list [_output_stats_connections $activefile];
    set num_clients [llength ${_client_data_list}];
    set client_table [join ${_client_data_list} "|"];
}

set values [concat $values \
		$status_values \
		$estatus_values \
		$num_clients];
lappend values $client_table;

if {$fmt eq "stdh"} {
    puts {#
# npemwin counters in the last period (ending at "stats_time" in unix seconds).
#
DATA_START};

    foreach k $keywords v $values {
	puts "$k=$v";
    }
    puts "DATA_END";
} elseif {$fmt eq "std"} {
    foreach k $keywords v $values {
	puts "$k=$v";
    }
} elseif {$fmt eq "csv"} {
    puts [join $values ","];
} elseif {$fmt eq "csvk"} {
    set r "";
    foreach k $keywords v $values {
	append r "$k=$v,";
    }
    puts [string trim $r ","];
} elseif {$fmt eq "xml"} {
    foreach k $keywords v $values {
	puts "<$k>$v</$k>";
    }
} else {
    puts "Invalid format: $fmt";
    return 1;
}
