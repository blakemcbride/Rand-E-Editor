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
open.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
open.o: /usr/include/bits/types.h /usr/include/libio.h
open.o: /usr/include/_G_config.h /usr/include/wchar.h
open.o: /usr/include/bits/wchar.h /usr/include/gconv.h
open.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
open.o: /usr/include/bits/stdio_lim.h /usr/include/string.h
open.o: /usr/include/sys/types.h /usr/include/time.h ff.h
open.o: /usr/include/limits.h
open.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/limits.h
open.o: /usr/include/errno.h /usr/include/bits/errno.h
open.o: /usr/include/linux/errno.h /usr/include/asm/errno.h
open.o: /usr/include/sys/stat.h /usr/include/bits/stat.h
close.o: ff.local.h ../include/c_env.h ../include/localenv.h
close.o: /usr/include/stdio.h /usr/include/features.h
close.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
close.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
close.o: /usr/include/bits/types.h /usr/include/libio.h
close.o: /usr/include/_G_config.h /usr/include/wchar.h
close.o: /usr/include/bits/wchar.h /usr/include/gconv.h
close.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
close.o: /usr/include/bits/stdio_lim.h /usr/include/string.h
close.o: /usr/include/sys/types.h /usr/include/time.h ff.h
close.o: /usr/include/limits.h
close.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/limits.h
close.o: /usr/include/errno.h /usr/include/bits/errno.h
close.o: /usr/include/linux/errno.h /usr/include/asm/errno.h
fd.o: ff.local.h ../include/c_env.h ../include/localenv.h
fd.o: /usr/include/stdio.h /usr/include/features.h /usr/include/sys/cdefs.h
fd.o: /usr/include/gnu/stubs.h
fd.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
fd.o: /usr/include/bits/types.h /usr/include/libio.h /usr/include/_G_config.h
fd.o: /usr/include/wchar.h /usr/include/bits/wchar.h /usr/include/gconv.h
fd.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
fd.o: /usr/include/bits/stdio_lim.h /usr/include/string.h
fd.o: /usr/include/sys/types.h /usr/include/time.h ff.h /usr/include/limits.h
fd.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/limits.h
fd.o: /usr/include/errno.h /usr/include/bits/errno.h
fd.o: /usr/include/linux/errno.h /usr/include/asm/errno.h
free.o: ff.local.h ../include/c_env.h ../include/localenv.h
free.o: /usr/include/stdio.h /usr/include/features.h /usr/include/sys/cdefs.h
free.o: /usr/include/gnu/stubs.h
free.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
free.o: /usr/include/bits/types.h /usr/include/libio.h
free.o: /usr/include/_G_config.h /usr/include/wchar.h
free.o: /usr/include/bits/wchar.h /usr/include/gconv.h
free.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
free.o: /usr/include/bits/stdio_lim.h /usr/include/string.h
free.o: /usr/include/sys/types.h /usr/include/time.h ff.h
free.o: /usr/include/limits.h
free.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/limits.h
getc.o: ff.local.h ../include/c_env.h ../include/localenv.h
getc.o: /usr/include/stdio.h /usr/include/features.h /usr/include/sys/cdefs.h
getc.o: /usr/include/gnu/stubs.h
getc.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
getc.o: /usr/include/bits/types.h /usr/include/libio.h
getc.o: /usr/include/_G_config.h /usr/include/wchar.h
getc.o: /usr/include/bits/wchar.h /usr/include/gconv.h
getc.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
getc.o: /usr/include/bits/stdio_lim.h /usr/include/string.h
getc.o: /usr/include/sys/types.h /usr/include/time.h ff.h
getc.o: /usr/include/limits.h
getc.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/limits.h
getc.o: /usr/include/errno.h /usr/include/bits/errno.h
getc.o: /usr/include/linux/errno.h /usr/include/asm/errno.h
grow.o: ff.local.h ../include/c_env.h ../include/localenv.h
grow.o: /usr/include/stdio.h /usr/include/features.h /usr/include/sys/cdefs.h
grow.o: /usr/include/gnu/stubs.h
grow.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
grow.o: /usr/include/bits/types.h /usr/include/libio.h
grow.o: /usr/include/_G_config.h /usr/include/wchar.h
grow.o: /usr/include/bits/wchar.h /usr/include/gconv.h
grow.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
grow.o: /usr/include/bits/stdio_lim.h /usr/include/string.h
grow.o: /usr/include/sys/types.h /usr/include/time.h ff.h
grow.o: /usr/include/limits.h
grow.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/limits.h
grow.o: /usr/include/errno.h /usr/include/bits/errno.h
grow.o: /usr/include/linux/errno.h /usr/include/asm/errno.h
pos.o: ff.local.h ../include/c_env.h ../include/localenv.h
pos.o: /usr/include/stdio.h /usr/include/features.h /usr/include/sys/cdefs.h
pos.o: /usr/include/gnu/stubs.h
pos.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
pos.o: /usr/include/bits/types.h /usr/include/libio.h
pos.o: /usr/include/_G_config.h /usr/include/wchar.h
pos.o: /usr/include/bits/wchar.h /usr/include/gconv.h
pos.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
pos.o: /usr/include/bits/stdio_lim.h /usr/include/string.h
pos.o: /usr/include/sys/types.h /usr/include/time.h ff.h
pos.o: /usr/include/limits.h
pos.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/limits.h
pos.o: /usr/include/errno.h /usr/include/bits/errno.h
pos.o: /usr/include/linux/errno.h /usr/include/asm/errno.h
putc.o: ff.local.h ../include/c_env.h ../include/localenv.h
putc.o: /usr/include/stdio.h /usr/include/features.h /usr/include/sys/cdefs.h
putc.o: /usr/include/gnu/stubs.h
putc.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
putc.o: /usr/include/bits/types.h /usr/include/libio.h
putc.o: /usr/include/_G_config.h /usr/include/wchar.h
putc.o: /usr/include/bits/wchar.h /usr/include/gconv.h
putc.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
putc.o: /usr/include/bits/stdio_lim.h /usr/include/string.h
putc.o: /usr/include/sys/types.h /usr/include/time.h ff.h
putc.o: /usr/include/limits.h
putc.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/limits.h
putc.o: /usr/include/errno.h /usr/include/bits/errno.h
putc.o: /usr/include/linux/errno.h /usr/include/asm/errno.h
read.o: ff.local.h ../include/c_env.h ../include/localenv.h
read.o: /usr/include/stdio.h /usr/include/features.h /usr/include/sys/cdefs.h
read.o: /usr/include/gnu/stubs.h
read.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
read.o: /usr/include/bits/types.h /usr/include/libio.h
read.o: /usr/include/_G_config.h /usr/include/wchar.h
read.o: /usr/include/bits/wchar.h /usr/include/gconv.h
read.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
read.o: /usr/include/bits/stdio_lim.h /usr/include/string.h
read.o: /usr/include/sys/types.h /usr/include/time.h ff.h
read.o: /usr/include/limits.h
read.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/limits.h
read.o: /usr/include/errno.h /usr/include/bits/errno.h
read.o: /usr/include/linux/errno.h /usr/include/asm/errno.h
seek.o: ff.local.h ../include/c_env.h ../include/localenv.h
seek.o: /usr/include/stdio.h /usr/include/features.h /usr/include/sys/cdefs.h
seek.o: /usr/include/gnu/stubs.h
seek.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
seek.o: /usr/include/bits/types.h /usr/include/libio.h
seek.o: /usr/include/_G_config.h /usr/include/wchar.h
seek.o: /usr/include/bits/wchar.h /usr/include/gconv.h
seek.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
seek.o: /usr/include/bits/stdio_lim.h /usr/include/string.h
seek.o: /usr/include/sys/types.h /usr/include/time.h ff.h
seek.o: /usr/include/limits.h
seek.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/limits.h
seek.o: /usr/include/errno.h /usr/include/bits/errno.h
seek.o: /usr/include/linux/errno.h /usr/include/asm/errno.h
size.o: ff.local.h ../include/c_env.h ../include/localenv.h
size.o: /usr/include/stdio.h /usr/include/features.h /usr/include/sys/cdefs.h
size.o: /usr/include/gnu/stubs.h
size.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
size.o: /usr/include/bits/types.h /usr/include/libio.h
size.o: /usr/include/_G_config.h /usr/include/wchar.h
size.o: /usr/include/bits/wchar.h /usr/include/gconv.h
size.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
size.o: /usr/include/bits/stdio_lim.h /usr/include/string.h
size.o: /usr/include/sys/types.h /usr/include/time.h ff.h
size.o: /usr/include/limits.h
size.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/limits.h
size.o: /usr/include/errno.h /usr/include/bits/errno.h
size.o: /usr/include/linux/errno.h /usr/include/asm/errno.h
sync.o: ff.local.h ../include/c_env.h ../include/localenv.h
sync.o: /usr/include/stdio.h /usr/include/features.h /usr/include/sys/cdefs.h
sync.o: /usr/include/gnu/stubs.h
sync.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
sync.o: /usr/include/bits/types.h /usr/include/libio.h
sync.o: /usr/include/_G_config.h /usr/include/wchar.h
sync.o: /usr/include/bits/wchar.h /usr/include/gconv.h
sync.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
sync.o: /usr/include/bits/stdio_lim.h /usr/include/string.h
sync.o: /usr/include/sys/types.h /usr/include/time.h ff.h
sync.o: /usr/include/limits.h
sync.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/limits.h
flush.o: ff.local.h ../include/c_env.h ../include/localenv.h
flush.o: /usr/include/stdio.h /usr/include/features.h
flush.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
flush.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
flush.o: /usr/include/bits/types.h /usr/include/libio.h
flush.o: /usr/include/_G_config.h /usr/include/wchar.h
flush.o: /usr/include/bits/wchar.h /usr/include/gconv.h
flush.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
flush.o: /usr/include/bits/stdio_lim.h /usr/include/string.h
flush.o: /usr/include/sys/types.h /usr/include/time.h ff.h
flush.o: /usr/include/limits.h
flush.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/limits.h
flush.o: /usr/include/errno.h /usr/include/bits/errno.h
flush.o: /usr/include/linux/errno.h /usr/include/asm/errno.h
sort.o: ff.local.h ../include/c_env.h ../include/localenv.h
sort.o: /usr/include/stdio.h /usr/include/features.h /usr/include/sys/cdefs.h
sort.o: /usr/include/gnu/stubs.h
sort.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
sort.o: /usr/include/bits/types.h /usr/include/libio.h
sort.o: /usr/include/_G_config.h /usr/include/wchar.h
sort.o: /usr/include/bits/wchar.h /usr/include/gconv.h
sort.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
sort.o: /usr/include/bits/stdio_lim.h /usr/include/string.h
sort.o: /usr/include/sys/types.h /usr/include/time.h ff.h
sort.o: /usr/include/limits.h
sort.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/limits.h
write.o: ff.local.h ../include/c_env.h ../include/localenv.h
write.o: /usr/include/stdio.h /usr/include/features.h
write.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
write.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
write.o: /usr/include/bits/types.h /usr/include/libio.h
write.o: /usr/include/_G_config.h /usr/include/wchar.h
write.o: /usr/include/bits/wchar.h /usr/include/gconv.h
write.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
write.o: /usr/include/bits/stdio_lim.h /usr/include/string.h
write.o: /usr/include/sys/types.h /usr/include/time.h ff.h
write.o: /usr/include/limits.h
write.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/limits.h
write.o: /usr/include/errno.h /usr/include/bits/errno.h
write.o: /usr/include/linux/errno.h /usr/include/asm/errno.h
point.o: ff.local.h ../include/c_env.h ../include/localenv.h
point.o: /usr/include/stdio.h /usr/include/features.h
point.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
point.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
point.o: /usr/include/bits/types.h /usr/include/libio.h
point.o: /usr/include/_G_config.h /usr/include/wchar.h
point.o: /usr/include/bits/wchar.h /usr/include/gconv.h
point.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
point.o: /usr/include/bits/stdio_lim.h /usr/include/string.h
point.o: /usr/include/sys/types.h /usr/include/time.h ff.h
point.o: /usr/include/limits.h
point.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/limits.h
move.o: ../include/c_env.h
fill.o: ../include/c_env.h
debug.o: ff.local.h ../include/c_env.h ../include/localenv.h
debug.o: /usr/include/stdio.h /usr/include/features.h
debug.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
debug.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
debug.o: /usr/include/bits/types.h /usr/include/libio.h
debug.o: /usr/include/_G_config.h /usr/include/wchar.h
debug.o: /usr/include/bits/wchar.h /usr/include/gconv.h
debug.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
debug.o: /usr/include/bits/stdio_lim.h /usr/include/string.h
debug.o: /usr/include/sys/types.h /usr/include/time.h ff.h
debug.o: /usr/include/limits.h
debug.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/limits.h
