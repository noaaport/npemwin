dnl
dnl $Id$
dnl

define(match_file,
lappend cond {
    [regexp {$2} $1]
}
lappend action {
    set rc_status 0;
    proc_file $rc(seq) $rc(fpath) "$3" "$4" a;
})

define(match_and_file,
lappend cond {
    [regexp {$2} $1] && [regexp {$4} $3]
}
lappend action {
    set rc_status 0;
    proc_file $rc(seq) $rc(fpath) "$5" "$6" a;
})

define(match_or_file,
lappend cond {
    [regexp {$2} $1] || [regexp {$4} $3]
}
lappend action {
    set rc_status 0;
    proc_file $rc(seq) $rc(fpath) "$5" "$6" a;
})

define(matchstop_file,
lappend cond {
    [regexp {$2} $1]
}
lappend action {
    set rc_status 0;
    proc_file $rc(seq) $rc(fpath) "$3" "$4" a;
    break;
})

define(matchstop_and_file,
lappend cond {
    [regexp {$2} $1] && [regexp {$4} $3]
}
lappend action {
    set rc_status 0;
    proc_file $rc(seq) $rc(fpath) "$5" "$6" a;
    break;
})

define(matchstop_or_file,
lappend cond {
    [regexp {$2} $1] || [regexp {$4} $3]
}
lappend action {
    set rc_status 0;
    proc_file $rc(seq) $rc(fpath) "$5" "$6" a;
    break;
})

define(match_file_noappend,
lappend cond {
    [regexp {$2} $1]
}
lappend action {
    set rc_status 0;
    proc_file $rc(seq) $rc(fpath) "$3" "$4" w;
})

define(matchstop_file_noappend,
lappend cond {
    [regexp {$2} $1]
}
lappend action {
    set rc_status 0;
    proc_file $rc(seq) $rc(fpath) "$3" "$4" w;
    break;
})

define(match_stop,
lappend cond {
    [regexp {$2} $1]
}
lappend action {
    break;
})
