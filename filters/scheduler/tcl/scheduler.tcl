#!%TCLSH%
#
# $Id$
#

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

#
# Default schedule
#
set scheduler(rc) "scheduler.conf";

# Location (use the last one found)
set scheduler(confdirs) [linsert $gf(localconfdirs) 0 $gf(confdir)];

#
# Functions of this script
#

proc match_timespec {spec} {
#
# Returns 1 if the argument passed matches the current date-time
# or 0 otherwise.

    set match 0;
    switch -glob $spec {
	M* {set match [match_minutespec $spec]}
        H* {set match [match_hourspec $spec]}
        D* {set match [match_dayspec $spec]}
        W* {set match [match_wdayspec $spec]}
        default {log_msg "Error: Unrecognized time specification: $spec"}
    }

    return $match;
}

proc match_minutespec {code} {

    if {$code == "M"} {
        return 1;
    }

    set seconds [clock seconds];
    set mm [clock format $seconds -format "%M"];
    regsub {0(.+)} $mm {\1} m;

    set match 0;
    switch -glob $code {
        M/* {
            set specparts [split $code "/"];
            set interval [lindex $specparts 1];
            if {[expr $m % $interval] == 0} {
                set match 1;
            }
        }
        M=* {
            set specparts [split $code "="];
            set minutelist [lindex $specparts 1];
            if {[regexp $minutelist $mm]} {
                set match 1;
            }
        }           
        default {log_msg "Error: Unrecognized hour specification: $code"}
    }

    return $match;
}

proc match_hourspec {hourspec} {

    set seconds [clock seconds];
    set hhmm [clock format $seconds -format "%H%M"];
    set mm [clock format $seconds -format "%M"];
    set  Hmm "H$mm"; 
    set hour [clock format $seconds -format "%k"];

    set match 0;
    switch -glob $hourspec {
        H*/* {
            set specparts [split $hourspec "/"];
            set interval [lindex $specparts 1];
	    set Hmmspec [lindex $specparts 0];
            if {([expr $hour % $interval] == 0) && \
		    [regexp $Hmmspec $Hmm]} {
                set match 1;
            }
        }
        H=* {
            set specparts [split $hourspec "="];
            set hhmmlist [lindex $specparts 1];
            if {[regexp $hhmmlist $hhmm]} {
                set match 1;
            }
        }           
        default {log_msg "Error: Unrecognized daily specification: $hourspec"}
    }

    return $match;
}

proc match_dayspec {dayspec} {

    set seconds [clock seconds];
    set ddhhmm [clock format $seconds -format "%d%H%M"];
    set hhmm [clock format $seconds -format "%H%M"];
    set  Dhhmm "D$hhmm"; 
    set day [clock format $seconds -format "%e"];

    set match 0;
    switch -glob $dayspec {
        D*/* {
            set specparts [split $dayspec "/"];
            set interval [lindex $specparts 1];
	    set Dhhmmspec [lindex $specparts 0];
            if {([expr $day % $interval] == 0) && \
		    [regexp $Dhhmmspec $Dhhmm]} {
                set match 1;
            }
        }
        D=* {
            set specparts [split $dayspec "="];
            set ddhhmmlist [lindex $specparts 1];
            if {[regexp $ddhhmmlist $ddhhmm]} {
                set match 1;
            }
        }           
        default {log_msg "Error: Unrecognized daily specification: $dayspec"}
    }

    return $match;
}

proc match_wdayspec {wdayspec} {

    set seconds [clock seconds];
    set wdayhhmm [clock format $seconds -format "%w%H%M"];
    set hhmm [clock format $seconds -format "%H%M"];
    set wday [clock format $seconds -format "%w"];
    set Whhmm "W$hhmm";

    set match 0;
    switch -glob $wdayspec {
        W*/* {
            set specparts [split $wdayspec "/"];
            set interval [lindex $specparts 1];
	    set Whhmmspec [lindex $specparts 0];
            if {([expr $wday % $interval] == 0) && \
		    [regexp $Whhmmspec $Whhmm]} {
                set match 1;
            }
        }
        W=* {
            set specparts [split $wdayspec "="];
            set wdayhhmmlist [lindex $specparts 1];
            if {[regexp $wdayhhmmlist $wdayhhmm]} {
                set match 1;
            }
        }           
        default {log_msg "Error: Unrecognized weekly specification: $wdayspec"}
    }

    return $match;
}

#
# main
#


set conffile "";
foreach _d $scheduler(confdirs) {
    set _f [file join ${_d} $scheduler(rc)]; 
    if {[file exists ${_f}]} {
	set conffile ${_f};
    }
}
if {$conffile == ""} {
    log_msg "$scheduler(rc) not found.";
    return 1;
}

set schedule [list];
source $conffile;

# First build the list of programs to execute, according to whether
# their time specification is satisfied, and then execute those in the list.
# It must be done in this order so that the verification of the time
# specification is valid. (Otherwise a program's time spec is not checked
# until after other programs have finished and insted of "now".
set program_list [list];
foreach entry $schedule {

    set parts [split $entry :];
    if {[llength $parts] != 2} {
	log_msg "Error: Invalid specification: $entry";	
	continue;
    }
    set code [string trim [lindex $parts 0]];
    set program [string trim [lindex $parts 1]];

    if {[match_timespec $code] == 0} {
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
