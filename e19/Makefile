#
#	Copyright abandoned, 1983, The Rand Corporation
#

# Rand Editor with LA package
#
#       MAKEFILE FOR RAND "LA" LINE-ACCESS I/O PACKAGE
#
# Require GNU make (use "ifeq" GNU make syntax)

# -----------------------------------------------------------
# On a sun with Solaris :  (F.Perriollat Dec 1996)
# do not use -O : in e.t.c result in strang effect which give
#       a core dump on deleting char on command line !!!!
# -----------------------------------------------------------
# MYCFLAGS flag value : include
#       -DNOX11 if you do not want X11 package
#       -DNOXKB if your X11 does not provide X11 Keyboard Extension
# -----------------------------------------------------------

ifndef TARG_OS
    TARG_OS := $(shell uname -s)
endif

MYCFLAGS=

ifeq ($(TARG_OS), Solaris)
    MYCFLAGS = -DNOXKB -DHAVE_UTMPX_H
endif

ifeq ($(TARG_OS), LynxOS)
    MYCFLAGS = -DNOX11 -DNOstrcasecmp
#   MYCFLAGS += -Dinfo=myinfo
endif

# CFLAGS = -O -I. -I../include -I../la1 -I../ff3
CFLAGS = -I. -I../include -I../la1 -I../ff3 -g $(MYCFLAGS)



# OBJS=   e.main.o e.o    e.iit.o e.q.o   e.tb.o  e.put.o e.nm.o  e.sv.o \
#         e.wi.o  e.wk.o  e.u.o   e.f.o   e.dif.o e.h.o   e.se.o  e.e.o \
#         e.mk.o  e.ra.o  e.re.o  e.t.o   e.ru.o  e.cm.o  e.d.o   e.it.o \
#         e.la.o  e.m.o   e.p.o   e.pa.o  e.sb.o  e.tt.o  e.x.o \
#         e.keyboard_map.o e.resize.o e.who.o

SRCS=   e.main.c e.c    e.iit.c e.q.c   e.tb.c  e.put.c e.nm.c  e.sv.c \
	e.wi.c  e.wk.c  e.u.c   e.f.c   e.dif.c e.h.c   e.se.c  e.e.c \
	e.mk.c  e.ra.c  e.re.c  e.t.c   e.ru.c  e.cm.c  e.d.c   e.it.c \
	e.la.c  e.m.c   e.p.c   e.pa.c  e.sb.c  e.tt.c  e.x.c \
	e.keyboard_map.c e.resize.c e.who.c

ifeq ($(TARG_OS), Solaris)
	SRCS += my_scandir.c
endif

OBJS= $(SRCS:.c=.o)

LIBS=   ../la1/libla.a ../ff3/libff.a ../lib/libtmp.a
LIBSYS= -lcurses

ifeq ($(TARG_OS), Solaris)
    LIBSYS= -lncurses
endif

ifeq ($(TARG_OS), Linux)
    LIBSYS= -lncurses
endif

LIBX=   -L/usr/X11R6/lib -lXt -lX11 -lxkbfile

ifeq ($(TARG_OS), Solaris)
    LIBX= -L/usr/X11R6/lib -lXt -lX11
    LIBSYS += -lxnet
endif

ifeq ($(TARG_OS), LynxOS)
    LIBX=
    LIBSYS += -lnetinet
endif


# For Ultrix, following line is needed !!
#TERMCAP = -ltermcap
TERMCAP =

NOX11=1

ALL=a.out

$(ALL): $(OBJS) e.r.o  $(LIBS)
	$(CC) $(CFLAGS) -o $(ALL) $(OBJS) e.r.o  $(LIBS) $(LIBX) $(LIBSYS) $(TERMCAP)

e.r.c:
	cd ../; make e19/e.r.c

clean:
	rm -f $(OBJS) $(ALL)  e.r.c e.r.o ,* .e?[1-9] .e?[1-9].* core *.bak *.BAK
	cd term; rm -f ,* .e?[1-9] .e?[1-9].* core a.out *.bak *.BAK

depend:
	makedepend -- $(CFLAGS) -- $(SRCS)
e.h:
	@echo Why doyou try to overwrite e.h? 

test:
	@echo TARG_OS is $(TARG_OS)
	@echo TARG_OS is defined : $(origin TARG_OS)
	@echo CFLAGS = $(CFLAGS)
	@echo LIBSYS = $(LIBSYS)
	@echo LIBX = $(LIBX)
	@echo -e "OBJS is\n $(OBJS)"


# DO NOT DELETE THIS LINE -- make depend depends on it.

e.main.o: e.h ../include/c_env.h ../include/localenv.h /usr/include/stdio.h
e.main.o: /usr/include/features.h /usr/include/sys/cdefs.h
e.main.o: /usr/include/gnu/stubs.h
e.main.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
e.main.o: /usr/include/bits/types.h /usr/include/libio.h
e.main.o: /usr/include/_G_config.h /usr/include/wchar.h
e.main.o: /usr/include/bits/wchar.h /usr/include/gconv.h
e.main.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
e.main.o: /usr/include/bits/stdio_lim.h /usr/include/string.h
e.main.o: /usr/include/unistd.h /usr/include/bits/posix_opt.h
e.main.o: /usr/include/bits/confname.h /usr/include/sys/types.h
e.main.o: /usr/include/time.h ../la1/la.h ../ff3/ff.h /usr/include/limits.h
e.main.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/limits.h e.t.h
e.o: e.h ../include/c_env.h ../include/localenv.h /usr/include/stdio.h
e.o: /usr/include/features.h /usr/include/sys/cdefs.h
e.o: /usr/include/gnu/stubs.h
e.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
e.o: /usr/include/bits/types.h /usr/include/libio.h /usr/include/_G_config.h
e.o: /usr/include/wchar.h /usr/include/bits/wchar.h /usr/include/gconv.h
e.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
e.o: /usr/include/bits/stdio_lim.h /usr/include/string.h
e.o: /usr/include/unistd.h /usr/include/bits/posix_opt.h
e.o: /usr/include/bits/confname.h /usr/include/sys/types.h
e.o: /usr/include/time.h ../la1/la.h ../ff3/ff.h /usr/include/limits.h
e.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/limits.h e.t.h e.e.h
e.o: e.it.h e.tt.h e.wi.h e.fn.h e.sg.h /usr/include/termio.h
e.o: /usr/include/termios.h /usr/include/bits/termios.h
e.o: /usr/include/sys/ioctl.h /usr/include/bits/ioctls.h
e.o: /usr/include/asm/ioctls.h /usr/include/asm/ioctl.h
e.o: /usr/include/bits/ioctl-types.h /usr/include/sys/ttydefaults.h e.inf.h
e.o: /usr/include/sys/stat.h /usr/include/bits/stat.h /usr/include/fcntl.h
e.o: /usr/include/bits/fcntl.h /usr/include/signal.h
e.o: /usr/include/bits/sigset.h /usr/include/bits/signum.h
e.iit.o: /usr/include/string.h /usr/include/features.h
e.iit.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
e.iit.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h e.h
e.iit.o: ../include/c_env.h ../include/localenv.h /usr/include/stdio.h
e.iit.o: /usr/include/bits/types.h /usr/include/libio.h
e.iit.o: /usr/include/_G_config.h /usr/include/wchar.h
e.iit.o: /usr/include/bits/wchar.h /usr/include/gconv.h
e.iit.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
e.iit.o: /usr/include/bits/stdio_lim.h /usr/include/unistd.h
e.iit.o: /usr/include/bits/posix_opt.h /usr/include/bits/confname.h
e.iit.o: /usr/include/sys/types.h /usr/include/time.h ../la1/la.h ../ff3/ff.h
e.iit.o: /usr/include/limits.h
e.iit.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/limits.h e.t.h
e.iit.o: e.it.h
e.q.o: /usr/include/sys/utsname.h /usr/include/features.h
e.q.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
e.q.o: /usr/include/bits/utsname.h e.h ../include/c_env.h
e.q.o: ../include/localenv.h /usr/include/stdio.h
e.q.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
e.q.o: /usr/include/bits/types.h /usr/include/libio.h
e.q.o: /usr/include/_G_config.h /usr/include/wchar.h
e.q.o: /usr/include/bits/wchar.h /usr/include/gconv.h
e.q.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
e.q.o: /usr/include/bits/stdio_lim.h /usr/include/string.h
e.q.o: /usr/include/unistd.h /usr/include/bits/posix_opt.h
e.q.o: /usr/include/bits/confname.h /usr/include/sys/types.h
e.q.o: /usr/include/time.h ../la1/la.h ../ff3/ff.h /usr/include/limits.h
e.q.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/limits.h e.t.h e.m.h
e.q.o: e.cm.h e.fn.h e.tt.h e.it.h /usr/include/signal.h
e.q.o: /usr/include/bits/sigset.h /usr/include/bits/signum.h
e.tb.o: e.h ../include/c_env.h ../include/localenv.h /usr/include/stdio.h
e.tb.o: /usr/include/features.h /usr/include/sys/cdefs.h
e.tb.o: /usr/include/gnu/stubs.h
e.tb.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
e.tb.o: /usr/include/bits/types.h /usr/include/libio.h
e.tb.o: /usr/include/_G_config.h /usr/include/wchar.h
e.tb.o: /usr/include/bits/wchar.h /usr/include/gconv.h
e.tb.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
e.tb.o: /usr/include/bits/stdio_lim.h /usr/include/string.h
e.tb.o: /usr/include/unistd.h /usr/include/bits/posix_opt.h
e.tb.o: /usr/include/bits/confname.h /usr/include/sys/types.h
e.tb.o: /usr/include/time.h ../la1/la.h ../ff3/ff.h /usr/include/limits.h
e.tb.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/limits.h e.t.h e.m.h
e.tb.o: e.cm.h
e.put.o: e.h ../include/c_env.h ../include/localenv.h /usr/include/stdio.h
e.put.o: /usr/include/features.h /usr/include/sys/cdefs.h
e.put.o: /usr/include/gnu/stubs.h
e.put.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
e.put.o: /usr/include/bits/types.h /usr/include/libio.h
e.put.o: /usr/include/_G_config.h /usr/include/wchar.h
e.put.o: /usr/include/bits/wchar.h /usr/include/gconv.h
e.put.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
e.put.o: /usr/include/bits/stdio_lim.h /usr/include/string.h
e.put.o: /usr/include/unistd.h /usr/include/bits/posix_opt.h
e.put.o: /usr/include/bits/confname.h /usr/include/sys/types.h
e.put.o: /usr/include/time.h ../la1/la.h ../ff3/ff.h /usr/include/limits.h
e.put.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/limits.h e.t.h e.m.h
e.put.o: e.cm.h e.e.h
e.nm.o: e.h ../include/c_env.h ../include/localenv.h /usr/include/stdio.h
e.nm.o: /usr/include/features.h /usr/include/sys/cdefs.h
e.nm.o: /usr/include/gnu/stubs.h
e.nm.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
e.nm.o: /usr/include/bits/types.h /usr/include/libio.h
e.nm.o: /usr/include/_G_config.h /usr/include/wchar.h
e.nm.o: /usr/include/bits/wchar.h /usr/include/gconv.h
e.nm.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
e.nm.o: /usr/include/bits/stdio_lim.h /usr/include/string.h
e.nm.o: /usr/include/unistd.h /usr/include/bits/posix_opt.h
e.nm.o: /usr/include/bits/confname.h /usr/include/sys/types.h
e.nm.o: /usr/include/time.h ../la1/la.h ../ff3/ff.h /usr/include/limits.h
e.nm.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/limits.h e.t.h
e.nm.o: e.inf.h e.m.h e.cm.h /usr/include/sys/stat.h /usr/include/bits/stat.h
e.sv.o: e.h ../include/c_env.h ../include/localenv.h /usr/include/stdio.h
e.sv.o: /usr/include/features.h /usr/include/sys/cdefs.h
e.sv.o: /usr/include/gnu/stubs.h
e.sv.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
e.sv.o: /usr/include/bits/types.h /usr/include/libio.h
e.sv.o: /usr/include/_G_config.h /usr/include/wchar.h
e.sv.o: /usr/include/bits/wchar.h /usr/include/gconv.h
e.sv.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
e.sv.o: /usr/include/bits/stdio_lim.h /usr/include/string.h
e.sv.o: /usr/include/unistd.h /usr/include/bits/posix_opt.h
e.sv.o: /usr/include/bits/confname.h /usr/include/sys/types.h
e.sv.o: /usr/include/time.h ../la1/la.h ../ff3/ff.h /usr/include/limits.h
e.sv.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/limits.h e.t.h e.m.h
e.sv.o: e.cm.h /usr/include/sys/stat.h /usr/include/bits/stat.h
e.wi.o: e.h ../include/c_env.h ../include/localenv.h /usr/include/stdio.h
e.wi.o: /usr/include/features.h /usr/include/sys/cdefs.h
e.wi.o: /usr/include/gnu/stubs.h
e.wi.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
e.wi.o: /usr/include/bits/types.h /usr/include/libio.h
e.wi.o: /usr/include/_G_config.h /usr/include/wchar.h
e.wi.o: /usr/include/bits/wchar.h /usr/include/gconv.h
e.wi.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
e.wi.o: /usr/include/bits/stdio_lim.h /usr/include/string.h
e.wi.o: /usr/include/unistd.h /usr/include/bits/posix_opt.h
e.wi.o: /usr/include/bits/confname.h /usr/include/sys/types.h
e.wi.o: /usr/include/time.h ../la1/la.h ../ff3/ff.h /usr/include/limits.h
e.wi.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/limits.h e.t.h e.cm.h
e.wi.o: e.inf.h e.tt.h e.wi.h
e.wk.o: e.h ../include/c_env.h ../include/localenv.h /usr/include/stdio.h
e.wk.o: /usr/include/features.h /usr/include/sys/cdefs.h
e.wk.o: /usr/include/gnu/stubs.h
e.wk.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
e.wk.o: /usr/include/bits/types.h /usr/include/libio.h
e.wk.o: /usr/include/_G_config.h /usr/include/wchar.h
e.wk.o: /usr/include/bits/wchar.h /usr/include/gconv.h
e.wk.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
e.wk.o: /usr/include/bits/stdio_lim.h /usr/include/string.h
e.wk.o: /usr/include/unistd.h /usr/include/bits/posix_opt.h
e.wk.o: /usr/include/bits/confname.h /usr/include/sys/types.h
e.wk.o: /usr/include/time.h ../la1/la.h ../ff3/ff.h /usr/include/limits.h
e.wk.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/limits.h e.t.h
e.wk.o: e.inf.h
e.u.o: e.h ../include/c_env.h ../include/localenv.h /usr/include/stdio.h
e.u.o: /usr/include/features.h /usr/include/sys/cdefs.h
e.u.o: /usr/include/gnu/stubs.h
e.u.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
e.u.o: /usr/include/bits/types.h /usr/include/libio.h
e.u.o: /usr/include/_G_config.h /usr/include/wchar.h
e.u.o: /usr/include/bits/wchar.h /usr/include/gconv.h
e.u.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
e.u.o: /usr/include/bits/stdio_lim.h /usr/include/string.h
e.u.o: /usr/include/unistd.h /usr/include/bits/posix_opt.h
e.u.o: /usr/include/bits/confname.h /usr/include/sys/types.h
e.u.o: /usr/include/time.h ../la1/la.h ../ff3/ff.h /usr/include/limits.h
e.u.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/limits.h e.t.h e.m.h
e.u.o: e.cm.h e.tt.h /usr/include/sys/stat.h /usr/include/bits/stat.h
e.u.o: /usr/include/pwd.h
e.f.o: e.h ../include/c_env.h ../include/localenv.h /usr/include/stdio.h
e.f.o: /usr/include/features.h /usr/include/sys/cdefs.h
e.f.o: /usr/include/gnu/stubs.h
e.f.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
e.f.o: /usr/include/bits/types.h /usr/include/libio.h
e.f.o: /usr/include/_G_config.h /usr/include/wchar.h
e.f.o: /usr/include/bits/wchar.h /usr/include/gconv.h
e.f.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
e.f.o: /usr/include/bits/stdio_lim.h /usr/include/string.h
e.f.o: /usr/include/unistd.h /usr/include/bits/posix_opt.h
e.f.o: /usr/include/bits/confname.h /usr/include/sys/types.h
e.f.o: /usr/include/time.h ../la1/la.h ../ff3/ff.h /usr/include/limits.h
e.f.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/limits.h e.t.h
e.f.o: /usr/include/sys/stat.h /usr/include/bits/stat.h
e.h.o: e.h ../include/c_env.h ../include/localenv.h /usr/include/stdio.h
e.h.o: /usr/include/features.h /usr/include/sys/cdefs.h
e.h.o: /usr/include/gnu/stubs.h
e.h.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
e.h.o: /usr/include/bits/types.h /usr/include/libio.h
e.h.o: /usr/include/_G_config.h /usr/include/wchar.h
e.h.o: /usr/include/bits/wchar.h /usr/include/gconv.h
e.h.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
e.h.o: /usr/include/bits/stdio_lim.h /usr/include/string.h
e.h.o: /usr/include/unistd.h /usr/include/bits/posix_opt.h
e.h.o: /usr/include/bits/confname.h /usr/include/sys/types.h
e.h.o: /usr/include/time.h ../la1/la.h ../ff3/ff.h /usr/include/limits.h
e.h.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/limits.h e.t.h e.cm.h
e.h.o: e.m.h e.tt.h e.h.h /usr/include/sys/time.h /usr/include/bits/time.h
e.h.o: /usr/include/sys/select.h /usr/include/bits/select.h
e.h.o: /usr/include/bits/sigset.h
e.se.o: e.h ../include/c_env.h ../include/localenv.h /usr/include/stdio.h
e.se.o: /usr/include/features.h /usr/include/sys/cdefs.h
e.se.o: /usr/include/gnu/stubs.h
e.se.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
e.se.o: /usr/include/bits/types.h /usr/include/libio.h
e.se.o: /usr/include/_G_config.h /usr/include/wchar.h
e.se.o: /usr/include/bits/wchar.h /usr/include/gconv.h
e.se.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
e.se.o: /usr/include/bits/stdio_lim.h /usr/include/string.h
e.se.o: /usr/include/unistd.h /usr/include/bits/posix_opt.h
e.se.o: /usr/include/bits/confname.h /usr/include/sys/types.h
e.se.o: /usr/include/time.h ../la1/la.h ../ff3/ff.h /usr/include/limits.h
e.se.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/limits.h e.t.h e.m.h
e.se.o: e.cm.h e.se.h
e.e.o: e.h ../include/c_env.h ../include/localenv.h /usr/include/stdio.h
e.e.o: /usr/include/features.h /usr/include/sys/cdefs.h
e.e.o: /usr/include/gnu/stubs.h
e.e.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
e.e.o: /usr/include/bits/types.h /usr/include/libio.h
e.e.o: /usr/include/_G_config.h /usr/include/wchar.h
e.e.o: /usr/include/bits/wchar.h /usr/include/gconv.h
e.e.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
e.e.o: /usr/include/bits/stdio_lim.h /usr/include/string.h
e.e.o: /usr/include/unistd.h /usr/include/bits/posix_opt.h
e.e.o: /usr/include/bits/confname.h /usr/include/sys/types.h
e.e.o: /usr/include/time.h ../la1/la.h ../ff3/ff.h /usr/include/limits.h
e.e.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/limits.h e.t.h e.m.h
e.e.o: e.cm.h e.e.h
e.mk.o: e.h ../include/c_env.h ../include/localenv.h /usr/include/stdio.h
e.mk.o: /usr/include/features.h /usr/include/sys/cdefs.h
e.mk.o: /usr/include/gnu/stubs.h
e.mk.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
e.mk.o: /usr/include/bits/types.h /usr/include/libio.h
e.mk.o: /usr/include/_G_config.h /usr/include/wchar.h
e.mk.o: /usr/include/bits/wchar.h /usr/include/gconv.h
e.mk.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
e.mk.o: /usr/include/bits/stdio_lim.h /usr/include/string.h
e.mk.o: /usr/include/unistd.h /usr/include/bits/posix_opt.h
e.mk.o: /usr/include/bits/confname.h /usr/include/sys/types.h
e.mk.o: /usr/include/time.h ../la1/la.h ../ff3/ff.h /usr/include/limits.h
e.mk.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/limits.h e.t.h
e.mk.o: e.inf.h e.m.h
e.ra.o: e.h ../include/c_env.h ../include/localenv.h /usr/include/stdio.h
e.ra.o: /usr/include/features.h /usr/include/sys/cdefs.h
e.ra.o: /usr/include/gnu/stubs.h
e.ra.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
e.ra.o: /usr/include/bits/types.h /usr/include/libio.h
e.ra.o: /usr/include/_G_config.h /usr/include/wchar.h
e.ra.o: /usr/include/bits/wchar.h /usr/include/gconv.h
e.ra.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
e.ra.o: /usr/include/bits/stdio_lim.h /usr/include/string.h
e.ra.o: /usr/include/unistd.h /usr/include/bits/posix_opt.h
e.ra.o: /usr/include/bits/confname.h /usr/include/sys/types.h
e.ra.o: /usr/include/time.h ../la1/la.h ../ff3/ff.h /usr/include/limits.h
e.ra.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/limits.h e.t.h e.m.h
e.ra.o: e.cm.h
e.re.o: e.h ../include/c_env.h ../include/localenv.h /usr/include/stdio.h
e.re.o: /usr/include/features.h /usr/include/sys/cdefs.h
e.re.o: /usr/include/gnu/stubs.h
e.re.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
e.re.o: /usr/include/bits/types.h /usr/include/libio.h
e.re.o: /usr/include/_G_config.h /usr/include/wchar.h
e.re.o: /usr/include/bits/wchar.h /usr/include/gconv.h
e.re.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
e.re.o: /usr/include/bits/stdio_lim.h /usr/include/string.h
e.re.o: /usr/include/unistd.h /usr/include/bits/posix_opt.h
e.re.o: /usr/include/bits/confname.h /usr/include/sys/types.h
e.re.o: /usr/include/time.h ../la1/la.h ../ff3/ff.h /usr/include/limits.h
e.re.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/limits.h e.t.h e.m.h
e.re.o: e.cm.h e.se.h
e.t.o: e.h ../include/c_env.h ../include/localenv.h /usr/include/stdio.h
e.t.o: /usr/include/features.h /usr/include/sys/cdefs.h
e.t.o: /usr/include/gnu/stubs.h
e.t.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
e.t.o: /usr/include/bits/types.h /usr/include/libio.h
e.t.o: /usr/include/_G_config.h /usr/include/wchar.h
e.t.o: /usr/include/bits/wchar.h /usr/include/gconv.h
e.t.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
e.t.o: /usr/include/bits/stdio_lim.h /usr/include/string.h
e.t.o: /usr/include/unistd.h /usr/include/bits/posix_opt.h
e.t.o: /usr/include/bits/confname.h /usr/include/sys/types.h
e.t.o: /usr/include/time.h ../la1/la.h ../ff3/ff.h /usr/include/limits.h
e.t.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/limits.h e.t.h e.inf.h
e.t.o: e.m.h e.cm.h e.tt.h e.wi.h /usr/include/errno.h
e.t.o: /usr/include/bits/errno.h /usr/include/linux/errno.h
e.t.o: /usr/include/asm/errno.h /usr/include/sys/time.h
e.t.o: /usr/include/bits/time.h /usr/include/sys/select.h
e.t.o: /usr/include/bits/select.h /usr/include/bits/sigset.h
e.ru.o: e.h ../include/c_env.h ../include/localenv.h /usr/include/stdio.h
e.ru.o: /usr/include/features.h /usr/include/sys/cdefs.h
e.ru.o: /usr/include/gnu/stubs.h
e.ru.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
e.ru.o: /usr/include/bits/types.h /usr/include/libio.h
e.ru.o: /usr/include/_G_config.h /usr/include/wchar.h
e.ru.o: /usr/include/bits/wchar.h /usr/include/gconv.h
e.ru.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
e.ru.o: /usr/include/bits/stdio_lim.h /usr/include/string.h
e.ru.o: /usr/include/unistd.h /usr/include/bits/posix_opt.h
e.ru.o: /usr/include/bits/confname.h /usr/include/sys/types.h
e.ru.o: /usr/include/time.h ../la1/la.h ../ff3/ff.h /usr/include/limits.h
e.ru.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/limits.h e.t.h e.e.h
e.ru.o: e.m.h e.cm.h e.ru.h /usr/include/signal.h /usr/include/bits/sigset.h
e.ru.o: /usr/include/bits/signum.h
e.cm.o: /usr/include/time.h /usr/include/bits/types.h /usr/include/features.h
e.cm.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
e.cm.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
e.cm.o: /usr/include/unistd.h /usr/include/bits/posix_opt.h
e.cm.o: /usr/include/bits/confname.h /usr/include/dirent.h
e.cm.o: /usr/include/bits/dirent.h /usr/include/string.h
e.cm.o: /usr/include/sys/stat.h /usr/include/bits/stat.h e.h
e.cm.o: ../include/c_env.h ../include/localenv.h /usr/include/stdio.h
e.cm.o: /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h
e.cm.o: /usr/include/bits/wchar.h /usr/include/gconv.h
e.cm.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
e.cm.o: /usr/include/bits/stdio_lim.h /usr/include/sys/types.h ../la1/la.h
e.cm.o: ../ff3/ff.h /usr/include/limits.h
e.cm.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/limits.h e.t.h e.e.h
e.cm.o: e.m.h e.ru.h e.cm.h e.wi.h e.tt.h /usr/include/signal.h
e.cm.o: /usr/include/bits/sigset.h /usr/include/bits/signum.h
e.d.o: e.h ../include/c_env.h ../include/localenv.h /usr/include/stdio.h
e.d.o: /usr/include/features.h /usr/include/sys/cdefs.h
e.d.o: /usr/include/gnu/stubs.h
e.d.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
e.d.o: /usr/include/bits/types.h /usr/include/libio.h
e.d.o: /usr/include/_G_config.h /usr/include/wchar.h
e.d.o: /usr/include/bits/wchar.h /usr/include/gconv.h
e.d.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
e.d.o: /usr/include/bits/stdio_lim.h /usr/include/string.h
e.d.o: /usr/include/unistd.h /usr/include/bits/posix_opt.h
e.d.o: /usr/include/bits/confname.h /usr/include/sys/types.h
e.d.o: /usr/include/time.h ../la1/la.h ../ff3/ff.h /usr/include/limits.h
e.d.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/limits.h e.t.h e.tt.h
e.d.o: e.it.h /usr/include/signal.h /usr/include/bits/sigset.h
e.d.o: /usr/include/bits/signum.h
e.it.o: e.h ../include/c_env.h ../include/localenv.h /usr/include/stdio.h
e.it.o: /usr/include/features.h /usr/include/sys/cdefs.h
e.it.o: /usr/include/gnu/stubs.h
e.it.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
e.it.o: /usr/include/bits/types.h /usr/include/libio.h
e.it.o: /usr/include/_G_config.h /usr/include/wchar.h
e.it.o: /usr/include/bits/wchar.h /usr/include/gconv.h
e.it.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
e.it.o: /usr/include/bits/stdio_lim.h /usr/include/string.h
e.it.o: /usr/include/unistd.h /usr/include/bits/posix_opt.h
e.it.o: /usr/include/bits/confname.h /usr/include/sys/types.h
e.it.o: /usr/include/time.h ../la1/la.h ../ff3/ff.h /usr/include/limits.h
e.it.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/limits.h e.t.h e.it.h
e.la.o: e.h ../include/c_env.h ../include/localenv.h /usr/include/stdio.h
e.la.o: /usr/include/features.h /usr/include/sys/cdefs.h
e.la.o: /usr/include/gnu/stubs.h
e.la.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
e.la.o: /usr/include/bits/types.h /usr/include/libio.h
e.la.o: /usr/include/_G_config.h /usr/include/wchar.h
e.la.o: /usr/include/bits/wchar.h /usr/include/gconv.h
e.la.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
e.la.o: /usr/include/bits/stdio_lim.h /usr/include/string.h
e.la.o: /usr/include/unistd.h /usr/include/bits/posix_opt.h
e.la.o: /usr/include/bits/confname.h /usr/include/sys/types.h
e.la.o: /usr/include/time.h ../la1/la.h ../ff3/ff.h /usr/include/limits.h
e.la.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/limits.h e.t.h e.m.h
e.la.o: /usr/include/signal.h /usr/include/bits/sigset.h
e.la.o: /usr/include/bits/signum.h
e.m.o: e.h ../include/c_env.h ../include/localenv.h /usr/include/stdio.h
e.m.o: /usr/include/features.h /usr/include/sys/cdefs.h
e.m.o: /usr/include/gnu/stubs.h
e.m.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
e.m.o: /usr/include/bits/types.h /usr/include/libio.h
e.m.o: /usr/include/_G_config.h /usr/include/wchar.h
e.m.o: /usr/include/bits/wchar.h /usr/include/gconv.h
e.m.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
e.m.o: /usr/include/bits/stdio_lim.h /usr/include/string.h
e.m.o: /usr/include/unistd.h /usr/include/bits/posix_opt.h
e.m.o: /usr/include/bits/confname.h /usr/include/sys/types.h
e.m.o: /usr/include/time.h ../la1/la.h ../ff3/ff.h /usr/include/limits.h
e.m.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/limits.h e.t.h e.cm.h
e.m.o: e.inf.h e.m.h e.tt.h e.e.h /usr/include/sys/time.h
e.m.o: /usr/include/bits/time.h /usr/include/sys/select.h
e.m.o: /usr/include/bits/select.h /usr/include/bits/sigset.h
e.p.o: e.h ../include/c_env.h ../include/localenv.h /usr/include/stdio.h
e.p.o: /usr/include/features.h /usr/include/sys/cdefs.h
e.p.o: /usr/include/gnu/stubs.h
e.p.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
e.p.o: /usr/include/bits/types.h /usr/include/libio.h
e.p.o: /usr/include/_G_config.h /usr/include/wchar.h
e.p.o: /usr/include/bits/wchar.h /usr/include/gconv.h
e.p.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
e.p.o: /usr/include/bits/stdio_lim.h /usr/include/string.h
e.p.o: /usr/include/unistd.h /usr/include/bits/posix_opt.h
e.p.o: /usr/include/bits/confname.h /usr/include/sys/types.h
e.p.o: /usr/include/time.h ../la1/la.h ../ff3/ff.h /usr/include/limits.h
e.p.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/limits.h e.t.h e.cm.h
e.p.o: e.inf.h e.m.h /usr/include/ctype.h /usr/include/endian.h
e.p.o: /usr/include/bits/endian.h
e.pa.o: e.h ../include/c_env.h ../include/localenv.h /usr/include/stdio.h
e.pa.o: /usr/include/features.h /usr/include/sys/cdefs.h
e.pa.o: /usr/include/gnu/stubs.h
e.pa.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
e.pa.o: /usr/include/bits/types.h /usr/include/libio.h
e.pa.o: /usr/include/_G_config.h /usr/include/wchar.h
e.pa.o: /usr/include/bits/wchar.h /usr/include/gconv.h
e.pa.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
e.pa.o: /usr/include/bits/stdio_lim.h /usr/include/string.h
e.pa.o: /usr/include/unistd.h /usr/include/bits/posix_opt.h
e.pa.o: /usr/include/bits/confname.h /usr/include/sys/types.h
e.pa.o: /usr/include/time.h ../la1/la.h ../ff3/ff.h /usr/include/limits.h
e.pa.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/limits.h e.t.h e.m.h
e.pa.o: e.cm.h
e.sb.o: e.h ../include/c_env.h ../include/localenv.h /usr/include/stdio.h
e.sb.o: /usr/include/features.h /usr/include/sys/cdefs.h
e.sb.o: /usr/include/gnu/stubs.h
e.sb.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
e.sb.o: /usr/include/bits/types.h /usr/include/libio.h
e.sb.o: /usr/include/_G_config.h /usr/include/wchar.h
e.sb.o: /usr/include/bits/wchar.h /usr/include/gconv.h
e.sb.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
e.sb.o: /usr/include/bits/stdio_lim.h /usr/include/string.h
e.sb.o: /usr/include/unistd.h /usr/include/bits/posix_opt.h
e.sb.o: /usr/include/bits/confname.h /usr/include/sys/types.h
e.sb.o: /usr/include/time.h ../la1/la.h ../ff3/ff.h /usr/include/limits.h
e.sb.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/limits.h e.t.h e.fn.h
e.sb.o: e.sg.h /usr/include/termio.h /usr/include/termios.h
e.sb.o: /usr/include/bits/termios.h /usr/include/sys/ioctl.h
e.sb.o: /usr/include/bits/ioctls.h /usr/include/asm/ioctls.h
e.sb.o: /usr/include/asm/ioctl.h /usr/include/bits/ioctl-types.h
e.sb.o: /usr/include/sys/ttydefaults.h /usr/include/signal.h
e.sb.o: /usr/include/bits/sigset.h /usr/include/bits/signum.h
e.sb.o: /usr/include/sys/stat.h /usr/include/bits/stat.h /usr/include/fcntl.h
e.sb.o: /usr/include/bits/fcntl.h /usr/include/pwd.h
e.tt.o: e.h ../include/c_env.h ../include/localenv.h /usr/include/stdio.h
e.tt.o: /usr/include/features.h /usr/include/sys/cdefs.h
e.tt.o: /usr/include/gnu/stubs.h
e.tt.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
e.tt.o: /usr/include/bits/types.h /usr/include/libio.h
e.tt.o: /usr/include/_G_config.h /usr/include/wchar.h
e.tt.o: /usr/include/bits/wchar.h /usr/include/gconv.h
e.tt.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
e.tt.o: /usr/include/bits/stdio_lim.h /usr/include/string.h
e.tt.o: /usr/include/unistd.h /usr/include/bits/posix_opt.h
e.tt.o: /usr/include/bits/confname.h /usr/include/sys/types.h
e.tt.o: /usr/include/time.h ../la1/la.h ../ff3/ff.h /usr/include/limits.h
e.tt.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/limits.h e.t.h e.tt.h
e.tt.o: e.sg.h /usr/include/termio.h /usr/include/termios.h
e.tt.o: /usr/include/bits/termios.h /usr/include/sys/ioctl.h
e.tt.o: /usr/include/bits/ioctls.h /usr/include/asm/ioctls.h
e.tt.o: /usr/include/asm/ioctl.h /usr/include/bits/ioctl-types.h
e.tt.o: /usr/include/sys/ttydefaults.h term/standard.c term/tcap.c
e.tt.o: term/vt100.c
e.x.o: e.h ../include/c_env.h ../include/localenv.h /usr/include/stdio.h
e.x.o: /usr/include/features.h /usr/include/sys/cdefs.h
e.x.o: /usr/include/gnu/stubs.h
e.x.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
e.x.o: /usr/include/bits/types.h /usr/include/libio.h
e.x.o: /usr/include/_G_config.h /usr/include/wchar.h
e.x.o: /usr/include/bits/wchar.h /usr/include/gconv.h
e.x.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
e.x.o: /usr/include/bits/stdio_lim.h /usr/include/string.h
e.x.o: /usr/include/unistd.h /usr/include/bits/posix_opt.h
e.x.o: /usr/include/bits/confname.h /usr/include/sys/types.h
e.x.o: /usr/include/time.h ../la1/la.h ../ff3/ff.h /usr/include/limits.h
e.x.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/limits.h e.t.h e.e.h
e.x.o: e.inf.h e.m.h e.sg.h /usr/include/termio.h /usr/include/termios.h
e.x.o: /usr/include/bits/termios.h /usr/include/sys/ioctl.h
e.x.o: /usr/include/bits/ioctls.h /usr/include/asm/ioctls.h
e.x.o: /usr/include/asm/ioctl.h /usr/include/bits/ioctl-types.h
e.x.o: /usr/include/sys/ttydefaults.h /usr/include/signal.h
e.x.o: /usr/include/bits/sigset.h /usr/include/bits/signum.h
e.keyboard_map.o: /usr/include/stdlib.h /usr/include/features.h
e.keyboard_map.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
e.keyboard_map.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
e.keyboard_map.o: /usr/include/signal.h /usr/include/bits/sigset.h
e.keyboard_map.o: /usr/include/bits/types.h /usr/include/bits/signum.h e.h
e.keyboard_map.o: ../include/c_env.h ../include/localenv.h
e.keyboard_map.o: /usr/include/stdio.h /usr/include/libio.h
e.keyboard_map.o: /usr/include/_G_config.h /usr/include/wchar.h
e.keyboard_map.o: /usr/include/bits/wchar.h /usr/include/gconv.h
e.keyboard_map.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
e.keyboard_map.o: /usr/include/bits/stdio_lim.h /usr/include/string.h
e.keyboard_map.o: /usr/include/unistd.h /usr/include/bits/posix_opt.h
e.keyboard_map.o: /usr/include/bits/confname.h /usr/include/sys/types.h
e.keyboard_map.o: /usr/include/time.h ../la1/la.h ../ff3/ff.h
e.keyboard_map.o: /usr/include/limits.h
e.keyboard_map.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/limits.h
e.keyboard_map.o: e.t.h e.cm.h e.it.h e.tt.h /usr/include/termios.h
e.keyboard_map.o: /usr/include/bits/termios.h /usr/include/termio.h
e.keyboard_map.o: /usr/include/sys/ioctl.h /usr/include/bits/ioctls.h
e.keyboard_map.o: /usr/include/asm/ioctls.h /usr/include/asm/ioctl.h
e.keyboard_map.o: /usr/include/bits/ioctl-types.h
e.keyboard_map.o: /usr/include/sys/ttydefaults.h /usr/include/sys/time.h
e.keyboard_map.o: /usr/include/bits/time.h /usr/include/sys/select.h
e.keyboard_map.o: /usr/include/bits/select.h /usr/include/setjmp.h
e.keyboard_map.o: /usr/include/bits/setjmp.h /usr/include/X11/Xlib.h
e.keyboard_map.o: /usr/include/X11/X.h /usr/include/X11/Xfuncproto.h
e.keyboard_map.o: /usr/include/X11/Xosdefs.h /usr/include/X11/Intrinsic.h
e.keyboard_map.o: /usr/include/X11/Xutil.h /usr/include/X11/Xresource.h
e.keyboard_map.o: /usr/include/X11/Core.h /usr/include/X11/Composite.h
e.keyboard_map.o: /usr/include/X11/Constraint.h /usr/include/X11/Object.h
e.keyboard_map.o: /usr/include/X11/RectObj.h /usr/include/X11/Xatom.h
e.keyboard_map.o: /usr/include/X11/keysym.h /usr/include/X11/keysymdef.h
e.keyboard_map.o: /usr/include/X11/XKBlib.h
e.keyboard_map.o: /usr/include/X11/extensions/XKBstr.h
e.keyboard_map.o: /usr/include/X11/extensions/XKB.h
e.keyboard_map.o: /usr/include/X11/extensions/XKBfile.h
e.keyboard_map.o: /usr/include/X11/extensions/XKBgeom.h
e.resize.o: /usr/include/string.h /usr/include/features.h
e.resize.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
e.resize.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
e.resize.o: /usr/include/signal.h /usr/include/bits/sigset.h
e.resize.o: /usr/include/bits/types.h /usr/include/bits/signum.h e.h
e.resize.o: ../include/c_env.h ../include/localenv.h /usr/include/stdio.h
e.resize.o: /usr/include/libio.h /usr/include/_G_config.h
e.resize.o: /usr/include/wchar.h /usr/include/bits/wchar.h
e.resize.o: /usr/include/gconv.h
e.resize.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
e.resize.o: /usr/include/bits/stdio_lim.h /usr/include/unistd.h
e.resize.o: /usr/include/bits/posix_opt.h /usr/include/bits/confname.h
e.resize.o: /usr/include/sys/types.h /usr/include/time.h ../la1/la.h
e.resize.o: ../ff3/ff.h /usr/include/limits.h
e.resize.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/limits.h e.t.h
e.resize.o: e.cm.h e.inf.h e.tt.h e.wi.h
e.who.o: /usr/include/stdio.h /usr/include/features.h
e.who.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
e.who.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
e.who.o: /usr/include/bits/types.h /usr/include/libio.h
e.who.o: /usr/include/_G_config.h /usr/include/wchar.h
e.who.o: /usr/include/bits/wchar.h /usr/include/gconv.h
e.who.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
e.who.o: /usr/include/bits/stdio_lim.h /usr/include/sys/types.h
e.who.o: /usr/include/time.h /usr/include/stdlib.h /usr/include/unistd.h
e.who.o: /usr/include/bits/posix_opt.h /usr/include/bits/confname.h
e.who.o: /usr/include/string.h /usr/include/netdb.h /usr/include/netinet/in.h
e.who.o: /usr/include/stdint.h /usr/include/bits/wordsize.h
e.who.o: /usr/include/bits/socket.h /usr/include/limits.h
e.who.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/limits.h
e.who.o: /usr/include/bits/sockaddr.h /usr/include/asm/socket.h
e.who.o: /usr/include/asm/sockios.h /usr/include/bits/in.h
e.who.o: /usr/include/endian.h /usr/include/bits/endian.h
e.who.o: /usr/include/bits/byteswap.h /usr/include/bits/netdb.h
e.who.o: /usr/include/sys/socket.h /usr/include/sys/uio.h
e.who.o: /usr/include/bits/uio.h /usr/include/arpa/inet.h /usr/include/utmp.h
e.who.o: /usr/include/bits/utmp.h /usr/include/paths.h
e.who.o: /usr/include/sys/time.h /usr/include/bits/time.h
e.who.o: /usr/include/sys/select.h /usr/include/bits/select.h
e.who.o: /usr/include/bits/sigset.h /usr/include/sys/stat.h
e.who.o: /usr/include/bits/stat.h /usr/include/errno.h
e.who.o: /usr/include/bits/errno.h /usr/include/linux/errno.h
e.who.o: /usr/include/asm/errno.h
