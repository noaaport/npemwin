#!/usr/bin/tclsh
#
# $Id$
#
# Originally this script was calling nbspinsert. This version uses nbspmcast
# (and in particular it does not require tclx).
#

## The common defaults
set _defaultsfile "/usr/local/etc/npemwin/filters.conf";
if {[file exists ${_defaultsfile}] == 0} {
    puts "nbspfilter disabled: ${_defaultsfile} not found.";
    return 1;
}
source ${_defaultsfile};
unset _defaultsfile;

## The filter library
if {[file exists $gf(filterlib)] == 0} {
    puts "nbspfilter disabled: $gf(filterlib) not found.";
    return 1;
}
source $gf(filterlib);

# Defaults
set nbspfilter(conf)	[file join $gf(confdir) "nbspfilter.conf"];
set nbspfilter(confdirs) $gf(localconfdirs);
#
# These are configurable in the configuration file(s)
#
set nbspfilter(verbose) 0;

# The default optional configuration file for this filter.
#
if {[file exists $nbspfilter(conf)] == 1} {
    source $nbspfilter(conf);
}

proc main {} {

    global errorInfo;

    while {[gets stdin line] >= 0} {
	if {[regexp {^\s*$} $line]} {
	    continue;
	}
	set args [split $line];
	set argsc [llength $args];
    	if {$argsc != 2} {
            log_msg "Incorrect number of arguments: $args";
	    continue;
        }

    	set prodname [lindex $args 0];
    	set fpath [lindex $args 1];
    
        set status [catch {process $prodname $fpath} errmsg];
	if {$status == 1} {
	    log_msg "Error processing $prodname";
	    log_msg $errmsg;
	    log_msg $errorInfo;
	}
    }
}

proc process {prodname fpathin} {

    global nbspfilter;

    filterlib_get_rcvars rc $prodname $fpathin;

    # Only process text files
    if {$rc(txtflag) == 0} {
	return;
    }

    if {$nbspfilter(verbose) == 1} {
	log_msg "Inserting: $fpathin";
    }

    # Send to nbsp
    set status [catch {
	exec nbspmcast -b $fpathin;
    } errmsg];
 
    if {$status != 0} {
	log_msg "Error processing $fpathin";
	log_msg $errmsg;
    }
}


main
