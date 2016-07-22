#
#	Copyright abandoned, 1983, The Rand Corporation
#

# Rand Editor with LA package
#
#       MAKEFILE FOR RAND "LA" LINE-ACCESS I/O PACKAGE
#
CFLAGS = -O -I. -I../include

ALL=center fill run
all: $(ALL)
center: center.c
	$(CC) $(CFLAGS) -o $@ $@.c

fill: fill.c
	$(CC) $(CFLAGS) -o $@ $@.c

run: run.c
	$(CC) $(CFLAGS) -o $@ $@.c

clean:
	rm -f $(ALL) ,* .e?[1-9] .e?[1-9].* core *.bak *.BAK

depend:
	makedepend -- $(CFLAGS) -- center.c fill.c run.c

# DO NOT DELETE THIS LINE -- make depend depends on it.

center.o: /usr/include/stdio.h /usr/include/features.h
center.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
center.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
center.o: /usr/include/bits/types.h /usr/include/libio.h
center.o: /usr/include/_G_config.h /usr/include/wchar.h
center.o: /usr/include/bits/wchar.h /usr/include/gconv.h
center.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
center.o: /usr/include/bits/stdio_lim.h /usr/include/ctype.h
center.o: /usr/include/endian.h /usr/include/bits/endian.h
center.o: /usr/include/sys/types.h /usr/include/time.h
center.o: /usr/include/sys/stat.h /usr/include/bits/stat.h
fill.o: /usr/include/stdio.h /usr/include/features.h /usr/include/sys/cdefs.h
fill.o: /usr/include/gnu/stubs.h
fill.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
fill.o: /usr/include/bits/types.h /usr/include/libio.h
fill.o: /usr/include/_G_config.h /usr/include/wchar.h
fill.o: /usr/include/bits/wchar.h /usr/include/gconv.h
fill.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
fill.o: /usr/include/bits/stdio_lim.h /usr/include/ctype.h
fill.o: /usr/include/endian.h /usr/include/bits/endian.h
fill.o: /usr/include/sys/types.h /usr/include/time.h /usr/include/sys/stat.h
fill.o: /usr/include/bits/stat.h
run.o: /usr/include/stdio.h /usr/include/features.h /usr/include/sys/cdefs.h
run.o: /usr/include/gnu/stubs.h
run.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
run.o: /usr/include/bits/types.h /usr/include/libio.h
run.o: /usr/include/_G_config.h /usr/include/wchar.h
run.o: /usr/include/bits/wchar.h /usr/include/gconv.h
run.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
run.o: /usr/include/bits/stdio_lim.h
