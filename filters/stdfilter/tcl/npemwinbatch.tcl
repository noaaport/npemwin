#!%TCLSH%
#
# $Id$
#
# Usage: npemwinbatch get [-q] [-c] [-e expr] [-f extension] [-h hhmm] [-n]
#    [-b basedir] [-i invdir] [-s spooldir] [ -I invurl] [-S spoolurl]
#    [ -m curlmaxtime] [-r curlretries] <host:port>};
#
# [-q] => Stay quiet
# [-c] => Create basedir if it does not exist (subdirs are created anyawy)
# [-e] => Only files for which "uwildmat $expr $ppath" is true are considered.
#         ppath is of the form <wfo>/<fbasename> for txt files, or just
#	  <fbasename> for non-text files.
#         Note: the <expr> is actually a "uwildregex" expression.
# [-f] => The extension of the batch inventory files (.q)
# [-h] => hhmm otherwise current hhmm (UTC) is used.
# [-n] => Do not download files, just inventory (for nfs mounts).
# [-b, -i, -s] => Local subdirectory names
# [-I, -S] => Inventory and spool argments for /_get/ url.
# [-m, -r] => max time (secs) and retries for curl
#
# Usage: npemwinbatch update [-q] [-c] [-t unixsecs] [-l logfile]
#    [-e expr] [-f extension] [-n] [-b basedir] [-i invdir] [-s filesdir]
#    [-I invurl] [-S spoolurl] [ -m curlmaxtime] [-r curlretries] <host:port>};
#
# [-t] => download since unix secs
# [-l] => where to save the time (unixsecs) of this run
#
# The other options are passed to npemwinbatch get
#
# Usage: npemwinbatch filter [-q] [-e expr] [-f fext] [-k]
#	[-b basedir] [-i invdir] [-s filesdir]
#	[-p filterpath] [-x paths] -a | <hhmm>};
#
# [-k] => Keep the inventory q file when finished. 
# [-p] => filter (processor) path
# [-x] => paths to add to PATH (default is /usr/local/libexec/npemwin)
# [-a] => process all inventory files in inventory dir.
#
# NOTE: Uses "curl"
#
package require cmdline;

set usage(common) {Usage: npemwinbatch [get|update|filter] <options> <arg>};

set usage(get) {Usage: npemwinbatch get [-q] [-c] [-e expr] [-f extension]
    [-h hhmm] [-n] [-b basedir] [-i invdir] [-s filesdir]
    [ -I invurl] [-S spoolurl] [-m maxtime] [-r retries] <host:port>};

set optlist(get) {q c {e.arg ""} {f.arg ".q"} {h.arg ""} n
    {b.arg "batch"} {i.arg "batchq"} {s.arg "files"}
    {I.arg "inv"} {S.arg "spool"}  {m.arg 10} {r.arg 3}};

set usage(update) {Usage: npemwinbatch update [-q] [-c] [-t unixsecs]
    [-l logfile] [-e expr] [-f extension] [-n]
    [-b basedir] [-i invdir] [-s filesdir]
    [-I invurl] [-S spoolurl] [-m maxtime] [-r retries] <host:port>};

set optlist(update) {q c {t.arg ""} {l.arg "batch.log"} {e.arg ""}
    {f.arg ".q"} n {b.arg "batch"} {i.arg "batchq"} {s.arg "files"}
    {I.arg "inv"} {S.arg "spool"}  {m.arg 10} {r.arg 3}};

set usage(filter) {Usage: npemwinbatch filter [-q] [-e expr] [-f extension] [-k]
    [-b basedir] [-i invdir] [-s spooldir]
    [-p filterpath] [-x paths] -a | <hhmm>};

set optlist(filter) {{q 0} {a 0} {e.arg ""} {f.arg ".q"} {k 0}
    {b.arg "batch"} {i.arg "batchq"} {s.arg "files"}
    {p.arg ""} {x.arg "/usr/local/libexec/npemwin"}};

proc proc_get {} {

    global argv option usage;

    #
    # Check <host:port>
    #
    set argc [llength $argv];
    if {$argc != 1} {
	puts stderr $usage(get);
	return 1;
    }
    append url_host "http://" [lindex $argv 0];

    set url_inv ${url_host}/_get/$option(I);
    set url_spool ${url_host}/_get/$option(S);
    set local_basedir $option(b);

    if {($option(c) == 0) && ([file isdirectory $local_basedir] == 0)} {
	puts stderr "No such directory: $local_basedir.";
	return 1;
    }

    if {$option(h) eq ""} {
	set seconds [expr [clock seconds] - 60];
	set hhmm [clock format $seconds -format "%H%M" -gmt true];
    } else {
	set hhmm $option(h);
    }
    set local_logfile [file join $local_basedir $option(i) ${hhmm}$option(f)];

    set status [catch {
	set flist [split [exec curl -f -s -S -m $option(m) ${url_inv}/${hhmm}] \
		       "\n"];
    } errmsg];
    if {$status != 0} {
	puts stderr $errmsg;
	return 1;
    }

    set local_flist [list];
    foreach f $flist {
	set fparts [split $f];
	set wfo [lindex $fparts 3];
	set fbasename [lindex $fparts 5];
	set fext [string range [file extension $fbasename] 1 end];
	if {$fext eq "txt"} {
	    set ppath [file join ${wfo} ${fbasename}];
	} else {
	    set ppath ${fbasename};
	}

	if {[uwildmat $option(e) $ppath 1] == 0} {
	    continue;
	}

	if {$option(n) != 0} {
	    lappend local_flist $f;
	    continue;
	}

	if {$option(q) == 0} {
	    puts -nonewline "Retrieving ${fbasename} ... ";
	}

	set local_ppath [file join $local_basedir $option(s) $fext $ppath];
	set status [catch {
	    file mkdir [file dirname $local_ppath];
	    proc_curl $local_ppath ${url_spool}/${ppath} \
		$option(m) $option(r) $option(q);
	} errmsg];

	if {$status == 0} {
	    if {$option(q) == 0} {
		puts "done";
	    }
	    # Update the local inventory list
	    lappend local_flist $f;
	} else {
	    puts stderr $errmsg;
	}
    }

    # Write the local inventory file
    file mkdir [file dirname $local_logfile];
    exec echo [join $local_flist "\n"] > $local_logfile;

    return 0;
}

proc proc_update {} {

    global argv option usage;

    #
    # Check <host:port>
    #
    set argc [llength $argv];
    if {$argc == 0} {
	puts stderr $usage(update);
	return 1;
    }
    append hostport [lindex $argv 0];

    set now [clock seconds];
    set hhmm_now [clock format $now -format "%H%M" -gmt true];

    if {$option(l) ne ""} {
	set batch_logfile [file join $option(b) $option(i) $option(l)];
    } else {
	set batch_logfile "";
    }

    if {($option(t) eq "") && ([file exists $batch_logfile] == 0)} {
	set option(h) "";
	set status [proc_get];
	if {$status != 0} {
	    return 1;
	}
	if {$batch_logfile ne ""} {
	    exec echo $now > $batch_logfile;
	}

	return 0;
    } elseif {$option(t) ne ""} {
	set last_time $option(t);
    } else {
	set last_time [string trim [exec cat $batch_logfile]];
    }

    if {$last_time >= $now} {
	puts stderr "Invalid value of -t option or $option(l)";
	return 1;
    }

    set t $last_time;
    set hhmm [clock format $t -format "%H%M" -gmt true];
    while {$hhmm ne $hhmm_now} {
	set option(h) $hhmm;
	set status [proc_get];
	if {$status != 0} {
	    return 1;
	}
	incr t 60;
	set hhmm [clock format $t -format "%H%M" -gmt true];
    }

    if {$option(l) ne ""} {
	exec echo $now > $batch_logfile;
    }
    
    return 0;
}

proc proc_filter {} {

    global argv option usage;
    global env;

    set conflict_a 0;
    set status 0;

    #
    # Check <hhmm>
    #
    set argc [llength $argv];
    if {$argc > 1} {
	puts stderr $usage(filter);
	return 1;
    } elseif {$argc == 1} {
	set hhmm [lindex $argv 0];
	incr conflict_a;
    } else {
	set hhmm "";
    }

    if {$option(a) != 0} {
	incr conflict_a;
    }

    if {$conflict_a != 1} {
	puts stderr $usage(filter);
	return  1;
    }

    if {$hhmm ne ""} {
	set status [proc_filter_logfile $hhmm];
	return $status;
    }

    if {$option(x) ne ""} {
	append env(PATH) ":" $option(x);
    }

    # Process all files in inventory dir.
    foreach local_logfile [glob -nocomplain -directory \
			       [file join $option(b) $option(i)] *$option(f)] {

	set hhmm [file rootname [file tail $local_logfile]];
	set status [proc_filter_logfile $hhmm];
	if {$status != 0} {
	    break;
	}
    }

    return $status;
}

proc proc_filter_logfile {hhmm} {

    global option;

    set local_basedir $option(b);
    set local_logfile [file join $local_basedir $option(i) ${hhmm}$option(f)];

    if {[file exists $local_logfile] == 0} {
	puts stderr "$local_logfile not found.";
	return 1;
    }
    set flist [split [string trimright [exec cat $local_logfile]] "\n"];

    set batch_flist [list];
    foreach f $flist {
	set fparts [split $f];
	set fproductname [lindex $fparts 1];
	set wfo [lindex $fparts 3];
	set fbasename [lindex $fparts end];
	set fext [string range [file extension $fbasename] 1 end];

	if {$fext eq "txt"} {
	    set ppath [file join ${wfo} ${fbasename}];
	} else {
	    set ppath ${fbasename};
	}

	if {[uwildmat $option(e) $ppath 1] == 0} {
	    continue;
	}

	set fpath [file join [pwd] $local_basedir $option(s) $fext $ppath];
	set f [list $fproductname $fpath];
	lappend batch_flist $f;
    }

    set filterpath $option(p);
    if {$filterpath ne ""} {
	exec echo [join $batch_flist "\n"] | $filterpath;
    } else {
	puts [join $batch_flist "\n"];
    }

    if {$option(k) == 0} {
	file delete $local_logfile;
    }

    return 0;
}

proc proc_curl {local_path url_path curl_timeout curl_retries quiet} {

    set status 0;
    set count 0;

    while {$count <= $curl_retries} {
	set status [catch {
	    exec curl -f -s -S -m $curl_timeout -o $local_path $url_path;
	} errmsg];

	if {$status == 0} {
	    break;
	} elseif {$quiet == 0} {
	    puts -nonewline "retrying ..."
	} 
	incr count;
    }

    if {$status != 0} {
	puts stderr $errmsg;
    }

    return $status;
}

# This function is a copy from the nbsp filters library, modified to include
# the last argument.
#
proc uwildmat {uwildregex str ifempty} {
#
# Behaves similarly to inn's uwildmat(), but uses re's rather than
# glob style patters. Using only as pair of expressions, the second
# one of which is negated, is equivalent to use an accept and then reject
# pattern as described in nbspd.conf. This mechanism generalizes that.
#
    if {$uwildregex eq ""} {
	return $ifempty;
    }

    set pattlist [split $uwildregex ","];
    set status 0;

    foreach p $pattlist {
	set v 1;
	set regex $p;
	if {[string range $p 0 0] eq "!"} {
	    set v 0;
	    set regex [string range $p 1 end];
	}
	if {[regexp $regex $str]} {
	    set status $v;
	}
    }

    return $status;
}

#
# main
#
if {$argc == 0} {
    puts stderr $usage(common);
    exit 1;
}
set cmd [lindex $argv 0];
set argv [lreplace $argv 0 0];

if {$cmd eq "get"} {
    array set option [::cmdline::getoptions argv $optlist(get) $usage(get)];
    set status [proc_get];
} elseif {$cmd eq "update"} {
    array set option [::cmdline::getoptions argv $optlist(update) \
			  $usage(update)];
    set status [proc_update];
} elseif {$cmd eq "filter"} {
    array set option [::cmdline::getoptions argv $optlist(filter) \
			  $usage(filter)];
    set status [proc_filter];
} else {
    puts stderr $usage(common);
    exit 1;
}

exit $status;
