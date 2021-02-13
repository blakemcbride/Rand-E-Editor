#
#       MAKEFILE FOR RAND "LA" LINE-ACCESS I/O PACKAGE
#
ARFLAGS = -r
# CFLAGS = -O -I. -I../include
CFLAGS = -I. -I../include -g

ALL=libtmp.a

OBJS=   getlong.o getshort.o intss.o putlong.o putshort.o \
	delay.o  empty.o   fatal.o

SRCS=   getlong.c getshort.c intss.c putlong.c putshort.c \
	delay.c  empty.c   fatal.c

$(ALL)  : $(OBJS)
	$(AR) $(ARFLAGS) $@ $(OBJS)

clean:
	rm -f $(OBJS) $(ALL)  ,* .e?[1-9] .e?[1-9].* core *.bak *.BAK *~ \#*

depend:
	makedepend -- $(CFLAGS) -- $(SRCS)

# DO NOT DELETE THIS LINE -- make depend depends on it.

getlong.o: /usr/include/stdio.h /usr/include/features.h
getlong.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
getlong.o: /usr/include/bits/types.h
getlong.o: /usr/include/wchar.h
getlong.o: /usr/include/bits/wchar.h /usr/include/gconv.h
getlong.o: /usr/include/bits/stdio_lim.h
getshort.o: /usr/include/stdio.h /usr/include/features.h
getshort.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
getshort.o: /usr/include/bits/types.h
getshort.o: /usr/include/wchar.h
getshort.o: /usr/include/bits/wchar.h /usr/include/gconv.h
getshort.o: /usr/include/bits/stdio_lim.h
putlong.o: /usr/include/stdio.h /usr/include/features.h
putlong.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
putlong.o: /usr/include/bits/types.h
putlong.o: /usr/include/wchar.h
putlong.o: /usr/include/bits/wchar.h /usr/include/gconv.h
putlong.o: /usr/include/bits/stdio_lim.h ../include/c_env.h
putshort.o: /usr/include/stdio.h /usr/include/features.h
putshort.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
putshort.o: /usr/include/bits/types.h
putshort.o: /usr/include/wchar.h
putshort.o: /usr/include/bits/wchar.h /usr/include/gconv.h
putshort.o: /usr/include/bits/stdio_lim.h ../include/c_env.h
empty.o: ../include/localenv.h /usr/include/stdio.h /usr/include/features.h
empty.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
empty.o: /usr/include/bits/types.h
empty.o: /usr/include/wchar.h
empty.o: /usr/include/bits/wchar.h /usr/include/gconv.h
empty.o: /usr/include/bits/stdio_lim.h /usr/include/string.h
empty.o: /usr/include/bits/ioctl-types.h /usr/include/sys/ttydefaults.h
