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

    # The SatServers - copied from the DirectList
    set mserversatlist $mserverdirlist;
    set mserversatlist_raw "/SatServers/${body}\\SatServers\\"

    # If the ServerList is empty, copy the PublicList
    if {$mserverlist eq ""} {
	set mserverlist $mserverpublist;
	set mserverlist_raw $mserverpublist_raw;
    }

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

    # The SatServers - whatever that is
    set body "";
    set mserversatlist "";
    set mserversatlist_raw "/SatServers/${body}\\SatServers\\"

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

proc bbserver_save_serverlist_p12_unused {data} {
#
# This is the version that uses xsxp. which we are not using (yet) as
# explained in xml.README.
# 
    global bbserver;

    # Extract each section as raw xml

    set body "";
    if {[regexp {<Public>(.*)</Public>} $data match body] == 0} {
	set mserverpublist_xml "";
    } else {
	set mserverpublist_xml $match;
    }

    set body "";
    if {[regexp {<Direct>(.*)</Direct>} $data match body] == 0} {
	set mserverdirlist_xml "";
    } else {
	set mserverdirlist_xml $match;
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

    set mserverpublist [join $mlist(Public) "\n"];
    set mserverdirlist [join $mlist(Direct) "\n"];

    # Construct the bb style raw files
    append mserverlist_raw "/ServerList/" \
	[string map {\t :} [join $mlist(Public) "|"]] "\\ServerList\\";

    append mserversatlist_raw "/SatServers/" \
	[string map {\t :} [join $mlist(Direct) "+"]] "\\SatServers\\";

    # Save the lists
    foreach type [list pub dir] {
	set xmlname \$mserver${type}list_xml;
	set name \$mserver${type}list;

	eval bbserver_write_serverlist $bbserver(mserver${type}list_xml) \
	    $xmlname;

	eval bbserver_write_serverlist $bbserver(mserver${type}list) $name
    }

    bbserver_write_serverlist $bbserver(mserverlist_raw) $mserverlist_raw;
    bbserver_write_serverlist $bbserver(mserversatlist_raw) \
	$mserversatlist_raw;

    file rename -force $bbserver(mserverpublist) $bbserver(mserverlist);
    file rename -force $bbserver(mserverdirlist) $bbserver(mserversatlist);
}

proc bbserver_save_serverlist_p12 {data} {
#
# This is the version that does NOT use xsxp, as a workaround
# to the problems explained in xml.README.
# 
    global bbserver;

    # Extract each section as raw xml

    foreach Type [list Public Direct] {
	set body "";
	set type [string range [string tolower $Type] 0 2];
	set re "<$Type>(.*)</$Type>";
	if {[regexp $re $data match body] == 0} {
	    set mserverlist_xml($type) "";
	    set mserverlist_xml_body($type) "";
	} else {
	    # mserverlist_xml_body() does not include <Type> </Type> 
	    set mserverlist_xml($type) $match;
	    set mserverlist_xml_body($type) $body;
	}
    }

    # This extracts the two lists in tabular form
    foreach type [list pub dir] {
	set data $mserverlist_xml_body($type);
	set dataparts [::textutil::split::splitx $data "</server>"];
	foreach entry $dataparts {
	    if {[regexp \
	{<server>\s*<name>\s*([^<\s]+)\s*</name>\s*<port>\s*(\d+)\s*</port>} \
		     $entry match name port]} {

		if {($name eq "") || ($port eq "")} {
		    continue;
		} else {
		    lappend mlist($type) \
			"[string trim $name]\t[string trim $port]";
		}
	    }
	}
    }

    set mserverlist(pub) [join $mlist(pub) "\n"];
    set mserverlist(dir) [join $mlist(dir) "\n"];

    # Construct the bb style raw files
    append mserverlist_raw "/ServerList/" \
	[string map {\t :} [join $mlist(pub) "|"]] "\\ServerList\\";

    append mserversatlist_raw "/SatServers/" \
	[string map {\t :} [join $mlist(dir) "+"]] "\\SatServers\\";

    # Save the lists
    foreach type [list pub dir] {
	bbserver_write_serverlist $bbserver(mserver${type}list_xml) \
	    $mserverlist_xml($type);

	bbserver_write_serverlist $bbserver(mserver${type}list) \
	    $mserverlist($type);
    }

    bbserver_write_serverlist $bbserver(mserverlist_raw) $mserverlist_raw;
    bbserver_write_serverlist $bbserver(mserversatlist_raw) \
	$mserversatlist_raw;

    file rename -force $bbserver(mserverpublist) $bbserver(mserverlist);
    file rename -force $bbserver(mserverdirlist) $bbserver(mserversatlist);
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
