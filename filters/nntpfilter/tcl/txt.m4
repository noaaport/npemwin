dnl
dnl $Id$
dnl

define(m4wmoid, $rc(header_wmoid))dnl
define(m4station, $rc(header_station))dnl

match_text_one(m4wmoid, ^n, misc.adm)
match_text_one($rc(bodypart), URGENT, urgent)
match_text_one(m4wmoid, ^[crstu], data)
match_text_one(m4wmoid, ^a, summary)
match_text_one(m4wmoid, ^f, forecast)
match_text_one(m4wmoid, ^w, warnings)

match_text_one(m4wmoid, .+, txt.m4station)
match_text_one($rc(bodypart), URGENT, urgent.m4station)
match_text_one(m4wmoid, ^[crstu], data.m4station, m4stop)
match_text_one(m4wmoid, ^a, summary.m4station, m4stop)
match_text_one(m4wmoid, ^f, forecast.m4station, m4stop)
match_text_one(m4wmoid, ^w, warnings.m4station, m4stop)
