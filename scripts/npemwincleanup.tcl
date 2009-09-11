#!%TCLSH%
#
# $Id$
#
set usage "usage: nbspcleanup <configfile>";

package require cmdline;

proc match_timespec {spec} {
#
# Returns 1 if the argument passed matches the current date-time
# or 0 otherwise.
#
    set match 0;
    switch -glob $spec {
        H* {set match [match_hourspec $spec]}
        D* {set match [match_dayspec $spec]}
        W* {set match [match_wdayspec $spec]}
        default {puts "Error: Unrecognized time specification: $spec"}
    }

    return $match;
}

proc match_hourspec {hour} {

    if {$hour == "H"} {
        return 1;
    }

    set seconds [clock seconds];
    set hh [clock format $seconds -format "%H"];
    set h [clock format $seconds -format "%k"];

    set match 0;
    switch -glob $hour {
        H/* {
            set specparts [split $hour "/"];
            set interval [lindex $specparts 1];
            if {[expr $h % $interval] == 0} {
                set match 1;
            }
        }
        H=* {
            set specparts [split $hour "="];
            set hourlist [lindex $specparts 1];
            if {[regexp $hourlist $hh]} {
                set match 1;
            }
        }           
        default {puts "Error: Unrecognized hour specification: $hour"}
    }

    return $match;
}

proc match_dayspec {dayspec} {

    set seconds [clock seconds];
    set ddhh [clock format $seconds -format "%d%H"];
    set hh [clock format $seconds -format "%H"];
    set  Dhh "D$hh"; 
    set day [clock format $seconds -format "%e"];

    set match 0;
    switch -glob $dayspec {
        D*/* {
            set specparts [split $dayspec "/"];
            set interval [lindex $specparts 1];
	    set Dhhspec [lindex $specparts 0];
            if {([expr $day % $interval] == 0) && \
		    [regexp $Dhhspec $Dhh]} {
                set match 1;
            }
        }
        D=* {
            set specparts [split $dayspec "="];
            set ddhhlist [lindex $specparts 1];
            if {[regexp $ddhhlist $ddhh]} {
                set match 1;
            }
        }           
        default {puts "Error: Unrecognized daily specification: $dayspec"}
    }

    return $match;
}

proc match_wdayspec {wdayspec} {

    set seconds [clock seconds];
    set wdayhh [clock format $seconds -format "%w%H"];
    set hh [clock format $seconds -format "%H"];
    set wday [clock format $seconds -format "%w"];
    set Whh "W$hh";

    set match 0;
    switch -glob $wdayspec {
        W*/* {
            set specparts [split $wdayspec "/"];
            set interval [lindex $specparts 1];
	    set Whhspec [lindex $specparts 0];
            if {([expr $wday % $interval] == 0) && \
		    [regexp $Whhspec $Whh]} {
                set match 1;
            }
        }
        W=* {
            set specparts [split $wdayspec "="];
            set wdayhhlist [lindex $specparts 1];
            if {[regexp $wdayhhlist $wdayhh]} {
                set match 1;
            }
        }           
        default {puts "Error: Unrecognized weekly specification: $wdayspec"}
    }

    return $match;
}

proc process_dir {dir expression} {

    if {[file isdirectory $dir] == 0} {
	puts "Warning: $dir not found.";
	return;
    }

    puts "### Scheduled cleanup of $dir - [exec date]";

    set pwd [pwd];
    cd $dir;

    if {[regexp {^-c} $expression]} {
	process_dir_bycount "." $expression;
    } elseif {[regexp {^-i} $expression]} {
	process_dir_byinv "." $expression;
    } else {
	process_dir_bytime "." $expression;
    }

    cd $pwd;
}

proc process_dir_bytime {dir expression} {

    # In FreeBSD -delete always returns true, but in linux it can give
    # an error. If the error is not catched, the script exits at this
    # point.

    set status [catch {
	set output [eval exec find $dir $expression -delete -print];
	puts $output;
    } errmsg];

    if {$status != 0} {
	puts $errmsg;
    }
}

proc process_dir_bycount {dir expression} {
#
# expression is a cmd line option of the form 
#
# "-c <count> [-d] [-e <excludepatt>]"
#
# If "-d" is given, then directories are treated as the files, instead
# of being processed recursively (e.g., when dir has subdirectories
# with names like 20090223 20090224 ...
#
    set optlist {{c.arg 0} {d} {e.arg ""}};
    set exprv [split $expression];
    array set option [::cmdline::getoptions exprv $optlist];
    set count $option(c);
    set excludepatt $option(e);

    set filelist [lsort [glob -directory $dir -nocomplain -types f *]];
    set dirlist [lsort [glob -directory $dir -nocomplain -types d *]];
    set filecount [llength $filelist];
    set dircount [llength $dirlist];

    if {$count < 0} {
	set count 0;
    }

    if {$filecount > $count} {
	set max_index_del [expr $filecount - $count - 1];
	set deletelist [lrange $filelist 0 $max_index_del];
	foreach f $deletelist {
	    if {($excludepatt eq "") || \
		    ([regexp $excludepatt [file tail $f]] == 0)} {
		file delete $f;
		puts $f;
	    }
	}
    }

    if {$option(d) == 0} {
	foreach d $dirlist {
	    process_dir_bycount $d $expression;
	}
	return;
    }

    # If option "d" is set, then process the directories the same as the
    # files, but delete with "-force".

    if {$dircount > $count} {
	set max_index_del [expr $dircount - $count - 1];
	set deletelist [lrange $dirlist 0 $max_index_del];
	foreach f $deletelist {
	    if {($excludepatt eq "") || \
		    ([regexp $excludepatt [file tail $f]] == 0)} {
		file delete -force $f;
		puts $f;
	    }
	}
    }
}

proc process_dir_byinv {dir expression} {
#
# Here expression is a cmd line option of the form: -i <find options>.
# We must delete the "-i" before passing the expression to find.
#
# $dir must be the parent directory of a set of "inventory directories".
# An inventory directory contains only inventory files (see nbspinvrm.c
# for the format and assumtions about an inventory file). This function
# finds the inventory directories within $dir (only at depth 1) that match
# the $expression. Then, for each of them, it calls ``find -exec nbspinvrm''
# to process each inventory file. It deletes each inventory after processing
# all the files in it.

    set exprv [split $expression];
    set expression [join [lrange $exprv 1 end] " "];

    # First get the list of inventory files and then process them one by one.
    # Each inventory file should must contain files with
    # the same [file dirname] and the first entry must be a full path.
    # Having inventory files in $dir actually defeats the purpose of this
    # function, since what we want to avoid is ``find'' evaluating the
    # $expression for individual files. But it could be used if the number of
    # of files is small, and for this reason we leave this here.

    set status [catch {
	set output [eval exec find $dir $expression -type f -depth 1 \
			-exec nbspinvrm -s -v {\{\}} {+}];
	if {$output ne ""} {
	    puts $output;
	}
    } errmsg];

    if {$status != 0} {
	puts $errmsg;
    }

    # Now process each inventory subdirectory. Each subdirectory is assumed to
    # contain inventory files only.
    set status [catch {
	set invlist [split \
	[string trim [eval exec find $dir $expression -type d -depth 1]] "\n"];
    } errmsg];

    if {$status != 0} {
	puts $errmsg;
    }

    foreach invdir $invlist {
	set status [catch {
	    set output [eval exec find $invdir -type f \
			    -exec nbspinvrm -s -v {\{\}} {+}];
	    if {$output ne ""} {
		puts $output;
	    }
	} errmsg];

	if {$status != 0} {
	    puts $errmsg;
	} else {
	    file delete $invdir;
	}
    }
}

#
# main
#
puts "- Scheduled run of nbspcleanup - [exec date]";

set argc [llength $argv];
if {$argc == 0} {
    puts $usage;
    exit 1;
}

set conffile [lindex $argv 0];

set body [exec cat $conffile];
foreach line [split $body "\n"] {

    if {[regexp {^#|^\s*$} $line]} {
	continue;
    }
    
    set parts [split $line :];
    set hour [string trim [lindex $parts 0]];
    set dir [string trim [lindex $parts 1]];
    if {[llength $parts] == 3} {
        set excludedirs "";
        set options [string trim [lindex $parts 2]];
    } elseif {[llength $parts] == 4} {
        set excludedirs [string trim [lindex $parts 2]];
        set options [string trim [lindex $parts 3]];
    } else {
	puts "Error: Invalid specification: $line";	
	continue;
    }

    if {[match_timespec $hour] == 0} {
	continue;
    }

    if {$excludedirs == ""} {
	process_dir $dir $options;
    } else {
	foreach subdir [glob -directory $dir -nocomplain -type d *] {
	    if {[regexp $excludedirs [file tail $subdir]]} {
		continue;
	    }
	    process_dir $subdir $options;
	}
    }
}
