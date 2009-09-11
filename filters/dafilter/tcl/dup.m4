dnl
dnl $Id$
dnl

#
# Subsets of upperair that DA can import
#
# Pireps (rc(nawips) = pirep)
match_file($rc(wmoid), ^u[ab], pirep, ${ymdh}.airep)

match_file($rc(wmoid), ^ud, acars, ${ymdh}.amdar)
match_and_file($rc(wmoid), ^u[efklms], $rc(nawips), ^tt(aa|bb|cc|dd),
upperair, ${ymdh}.fm35)

# 
# Duplicates of warnings for grlevelx programs
#
match_or_file($rc(awips1),
cem|cfw|ffw|fls|flw|hls|hwo|npw|rfw|sps|svr|svs|tor|wsw,
$rc(body), EAS ACTIVATION, warnings, $ymdh.$rc(AWIPS1))

#
# Duplicate of nhc hurricane-related products.
#
match_file($rc(awips1), tcm, hurricane/track, $yyyy.$rc(awips))
match_file_noappend($rc(awips1), tcm|tcp|tcd|pws,
hurricane/forecast, $rc(awips).txt)

match_file($rc(awips1), chg, hurricane/model, $yyyy.$rc(awips))
match_file_noappend($rc(awips1), chg, hurricane/model, $rc(awips).txt)

#
# Hazcollect
#
match_file($rc(wmoid), wousii|woakii|woca42|wohw40|wogm40|wozs40,
hazcollect, ${ymdh}.$rc(awips1))

#
# Standard rules
#
