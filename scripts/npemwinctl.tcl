#!%TCLSH%
#
# $Id$
#
# usage: npemwinctl <start|stop>
#
# This script calls the installed init script
#
set usage {npemwinctl <start|stop>};

proc errx {s} {

    global argv0;

    set name [file tail $argv0];
    puts stderr "$name: $s";

    exit 1;
}

#
source "/usr/local/etc/npemwin/filters.conf";
source $gf(filterlib);

# configuration
set npemwinctl(rcfpath) "%RCFPATH%";

#
# init
#

if {$argc != 1} {
    errx $usage;
}
set stage [lindex $argv 0];

#
# main
#

set status [catch {
    exec $npemwinctl(rcfpath) $stage;
} errmsg];

if {$status != 0} {
    errx $errmsg;
}

return $status;
