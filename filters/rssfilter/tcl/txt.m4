dnl
dnl $Id$
dnl

define(m4wmoid, $rc(header_wmoid))dnl
define(m4station, $rc(header_station))dnl
define(m4body, $rc(bodypart))dnl

dnl match_text_one(m4wmoid, ^n, emwin.misc.adm)
dnl match_text_one(m4body, URGENT, emwin.urgent)
dnl match_text_one(m4wmoid, ^[crstu], emwin.data)
dnl match_text_one(m4wmoid, ^a, emwin.summary)
dnl match_text_one(m4wmoid, ^f, emwin.forecast)
dnl match_text_one(m4wmoid, ^w, emwin.warnings)

match_text_one(m4wmoid, .+, emwin.txt.m4station, All, st)
match_text_one(m4body, URGENT, emwin.urgent.m4station, Urgent, st)
match_text_one(m4wmoid, ^[crstu], emwin.data.m4station, Data, st, m4stop)
match_text_one(m4wmoid, ^a, emwin.summary.m4station, Summary, st, m4stop)
match_text_one(m4wmoid, ^f, emwin.forecast.m4station, Forecast, st, m4stop)
match_text_one(m4wmoid, ^w, emwin.warnings.m4station, Warnings, st, m4stop)
