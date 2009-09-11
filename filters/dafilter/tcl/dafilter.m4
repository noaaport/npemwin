#
# $Id$
#
dnl 
dnl Definitions and macros (required)
dnl
divert(-1)
include(`defs.m4')
divert(0)
include(`equivs.m4')dnl

dnl Filter rules
dnl
dnl Put first the duplicates for special applicatiobns (e.g.,
dnl grlevel warnings, hurricanes), then the "standard" ones.
dnl 
dnl Put pipes first, and for those products that are processed by
dnl the decoders and also by file.m4, in pipe.m4 we use the
dnl match_file() macro instead of matchstop_file.

include(`dup.m4')
include(`pipe.m4')
include(`file.m4')

`# This will store in the subdirectory "unprocessed" all the files that are
# not processed by any of the rules.
#
# if {$rc_status == 1} {
#    proc_file $rc(seq) $rc(fpath) "unprocessed" "$ymdh.$rc(fname).$rc(seq)" w;
#}'
