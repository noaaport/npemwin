#
# $Id$
#

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

proc bbserver_read_master {} {

    global bbserver;

    if {($bbserver(protocol) eq "P11") || ($bbserver(protocol) eq "P10")} {
	bbserver_read_master_p11;
    } elseif {$bbserver(protocol) eq "P12"} {
	bbserver_read_master_p12;
    } else {
	log_msg "Invalid protocol $bbserver(protocol).";
    } 
}

proc bbserver_read_master_p11 {} {
#
# This function is called only when the socket is readable; in particular
# there is no need to check here if the connection is open.
# We are assuming here the new (1.1) format (but see NOTE below)
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
# NOTE: The function is designed to be used with the new 1.1 version of
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
		bbserver_save_serverlist_p10 $data;
		log_msg "Wrote P10 server list";
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
    bbserver_save_serverlist_p11 $data;
    log_msg "Wrote P11 server list";
    bbserver_send_status_update;
}

proc bbserver_read_master_p12 {} {
#
# This function is called only when the socket is readable; in particular
# there is no need to check here if the connection is open.
# We are assuming here the new (1.2) xml format.
#
# <ServerList>
#   <Public>
#      <server>
#        <name>...</name>
#        <port>...</port>
#      </server>
#      ...
#   </Public>
#
#   <Direct>
#       ....
#   </Direct>
# </ServerList>
#
#
# Each section is saved in its own files (raw = xml and txt). In addition
# a fourth file is generated for the SatList to pass it to the clients.

    global bbserver;

    # To grab the packet, we continue reading until the accumulated
    # data matches the pattern "</ServerList>$" (i.e., at the end).

    set data "";
    set eagain 0;
    set done 0;
    while {$done == 0} {
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
		log_msg "Timedout reading from the master host";
		bbserver_close_connection;
		return;
	    } else {
		# Wait a little and loop again		
		incr eagain;
		after [expr $bbserver(retrysecs) * 1000];
	    }
	}

	append data $c;
	if {[regexp {</ServerList>$} $data]} {
	    set done 1;
	}
    }
    bbserver_save_serverlist_p12 $data;
    log_msg "Wrote P12 server list";
    bbserver_send_status_update;
}
