dnl
dnl $Id$
dnl

define(m4stop, break;)dnl

define(m4send_rss,
    send_rss [list $rc_rss_subchannel $rc_rss_channel_desc $rc_rss_template \
	$rc(prodname) $rc(header) $rc(fpath)];
)dnl

define(match_text_one,
lappend cond {
    ($rc(ftype) == "txt") && [regexp {$2} $1]
}
lappend action {
    set rc_status 0;
    set rc_rss_subchannel "$3";
    set rc_rss_channel_desc "$4";
    set rc_rss_template "$5";
    m4send_rss
    $6
})dnl

define(match_text_two,
lappend cond {
    ($rc(ftype) == "txt") && [regexp {$2} $1] && [regexp {$4} $3]
}
lappend action {
    set rc_status 0;
    set rc_rss_subchannel "$5";
    set rc_rss_channel_desc "$6";
    set rc_rss_template "$7";
    m4send_rss
    $8
})dnl

define(match_text_one_except,
lappend cond {
    ($rc(ftype) == "txt") && [regexp {$2} $1] && ![regexp {$4} $3]
}
lappend action {
    set rc_status 0;
    set rc_rss_subchannel "$5";
    set rc_rss_channel_desc "$6";
    set rc_rss_template "$7";
    m4send_rss
    $8
})dnl

define(match_img_one,
lappend cond {
    [regexp {gif|jpg} $rc(ftype)] && [regexp {$2} $1]
}
lappend action {
    set rc_status 0;
    set rc_rss_subchannel "$3";
    set rc_rss_channel_desc "$4";
    set rc_rss_template "$5";
    m4send_rss
    $6
})dnl
