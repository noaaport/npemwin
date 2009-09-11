dnl
dnl $Id$
dnl

define(match_file,
lappend cond {
  [regexp {$2} $1]
}
lappend action {
  set rc_status 0;
  filter_file $rc(seq) $rc(fpath) "$3" "$4" a;
})

define(match_and_file,
lappend cond {
  [regexp {$2} $1] && [regexp {$4} $3]
}
lappend action {
  set rc_status 0;
  filter_file $rc(seq) $rc(fpath) "$5" "$6" a;
})

define(matchstop_file,
lappend cond {
  [regexp {$2} $1]
}
lappend action {
  set rc_status 0;
  filter_file $rc(seq) $rc(fpath) "$3" "$4" a;	
  break;
})

define(matchstop_and_file,
lappend cond {
  [regexp {$2} $1] && [regexp {$4} $3]
}
lappend action {
  set rc_status 0;
  filter_file $rc(seq) $rc(fpath) "$5" "$6" a;
  break;
})

define(match_file_noappend,
lappend cond {
  [regexp {$2} $1]
}
lappend action {
  set rc_status 0;
  filter_file $rc(seq) $rc(fpath) "$3" "$4" w;
})

define(matchstop_file_noappend,
lappend cond {
  [regexp {$2} $1]
}
lappend action {
  set rc_status 0;
  filter_file $rc(seq) $rc(fpath) "$3" "$4" w;
  break;
})

define(match_pipe,
if {([info exists gpfilter($3,enable)] == 0) || \
	($gpfilter($3,enable) == 1)} {
  lappend cond {
    [regexp {$2} $1]
  }
  lappend action {
    set rc_status 0;
    filter_pipe $rc(seq) $rc(fpath) "$3" "$4" "$5";
  }
})

define(match_and_pipe,
if {([info exists gpfilter($5,enable)] == 0) || \
	($gpfilter($5,enable) == 1)} {
  lappend cond {
    [regexp {$2} $1] && [regexp {$4} $3]
  }
  lappend action {
    set rc_status 0;
    filter_pipe $rc(seq) $rc(fpath) "$5" "$6" "$7";
  }
})

define(matchstop_pipe,
if {([info exists gpfilter($3,enable)] == 0) || \
	($gpfilter($3,enable) == 1)} {
  lappend cond {
    [regexp {$2} $1]
  }
  lappend action {
    set rc_status 0;
    filter_pipe $rc(seq) $rc(fpath) "$3" "$4" "$5";
    break;
  }
})

define(matchstop_and_pipe,
if {([info exists gpfilter($5,enable)] == 0) || \
	($gpfilter($5,enable) == 1)} {
  lappend cond {
    [regexp {$2} $1] && [regexp {$4} $3]
  }
  lappend action {
    set rc_status 0;
    filter_pipe $rc(seq) $rc(fpath) "$5" "$6" "$7";
    break;
  }
})

define(match_stop,
lappend cond {
  [regexp {$2} $1]
}
lappend action {
  break;
})

define(stopmatch,
lappend cond {
  ($rc_status == 0)
}
lappend action {
  break;
})
