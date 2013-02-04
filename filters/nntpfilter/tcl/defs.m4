dnl
dnl $Id$
dnl

define(m4stop, break;)dnl

dnl
dnl define(m4set_text_subject,
dnl    set rc_nntp_subject "";
dnl    append rc_nntp_subject $rc(header_station) "_" \
dnl	$rc(header_wmoid) "_" $rc(header_time) "_" $rc(prodname);
dnl )
dnl

define(m4set_text_subject,
    set rc_nntp_subject [subst $nntpfilter(txt_subject)];
)dnl

define(m4set_img_subject,
    set rc_nntp_subject [subst $nntpfilter(img_subject)];
)dnl

define(m4send_nntp,
    send_nntp $rc(fpath) $rc_nntp_subgroup $rc_nntp_subject $rc_nntp_encode;
)dnl

define(match_text_one,
lappend cond {
    ($rc(ftype) eq "txt") && [regexp {$2} $1]
}
lappend action {
    set rc_status 0;
    set rc_nntp_subgroup $3;
    set rc_nntp_encode  0;
    m4set_text_subject
    m4send_nntp
    $4
})dnl

define(match_text_two,
lappend cond {
    ($rc(ftype) eq "txt") && [regexp {$2} $1] && [regexp {$4} $3]
}
lappend action {
    set rc_status 0;
    set rc_nntp_subgroup $5;
    set rc_nntp_encode  0;
    m4set_text_subject
    m4send_nntp
    $6
})dnl

define(match_text_one_except,
lappend cond {
    ($rc(ftype) eq "txt") && [regexp {$2} $1] && ![regexp {$4} $3]
}
lappend action {
    set rc_status 0;
    set rc_nntp_subgroup $5;
    set rc_nntp_encode  0;
    m4set_text_subject
    m4send_nntp
    $6
})dnl

define(match_img_one,
lappend cond {
    [regexp {gif|jpg} $rc(ftype)] && [regexp {$2} $1]
}
lappend action {
    set rc_status 0;
    set rc_nntp_subgroup $3;
    set rc_nntp_encode  1;
    m4set_img_subject
    m4send_nntp
    $4
})dnl
