#
#       MAKEFILE FOR RAND "LA" LINE-ACCESS I/O PACKAGE
#
ARFLAGS = -r
# CFLAGS = -O -I. -I../include
CFLAGS = -I. -I../include -g

ALL=libff.a

OBJS=   open.o  close.o fd.o   free.o getc.o grow.o  pos.o \
	putc.o  read.o  seek.o size.o sync.o flush.o sort.o \
	write.o point.o move.o fill.o debug.o

SRCS=   open.c  close.c fd.c   free.c getc.c grow.c  pos.c \
	putc.c  read.c  seek.c size.c sync.c flush.c sort.c \
	write.c point.c move.c fill.c debug.c


$(ALL) : $(OBJS)
	$(AR) $(ARFLAGS) $@ $(OBJS)

clean:
	rm -f $(OBJS) $(ALL)  ,* .e?[1-9] .e?[1-9].* core *.bak *.BAK *~ \#*

depend:
	makedepend -- $(CFLAGS) -- $(SRCS)

# DO NOT DELETE THIS LINE -- make depend depends on it.

open.o: ff.local.h ../include/c_env.h ../include/localenv.h
open.o: /usr/include/stdio.h /usr/include/features.h /usr/include/sys/cdefs.h
open.o: /usr/include/gnu/stubs.h
open.o: /usr/include/bits/types.h
open.o: /usr/include/wchar.h
open.o: /usr/include/bits/wchar.h /usr/include/gconv.h
open.o: /usr/include/bits/stdio_lim.h /usr/include/string.h
open.o: /usr/include/sys/types.h /usr/include/time.h ff.h
open.o: /usr/include/limits.h
open.o: /usr/include/sys/stat.h /usr/include/bits/stat.h
close.o: ff.local.h ../include/c_env.h ../include/localenv.h
close.o: /usr/include/stdio.h /usr/include/features.h
close.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
close.o: /usr/include/bits/types.h
close.o: /usr/include/wchar.h
close.o: /usr/include/bits/wchar.h /usr/include/gconv.h
close.o: /usr/include/bits/stdio_lim.h /usr/include/string.h
close.o: /usr/include/sys/types.h /usr/include/time.h ff.h
close.o: /usr/include/limits.h
fd.o: ff.local.h ../include/c_env.h ../include/localenv.h
fd.o: /usr/include/stdio.h /usr/include/features.h /usr/include/sys/cdefs.h
fd.o: /usr/include/gnu/stubs.h
fd.o: /usr/include/bits/types.h
fd.o: /usr/include/wchar.h /usr/include/bits/wchar.h /usr/include/gconv.h
fd.o: /usr/include/bits/stdio_lim.h /usr/include/string.h
fd.o: /usr/include/sys/types.h /usr/include/time.h ff.h /usr/include/limits.h
free.o: ff.local.h ../include/c_env.h ../include/localenv.h
free.o: /usr/include/stdio.h /usr/include/features.h /usr/include/sys/cdefs.h
free.o: /usr/include/gnu/stubs.h
free.o: /usr/include/bits/types.h
free.o: /usr/include/wchar.h
free.o: /usr/include/bits/wchar.h /usr/include/gconv.h
free.o: /usr/include/bits/stdio_lim.h /usr/include/string.h
free.o: /usr/include/sys/types.h /usr/include/time.h ff.h
free.o: /usr/include/limits.h
getc.o: ff.local.h ../include/c_env.h ../include/localenv.h
getc.o: /usr/include/stdio.h /usr/include/features.h /usr/include/sys/cdefs.h
getc.o: /usr/include/gnu/stubs.h
getc.o: /usr/include/bits/types.h
getc.o: /usr/include/wchar.h
getc.o: /usr/include/bits/wchar.h /usr/include/gconv.h
getc.o: /usr/include/bits/stdio_lim.h /usr/include/string.h
getc.o: /usr/include/sys/types.h /usr/include/time.h ff.h
getc.o: /usr/include/limits.h
grow.o: ff.local.h ../include/c_env.h ../include/localenv.h
grow.o: /usr/include/stdio.h /usr/include/features.h /usr/include/sys/cdefs.h
grow.o: /usr/include/gnu/stubs.h
grow.o: /usr/include/bits/types.h
grow.o: /usr/include/wchar.h
grow.o: /usr/include/bits/wchar.h /usr/include/gconv.h
grow.o: /usr/include/bits/stdio_lim.h /usr/include/string.h
grow.o: /usr/include/sys/types.h /usr/include/time.h ff.h
grow.o: /usr/include/limits.h
pos.o: ff.local.h ../include/c_env.h ../include/localenv.h
pos.o: /usr/include/stdio.h /usr/include/features.h /usr/include/sys/cdefs.h
pos.o: /usr/include/gnu/stubs.h
pos.o: /usr/include/bits/types.h
pos.o: /usr/include/wchar.h
pos.o: /usr/include/bits/wchar.h /usr/include/gconv.h
pos.o: /usr/include/bits/stdio_lim.h /usr/include/string.h
pos.o: /usr/include/sys/types.h /usr/include/time.h ff.h
pos.o: /usr/include/limits.h
putc.o: ff.local.h ../include/c_env.h ../include/localenv.h
putc.o: /usr/include/stdio.h /usr/include/features.h /usr/include/sys/cdefs.h
putc.o: /usr/include/gnu/stubs.h
putc.o: /usr/include/bits/types.h
putc.o: /usr/include/wchar.h
putc.o: /usr/include/bits/wchar.h /usr/include/gconv.h
putc.o: /usr/include/bits/stdio_lim.h /usr/include/string.h
putc.o: /usr/include/sys/types.h /usr/include/time.h ff.h
putc.o: /usr/include/limits.h
read.o: ff.local.h ../include/c_env.h ../include/localenv.h
read.o: /usr/include/stdio.h /usr/include/features.h /usr/include/sys/cdefs.h
read.o: /usr/include/gnu/stubs.h
read.o: /usr/include/bits/types.h
read.o: /usr/include/wchar.h
read.o: /usr/include/bits/wchar.h /usr/include/gconv.h
read.o: /usr/include/bits/stdio_lim.h /usr/include/string.h
read.o: /usr/include/sys/types.h /usr/include/time.h ff.h
read.o: /usr/include/limits.h
seek.o: ff.local.h ../include/c_env.h ../include/localenv.h
seek.o: /usr/include/stdio.h /usr/include/features.h /usr/include/sys/cdefs.h
seek.o: /usr/include/gnu/stubs.h
seek.o: /usr/include/bits/types.h
seek.o: /usr/include/wchar.h
seek.o: /usr/include/bits/wchar.h /usr/include/gconv.h
seek.o: /usr/include/bits/stdio_lim.h /usr/include/string.h
seek.o: /usr/include/sys/types.h /usr/include/time.h ff.h
seek.o: /usr/include/limits.h
size.o: ff.local.h ../include/c_env.h ../include/localenv.h
size.o: /usr/include/stdio.h /usr/include/features.h /usr/include/sys/cdefs.h
size.o: /usr/include/gnu/stubs.h
size.o: /usr/include/bits/types.h
size.o: /usr/include/wchar.h
size.o: /usr/include/bits/wchar.h /usr/include/gconv.h
size.o: /usr/include/bits/stdio_lim.h /usr/include/string.h
size.o: /usr/include/sys/types.h /usr/include/time.h ff.h
size.o: /usr/include/limits.h
sync.o: ff.local.h ../include/c_env.h ../include/localenv.h
sync.o: /usr/include/stdio.h /usr/include/features.h /usr/include/sys/cdefs.h
sync.o: /usr/include/gnu/stubs.h
sync.o: /usr/include/bits/types.h
sync.o: /usr/include/wchar.h
sync.o: /usr/include/bits/wchar.h /usr/include/gconv.h
sync.o: /usr/include/bits/stdio_lim.h /usr/include/string.h
sync.o: /usr/include/sys/types.h /usr/include/time.h ff.h
sync.o: /usr/include/limits.h
flush.o: ff.local.h ../include/c_env.h ../include/localenv.h
flush.o: /usr/include/stdio.h /usr/include/features.h
flush.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
flush.o: /usr/include/bits/types.h
flush.o: /usr/include/wchar.h
flush.o: /usr/include/bits/wchar.h /usr/include/gconv.h
flush.o: /usr/include/bits/stdio_lim.h /usr/include/string.h
flush.o: /usr/include/sys/types.h /usr/include/time.h ff.h
flush.o: /usr/include/limits.h
sort.o: ff.local.h ../include/c_env.h ../include/localenv.h
sort.o: /usr/include/stdio.h /usr/include/features.h /usr/include/sys/cdefs.h
sort.o: /usr/include/gnu/stubs.h
sort.o: /usr/include/bits/types.h
sort.o: /usr/include/wchar.h
sort.o: /usr/include/bits/wchar.h /usr/include/gconv.h
sort.o: /usr/include/bits/stdio_lim.h /usr/include/string.h
sort.o: /usr/include/sys/types.h /usr/include/time.h ff.h
sort.o: /usr/include/limits.h
write.o: ff.local.h ../include/c_env.h ../include/localenv.h
write.o: /usr/include/stdio.h /usr/include/features.h
write.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
write.o: /usr/include/bits/types.h
write.o: /usr/include/wchar.h
write.o: /usr/include/bits/wchar.h /usr/include/gconv.h
write.o: /usr/include/bits/stdio_lim.h /usr/include/string.h
write.o: /usr/include/sys/types.h /usr/include/time.h ff.h
write.o: /usr/include/limits.h
point.o: ff.local.h ../include/c_env.h ../include/localenv.h
point.o: /usr/include/stdio.h /usr/include/features.h
point.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
point.o: /usr/include/bits/types.h
point.o: /usr/include/wchar.h
point.o: /usr/include/bits/wchar.h /usr/include/gconv.h
point.o: /usr/include/bits/stdio_lim.h /usr/include/string.h
point.o: /usr/include/sys/types.h /usr/include/time.h ff.h
point.o: /usr/include/limits.h
move.o: ../include/c_env.h
fill.o: ../include/c_env.h
debug.o: ff.local.h ../include/c_env.h ../include/localenv.h
debug.o: /usr/include/stdio.h /usr/include/features.h
debug.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
debug.o: /usr/include/bits/types.h
debug.o: /usr/include/wchar.h
debug.o: /usr/include/bits/wchar.h /usr/include/gconv.h
debug.o: /usr/include/bits/stdio_lim.h /usr/include/string.h
debug.o: /usr/include/sys/types.h /usr/include/time.h ff.h
debug.o: /usr/include/limits.h
