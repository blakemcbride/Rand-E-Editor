#ifdef COMMENT
--------
file e.sb.c
    misc subroutines
#endif

#ifdef COMMENT
	Copyright abandoned, 1983, The Rand Corporation
#endif

#include "e.h"
#include "e.fn.h"
#include "e.sg.h"

#include SIG_INCL
#include <sys/stat.h>
#ifdef SYSIII
#include <fcntl.h>
#endif

#include <pwd.h>


extern void getpath ();
extern void fixtty ();
extern void cleanup ();
extern void la_abort ();
extern void fatal ();
extern void dofatal ();
extern void fatalpr ();
extern void dbgpr ();
extern void flushkeys ();
extern void edscrfile ();
extern void monexit ();
extern char default_tmppath [];

void _cleanup ()
{
    return;
}

#ifdef COMMENT
void
getpath (name, path, tryagain)
    char *name;
    char **path;
    Flag tryagain;
.
    /*
    Find full pathname for the executable file "name".
    Must be called before path is used, and if path is used
    in a child process, getpath should be been called in the PARENT
    so that it won't have to do any work on subsequent calls.
    */
.
    NOTE: Should use PATH environment variable!!!
#endif
void
getpath (name, path, tryagain)
register char *name;
char **path;
Flag tryagain;
{
    static char usrbin[] = "/usr/bin/";
    register char *cp1;
    register char *cp2;

    if (*path && !tryagain)
	return;

    if (access (name, 1) == 0) {  /* try current directory */
	*path = append (name, "");
	return;
    }

    cp1 = append (getmypath (), usrbin + 4);  /* try user's bin directory */
    cp2 = append (cp1, name);
    sfree (cp1);
    if (access (cp2, 1) == 0) {
	*path  = cp2;
	return;
    }
    sfree (cp2);

    cp2 = append (usrbin + 4, name);    /* try /bin */
    if (access (cp2, 1) == 0) {
	*path  = cp2;
	return;
    }
    sfree (cp2);

    *path = append (usrbin, name);  /* try /usr/bin */
    return;
}

#ifdef COMMENT
char *
getmypath ()
.
    /*
    Get user's login directory.
    Store it in the global 'mypath'.
    Return mypath.
    */
#endif
char *
getmypath ()
{
#if 0
    /* old fation !!! */
    register char *cp;
    register Short i;
    char    *path;
    char     pwbuf[132];
    uid_t    uid;

    uid = userid;
    if (mypath)
	return mypath;
    if (getpw (uid, pwbuf))
	fatal (FATALIO, "getpw failed!");

    for (cp = pwbuf; *cp != ':'; cp++)
	continue;               /* define my name             */
    *cp = '\0';                 /* terminate it               */
    myname = append (pwbuf, "");
    for (i = 4; i--;)           /* skip some fields           */
	while (*cp++ != ':')
	    continue;
    for (path = cp; *cp != ':'; cp++)
	continue;               /* and my path                */
    *cp = 0;                    /* make it asciz              */
    mypath = append (path, "");
#else

    /* use Posix call getowuid */
    struct passwd *p;
    uid_t uid;

    if ( mypath ) return mypath;

    uid=userid;
    p = getpwuid (uid);
    if ( p ) {
	myname = append (p->pw_name, "");
	mypath = append (p->pw_dir, "");
    } else {
	myname = "???";
	mypath = &default_tmppath [0];
    }
#endif
    return mypath;
}

#ifdef COMMENT
void
fixtty ()
.
    Restore tty to original state.
#endif
void
fixtty ()
{
    extern void reset_term ();

    d_put (0);
    if (ostyflg) {
#ifdef SYSIII
	ioctl (STDOUT, TCSETAW, &out_termio);           /* System III */
#else
	ioctl (STDOUT, TIOCSETN, &outstty);
#endif /* SYSIII */
	ostyflg = NO;
#ifdef MESG_NO
	if (ttynstr != NULL)
	    chmod (ttynstr, (int) (07777 & oldttmode));
#endif
    }
    if (istyflg) {
#ifdef SYSIII
	ioctl (STDIN, TCSETA, &in_termio);  /* System III */
	fcntl (STDIN, F_SETFL, fcntlsave);

#else /* -SYSIII */
	ioctl (STDIN, TIOCSETN, &instty);
#ifdef  CBREAK
	if (cbreakflg) {
#ifdef  TIOCSETC
	    (void) ioctl (STDIN, TIOCSETC, &spchars);
#endif  /* TIOCSETC */
#ifdef  TIOCSLTC
	    (void) ioctl (STDIN, TIOCSLTC, &lspchars);
#endif  /* TIOCSLTC */
	/*  cbreakflg = NO;  don't reset: screws up stop cmd and recoveries */
	}
#endif /* CBREAK */
#endif /* -SYSIII */

	istyflg = NO;
    }
    reset_term ();  /* reset to standard character set */
}

#ifdef COMMENT
void
cleanup (filclean, rmkeysflg)
    Flag filclean;
    Flag rmkeysflg;
.
    Cleanup before exiting
    If filclean != 0, remove changes file.
    If rmkeysflg != 0, remove keystroke file.
#endif
void
cleanup (filclean, rmkeysflg)
Flag filclean;
Flag rmkeysflg;
{
    if (filclean && la_cfile) {
	/* do we even need to close it? */
	unlink (la_cfile);
    }

    if (keyfile != NULL) { /* cleanup may be called before keyfile is open */
	if (rmkeysflg) {
	    unlink (keytmp);
	    unlink (bkeytmp);
	}
	else {
	    flushkeys ();       /* one last check for write errors */
	    fclose (keyfile);
	}
    }
    return;
}

#ifdef  SIGNALS
#ifdef COMMENT
/* XXXXXXXXXXXXXXXXXXXXXXX */
void
sig (num)
    unsigned Small num;
.
    Catch a signal, and call fatal.
#endif
#ifdef SIGARG
/* XXXXXXXXXXXXXXXXXXXXXXX */
void
sig (num)
Uint num;
{
    char errstr[100];
    static char *signames[] = {
	0,
#ifdef SYSIII
	"Hangup",
	"Interrupt",
	"Quit",
	"Illegal instruction",
	"Trace/BPT trap",
	"IOT trap",
	"EMT trap",
	"Floating exception",
	"Killed",
	"Bus error",
	"Segmentation fault",
	"Bad system call",
	"Broken pipe",
	"Alarm clock",
	"Terminated",
	"User signal 1",
	"User signal 2",
	"Child exited",
	"Power fail",
#else
	"Hangup",
	"Interrupt",
	"Quit",
	"Illegal instruction",
	"Trace/BPT trap",
	"IOT trap",
	"EMT trap",
	"Floating exception",
	"Killed",
	"Bus error",
	"Segmentation fault",
	"Bad system call",
	"Broken pipe",
	"Alarm clock",
	"Terminated",
	"Signal 16",
	"Stopped (signal)",
	"Stopped",
	"Continued",
	"Child exited",
	"Stopped (tty input)",
	"Stopped (tty output)",
	"Tty input interrupt",
	"Cputime limit exceeded",
	"Filesize limit exceeded",
#endif
    };

    if (num > NSIG)  /* hedge against old signal routine */
	num = 0;
    if (   num >= sizeof signames / sizeof signames[0]
	|| signames[num] == 0
       )
	sprintf (errstr, "Signal %d.", num);
    else
	sprintf (errstr, "Caught signal: %s.", signames[num]);
    fatal (FATALSIG, errstr);
    /* never returns */
}
#else
int
sig ()
{
    fatal (FATALSIG, "Fatal Trap");
    /* never returns */
}
#endif
#endif

#ifdef COMMENT
void
la_abort (msg, a1,a2,a3,a4,a5,a6,a7,a8,a9)
    char *msg;
.
    Called by the LA Package on fatal trouble.
    Pass the information on to a call to fatal().
#endif
/* VARARGS1 */
void
la_abort (msg, a1,a2,a3,a4,a5,a6,a7,a8,a9)
char *msg;
long a1,a2,a3,a4,a5,a6,a7,a8,a9;
{
    dofatal (LAFATALBUG, msg,a1,a2,a3,a4,a5,a6,a7,a8,a9);
    /* never returns */
    /* NOTREACHED */
}

char *bugwas;

#ifdef COMMENT
void
fatal (type, msg,a1,a2,a3,a4,a5,a6,a7,a8,a9)
    Flag    type;
    char   *msg;
.
    /*
    Fix the tty modes and the screen for exit.
    Handle a fatal error condition.
    Tell the user what happened.
    Copy the crash information doc file to the user's ternminal.
    Exit, with a core dump if appropriate.
    */
#endif
/* VARARGS2 */
void
fatal (type, msg,a1,a2,a3,a4,a5,a6,a7,a8,a9)
Flag    type;
char   *msg;
long a1,a2,a3,a4,a5,a6,a7,a8,a9;
{
     dofatal (type, msg, a1,a2,a3,a4,a5,a6,a7,a8,a9);
     /* NOTREACHED */
}

#ifdef COMMENT
void
dofatal (type, msg, a1,a2,a3,a4,a5,a6,a7,a8,a9)
    Flag    type;
    char  *msg;
.
    Invoked by fatal to do the work.
#endif
/* VARARGS2 */
void
dofatal (type, msg, a1,a2,a3,a4,a5,a6,a7,a8,a9)
Flag    type;
char  *msg;
long a1,a2,a3,a4,a5,a6,a7,a8,a9;
{
    char bugstr[BUFSIZ];

    if (ischild)
	exit (-1);
    fixtty ();
    if (windowsup) {
	screenexit (YES);
	windowsup = NO;
    }
    d_put (0);

    fatalpr ("\
\007=*==*==*==*==*==*=\n\
The editor just crashed because:\n    ");

    switch (type) {
    case FATALEXDUMP:
	fatalpr (bugwas =
		 "you gave the command to exit with the \"dump\" option.\n");
	break;

    case FATALMEM:
	fatalpr (bugwas = "you ran out of memory space.\n");
	break;

    case FATALIO:
	strcpy (bugstr, "\
you had fatal trouble with disk I/O (usually means no space on disk):\n");
	sprintf (&bugstr[strlen (bugstr)], msg,a1,a2,a3,a4,a5,a6,a7,a8,a9);
	strcat (bugstr, " ***\n");
	fatalpr (bugwas = bugstr);
	break;

    case FATALSIG:
    case FATALBUG:
    case LAFATALBUG:
	strcpy (bugstr, "the editor just caught a ");
#ifdef SIGNALS
	strcat (&bugstr[strlen (bugstr)],
		type == FATALSIG ? "signal.\n" :
		type == FATALBUG ? "bug in itself.\n" :
		"bug in itself (LA Package).\n");
#else
	strcat (&bugstr[strlen (bugstr)],
		type == FATALBUG ? "bug in itself.\n" :
		"bug in itself (LA Package).\n");
#endif
	sprintf (&bugstr[strlen (bugstr)], msg,a1,a2,a3,a4,a5,a6,a7,a8,a9);
	strcat (bugstr, "\n");
	fatalpr (bugwas = bugstr);
	break;
    }
    putchar ('\n');
    fflush (stdout);

    /* copy out the crashdoc (xdir_crash file) to the terminal */
    Block {
	/* XXXXXXXXXXXXXXXXXXXXXXXx */
	extern char * xdir_crash;
	int tmp;

	close (MAXSTREAMS - 1); /* pick any fd, just so one is available */
	close (MAXSTREAMS - 2); /* pick any fd, just so one is available */
	tmp = dup (1);      /* so that filecopy can close it */
	if (   (tmp = filecopy (xdir_crash, 0, (char *) 0, tmp, NO)) == -1
	    || tmp == -3
	   )
	    fatalpr ("Please notify the system administrators\n\
that the editor couldn't read file \"%s\".\n", xdir_crash);
    }

    if (keyfile != NULL)
	fclose (keyfile);
    if (replaying && keysmoved && inpfname && bkeytmp && strcmp (inpfname, bkeytmp) == 0)
	mv (bkeytmp, keytmp);
    fflush (stdout);

    if (loginflg) {
	signal (SIGINT, SIG_DFL);
	signal (SIGQUIT, SIG_DFL);
	sleep (60);
    }

    /* save disk space by truncating changes file */
    if (type != FATALEXDUMP)
	creat (la_cfile, 0600);

    switch (type) {
    case FATALEXDUMP:
    case FATALSIG:
    case FATALBUG:
    case FATALMEM:
	fatalpr ("\n\
(The following message comes to you from the shell:)\n\n");
	fflush (stdout);

	_cleanup ();
#ifdef SIGNALS
	{
	    /* set signals back to default for abort() */
	    int i;
	    for (i = 1; i <= NSIG; ++i)
		(void) signal (i, SIG_DFL);
	}
#endif
	abort();

    case FATALIO:
    default:
#ifdef SIGNALS
	{
	    /* set signals back to default */
	    int i;
	    for (i = 1; i <= NSIG; ++i)
		(void) signal (i, SIG_DFL);
	}
#endif
    }
#ifdef PROFILE
    monexit (-1);
    /* NOTREACHED */
#else
    exit (-1);
#endif
    /* never returns */
    /* NOTREACHED */
}

#define NUMADRS 1500

#ifdef COMMENT
char *
gsalloc (oldp, ncopy, newsize, fatalflg)
    char *oldp;
    int ncopy;
    int newsize;
    Flag fatalflg;
.
    'Grow salloc' - Like the new V7 realloc call.
    Allocs a new size block of memory for the item and copies the item
    from its old position to the new position.
    Returns the new position or NULL if salloc returned NULL and fatalflg
    is NO.
#endif
char *
gsalloc (oldp, ncopy, newsize, fatalflg)
char *oldp;
int ncopy;
int newsize;
Flag fatalflg;
{
    char *newp;

    newp = salloc (newsize, fatalflg);
    if ((ncopy = min (ncopy, newsize)) > 0)
	move (oldp, newp, (Uint) ncopy);
    sfree (oldp);
    return newp;
}

#ifdef COMMENT
char *
okalloc (n)
    int n;
.
    Do a calloc().
#endif
char *
okalloc (n)
int n;
{
    extern char *calloc ();

    return calloc ((unsigned) n, 1);
}


#ifdef COMMENT
char *
salloc (n, fatalflg)
    int n;
    Flag fatalflg;
.
    Do a calloc().
    Call fatal if calloc() returned NULL and fatalflg != 0.
    Else return the pointer to the alloced memory.
#endif
char *
salloc (n, fatalflg)
int n;
Flag fatalflg;
{
    register char  *cp;
    extern char *calloc ();

    n = (n+15)/8;
    if ((cp = calloc ((unsigned) n , 8)) == NULL) {
	if (fatalflg)
	    fatal (FATALMEM, "");
	else
	    mesg (ERRALL + 1, "You have run out of memory. Get out NOW!");
    }


    return cp;
}


#ifdef COMMENT
char *
append (name, ext)
    char   *name;
    char   *ext;
.
    allocs enough space to hold the catenation of the strings name and ext.
    Returns pointer to new, alloced string.
#endif
char *
append (name, ext)
char   *name;
char   *ext;
{
    int lname;
    char *newname;

    /* old version -------
    register char  *c,
                   *d,
                   *newname;

    for (lname = 0, c = name; *c++; ++lname)
	continue;
    for (c = ext; *c++; ++lname)
	continue;
    for (newname = c = salloc (lname + 1, YES), d = name; *d; *c++ = *d++)
	continue;
    for (d = ext; *c++ = *d++;)
	continue;
    ------- */

    lname = strlen (name) + strlen (ext);
    newname = salloc (lname + 1, YES);
    strcpy (newname, name);
    strcat (newname, ext);
    return newname;
}

#ifdef COMMENT
char *
copy (s1, s2)
    char *s1,
    char *s2;
.
    Copy s1 to s2 and return address of '\0' at end of new s2.
#endif
char *
copy (s1, s2)
register char *s1;
register char *s2;
{
    while (*s2++ = *s1++)
	continue;
    return --s2;
}

#ifdef COMMENT
char *
s2i (s, i)
    char   *s;
    int    *i;
.
    Converts string s to int and returns value in i.
    Returns pointer to the first char encountered that is not part
    of a valid decimal number.
    If the returned pointer == s, then no number was converted.
#endif
char *
s2i (s, i)
char   *s;
int    *i;
{
    register Char   lc;
    register Char   c;
    register Short  val;
    Flag    maxi;
    Short   sign;

    maxi = NO;
    sign = 1;
    val = 0;
    lc = Z;
    for (c = *s; ; lc = c, c = *++s) {
	if (c >= '0' && c <= '9') {
	    if (maxi)
		continue;
	    if ((val = 10 * val + c - '0') < 0 && sign > 0)
		maxi = YES;
	    continue;
	}
	else if (c == '-' && lc == 0) {
	    sign = -1;
	    continue;
	}
	else if (lc == '-')
	    s--;
	break;
    }
    if (maxi)
	*i = MAXINT;
    else
	*i = val * sign;
    return s;
}


#ifdef COMMENT
Flag
mv (name1, name2)
    char name1[];
    char name2[];
.
    Rename file name1 to file name2 with link() and unlink().
    Return YES if all went OK, else NO.
#endif
Flag
mv (name1, name2)
char name1[];
char name2[];
{
#ifdef RENAME
    return rename (name1, name2) != -1;
#else
    unlink (name2);
    return     link (name1, name2) != -1
	  && unlink (name1) != -1;
#endif
}

#ifdef COMMENT
void
fatalpr (fmt,a1,a2,a3,a4,a5,a6,a7,a8,a9)
char *fmt;
.
    Like a printf, but the output also goes out to the debug output file,
    if any.
#endif
/* VARARGS1 */
void
fatalpr (fmt,a1,a2,a3,a4,a5,a6,a7,a8,a9)
char *fmt;
long a1,a2,a3,a4,a5,a6,a7,a8,a9;
{
    dbgpr (fmt, a1,a2,a3,a4,a5,a6,a7,a8,a9);
    fprintf(stdout, fmt, a1,a2,a3,a4,a5,a6,a7,a8,a9);
    return;
}


#ifdef COMMENT
void
dbgpr (fmt,a1,a2,a3,a4,a5,a6,a7,a8,a9)
char *fmt;
.
    For use with the "-d=" option.  If the option is invoked, then
    all calls to dbgpr will print out to the file specified, else
    dbgpr is a noop.
#endif
/* VARARGS1 */
void
dbgpr (fmt,a1,a2,a3,a4,a5,a6,a7,a8,a9)
char *fmt;
long a1,a2,a3,a4,a5,a6,a7,a8,a9;
{
    if (dbgfile != NULL) {
	fprintf(dbgfile, fmt, a1,a2,a3,a4,a5,a6);
	fflush (dbgfile);
    }
    return;
}




#ifdef COMMENT
void
flushkeys ()
.
    Flush the keys to the keystroke file.
#endif
void
flushkeys ()
{
    fflush (keyfile);
    if (ferror (keyfile))
	fatal (FATALIO, "ERROR WRITING keys FILE.");
    return;
}

#ifdef COMMENT
Flag
okwrite ()
.
    Return YES if it is ok to modify curfile, else NO.
#endif
Flag
okwrite ()
{
    return (fileflags[curfile] & CANMODIFY) ? YES : NO;
}

#ifdef COMMENT
/* VARARGS 5 */
Small
filecopy (file1, fd1, file2, fd2, doutime, crmode)
    Fd fd1;
    Fd fd2;
    char *file1;
    char *file2;
    Flag doutime;
    int crmode;
.
    /*
    Copy one file to another.
    If 'file1' is non-NULL, try to open it for reading,
    else assume 'fd1' is already open.
    If 'file2' is non-NULL, try to creat it for writing,
    else assume 'fd2' is already open for writing.
    If 'doutime' is non-0, set times of file2 to times of file1 (only
    possible if 'file2' is non-null.
    Returns:
      0  All went ok
     -1  Can't open 'file1'
     -2  Can't creat 'file2'
     -3  Read error
     -4  Write error
     */
#endif
/* VARARGS 4 */
Small
filecopy (file1, fd1, file2, fd2, doutime, crmode)
register Fd fd1;
register Fd fd2;
char *file1;
char *file2;
Flag doutime;
int crmode;
{
    char buf[BUFSIZ];
    register int j;
    Small retval;
    time_t times[2];

    if (   file1
	&& (fd1 = open (file1, 0)) < 0
       ) {
	if (!file2)
	    close (fd2);
	return -1;
    }

    if (file2) {
	if ((fd2 = creat (file2, crmode)) < 0) {
	    if (!file1)
		close (fd1);
	    return -2;
	}
    }
    else
	doutime = NO;
    if (doutime) Block {
	struct stat stbuf;
	fstat (fd1, &stbuf);
	times[0] = stbuf.st_atime;
	times[1] = stbuf.st_mtime;
    }

    for (;;) {
	if ((j = read (fd1, buf, sizeof buf)) == 0) {
	    retval = 0;
	    break;
	}
	if (j < 0)
	    retval = -3;
	else if (write (fd2, buf, j) != j)
	    retval = -4;
	else
	    continue;
	unlink (file2);
	break;
    }
    if (!file1)
	close (fd1);
    if (!file2)
	close (fd2);
    if (doutime)
	utime (file2, times);
    return retval;
}

#ifdef COMMENT
void
edscrfile (puflg)
    Flag puflg;
.
    Edit the 'scratch' file.
    If that fails, edit the 'default' file.
#endif
void
edscrfile (puflg)
Flag puflg;
{
    if (editfile (scratch, (Ncols) -1, (Nlines) -1, 2, puflg) != 1)
	eddeffile (puflg);
    return;
}

#ifdef PROFILE

Flag profiling;

#ifdef COMMENT
void
monexit (status)
    int status;
.
    call monitor(0), move mon.out to /usr/tmp/e/, and exit.
#endif
void
monexit (status)
int status;
{
    static char template[] = "/usr/tmp/e/pXXXXXX";
    char tempfile[sizeof template];
    extern char *mktemp ();

    if (profiling) {
	strcpy (tempfile, template);
	mktemp (tempfile);
	monout (tempfile);
	profil ((char *) 0, 0, 0, 0);
    }
    _cleanup ();
    _exit (status);
}

#ifdef VAX_MONITOR
/* /usr/src/libc/mon.c modified to take a filename other than mon.out */
static int *sbuf, ssiz;

monout(name)
char *name;
{
	register int o;

	profil(0, 0, 0, 0);
	o = creat(name, 0666);
	write(o, (char *) sbuf, ssiz);
	close(o);
	return;
}

monitor(lowpc, highpc, buf, bufsiz, cntsiz)
char *lowpc, *highpc;
int *buf, bufsiz;
{
	register int o;
	struct phdr {
		int *lpc;
		int *hpc;
		int ncnt;
	};
	struct cnt {
		int *pc;
		long ncall;
	};

	if (lowpc == 0) {
		monout ("mon.out");
		return;
	}
	sbuf = buf;
	ssiz = bufsiz;
	buf[0] = (int)lowpc;
	buf[1] = (int)highpc;
	buf[2] = cntsiz;
	o = sizeof(struct phdr) + cntsiz*sizeof(struct cnt);
	buf = (int *) (((int)buf) + o);
	bufsiz -= o;
	if (bufsiz<=0)
		return;
	o = ((highpc - lowpc)>>1);
	if(bufsiz < o)
		o = ((float) bufsiz / o) * 32768;
	else
		o = 0177777;
	profil(buf, bufsiz, lowpc, o);
}
#else
static int *sbuf, ssiz;

monout(name)
char *name;
{
	register int o;

	profil(0, 0, 0, 0);
	o = creat(name, 0666);
	write(o, (char *) sbuf, ssiz<<1);
	close(o);
	return;
}

monitor(lowpc, highpc, buf, bufsiz, cntsiz)
char *lowpc, *highpc;
int *buf, bufsiz;
{
	register int o;

	if (lowpc == 0) {
		monout("mon.out");
		return;
	}
	ssiz = bufsiz;
	buf[0] = (int)lowpc;
	buf[1] = (int)highpc;
	buf[2] = cntsiz;
	sbuf = buf;
	buf += 3*(cntsiz+1);
	bufsiz -= 3*(cntsiz+1);
	if (bufsiz<=0)
		return;
	o = ((highpc - lowpc)>>1) & 077777;
	if(bufsiz < o)
		o = ((long)bufsiz<<15) / o;
	else
		o = 077777;
	profil(buf, bufsiz<<1, lowpc, o<<1);
}
#endif

#endif

