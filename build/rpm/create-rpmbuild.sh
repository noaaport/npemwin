#!/bin/sh

# topdir="$HOME/devel/rpmbuild"

#
# for d in BUILD RPMS/i386 RPMS/i686 RPMS/noarch SOURCES SPECS SRPMS tmp
# do
#    mkdir -p $topdir/$d
# done
#

# yum install rpmdevtools

# echo "%_topdir $topdir" > $HOME/.rpmmacros
# echo "%_tmppath $topdir/tmp" >> $HOME/.rpmmacros

# or simply. In any case, revise definition of rpmroot inMakefile
rpmdev-setuptree
