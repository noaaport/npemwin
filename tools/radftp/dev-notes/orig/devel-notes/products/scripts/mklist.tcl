#!/usr/bin/tclsh
#
# Using "wsr88d-product-awips.txt", create a list that contains only
# those codes that have an awips, and trim the station ID from the
# elements.
#
# ./mklist.tcl > list.txt
#
set data [split [exec cat "wsr88d-product-awips.txt"] "\n"];

foreach line $data {
    set parts [split $line];
    if {[llength $parts] != 3} {
	continue;
    }

    puts [string tolower [string range $line 0 end-3]];
}
