#
# $Id$
#
# Source all the .tcl files in the extensions directories
# and their subdirectories.
#
package require fileutil;

if {[info exists Config(npemwinextlibdirs)] == 0} {
    return;
}
if {[llength $Config(npemwinextlibdirs)] == 0} {
    return;
}

# Source all the .tcl files in the extensions directories
# If there is a directory named the same as the file without
# the .tcl extension, then source all the .tcl files in all the
# subdirectories of that directory.

foreach d $Config(npemwinextlibdirs) {
    if {[file isdirectory $d] == 0} {
	continue;
    }

    foreach f [glob -nocomplain -directory $d *.tcl] {
        set status [catch {source $f} errmsg];
        if {$status != 0} {
            puts $errmsg;
	    continue;
        }

	set d1 [file rootname $f];
	if {[file isdirectory $d1] == 0} {
	    continue;
	}
	foreach f1 [::fileutil::findByPattern $d1 "*.tcl"] {
	    set status [catch {source $f1} errmsg];
	    if {$status != 0} {
		puts $errmsg;
	    }
	}
    }
}
