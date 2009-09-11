#
# $Id$
#
# Used as
#
#	awk -f default.awk default.list
#
# to create the defailt collective.

END {
  print "";
}

NR == 1 {
  printf("%s", tolower($1));
}

NR != 1 {
  printf("|%s", tolower($1));
}
