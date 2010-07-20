#!%TCLSH%
#
# $Id$
#

# This script is designed to be opened as a pipe. Periodically
# (e.g., every minute) it can be be sent a "report" command to
# instruct it to send the status update to the master host, but
# it has an internal periodic event for that too. It will
# also receive the "Server list" and it will produce a file for
# each of the list types: ServerList, PublicList, DirectList
# and SatList.
#
# NOTE: This script is meant to be used on the master host that sends the
# three lists. See the note in the function bbserver_read_master{}.

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
set bbserver(report_period_minutes) 4;
#
set bbserver(masterhost) "205.156.51.131";
set bbserver(masterport) 1001;
#
# These definitions must be in sync with the corresponding ones in
# npemwind.conf (defaults.h.in)
#
set bbserver(activefile) "/var/npemwin/stats/npemwind.active";
#
set bbserver(mserverlist) "/var/npemwin/stats/mserverlist.txt";
set bbserver(mserverlist_raw) "/var/npemwin/stats/mserverlist.raw";
#
set bbserver(mserverpublist) "/var/npemwin/stats/mserverpublist.txt";
set bbserver(mserverpublist_raw) "/var/npemwin/stats/mserverpublist.raw";
#
set bbserver(mserverdirlist) "/var/npemwin/stats/mserverdirlist.txt";
set bbserver(mserverdirlist_raw) "/var/npemwin/stats/mserverdirlist.raw";
#
set bbserver(mserversatlist) "/var/npemwin/stats/mserversatlist.txt";
set bbserver(mserversatlist_raw) "/var/npemwin/stats/mserversatlist.raw";

#
# Variables
#
set bbserver(numclients_c) "N";
# unset bbserver(socket);

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
    # Dynamic ip configuration
    #
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

proc bbserver_read_master {} {
#
# This function is called only when the socket is readable; in particular
# there is no need to check here if the connection is open.
# We are assuming here the new (1.1 ?) format
#
# ServerList|<hlist1>|
# /PublicList/<hlist2>\PublicList\
# /DirectList/<hlist2>\DirectLIst\
#
# where
#
# <hlist1> = host1:port1|...
# <hlist2> = host1:port1+...
#
# In the function "bbserver_save_serverlist", the data is reformated such that
# th data looks like
#
# /ServerList/<hlist1>\ServerList\
# /PublicList/<hlist2>\PublicList\
# /DirectList/<hlist2>\DirectLIst\
#
# Each section is saved in its own files (raw and txt). In addition
# a fourth file is generated for the SatList to pass it to the clients.
#
# NOTE: The function is designed to be used with the new version of
# the master host, but as a safety measure it tries to detect
# the old transmission format (only the ServerList) and proceed appropriately.

    global bbserver;

    # To grab the packet, we continue reading until we get the fourth
    # backalash (the last one after the DirectList)

    set data "";
    set slashcount 0;
    set eagain 0;
    while {$slashcount < 4} {
	set status [catch {
	    set c [read $bbserver(socket) 1];
	} errmsg];

	if {$status != 0} {
	    log_msg $errmsg;
	    bbserver_close_connection;
	    return;
	} elseif {$c eq ""} {
	    # This is possible if the channel is opened in nonblocking mode.
	    # Wait a little and retry until we reach the retry limit.
	    if {$eagain == $bbserver(retrytimes)} {
		# Assume that it is an old master host
		log_msg "Received old server list";
		bbserver_save_serverlist_old $data;
		log_msg "Wrote old server list";
		bbserver_send_status_update;
		return;
	    } else {
		# Wait a little and loop again		
		incr eagain;
		after [expr $bbserver(retrysecs) * 1000];
	    }
	}

	append data $c;
	if {$c eq "\\"} {
	    incr slashcount;
	}
    }
    bbserver_save_serverlist $data;
    log_msg "Wrote server list";
    bbserver_send_status_update;
}

proc bbserver_read_stdin {line} {

    if {$line eq "init"} {
	log_msg "bbserver registration started.";
	bbserver_send_status_update;
    } elseif {$line eq "report"} {
	bbserver_send_status_update;
    } else {
	log_msg "Invalid command ignored: $line";
    }
}

proc bbserver_save_serverlist {data} {

    global bbserver;

    # Fix the ServerList part to have the "standard" start/end keys
    regsub {ServerList\|} $data "/ServerList/" data1;
    regsub {\|/Public} $data1 "\\ServerList\\/Public" data;

    # Extract each section

    set body "";
    if {[regexp {/ServerList/(.*)\\ServerList\\} $data match body] == 0} {
	set mserverlist "";
    } else {
	set mserverlist [string map {: \t | \n} $body];
    }
    set mserverlist_raw "/ServerList/${body}\\ServerList\\"

    set body "";
    if {[regexp {/PublicList/(.*)\\PublicList\\} $data match body] == 0} {
	set mserverpublist "";
    } else {
	set mserverpublist [string map {: \t \+ \n} $body];
    }
    set mserverpublist_raw "/PublicList/${body}\\PublicList\\"

    set body "";
    if {[regexp {/DirectList/(.*)\\DirectList\\} $data match body] == 0} {
	set mserverdirlist "";
    } else {
	set mserverdirlist [string map {: \t \+ \n} $body];
    }
    set mserverdirlist_raw "/DirectList/${body}\\DirectList\\"

    # The SatList - whatever that is
    set body "";
    set mserversatlist "";
    set mserversatlist_raw "/SatList/${body}\\SatList\\"

    # Save the ServerList
    bbserver_write_serverlist $bbserver(mserverlist_raw) $mserverlist_raw
    bbserver_write_serverlist $bbserver(mserverlist) $mserverlist;

    # Save the other lists
    foreach type [list pub dir sat] {
	set rawname \$mserver${type}list_raw;
	set name \$mserver${type}list;

	eval bbserver_write_serverlist $bbserver(mserver${type}list_raw) \
	    $rawname;

	eval bbserver_write_serverlist $bbserver(mserver${type}list) $name
    }
}

proc bbserver_save_serverlist_old {data} {

    global bbserver;

    # Fix the ServerList part to have the "standard" start/end keys
    regsub {ServerList\|} $data "/ServerList/" data1;
    regsub {\|$} $data1 "\\ServerList\\" data;

    set body "";
    if {[regexp {/ServerList/(.*)\\ServerList\\} $data match body] == 0} {
	set mserverlist "";
    } else {
	set mserverlist [string map {: \t | \n} $body];
    }
    set mserverlist_raw "/ServerList/${body}\\ServerList\\"

    # The SatList - whatever that is
    set body "";
    set mserversatlist "";
    set mserversatlist_raw "/SatList/${body}\\SatList\\"

    bbserver_write_serverlist $bbserver(mserverlist_raw) $mserverlist_raw
    bbserver_write_serverlist $bbserver(mserverlist) $mserverlist;

    foreach type [list sat] {
	set rawname \$mserver${type}list_raw;
	set name \$mserver${type}list;

	eval bbserver_write_serverlist $bbserver(mserver${type}list_raw) \
	    $rawname;

	eval bbserver_write_serverlist $bbserver(mserver${type}list) $name
    }
}

proc bbserver_write_serverlist {fpath data} {

    set status [catch {
	set f [open $fpath w];
	puts $f $data;
    } errmsg];

    if {[info exists f]} {
	catch {close $f};
    }

    if {$status != 0} {
	log_msg "Could not write to $fpath";
    }
}

proc bbserver_open_connection {} {

    global bbserver;

    if {[info exists bbserver(socket)]} {
	return 0;    # no error
    }

    set mhost $bbserver(masterhost);
    set mport $bbserver(masterport);
    set retrysecs $bbserver(retrysecs);
    set retrytimes $bbserver(retrytimes);

    set i 0;
    while {$i <= $retrytimes} {
	incr i;
	log_msg "Trying connection to $mhost@$mport";
	set status [catch {
	    set socket [socket $mhost $mport];
	    fconfigure $socket -buffering none -blocking 0;
	    log_msg "Established connection to $mhost@$mport";
	} errmsg];
	if {$status != 0} {
	    if {[info exists socket]} {
		catch {close $socket};
		unset socket;
	    }
	    log_msg $errmsg;
	    after [expr $retrysecs * 1000];
	} else {
	    break;
	}
    }

    if {[info exists socket] == 0} {
	log_msg "Could not open connection to $mhost@$mport";
	return 1;
    }

    fileevent $socket readable bbserver_read_master;
    set bbserver(socket) $socket;

    return 0;
}

proc bbserver_close_connection {} {

    global bbserver;

    if {[info exists bbserver(socket)]} {
	catch {close $bbserver(socket)};
	unset bbserver(socket);
    }
}

proc bbserver_send_status_update {} {

    global bbserver;

    if {$bbserver(configured) == 0} {
	bbserver_configure;
    }

    if {$bbserver(configured) == 0} {
	return;
    }

    # Check that the connection is open, and try to open it if it is closed.
    if {[bbserver_open_connection] != 0} {
	return;
    }

    set bbserver(time) [clock format [clock seconds] \
			    -gmt true -format "%H:%M"];

    set bbserver(numclients) $bbserver(numclients_c);
    append bbserver(numclients) \
	[bbserver_get_number_clients $bbserver(activefile)];

    set bbserver_list [list $bbserver(time) \
			   $bbserver(addrandport) \
			   $bbserver(numclients) \
			   $bbserver(maxclients) \
			   $bbserver(privacy) \
			   $bbserver(rate) \
			   $bbserver(version) \
			   $bbserver(location) \
			   $bbserver(serveradmin)];

    set bbserver_str [join $bbserver_list "|"];
    
    set status [catch {
	puts $bbserver(socket) $bbserver_str;
    } errmsg];

    if {$status != 0} {
	log_msg $errmsg;
	bbserver_close_connection;
    } else {
	log_msg $bbserver_str;
    }
}

proc bbserver_main_loop {} {

    global bbserver;
    global done;

    set done 0;
    while {$done == 0} {
	fileevent stdin readable {set done 1}
	after [expr $bbserver(report_period_minutes) * 60000] {set done 2};
	vwait done;
	if {$done == 1} {
	    if {[gets stdin line] >= 0} {
		set done 0;    # restart
		bbserver_read_stdin $line;
	    }
	} else {
	    # timer expired
	    set done 0;        # restart
	    bbserver_send_status_update;
	}
    }
}

#
# main
#
bbserver_configure;
bbserver_open_connection;
bbserver_main_loop;
bbserver_close_connection;
