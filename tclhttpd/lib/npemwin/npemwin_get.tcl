#
# $Id$
#
# The function npemwin_get implements the domain handler
# 
# http://<host>:8016/_get/spool/<pname>,
# http://<host>:8016/_get/inv/<hhmm>,
# http://<host>:8016/_get/rss/<rsschannel>,
#
# which can be used to request file transfers after a panfilter notification,
# or periodically request the latest minutely inventory file.
# The transfered file is the raw file that the qrunner saves in the "files"
# directory. For text files, <pname> must of the form <station>/<emwinfname>;
# e.g., tjsj/zfpspnpr.1202930279980022.txt. For non-text files, the
# "<station>/" must be omitted. For the inventory files, <hhmm> should
# not contain the ".log" extension.
#
# The function npemwin_query implements
#
#  http://<host>:8016/_query/spool?type=gif|jpg&select=<glob pattern>
#  http://<host>:8016/_query/spool?type=txt&select=<kkkk>/<glob pattern>
#
# An additional argument can be passed as "&format=xml" to return an xml
# formatted list.
Direct_Url /_query npemwin_query
Url_PrefixInstall /_get [list npemwin_get /_get]

proc npemwin_get {prefix sock suffix} {
#
# The requests are of the form "_get/<subdir>/<name>". Here <name>
# can contain subdirectories as well.
#
    global Config;

    # Check that there are no "../" constructs, and then go past the "/spool".
    if {[regexp {\.{2}/} $suffix]} {
        Httpd_Error $sock 400 "Invalid name $suffix.";
	return;
    }

    if {[regexp {^(/spool/)(.+)} $suffix match s1 s2]} {
	# $suffix contains "<name>" without the leading "/"
	set suffix $s2;
	npemwin_get_spool $sock $suffix;
    } elseif {[regexp {^(/inv/)(.+)} $suffix match s1 s2]} {
	set suffix $s2;
	npemwin_get_inv $sock $suffix;
    } elseif {[regexp {^(/rss/)(.+)} $suffix match s1 s2]} {
	set suffix $s2;
	npemwin_get_rss $sock $suffix;	
    } else {
        Httpd_Error $sock 400 "Invalid specification.";
    }
}

proc npemwin_get_spool {sock suffix} {

    global Config;

    set type(txt) "text/plain";
    set type(gif) "image/gif";
    set type(jpg) "image/jpg";

    set pname $suffix;
    set subdir [string range [file extension $pname] 1 end];
    set fpath [file join $Config(npemwinfilesdir) $subdir $pname];

    if {[file exists $fpath] == 0} {
	Httpd_Error $sock 404 "$pname not found.";
	return;
    }

    set fext [string range [file extension $pname] 1 end];
    if {($fext eq "txt") || ($fext eq "gif") || ($fext eq "jpg")} {
	set mtype $type($fext);
    } else {
	set mtype "application/octet-stream";
    }

    Httpd_ReturnFile $sock $mtype $fpath;
}

proc npemwin_get_inv {sock suffix} {

    global Config;

    # The suffix does not include the leading slash, and it does _not_
    # include the extension of the inventory file.

    set pname "";
    append pname $suffix $Config(npemwininvfext);
    set fpath [file join $Config(npemwininvdir) $pname];
    set type "text/plain";

    if {[file exists $fpath] == 0} {
	Httpd_Error $sock 404 "$pname not found.";
	return;
    }

    Httpd_ReturnFile $sock $type $fpath;
}

proc npemwin_get_rss {sock suffix} {

    global Config;

    # The suffix does not include the leading slash, and it does _not_
    # include the extension of the rss file.

    set pname "";
    append pname $suffix $Config(npemwinrssfext);
    set fpath [file join $Config(npemwinrssdir) $pname];
    set type "text/xml";

    if {[file exists $fpath] == 0} {
	Httpd_Error $sock 404 "$pname not found.";
	return;
    }

    Httpd_ReturnFile $sock $type $fpath;
}

proc npemwin_query/spool {type select {format "csv"}} {

    global Config;

    set subdir $type;

    # The mechanism used to return content types is to set a global
    # variable with the same name as this function.
    set npemwin_query/spool "text/plain";

    if {$format eq "xml"} {
	set listtype "filelist";
	set r [npemwin_query_output_xml_start $listtype];
	set npemwin_query/spool "text/xml";
    } else {
	set r "";
    }

    set d [file join $Config(npemwinfilesdir) $subdir];
    if {[file isdirectory $d] == 0} {
	if {$format eq "xml"} {
	    append r [npemwin_query_output_xml_end $listtype];
	}
	return $r;
    }

    set filelist [glob -directory $d -nocomplain -tails $select];
    foreach f $filelist {
	if {$format eq "xml"} {
	    append r "<fname>" $f "</fname>\n";
	} else {
	    append r $f "\n";
	}
    }
    if {$format eq "xml"} {
	append r [npemwin_query_output_xml_end $listtype];
    }
    return $r;
}

proc npemwin_query_output_xml_start {list_type} {

    set r "<?spooler_${list_type} version=\"1.0\"?>\n\n";
    append r "<spooler_${list_type}>\n";

    return $r;
}

proc npemwin_query_output_xml_end {list_type} {

    set r "</spooler_${list_type}>\n";

    return $r;
}
