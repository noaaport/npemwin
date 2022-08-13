#
# $Id$
#

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
			   $bbserver(version_protocol) \
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
