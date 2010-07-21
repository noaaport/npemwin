#
# $Id$
#

proc bbserver_configure {} {

    global bbserver;

    # The run-time configuration file
    if {[file exists $bbserver(conf)] == 1} {
	source $bbserver(conf);
    }

    if {$bbserver(configured) == 0} {
	log_msg "bbserver.conf unconfigured.";
	return;
    }

    #
    # Dynamic configuration
    #

    set bbserver(version_protocol) $bbserver(version);
    if {$bbserver(protocol) ne "P10"} {
	append bbserver(version_protocol) "-" $bbserver(protocol);
    }

    # Dynamic ip configuration
    if {[regexp {^A/} $bbserver(addrandport)]} {
	set result [bbserver_get_myip $bbserver(addrandport)];
	if {$result == ""} {
	    log_msg "Invalid setting of bbserver(addrandport).";
	    set bbserver(configured) 0;
	    return;
	}
	set bbserver(addrandport) $result;
	unset result;
    }

    set bbserver(configured) 1;
}

proc bbserver_get_myip {bbstr} {
#
# Get the ip from the file specified like "A/path/myip:2211"
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

proc bbserver_get_number_clients {activefile} {

    if {[file exists $activefile] == 0} {
	return 0;
    }

    set n 0;
    set parts [split [exec head -n 1 $activefile]];
    set n [lindex $parts 3];
    
    return $n;
}
