dnl
dnl $Id$
dnl

# This comment, taken from the file daequivs.m4 file, apply here as well.
# The dafilter rules are borrowed from the nbsp dafilter, and some
# of the rc variables have different names between the two programs.
# These definitions set the equivalence between them. Since the
# npemwin rc(seqnumber) is not a number (but a "key"), we define
# a usable (but irrelevant seqnumber here). And also define a suitable
# rc(fname) variable with an nbsp-compatible "fname".

lappend cond {1}
lappend action {
  set rc(body) $rc(bodypart);
  set rc(wmoid) $rc(header_wmoid);
  set rc(station) $rc(header_station);
  set rc(seq) [clock clicks];
  set rc(fname) $rc(station)_$rc(wmoid)-$rc(awips);
}
