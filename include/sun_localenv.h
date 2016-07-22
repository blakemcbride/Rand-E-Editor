
/* system environment stuff for 4.2bsd */

#define index strchr            /* */
#define rindex strrchr          /* */
/* #define EMPTY     */              /* can implement empty(fd) subroutine call */
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
#define TDIR "/tmp/"

#define UNIXV7                  /* if this is Version 7   */
#undef  UNIXV6

#undef  NOIOCTL_H               /* there is no ioctl.h */
#undef  RDNDELAY                /* read call has NDELAY capability */
#ifdef __ultrix
#define VFORK
#else
#undef  VFORK                   /* system has vfork() */
#endif
#undef  TTYN                    /* use ttyn function */
#undef  VAX_MONITOR             /* use monitor() routine for vax  profiling */

#ifdef __alpha
#define _BSD
#endif

#define TERMCAP
#define  SYSIII

#define __PSCO

#ifdef __PSCO
#ifdef __Lynx
#ifdef __mc68000
#define RECOVERMSG  "/ps/local/147/e/recovermsg"
#define XDIR_KR     "/ps/local/147/e/kr."
#define XDIR_HELP   "/ps/local/147/e/helpkey"
#define XDIR_CRASH  "/ps/local/147/e/Crashdoc"
#define XDIR_DIR    "/ps/local/147/e"
#define XDIR_ERR    "/ps/local/147/e/errmsg"
#define XDIR_RUN    "/ps/local/147/e/run"
#define XDIR_FILL   "/ps/local/147/e/fill"
#define XDIR_JUST   "/ps/local/147/e/just"
#define XDIR_CENTER "/ps/local/147/e/center"
#define XDIR_PRINT  "/ps/local/147/e/print"
#else
#define RECOVERMSG  "/ps/local/386/e/recovermsg"
#define XDIR_KR     "/ps/local/386/e/kr."
#define XDIR_HELP   "/ps/local/386/e/helpkey"
#define XDIR_CRASH  "/ps/local/386/e/Crashdoc"
#define XDIR_DIR    "/ps/local/386/e"
#define XDIR_ERR    "/ps/local/386/e/errmsg"
#define XDIR_RUN    "/ps/local/386/e/run"
#define XDIR_FILL   "/ps/local/386/e/fill"
#define XDIR_JUST   "/ps/local/386/e/just"
#define XDIR_CENTER "/ps/local/386/e/center"
#define XDIR_PRINT  "/ps/local/386/e/print"
#endif
#else
#define RECOVERMSG  "/afs/cern.ch/group/pz/e/recovermsg"
#define XDIR_KR     "/afs/cern.ch/group/pz/e/kr."
#define XDIR_HELP   "/afs/cern.ch/group/pz/e/helpkey"
#define XDIR_CRASH  "/afs/cern.ch/group/pz/e/Crashdoc"
#define XDIR_DIR    "/afs/cern.ch/group/pz/e"
#define XDIR_ERR    "/afs/cern.ch/group/pz/e/errmsg"
#define XDIR_RUN    "/afs/cern.ch/group/pz/e/run"
#define XDIR_FILL   "/afs/cern.ch/group/pz/e/fill"
#define XDIR_JUST   "/afs/cern.ch/group/pz/e/just"
#define XDIR_CENTER "/afs/cern.ch/group/pz/e/center"
#define XDIR_PRINT  "/afs/cern.ch/group/pz/e/print"
#endif
#else
#define RECOVERMSG  "/afs/cern.ch/user/p/perrioll/e/recovermsg"
#define XDIR_KR     "/afs/cern.ch/user/p/perrioll/e/kr."
#define XDIR_HELP   "/afs/cern.ch/user/p/perrioll/e/helpkey"
#define XDIR_CRASH  "/afs/cern.ch/user/p/perrioll/e/Crashdoc"
#define XDIR_DIR    "/afs/cern.ch/user/p/perrioll/e"
#define XDIR_ERR    "/afs/cern.ch/user/p/perrioll/e/errmsg"
#define XDIR_RUN    "/afs/cern.ch/user/p/perrioll/e/run"
#define XDIR_FILL   "/afs/cern.ch/user/p/perrioll/e/fill"
#define XDIR_JUST   "/afs/cern.ch/user/p/perrioll/e/just"
#define XDIR_CENTER "/afs/cern.ch/user/p/perrioll/e/center"
#define XDIR_PRINT  "/afs/cern.ch/user/p/perrioll/e/print"
#endif
