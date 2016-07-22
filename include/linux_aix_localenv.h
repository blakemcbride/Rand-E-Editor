
/* system environment stuff for Linux and AIX */
/* ========================================== */

#include <stdio.h>
#include <string.h>

#ifndef index
#define index strchr            /* */
#endif
#ifndef rindex
#define rindex strrchr          /* */
#endif
#define EMPTY                   /* can implement empty(fd) subroutine call */
#define LINKS                   /* file system has links */
#define CANFORK                 /* system has fork() */
#define ABORT_SIG SIGILL        /* which signal does abort() use */
#define SIGARG                  /* signal catch routine has sig num as arg */
#define SIG_INCL <signal.h>
#define SGTT_INCL <sgtty.h>

#define TTYNAME                 /* use ttyname function */
#define SHORTUID                /* uid is a short, not a char (v7+) */
#define ENVIRON                 /* getenv() is implemented */
#define SIGNALS                 /* system has signals */
#define SYMLINKS                /* 4.2 symbolic links */
#define NDIR                    /* 4.2 dirrectory routines */
#define SYSMKDIR                /* use mkdir(name, mode) system call */
#define SYSRMDIR                /* use rmdir(name) system call */
#define SYSFCHOWN               /* use fchown(fd, ...) system call */
#define SYSFCHMOD               /* use fchmod(fd, ...) system call */
#define CLR_SUID_ON_WRITE       /* modifying a file clrs suid and sgid bits */
#define SYSSELECT               /* system has berkeley select system call */
#define RENAME                  /* system has berkeley rename system call */

/* TRAILING '"' , for temporaries */
#ifdef P_tmpdir
#define TDIR P_tmpdir
#else
#define TDIR "/tmp/"
#endif

#define UNIXV7                  /* if this is Version 7   */
#undef  UNIXV6

#undef  NOIOCTL_H               /* there is no ioctl.h */
#undef  RDNDELAY                /* read call has NDELAY capability */
#undef  VFORK                   /* system has vfork() */
#undef  TTYN                    /* use ttyn function */
#undef  VAX_MONITOR             /* use monitor() routine for vax  profiling */

#define TERMCAP

#define SYSIII      /* Unix System III (termios, curses) */


/* ANSI C new macro facility */
#define CAT_dir_file(x,y)  x #y
#define XCAT_dir_file(x,y) CAT_dir_file(x,y)
#define FULL_File_path(v)  XCAT_dir_file(XDIR_DIR,v)

/* Root of the Rand executable package (default default values)
 *  look into dynamicaly generated file : e19/e.r.c for current values
 */
#define XDIR_DIR    "/usr/local/Rand"

#define RECOVERMSG  FULL_File_path(/recovermsg)
#define XDIR_KR     FULL_File_path(/kr.)
#define XDIR_HELP   FULL_File_path(/helpkey)
#define XDIR_CRASH  FULL_File_path(/Crashdoc)
#define XDIR_ERR    FULL_File_path(/errmsg)
#define XDIR_RUN    FULL_File_path(/run)
#define XDIR_FILL   FULL_File_path(/fill)
#define XDIR_JUST   FULL_File_path(/just)
#define XDIR_CENTER FULL_File_path(/center)
#define XDIR_PRINT  FULL_File_path(/print)

#ifndef _NFILE
#define _NFILE 32
#endif
#ifndef _IOEOF
#define _IOEOF _IO_EOF_SEEN
#endif

