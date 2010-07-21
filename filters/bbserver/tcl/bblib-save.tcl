#
# $Id$
#

proc bbserver_save_serverlist_p11 {data} {

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

proc bbserver_save_serverlist_p10 {data} {

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

proc bbserver_save_serverlist_p12 {data} {

    global bbserver;

    # Extract each section as raw xml

    set body "";
    if {[regexp {<Public>(.*)</Public>} $data match body] == 0} {
	set mserverpublist_raw "";
    } else {
	set mserverpublist_raw $match;
    }

    set body "";
    if {[regexp {<Direct>(.*)</Direct>} $data match body] == 0} {
	set mserverdirlist_raw "";
    } else {
	set mserverdirlist_raw $match;
    }

    # This extracts the two lists in tabular form
    set pxml [xsxp::parse $data];
    foreach type [list Public Direct] {
	set i 0;
	set done 0;
	set mlist($type) [list];
	while {$done == 0} {
	    set name [xsxp::fetch $pxml [list $type server\#$i name] %PCDATA?];
	    set port [xsxp::fetch $pxml [list $type server\#$i port] %PCDATA?];
	    if {($name eq "") && ($port eq "")} {
		set done 1;
	    } elseif {($name eq "") || ($port eq "")} {
		continue;
	    } else {
		lappend mlist($type) "$name\t$port";
		incr i
	    }
	}
	
    }

    set mserverpublist [join mlist(Public) "\n"];
    set mserverdirlist [join mlist(Direct) "\n"];

    # The SatList - whatever that is
    set body "";
    set mserversatlist "";
    set mserversatlist_raw "<Sat>${body}</Sat>"

    # Save the lists
    foreach type [list pub dir sat] {
	set rawname \$mserver${type}list_raw;
	set name \$mserver${type}list;

	eval bbserver_write_serverlist $bbserver(mserver${type}list_raw) \
	    $rawname;

	eval bbserver_write_serverlist $bbserver(mserver${type}list) $name
    }

    # Copy the public list as the master servers list
    file copy $bbserver(mserverpublist_raw) $bbserver(mserverlist_raw);
    file copy $bbserver(mserverpublist) $bbserver(mserverlist);
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
