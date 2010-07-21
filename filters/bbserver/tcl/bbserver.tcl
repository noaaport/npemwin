#!%TCLSH%
#
# $Id$
#

# This script is designed to be opened as a pipe. Periodically
# (e.g., every minute) it can be be sent a "report" command to
# instruct it to send the status update to the master host, but
# it has an internal periodic event for that too. It will
# also receive the "Server list" and it will produce a file for
# each of the list types: ServerList, PublicList, DirectList
# and SatList.
#
# NOTE: This script is meant to be used on the master host that sends the
# three lists. See the note in the function bbserver_read_master{}.

package require xsxp;	# in turn requires tclxml

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

# Init
set bbserver(conf)    [file join $gf(confdir) "bbserver.conf"];
set bbserver(confdirs)  $gf(localconfdirs);
set bbserver(configured) 0;

# Defaults
set bbserver(retrysecs)  5;
set bbserver(retrytimes) 6;
set bbserver(report_period_minutes) 4;
#
set bbserver(masterhost) "205.156.51.131";
set bbserver(masterport) 1001;
#
# These definitions must be in sync with the corresponding ones in
# npemwind.conf (defaults.h.in)
#
set bbserver(activefile) "/var/npemwin/stats/npemwind.active";
#
set bbserver(mserverlist) "/var/npemwin/stats/mserverlist.txt";
set bbserver(mserverlist_raw) "/var/npemwin/stats/mserverlist.raw";
#
set bbserver(mserverpublist) "/var/npemwin/stats/mserverpublist.txt";
set bbserver(mserverpublist_raw) "/var/npemwin/stats/mserverpublist.raw";
#
set bbserver(mserverdirlist) "/var/npemwin/stats/mserverdirlist.txt";
set bbserver(mserverdirlist_raw) "/var/npemwin/stats/mserverdirlist.raw";
#
set bbserver(mserversatlist) "/var/npemwin/stats/mserversatlist.txt";
set bbserver(mserversatlist_raw) "/var/npemwin/stats/mserversatlist.raw";

# These are used only the bbserver sctipt
set bbserver(mserverpublist_xml) "/var/npemwin/stats/mserverpublist.xml";
set bbserver(mserverdirlist_xml) "/var/npemwin/stats/mserverdirlist.xml";

#
# Variables
#
set bbserver(numclients_c) "N";
set bbserver(version_protocol) "";   # the Vnn-Pmm combination
# unset bbserver(socket);

#
# Functions
#
@bblib@

#
# main
#
bbserver_configure;
bbserver_open_connection;
bbserver_main_loop;
bbserver_close_connection;
