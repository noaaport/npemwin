dnl
dnl $Id$
dnl

#
# Subsets of upperair that DA can import
#
# Pireps (rc(nawips) = pirep)
match_file($rc(wmoid), ^u[ab], pirep, ${ymdh}.airep)

match_file($rc(wmoid), ^ud, acars, ${ymdh}.amdar)
dnl match_and_file($rc(wmoid), ^u[efklms], $rc(nawips), ^tt(aa|bb|cc|dd),
dnl upperair, ${ymdh}.fm35)
match_and_file($rc(wmoid), ^u[efklms], $rc(awips1), ^air,
upperair, ${ymdh}.fm35)

# 
# Duplicates of warnings for grlevelx programs
#
dnl match_or_file($rc(awips1),
dnl cem|cfw|ffw|fls|flw|hls|hwo|npw|rfw|sps|svr|svs|tor|wsw,
dnl $rc(body), EAS ACTIVATION, warnings, $ymdh.$rc(AWIPS1))
dnl
match_file($rc(awips1),
svr|tor|ffw|svs|smw|mws,
warnings, ${ymdh}.$rc(AWIPS1))

#
# The new GR scheme
#
match_file($rc(awips1),
svr|tor|ffw|svs|smw|mws,
warnings, warnings_${ymd_h}.txt)

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
