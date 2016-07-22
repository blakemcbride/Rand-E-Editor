
# Makefile to produce the RAND e-19 editor
#
#   Must called by the GNU make program (gmake),
#       it uses conditional facility and shell function of GNU make.

# Check the consistancy of the target directory with ./include/localenv.h

# Check the consistancy of :
# --------------------------
#       the version and release with the rpm specification file
#       the curent directory name with ${SRCDIR}

#+++ parameters which can be overwrite by the make call
#       see Rand-editor-E19.58.spec


SHELL := /bin/sh

# get the current directory
PWD := $(shell pwd)
PWD1 := $(shell cd ..; pwd)
DIR1 := $(notdir $(PWD1))
ifeq ($(DIR1), LynxOS)
    DIR1 := ppc
endif

# Get the system identifier
ifndef OS
    OS := $(shell uname -s)
endif

ifeq ($(OS), SunOS)
    OS=Solaris
endif


# defaut builder (when logon as root)
BUILDER=perrioll

VERSION=E19
RELEASE=58

# Default value for package directory
PKGDIR=Rand
TARGETDIR_PREFIX=/ps/local
PROG=e

# Default utilities (must be GNU version)
TAR = gtar
INSTALL=ginstall

ifeq ($(OS), SunOS)
    OS=Solaris
endif

# Installation target system and directory
# ----------------------------------------

TARG_OS := $(OS)


# For cross compilation for Lynx OS, Power PC processor,  on sunps1 with
#   environnement defined by :
#       source /usr/lynx/3.1.0a/ppc/SETUP.csh
ifdef LYNXTARGET
    CC=gcc
    TARG_OS=LynxOS
endif

EXECDIR := $(TARGETDIR_PREFIX)/$(DIR1)
TARGETDIR := $(TARGETDIR_PREFIX)/$(TARG_OS)
TARGETDIRFLG := $(shell if test -d $(TARGETDIR); then echo "OK"; else echo "NO"; fi)
TARGETNAME := $(DIR1)

ifeq ($(TARG_OS), Linux)
    ifeq ($(TARGETDIRFLG), NO)
	TARGETDIR=/usr/local
	EXECDIR=$(TARGETDIR)
	TARGETNAME := Linux
	LINUXVERSION := $(PKGDIR)/Linux_version
    endif
endif

# ifeq ($(TARG_OS), AIX)
#     TARGETDIR=/ps/local/AIX
# endif
#
# ifeq ($(TARG_OS), Solaris)
#     TARGETDIR=/ps/local/Solaris
# endif

ifeq ($(TARG_OS), LynxOS)
    CC=gcc
    TARGETDIR := $(TARGETDIR_PREFIX)/ppc
    EXECDIR=$(TARGETDIR)
    TARGETNAME := ppc
endif

# Utilities to be used
# --------------------

ifeq ($(OS), LynxOS)
    INSTALL=install
    TAR=tar
endif

ifeq ($(OS), Linux)
    INSTALL=install
    TAR=tar
endif

#-----------------------
# notice : now LIBDIR and BINDIR must be the same directory

BINDIR1=$(EXECDIR)/$(PKGDIR)
BINDIR=$(TARGETDIR)/$(PKGDIR)
LIBDIR=$(BINDIR)
KBFDIR=$(BINDIR)/kbfiles
MANE=1
MANDIR=$(TARGETDIR)/man/man$(MANE)

TARPREFIX=$(PKGDIR)-$(VERSION)
TARFILE=$(TARPREFIX).$(RELEASE).tgz
TAREXCL=$(TARPREFIX).*.tgz*
TARALLSPEC=$(PKGDIR)-editor-*.spec
TAREXCLSPEC=$(PKGDIR)-editor-E19.?[!$(RELEASE)].spec
TARSPEC=$(PKGDIR)-editor-$(VERSION).$(RELEASE).spec
ARCHIVE=archv
BINTARFILEPFX=$(TARPREFIX).$(RELEASE).$(TARGETNAME).bin
BINTARFILE=$(TARPREFIX).$(RELEASE).$(TARGETNAME).bin.tgz

SRCDIR=$(PKGDIR)-$(VERSION).$(RELEASE)
SRCDIRFLG := $(shell if test -d ../$(SRCDIR); then echo "OK"; else echo "NO"; fi)
ifeq ($(SRCDIRFLG), NO)
    SRCDIR=$(PKGDIR)-$(VERSION)
endif

ifeq ($(TARG_OS), Linux)
    ifeq ($(TARGETDIRFLG), NO)
	TARGETDIR=/usr/local
    endif
endif

# ----------------------------------------------

all:    e19/le fill/fill
	@echo Ready to deliver for $(TARG_OS)

local:
	@./local.sh

e19/le: e19/e.r.c la1/libla.a ff3/libff.a lib/libtmp.a
	cd e19; $(MAKE) TARG_OS=$(TARG_OS) CC=$(CC)

e19/e.r.c::
	cd e19; ./NewVersion $(VERSION) $(RELEASE) $(PKGDIR) $(TARGETDIR) $(BUILDER);

la1/libla.a::
	cd la1; $(MAKE) TARG_OS=$(TARG_OS) CC=$(CC)

ff3/libff.a::
	cd ff3; $(MAKE) TARG_OS=$(TARG_OS) CC=$(CC)

lib/libtmp.a::
	cd lib; $(MAKE) TARG_OS=$(TARG_OS) CC=$(CC)

fill/fill::
	cd fill; $(MAKE) TARG_OS=$(TARG_OS) CC=$(CC)


.PHONY: tar bintar clean preinstall install depend target test

tar:
	$(MAKE) clean
	cd doc; $(MAKE)
	rm -f ./${TARFILE}.old ../${TARFILE}
	(if test ! -d ./${ARCHIVE}; then mkdir ./${ARCHIVE}; fi)
#       -(mv -f ${TAREXCL} ./${ARCHIVE}; mv ./${ARCHIVE}/${TARFILE} .)
#       mv -f ${TARALLSPEC} ./${ARCHIVE}
#       mv ./${ARCHIVE}/${TARSPEC} .
	(if test -f ./${TARFILE}; then mv -f ./${TARFILE} ./${TARFILE}.old; fi)
	cd ../; $(TAR) \
	 --exclude '*.bak' \
	 --exclude '*.old' --exclude '*.ref' --exclude '*.new' \
	 --exclude '*-old' --exclude '*-ref' --exclude '*-new' \
	 --exclude '*_old' --exclude '*_ref' --exclude '*_new' \
	 --exclude ./${SRCDIR}/archv --exclude ./${SRCDIR}/temp \
	 --exclude './${SRCDIR}/${TAREXCL}' \
	 --exclude './${SRCDIR}/${TAREXCLSPEC}' \
	 -czvf ./${TARFILE} ./${SRCDIR}; \
	 mv ./${TARFILE} ${SRCDIR}; \
	 cd ./${SRCDIR}

ifeq ($(OS), Linux)
#   Special bintar processing for Linux
#       gtar process correctly --exclude only on Linux (new version ?)
bintar:
	@echo "$(TAR) for ${BINTARFILE} in $(PWD)/.."
	(if test ! -d /tmp/${PKGDIR}; then mkdir /tmp/${PKGDIR}; fi)
	rm -f /tmp/$(LINUXVERSION)
	cp -p /proc/version /tmp/$(LINUXVERSION)
	rm -f ../${BINTARFILE}.old
	(if test -f ../${BINTARFILE}; then mv -f ../${BINTARFILE} ../${BINTARFILE}.old; fi)
	cd ${LIBDIR}; cd ../; $(TAR) \
	--exclude './${PKGDIR}/.e?[1-9]' \
	--exclude './${PKGDIR}/.e?[1-9].*' \
	--exclude './${PKGDIR}/,*' \
	--exclude './${PKGDIR}/.nfs*' \
	--exclude './${PKGDIR}/kbfiles/.e?[1-9].*' \
	--exclude './${PKGDIR}/kbfiles/.e?[1-9]*' \
	--exclude './${PKGDIR}/kbfiles/,*' \
	--exclude './${PKGDIR}/kbfiles/*.[0-9]*' \
	--exclude './${PKGDIR}/e19.*' \
	-cvf $(PWD)/../${BINTARFILEPFX}.tar ./${PKGDIR}
	cd ${LIBDIR}; cd ../; $(TAR) \
	-rvf $(PWD)/../${BINTARFILEPFX}.tar ./${PKGDIR}/e19.$(RELEASE)
	cd /tmp; $(TAR) \
	-rvf $(PWD)/../${BINTARFILEPFX}.tar ./$(LINUXVERSION)
	rm -f /tmp/$(LINUXVERSION)
	cd ${PWD}
	rm -f ../${BINTARFILEPFX}.tar.gz
	gzip ../${BINTARFILEPFX}.tar
	mv ../${BINTARFILEPFX}.tar.gz ../${BINTARFILE}

else

bintar:
	@echo -e "$(TAR) for ${BINTARFILE} \n into $(PWD)/.. \n from $(BINDIR1)"
	rm -f ../${BINTARFILE}.old
	(if test -f ../${BINTARFILE}; then mv -f ../${BINTARFILE} ../${BINTARFILE}.old; fi)
	cd ${BINDIR1}; rm -f .e?[1-9] .e?[1-9].* ,*
	cd ${BINDIR1}/kbfiles; rm -f .e?[1-9] .e?[1-9].* ,*
	cd ${BINDIR1}; cd ../; $(TAR) \
	    --exclude './${PKGDIR}/.e?[1-9]' \
	    --exclude './${PKGDIR}/.e?[1-9].*' \
	    --exclude './${PKGDIR}/,*' \
	    --exclude './${PKGDIR}/.nfs*' \
	    --exclude './${PKGDIR}/kbfiles/.e?[1-9].*' \
	    --exclude './${PKGDIR}/kbfiles/.e?[1-9]*' \
	    --exclude './${PKGDIR}/kbfiles/,*' \
	    --exclude './${PKGDIR}/kbfiles/*.[0-9]*' \
	    -cvf $(PWD)/../${BINTARFILEPFX}.tar ./${PKGDIR}
	@cd ${BINDIR1}; \
	    for f in e19.*; do \
		if test $$f != e19.$(RELEASE); then \
		    echo "--delete ./${PKGDIR}/$$f;"; \
		    $(TAR) --delete --file=$(PWD)/../${BINTARFILEPFX}.tar \
			   ./${PKGDIR}/$$f; \
		fi; \
	    done; \
	cd $(PWD)

	rm -f ../${BINTARFILEPFX}.tar.gz
	gzip ../${BINTARFILEPFX}.tar
	mv ../${BINTARFILEPFX}.tar.gz ../${BINTARFILE}

endif


clean:
	rm -f ,* a.out core .e?[1-9] .e?[1-9].*
	for f in fill la1 ff3 lib e19; do cd $$f; rm -f ,* .e?[1-9] .e?[1-9].*; $(MAKE) clean; cd ..; done
	for f in include help doc; do cd $$f; rm -f ,* .e?[1-9] .e?[1-9].*; cd ..; done
	for f in help/kbfiles; do cd $$f; rm -f ,* .e?[1-9] .e?[1-9].*; cd ../..; done
	for f in doc/man; do cd $$f; rm -f ,* .e?[1-9] .e?[1-9].*; cd ../..; done
	for f in contributed; do cd $$f; rm -f ,* .e?[1-9] .e?[1-9].*; cd ../..; done

preinstall:
	-mkdir -p $(MANDIR) $(BINDIR) $(LIBDIR) $(KBFDIR)
	$(INSTALL) -m 444 help/Crashdoc $(LIBDIR)
	$(INSTALL) -m 444 help/errmsg $(LIBDIR)
	$(INSTALL) -m 444 help/recovermsg $(LIBDIR)
	$(INSTALL) -m 444 help/helpkey $(LIBDIR)
	$(INSTALL) -m 444 doc/man/e.l $(MANDIR)/e.$(MANE)
	@/bin/rm -f $(KBFDIR)/universalkb $(KBFDIR)/xtermkb $(KBFDIR)/nxtermkb
	@/bin/rm -f $(KBFDIR)/vt200kbn $(KBFDIR)/linuxkb
	$(INSTALL) -m 444 help/kbfiles/vt200kbn $(KBFDIR)
	$(INSTALL) -m 444 help/kbfiles/linuxkb $(KBFDIR)
	(cd $(KBFDIR); ln -s vt200kbn ./universalkb)
	(cd $(KBFDIR); ln -s vt200kbn ./xtermkb)
	(cd $(KBFDIR); ln -s vt200kbn ./nxtermkb)

install:
	@-/bin/rm -f $(TARGETDIR)/bin/e19
	@/bin/rm -f $(LIBDIR)/e19 $(LIBDIR)/fill $(LIBDIR)/run $(LIBDIR)/center $(LIBDIR)/just
	$(INSTALL) --strip e19/a.out $(LIBDIR)/e19.$(RELEASE)
	$(INSTALL) --strip fill/fill fill/run fill/center $(LIBDIR)
	(cd $(LIBDIR); ln -s fill ./just)
	(cd $(LIBDIR); ln -s ./e19.$(RELEASE) ./e19)
	-(cd $(TARGETDIR)/bin; ln -s ../$(PKGDIR)/e19 ./$(PROG))

depend:
	for f in fill la1 ff3 lib e19; do cd $$f; $(MAKE) TARG_OS=$(TARG_OS) depend; cd ..; done


target:
	@echo
	@echo "Target system is assumed to be : $(TARG_OS)"
	@echo "    installation directory : $(BINDIR)"
	@echo

test:
	@echo -e "\n\nState for $(OS) platform, version $(VERSION).$(RELEASE)"
	@echo -e     "========================================\n"
	@echo "shell is $(SHELL)"
	@echo "MAKE is $(MAKE)"
	@echo "TAR is $(TAR)"
	@echo "CC is $(CC)"
	@echo "SRCDIR is $(SRCDIR) , current dir is $(PWD)"
	@echo "TARGETDIR is $(TARGETDIR)"
	@echo "TARGETDIRFLG is $(TARGETDIRFLG)"
	@echo -e "  bin tar file : $(BINTARFILE)\n\n"
	@echo "/tmp/LINUXVERSION is /tmp/$(LINUXVERSION)"

	cd e19; $(MAKE) TARG_OS=$(TARG_OS) test


.PHONY: truc

truc:
	@echo "shell is $(SHELL)"
	@a=`which e`; \
#        if test -L $$a; then b=`readlink $$a`; fi; \
#        echo "e is : \"$$a\" --"; echo a link to $$b
	@echo "LIBDIR is -$(LIBDIR)-"
	@cd $(BINDIR1); for f in e19.*; do \
	    if test $$f = e19.$(RELEASE); then \
		echo ++$$f++; ls -l $$f; \
	    else echo $$f; \
	    fi; \
	done
	@echo -e "PWD1 id $(PWD1) \nTARGETNAME is $(TARGETNAME)"
	@echo "BINDIR1 is $(BINDIR1)"
	@echo "BINTARFILE is $(BINTARFILE)"
	rm -f /tmp/list; gcc -v &>/tmp/list
	@cat /tmp/list
	rm -f /tmp/list
