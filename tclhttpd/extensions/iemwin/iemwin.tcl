#
# $Id$
#
# Functions to support the iEmwin Pool Project
#
# Usage: http://<server>[:<port>]/<command>
#
# where <command> is
#
# /_iemwin/stats[?format=std|yaml|xml|csv|csvk]
#
Direct_Url /_iemwin _iemwin;

set iemwin(conf) "iemwin.conf";
set iemwin(confdir) $Config(confdir);
set iemwin(localconfdirs) $Config(localconfdirs);
#
# Non-configurable
#
set iemwin(data_type) "npemwin/_iemwin/stats";  # <app><url>

# The local overrides
set _iemwinconf [file join $iemwin(confdir) $iemwin(conf)];
if {[file exists ${_iemwinconf}]} {
    source ${_iemwinconf};
}
unset _iemwinconf;

proc _iemwin/stats {{format "std"}} {

    global iemwin;

    # The mechanism used to return content types is to set a global
    # variable with the same name as this function.
    set _iemwin/stats "text/plain";

    if {$format eq "xml"} {
	set _iemwin/stats "text/xml";
	set r [iemwin_output_xml_start $iemwin(data_type)];
    } else {
	set r "";
    }

    append r [iemwin_output_stats $format];

    if {$format eq "xml"} {
	append r [iemwin_output_xml_end $iemwin(data_type)];
    }

    return $r;
}

proc iemwin_output_stats {format} {

    global iemwin;

    set status [catch {
	set data [exec npemwinstatcounters -f $format];
    } errmsg];

    if {$status != 0} {
	return "";
    }

    # Prepend the data_type and any other metadata before the data
    # output by npemwinstatcounters (unless the requested format is "stdh"
    # which contains the header from npemwinstatcounters).
    set r "";
    foreach k [list data_type] {
	if {$format eq "std"} {
	    append r "$k=$iemwin($k)\n";
	} elseif {$format eq "csv"} {
	    append r "$iemwin($k),";
	} elseif {$format eq "csvk"} {
	    append r "$k=$iemwin($k),";
	} elseif {$format eq "xml"} {
	    set r "<$k>$iemwin($k)</$k>\n";
	}
    }

    append r [string trim $data];

    return $r;
}

proc iemwin_output_xml_start {type} {

    set r "<?${type} version=\"1.0\"?>\n";

    return $r;
}

proc iemwin_output_xml_end {type} {

    set r "</${type}>\n";

    return $r;
}
