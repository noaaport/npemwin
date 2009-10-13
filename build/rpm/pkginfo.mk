#
# $Id$
#

# This is read by the (rpm) makefile to produce the <name>-<version>.spec
# file. The defaults are for FC, and the overrides are other flavors are below.
FLAVOR = $(shell ./flavor.sh)

package_build = 1
#rpmroot = /usr/local/src/redhat
rpmroot = /usr/src/redhat

# empty variables will be filled by make
Summary =  Noaaport Emwin server.
Name = ${name}
Version = ${version}
Release = ${package_build}
License =  BSD
Group =  Applications/Internet
Source =  ftp://www.noaaport.net/software/${pkgsrc_name}/src/${pkgsrc_name}.tgz
BuildRoot = ${rpmroot}/BUILD/${pkgsrc_name}/build/rpm/pkg
Requires = tcp_wrappers-libs db4 tcl tcllib gnuplot unzip

ifeq (${FLAVOR}, opensuse)
rpmroot = /usr/src/packages
Requires = tcpd libdb_4_5 tcl tcllib gnuplot unzip
endif

ifeq (${FLAVOR}, centos)
Requires = tcp_wrappers tcl tcllib gnuplot unzip
endif
