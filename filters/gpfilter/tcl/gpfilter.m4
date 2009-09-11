#
# $Id$
#
dnl 
dnl Definitions and macros (required)
dnl
divert(-1)
include(`defs.m4')
include(`macros.m4')
divert(0)
include(`equivs.m4')dnl

dnl Filter rules
dnl
dnl Put pipes first, and for those products that are processed by
dnl the decoders and also by file.m4, in pipe.m4 we use the
dnl match_pipe() macro (instead of matchstop_pipe).
dnl
include(`pipe.m4')
include(`file.m4')
