#!%TCLSH%
#
# $Id$
#
# Usage: startstop <start|stop>
#
set usage {Usage: startstop <start|stop>};

set defaultsfile "/usr/local/etc/npemwin/filters.conf";
## The common defaults
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

#
# Default schedule
#
set startstop(rc) "startstop.rc";

# Location (use the last one found)
set startstop(confdirs) [linsert $gf(localconfdirs) 0 $gf(confdir)];

#
# main
#

set startstop(stage) "";
if {$argc == 1} {
    set startstop(stage) [lindex $argv 0];
}
if {($startstop(stage) ne "start") && ($startstop(stage) ne "stop")} {
    puts "$argv0 disabled: $usage";
    return 1;
}

set conffile "";
foreach _d $startstop(confdirs) {
    set _f [file join ${_d} $startstop(rc)]; 
    if {[file exists ${_f}]} {
	set conffile ${_f};
    }
}
if {$conffile == ""} {
    log_msg "$startstop(rc) not found.";
    return 1;
}

set schedule [list];
source $conffile;
set program_list [list];

foreach entry $schedule {

    set parts [split $entry :];
    if {[llength $parts] != 2} {
        log_msg "Error: Invalid specification: $entry"; 
        continue;
    }
    set code [string trim [lindex $parts 0]];
    set program [string trim [lindex $parts 1]];

    if {$code ne $startstop(stage)} {
        continue;
    }

    lappend program_list $program;
}

foreach program $program_list {

    # The eval causes the $program string to be split on blanks
    # in case there are options, and any variables to be substituted
    # by their value.
    # "program" is not explicitly executed in the background here. That
    # can be specified in the <program> part in the configuration file itself,
    # depending on the particular conditions of the program that is being
    # executed.

    set status [catch {
        eval exec $program;
    } errmsg];

    if {$status != 0} {
	log_msg $errmsg;
    }     
}
