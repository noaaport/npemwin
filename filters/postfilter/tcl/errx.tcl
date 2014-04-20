#
# $Id$
#
package provide npemwin::errx 1.0;
package require fileutil;

namespace eval npemwin::errx {}
namespace eval npemwin::filelog {}
namespace eval npemwin::syslog {

    variable syslog;

    set syslog(usesyslog) 0;
}

proc npemwin::errx::warn s {

    global argv0;

    set name [file tail $argv0];
    puts stderr "$name: $s";
}

proc npemwin::errx::errc s {

    warn $s;
}

proc npemwin::errx::err s {

    warn $s;
    exit 1;
}

#
# syslog
#
proc npemwin::syslog::usesyslog {{flag 1}} {

    variable syslog;

    set syslog(usesyslog) $flag;
}

proc npemwin::syslog::_usesyslog_set {} {

    variable syslog;

    set r 0;
    if {$syslog(usesyslog) == 1} {
	set r 1;
    } elseif {[file pathtype $syslog(usesyslog)] eq "absolute"} {
	set r 1;
    }

    return $r;
}

proc npemwin::syslog::_log_msg s {

    global argv0;
    variable syslog;

    if {$syslog(usesyslog) == 1} {
	set name [file tail $argv0];
	exec logger -t $name -- $s;
    } elseif {[file pathtype $syslog(usesyslog)] eq "absolute"} {
	::npemwin::filelog::msg $syslog(usesyslog) $s;
    }
}

proc npemwin::syslog::msg s {

    variable syslog;

    if {[_usesyslog_set] == 1} {
	_log_msg $s;
    } else {
	puts $s;
    }
}

proc npemwin::syslog::warn s {

    variable syslog;

    if {[_usesyslog_set] == 1} {
	_log_msg "Warning: $s";
    } else {
	::npemwin::errx::warn $s;
    }
}

proc npemwin::syslog::errc s {

    variable syslog;

    if {[_usesyslog_set] == 1} {
	_log_msg "Error: $s";
    } else {
	::npemwin::errx::errc $s;
    }
}

proc npemwin::syslog::err s {

    variable syslog;

    if {[_usesyslog_set] == 1} {
	_log_msg "Error: $s";
	exit 1;
    } else {
	::npemwin::errx::err $s;
    }
}

#
# Log to a file
#
proc npemwin::filelog::msg {logfile s} {

    global argv0;

    set name [file tail $argv0];
    append msg $name ": " [clock format [clock seconds]] ": " $s "\n"; 

    set status [catch {
	::fileutil::appendToFile $logfile $msg;
    } errmsg];

    if {$status != 0} {
	puts stderr $errmsg;
    }
}
