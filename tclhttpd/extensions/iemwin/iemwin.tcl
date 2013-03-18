#
# $Id$
#
# Functions to support the iEmwin Pool Project
#
# Usage: http://<server>[:<port>]/<command>
#
# where <command> is
#
# /_iemwin/stats[?format=xml|csv]
#
Direct_Url /_iemwin _iemwin;

set iemwin(conf) "iemwin.conf";
set iemwin(confdir) $Config(confdir);
set iemwin(localconfdirs) $Config(localconfdirs);
#
set iemwin(upstream_master_file) $Config(npemwinemwinstatusfile);
set iemwin(npemwin_status_file) $Config(npemwinstatusfile);
set iemwin(active_clients_file) $Config(npemwinserveractivefile);
set iemwin(npemwind_pidfile) $Config(npemwinpidfile);

# Non-configurable
set iemwin(data_format) 1;

# The local overrides
set _iemwinconf [file join $iemwin(confdir) $iemwin(conf)];
if {[file exists ${_iemwinconf}]} {
    source ${_iemwinconf};
}
unset _iemwinconf;

proc _iemwin/stats {{format "csv"}} {

    global iemwin;

    # The mechanism used to return content types is to set a global
    # variable with the same name as this function.
    set _iemwin/stats "text/plain";

    if {$format eq "xml"} {
	set _iemwin/stats "text/xml";
	set type "stats";
	set r [iemwin_output_xml_start $type];
    } else {
	set r "";
    }

    append r [iemwin_output_stats $format];

    if {$format eq "xml"} {
	append r [iemwin_output_xml_end $type];
    }

    return $r;
}

proc iemwin_output_stats {format} {

    global iemwin;

    foreach f [list upstream_master_file \
		   npemwin_status_file \
		   active_clients_file \
		   npemwind_pidfile] {

	if {[file exists $iemwin($f)] == 0} {
	    return "";
	}
    }

    set data_format $iemwin(data_format);
    set start_time [file mtime $iemwin(npemwind_pidfile)];
    set upstream_master [iemwin_get_upstream_master];
    set npemwin_status [exec tail -n 1 $iemwin(npemwin_status_file)];

    set _client_data_list \
	[iemwin_output_stats_connections $iemwin(active_clients_file)];
    set num_clients [llength ${_client_data_list}];
    set client_table [join ${_client_data_list} "\n"];

    set r "";
    foreach key [list \
		     data_format \
		     start_time \
		     upstream_master \
		     npemwin_status \
		     num_clients \
		     client_table] {
	if {$format eq "csv"} {
	    eval append r $key "=" $$key;
	    append r ";\n";
	} elseif {$format eq "xml"} {
	    eval append r "<$key>" $$key "</$key>\n";
	}
    }

    return [string trimright $r ";\n"];
}

proc iemwin_output_xml_start {type} {

    set r "<?iemwin_${type} version=\"1.0\"?>\n";

    return $r;
}

proc iemwin_output_xml_end {type} {

    set r "</iemwin_${type}>\n";

    return $r;
}

#
# auxiliary functions
#
proc iemwin_output_stats_connections {file} {
#
# This function returns a tcl list, in which each element is the data
# for each connected client. The code is copied from npemwin_connections{}
# in functions.tcl in the main lib.
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

proc iemwin_get_upstream_master {} {
				 
    global iemwin;

    set upstream_servers_status_list \
	[split [exec cat $iemwin(upstream_master_file)] "\n"];
    
    # The current server is the one with the disconnect time equal to 0
    foreach server $upstream_servers_status_list {
	set disconnect_time [lindex $server 3];
	if {$disconnect_time == 0} {
	    break;
	}
    }

    return $server;
}
