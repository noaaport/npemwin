#
# $Id$
#
include ../../VERSION

# The defaults in spec.in are for SL, and the overrides are other
# flavors are below.
#
requires = $(shell cat rpm-requires)
rpmrelease = $(shell cat rpm-release)
rpmroot = ${HOME}/rpmbuild

flavor = $(shell ./flavor.sh)
arch =  $(shell uname -m)
dist = $(shell ./dist.sh)

ifeq (${flavor}, opensuse)
  ifeq (${arch}, i386)
    arch = i586
  endif
endif

tgzfile = ${name}-${version}.tgz
rpmfile = ${name}-${version}-${rpmrelease}${dist}.${arch}.rpm

defauls: package

spec: rpm-spec
tgz: ${tgzfile}

${tgzfile}:
	cd ../../..;\
	cp -r ${name} ${name}-${version};\
	tar -czf ${name}-${version}.tgz ${name}-${version};\
	rm -rf ${name}-${version}
	mv ../../../${name}-${version}.tgz .

rpm-spec: rpm-spec.in
	sed -e "/@name@/s//${name}/" \
	-e "/@version@/s//${version}/" \
	-e "/@rpmrelease@/s//${rpmrelease}/" \
	-e "/@requires@/s//${requires}/" \
	-e "/@postinstall@/r postinstall" \
	-e "/@postinstall@/d" \
	-e "/@postuninstall@/r postuninstall" \
	-e "/@postuninstall@/d" \
	-e "/@plist@/r rpm-plist" \
	-e "/@plist@/d" \
	rpm-spec.in > rpm-spec

package: rpm-spec ${tgzfile}
	mv ${tgzfile} ${rpmroot}/SOURCES
	rpmbuild -bb rpm-spec
	cp ${rpmroot}/RPMS/${arch}/${rpmfile} .

clean:
	rm -f rpm-spec ${tgzfile} ${rpmfile}
