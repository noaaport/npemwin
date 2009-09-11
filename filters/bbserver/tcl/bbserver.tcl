#!%TCLSH%
#
# $Id$
#

## The common defaults
set defaultsfile "/usr/local/etc/npemwin/filters.conf";
if {[file exists $defaultsfile] == 0} {
    puts "$argv0 disabled: $defaultsfile not found.";
    return 1;
}
source $defaultsfile;

## The filter library
if {[file exists $gf(filterlib)] == 0} {
    puts "$argv0 disabled: $gf(filterlib) not found.";
    return 1;
}
source $gf(filterlib);

# Init
set bbserver(conf)    [file join $gf(confdir) "bbserver.conf"];
set bbserver(confdirs)  $gf(localconfdirs);
set bbserver(configured) 0;
# Defaults
set bbserver(retrysecs)  5;
set bbserver(retrytimes) 6;
#
set bbserver(activefile) "/var/npemwin/stats/active";
#
set bbserver(masterhost) "205.156.51.131";
set bbserver(masterport) 1001;
# Variables
set bbserver(numclients_c) "N";

if {[file exists $bbserver(conf)] == 1} {
    source $bbserver(conf);
}

if {$bbserver(configured) == 0} {
    log_msg "bbserver.conf unconfigured.";
    return 1;
}

proc get_myip {bbstr} {
#
# Get the ip from the file specified like "A/path/myip:1000"
#
    if {[regexp {^A/} $bbstr] == 0} {
	return "";
    }

    set parts [split $bbstr ":"]
    set ipfile [string range [lindex $parts 0] 1 end];
    if {[file exists $ipfile] == 0} {
	return "";
    }

    set ip [string trim [exec head -n 1 $ipfile]];
    regsub $ipfile $bbstr $ip result;

    return $result;
}

proc get_number_clients {activefile} {

    if {[file exists $activefile] == 0} {
	return 0;
    }

    set n 0;
    set parts [split [exec head -n 1 $activefile]];
    set n [lindex $parts 3];
    
    return $n;
}

proc open_connection {mhost mport retrysecs retrytimes} {

    set i 0;
    while {$i <= $retrytimes} {
	incr i;
	log_msg "Trying connection to $mhost@$mport";
	set status [catch {
	    set socket [socket $mhost $mport];
	    log_msg "Established connection";
	    fconfigure $socket -buffering line;
	} errmsg];
	if {$status != 0} {
	    log_msg $errmsg;
	    after [expr $retrysecs * 1000];
	} else {
	    break;
	}
    }

    return $socket;
}

#
# Dynamic ip configuration
#
if {[regexp {^A/} $bbserver(addrandport)]} {
    set result [get_myip $bbserver(addrandport)];
    if {$result == ""} {
	log_msg "Invalid setting of bbserver(addrandport).";
	return 1;
    }
    set bbserver(addrandport) $result;
    unset result;
}

#
# main
#
set socket [open_connection \
		$bbserver(masterhost) $bbserver(masterport) \
		$bbserver(retrysecs) $bbserver(retrytimes)];

set bbserver(time) [clock format [clock seconds] \
			    -gmt true -format "%H:%M"];
set bbserver(numclients) \
    $bbserver(numclients_c)[get_number_clients $bbserver(activefile)];

set bbserver_list [list $bbserver(time) \
		       $bbserver(addrandport) \
		       $bbserver(numclients) \
		       $bbserver(maxclients) \
		       $bbserver(privacy) \
		       $bbserver(rate) \
		       $bbserver(version) \
		       $bbserver(location)];

set bbserver_str [join $bbserver_list "|"];

set status [catch {
    puts $socket $bbserver_str;
} errmsg];
if {$status != 0} {
    log_msg $errmsg;
} else {
    log_msg $bbserver_str;
}
close $socket;
