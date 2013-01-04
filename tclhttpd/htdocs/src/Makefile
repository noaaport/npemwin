BUILDDIR =	pub

CSSDIR	=	css
IMGDIR =	images
MOBILEDIR =	m
SUBDIRS =	${CSSDIR} ${IMGDIR} ${MOBILEDIR}

all:
	./mk-tml.sh ${BUILDDIR}
	cp -r ${SUBDIRS} ${BUILDDIR}

clean:
	rm -f *~
	rm -rf ${BUILDDIR}
