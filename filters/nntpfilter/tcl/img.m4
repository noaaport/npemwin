dnl
dnl $Id$
dnl

# If a sat or rad image (as sent by nbsp) can be identified, it is sent
# to the corresponding noaaport (npemwin) group. The rest are sent to the
# emwin.img group.

# Match a (nbsp) satellite image
match_img_one($rc(prodname) s s1, ^tig(.), sat.img.tig$s1, m4stop)dnl

# Match a (nbsp) radar image.
match_img_one($rc(prodname) s s1 s2 s3 s4 s5 s6,
^(n0(r|s|v|z)|n1(p|r|s|v)|n2(r|s)|n3(r|s)|ncr|nvl|net|ntp|nvw)(.{3}),
rad.img.$s6, m4stop)dnl

# Match the rest (as sent by regular emwin)
match_img_one($rc(prodname), .*, img, m4stop)
