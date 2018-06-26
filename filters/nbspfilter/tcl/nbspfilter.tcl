#!/usr/bin/tclsh
#
# $Id$
#
package require Tclx; # umask

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
set nbspfilter(nbspd_spooldir) "/var/noaaport/nbsp/spool";
set nbspfilter(nbspd_infifo) "/var/run/nbsp/infeed.fifo";
set nbspfilter(umask) "002";
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

    set spooldir $nbspfilter(nbspd_spooldir);
    set station $rc(header_station);
    set wmoid $rc(header_wmoid);
    set awips $rc(awips);
    set ymd $rc(ymd);
    append dhm $rc(d) $rc(H) $rc(M);

    # For the sequence number use the milliseconds seconds since midnight
    append _datestr $ymd "T000000";
    set s1 [clock scan ${_datestr} -gmt 1];
    append s1 "000";
    set s2 [clock milliseconds];
    set seq [expr $s2 - $s1];

    set fpathout_parentdir [file join $spooldir $station];

    append fname $station "_" $wmoid;
    if {$awips ne ""} {
	if {[string length $awips] == 6} {
	    append fname "-" $awips;
	} else {
	    append fname "+" $awips;
	}
    }

    append fbasename $fname "." $dhm "_" $seq;
    set fpathout [file join $fpathout_parentdir $fbasename];

    set finfo "$seq 0 0 0 0 $fname $fpathout";

    # The spooldir must exist
    if {[file isdirectory $spooldir] == 0} {
	return -code error "Spool directory does not exist: $spooldir";
    }

    set oldmask [umask];
    umask $nbspfilter(umask);

    if {$nbspfilter(verbose) == 1} {
	log_msg "Inserting: $fpathout";
    }
    set status [catch {
	file mkdir $fpathout_parentdir;
	proc_nbsp_insert_ccb $fpathin $fpathout;
	exec nbspinsert -f $nbspfilter(nbspd_infifo) $finfo;
    } errmsg];
 
    if {$status != 0} {
	file delete $fpathout;
	log_msg "Error processing $fpathin";
	log_msg $errmsg;
    }

    umask $oldmask;
}

proc proc_nbsp_insert_ccb {fpathin fpathout} {
#
# The only purpose of this function is to insert the ccb
# that the nbsp spool files have.
#
    set ccb [string repeat "0" 24];
    set status [catch {
	set fout [open $fpathout w];
	set fin  [open $fpathin r];
	fconfigure $fin -translation binary -encoding binary;
	fconfigure $fout -translation binary -encoding binary;

	puts -nonewline $fout $ccb;
	puts -nonewline $fout [read $fin];
    } errmsg];

    if {[info exists fout]} {
	close $fout;
    }

    if {[info exists fin]} {
	close $fin;
    }

    if {$status != 0} {
        log_msg $errmsg;
    }
}

main
