include ../Makefile.inc
include ../VERSION
include pkginfo.mk

pkgname = ${name}-${version}_${release}

default: build

../Makefile: ../Makefile.in
	cd ..; ./configure.sh

build: ../Makefile
	cd ..; make

pkginfo: pkginfo.mk Makefile
	@(echo "PKG=${PKG}"; \
	echo "BASEDIR=${BASEDIR}"; \
	echo "NAME=${NAME}"; \
	echo "VERSION=${VERSION}, issue ${release}";\
	echo "CATEGORY=${CATEGORY}";\
	echo "PSTAMP=${PSTAMP}";\
	echo "EMAIL=${EMAIL}";\
	echo "DESC=${DESC}") > pkginfo

install: build
	rm -r -f pkg ${PKG}
	ginstall -d pkg
	cd pkg  && ginstall -d `cat ../dirs`
	cd ..; make -e DESTDIR=`pwd`/solaris/pkg install

package: install pkginfo
	pkgmk -o -d . -b `pwd`/pkg
	gtar -czf ${pkgname}.tgz ${PKG} 	

clean:
	rm -f -r pkg *~ ${PKG} pkginfo
	cd ..; make clean
