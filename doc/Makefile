# Rand Editor documentation
#
#   Makefile for RAND documentation : produce a html version of the man page
#

ALL=Rand-E19_man.html
all: $(ALL)

Rand-E19_man.html: man/e.l
	man2html $? > $@

clean:
	rm -f $(ALL) ,* .e?[1-9] .e?[1-9].* core *.bak *.BAK
