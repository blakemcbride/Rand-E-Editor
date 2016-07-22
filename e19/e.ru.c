#ifdef COMMENT
--------
file e.ru.c
    functions requiring forking a subprocess
#endif

#ifdef COMMENT
	Copyright abandoned, 1983, The Rand Corporation
#endif


#include "e.h"
#include "e.e.h"
#include "e.m.h"
#include "e.cm.h"
#include "e.ru.h"

#include SIG_INCL

#define CANTEXEC (-2)

Flag    fill_hyphenate = NO;    /* default: don't split hyphenated words */

S_looktbl filltable [] = {
    "width"   , 0       ,
    0         , 0
};

extern void doexec ();

/* XXXXXXXXXXXXXXXXXXXXXXXXX */
static execabortmesg ();
static noforkmesg ();
static nopipemesg ();
static noexecmesg ();
static Cmdret runlines ();
static Flag dowait ();

extern void alarmproc ();


#ifdef COMMENT
Cmdret
filter (whichfilter, closeflg)
    Small   whichfilter;
    Flag    closeflg;
.
    Common code for the "fill", "justify", and "center" commands.
#endif
Cmdret
filter (whichfilter, closeflg)
Small   whichfilter;
Flag    closeflg;
{
    Nlines stline;
    register Cmdret tmp;
    register Cmdret retval = 0;
    char *cp;
    Cmdret fillopts ();

    if (!okwrite ())
	return NOWRITERR;

    cp = cmdopstr;
    tmplinewidth = linewidth;
    if ((tmp = scanopts (&cp, whichfilter != CENTERNAMEINDEX,
			 filltable, fillopts)) < 0)
	return tmp;
/*       3 marked area        \
 *       2 rectangle           \
 *       1 number of lines      > may have stopped on an unknown option
 *       0 no area spec        /
 *      -2 ambiguous option     ) see parmlines, parmcols for type of area
 *   <= -3 other error
 **/
    if (*cp != '\0')
	return CRBADARG;
    switch (tmp) {
    case 0:
#ifdef LMCMARG
	setmarg (&linewidth, tmplinewidth);
#else
	linewidth = tmplinewidth;
#endif
	if (whichfilter == PRINTNAMEINDEX) {
	    stline = 0;
	    parmlines = la_lsize (curlas);
	}
	else {
	    parmlines = whichfilter == CENTERNAMEINDEX? 1
			: lincnt (curwksp->wlin + cursorline, 1, 2);
	    stline = curwksp->wlin + cursorline;
	}
	retval = filtlines (whichfilter,
			     stline,
			      parmlines, YES, closeflg);
	break;

    case 1:
#ifdef LMCMARG
	setmarg (&linewidth, tmplinewidth);
#else
	linewidth = tmplinewidth;
#endif
	retval = filtlines (whichfilter, curwksp->wlin + cursorline,
			    parmlines, YES, closeflg);
	break;

    case 2:
	return NORECTERR;
    case 3:
	if (markcols)
	    return NORECTERR;
#ifdef LMCMARG
	setmarg (&linewidth, tmplinewidth);
#else
	linewidth = tmplinewidth;
#endif
	retval = filtmark (whichfilter, closeflg);
	break;

    default:
	return tmp;
    }
    return retval;
}
#ifdef LMCAUTO
#ifdef COMMENT
Cmdret
parsauto (cmdmod)
	Flag cmdmod;
.
    Parse the options on the auto command.
#endif
Cmdret
parsauto (cmdmod)
	Flag cmdmod;
{
    register Cmdret tmp;
    char *cp;
    Cmdret fillopts ();

    cp = cmdopstr;
    tmplinewidth = linewidth;
    if ((tmp = scanopts (&cp, YES,
			 filltable, fillopts)) < 0)
	return tmp;
    if (*cp != '\0')
	return CRBADARG;
#ifdef LMCMARG
    setmarg (&linewidth, tmplinewidth);
#endif
    infoauto (cmdmod);
    return CROK;
}
#endif
#ifdef COMMENT
Cmdret
fillopts (cp, str, tmp, equals)
    char *cp;
    char **str;
    Small tmp;
    Flag equals;
.
    Get the options to the "fill", "justify", etc. commands.
    For now the only option is "width=".
#endif
/* ARGSUSED */
Cmdret
fillopts (cp, str, tmp, equals)
char *cp;
char **str;
Small tmp;
Flag equals;
{
    if (tmp == 0) { /* "width" table entry */
	if ((tmp = doeq (&cp, &tmplinewidth)) < 0)
	    return tmp;
	*str = cp;
	return 1;
    }
    return -1;
}

#ifdef COMMENT
Cmdret
filtmark (whichfilter, closeflg)
    Small   whichfilter;
    Flag    closeflg;
.
    Common code for the "fill", "justify", and "center" commands
    when there is an area marked.
#endif
Cmdret
filtmark (whichfilter, closeflg)
Small   whichfilter;
Flag    closeflg;
{
    register Flag moved;
    register Cmdret retval = 0;

    if (markcols)
	return NORECTERR;
    moved = gtumark (YES);
	retval = filtlines (whichfilter, topmark (), marklines,
			    !moved, closeflg);
    if (moved)
	putupwin ();
    unmark ();
    return retval;
}

extern char *filters[], *filterpaths[];

#ifdef COMMENT
Cmdret
filtlines (whichfilter, from, number, puflg, closeflg)
    Small whichfilter;
    Nlines  from;
    Nlines  number;
    Flag    puflg;
    Flag    closeflg;
.
    Called by filter() and filtmark().
    Prepare for and call to runlines().
#endif
Cmdret
filtlines (whichfilter, from, number, puflg, closeflg)
register Small whichfilter;
Nlines  from;
Nlines  number;
Flag    puflg;
Flag    closeflg;
{
    char *args[5];              /* watch this size! */
    register Small ix;

    ix = Z;
    args[ix] = filters[whichfilter];
    switch (whichfilter) {
    case FILLNAMEINDEX:
    case JUSTNAMEINDEX:
    case CENTERNAMEINDEX: Block {
	    char buf[10];
	    sprintf (buf, "-l%d", linewidth);
	    args[++ix] = buf;
	    if (whichfilter != CENTERNAMEINDEX && fill_hyphenate == YES)
		args[++ix] = "-h";
	    break;
	}
    case PRINTNAMEINDEX:
	break;

    default:
	mesg (TELALL + 1, "BUG!");
	return CRBADARG;
    }
    args[++ix] = 0;

    mesg (TELALL + 2, cmdname, "ing.  Please wait.");
    d_put (0);
    loopflags.beep = YES;
    return runlines (filters[whichfilter], filterpaths[whichfilter],
		     args, from, number, QADJUST, puflg, closeflg, YES);
}

#ifdef COMMENT
Cmdret
run (cmdstr, cmdval)
    char *cmdstr;
    Short cmdval;
.
    Do the "run" command.
#endif
Cmdret
run (cmdstr, cmdval)
char *cmdstr;
Short cmdval;
{
    static char *shargs[] = {(char *) 0, "-c", 0, 0};
    register Small j;
    Cmdret   retval;
    Nlines  tlines;
    Nlines  stline,     /* start line of exec */
	    nmlines;    /* num of lines to send to program */
    Flag   moved = 0;
    char *cp;
#ifdef  ENVIRON
    extern char *getenv ();
#endif

    cp = cmdstr;
    j = getpartype (&cp, YES, NO, curwksp->wlin + cursorline);
    if (j == 1) {
	if (curmark)
	    return NOMARKERR;
	tlines = parmlines;
    }
    else if (j == 2)
	return NORECTERR;
    else if (curmark) {
	if (markcols)
	    return NORECTERR;
	moved = gtumark (YES);
	tlines = marklines;
	unmark ();
    }
    else
	tlines = 0;

    for (; *cp && *cp == ' '; cp++)
	continue;

    stline = curwksp->wlin + cursorline;
    nmlines = lincnt (stline, tlines, 1);

    mesg (TELALL + 2, "RUN: ", cp);
    d_put (0);
    loopflags.beep = YES;

#ifdef  ENVIRON
    if (!(shpath = getenv ("SHELL")))
#endif
	getpath ("sh", &shpath, NO);
    shargs[2] = cp;
    retval = runlines (cmdval == CMDRUN ? "run" : "feed",
		       shpath, shargs, stline, nmlines,
		       QRUN, !moved, cmdval == CMDRUN, NO);
    if (moved)
	putupwin ();
    return retval;
}

#ifdef VFORK
Flag execfailed;
#endif

#ifdef COMMENT
static
Cmdret
runlines (funcnm, progpath, args, from, number, iqbuf, puflg, closeflg, safe)
    char *funcnm;
    char   *progpath;
    char   *args[];
    Nlines  from;
    Nlines  number;
    Small   iqbuf;
    Flag    puflg;
    Flag    closeflg;
    flag    safe;       /* OK for program to write directly on changes file */
.
    Do the fork and exec, send the lines to the execed program,
    and arrange that the stdout and stderr of the program are collected
    as one stream and inserted in the current file.
    If closeflg is non-0, delete the lines sent to the program and put them
    into iqbuf.
#ifndef RUNSAFE /* RUNSAFE must be defined in e.h -- not here */
    /*
    If safe, then
	e | prog >> chgfile
    else
	e | prog | /etc/e/run >> chgfile
    The only purpose served by the second case is to prevent prog from
    seeking backwards on the changes file.  It'd be nice if we could do
    that without the extra process and pipe.
    */
#endif
#endif
static
Cmdret
runlines (funcnm, progpath, args, from, number, iqbuf, puflg, closeflg, safe)
char *funcnm;
char   *progpath;
char  **args;
Nlines  from;
Nlines  number;
Small   iqbuf;
Flag    puflg;
Flag    closeflg;
Flag    safe;       /* OK for program to write directly on changes file */
{
    /* XXXXXXXXXXXXXXXXXXXXXXX */
    extern char * xdir_run;
    static char *runargs[2];
    int  pipe1[2];
    int  child1;
#ifndef RUNSAFE
    int  child2;
    int  progid;
    Fd   output;
    long chgend;
    int  pipe2[2];
#endif

    runargs[0] = xdir_run;
    if (pipe (pipe1) == -1) {
	nopipemesg ();
	return CROK;
    }

    /* We are about to let a forked program write directly onto the end of
     * the changes file.  The FF buffer cache might contain the last
     * block of the changes file, and it might need to be written out, so
     * we must do it now, not after we have written onto the end of the
     * changes file or the new stuff might get clobbered with zeros
     **/
    getline (-1);   /* flush so that file will be its full size */
    ff_flush (la_chgffs);

    chgend = ff_seek (la_chgffs, (long) 0, 2);

    /* First we make the process that writes onto the change file */
    /* If safe, then this is the program to be run. */
#ifdef VFORK
    execfailed = NO;
    if ((child1 = vfork ()) == -1) {
#else
    if ((child1 = fork ()) == -1) {
#endif
	close (pipe1[0]);
	close (pipe1[1]);
	noforkmesg ();
	return CROK;
    }
    if (!child1) {
	/* child */
	int chgfd;
	ischild = YES;          /* looked at by fatal (..) */
	if ((chgfd = open (la_cfile, 1)) == -1)
	    _exit (-1);
	Block {
	    extern long lseek ();
	    (void) lseek (chgfd, chgend, 0);
	}
	/* application program run by sh */
	if (safe) {
	    if (!args[0])
		args[0] = progpath;
	    doexec (pipe1[0], chgfd, progpath, args);
	}
	else
	    doexec (pipe1[0], chgfd, runargs[0], runargs);
	/* NOTREACHED */
    }
    ischild = NO;
    close (pipe1[0]);
#ifdef VFORK
    if (execfailed) {
	close (pipe1[1]);
	noexecmesg (safe ? progpath : runargs[0]);
	return CROK;
    }
#endif

#ifndef RUNSAFE
    /* Next, if not safe, we make the 'run' process that goes between E and
       the program */
    if (safe) {
	progid = child1;
	output = pipe1[1];
	child2 = 0;
    }
    else {
	if (pipe (pipe2) == -1) Block {
	    int retstat;
	    /* this should never happen because we have reserved fd's for */
	    /* piping */
	    nopipemesg ();
 nofork:
	    close (pipe1[1]);
 killret:
	    kill (child1, SIGKILL);
	    while (wait (&retstat) != child1)
		continue;
	    return CROK;
	}
#ifdef VFORK
	if ((child2 = vfork ()) == -1) {
#else
	if ((child2 = fork ()) == -1) {
#endif
	    close (pipe2[0]);
	    close (pipe2[1]);
	    noforkmesg ();
	    goto nofork;
	}
	if (!child2) {
	    /* child */
	    ischild = YES;      /* looked at by fatal (..) */
	    if (!args[0])
		args[0] = progpath;
	    doexec (pipe2[0], pipe1[1], progpath, args);
	    /* NOTREACHED */
	}
#ifdef VFORK
	ischild = NO;
#endif
	close (pipe1[1]);
	close (pipe2[0]);
	output = pipe2[1];
	progid = child2;
#ifdef VFORK
	if (execfailed) {
	    close (pipe2[1]);
	    noexecmesg (progpath);
	    goto killret;
	}
#endif
    }
#endif

    /* send the lines to the program */
    if (number > 0) {
	if (number > la_lsize (curlas) - from)
	    number = la_lsize (curlas) - from;
	intok = YES;
#ifdef RUNSAFE
	if (la_lflush (curlas, from, number, pipe1[1], YES, EXECTIM, NO)
#else
	if (la_lflush (curlas, from, number, output, YES, EXECTIM, NO)
#endif
	    < number) {
	    intok = NO;
	    execabortmesg (funcnm);
	    return CROK;
	}
	intok = NO;
    }
#ifdef RUNSAFE
    close (pipe1[1]);
#else
    close (output);
#endif

#ifdef RUNSAFE
    if (   dowait (child1)
#else
    if (   dowait (progid, child1, child2)
#endif
	&& receive (chgend, from, closeflg ? number : 0, iqbuf, puflg)
       )
	return CROK;
    execabortmesg (funcnm);
    d_put (0);
    return CROK;
}

#ifdef COMMENT
void
doexec (fd0, fd1, path, args)
    Fd  fd0;
    Fd  fd1;
    char *path;
    char *args[];
.
    Do the exec for runlines().
#endif
void
doexec (fd0, fd1, path, args)
Fd  fd0;
Fd  fd1;
char *path;
char *args[];
{
    register int j;

    close (0);              /* hook up stdin to fd0 */
    dup (fd0);
    close (1);              /* hook up stdout and stderr to fd1 */
    dup (fd1);
    close (2);
    dup (fd1);
    for (j = 3; j < NOFILE; ) /* Pass on virgin fd state */
	close (j++);
    /* Don't need to set sigs back to defaults.  Since they are all caught,
     * they'll automatically be set to default.
     **/
    execv (path, args);
#ifdef VFORK
    execfailed = YES;
#endif
    _exit (CANTEXEC);
    /* NOTREACHED */
}

#ifdef RUNSAFE
#ifdef COMMENT
static Flag
dowait (progid)
    int progid;
.
    Wait for the child process to exit,
    and watch for the user to type CCINT.
    Alarms are used to periodically interrupt the wait system call so that
    we can see if the user has interrupted.
    If the user interrupts while we are waiting for the child process,
    kill it.
    Collect the exit status of the progid and print an error message if it
    is offensive.
    If interrupted by the user, return NO, else YES.
#endif
static Flag
dowait (progid)
Reg1 int progid;
{
#else
#ifdef COMMENT
static Flag
dowait (progid, child1, child2)
    int progid;
    int child1;
    int child2;
.
    Wait for the child processes child1 and child2 to exit,
    and watch for the user to type CCINT.
    Alarms are used to periodically interrupt the wait system call so that
    we can see if the user has interrupted.
    If the user interrupts while we are waiting for either or both of the
    child processes, kill it or them.
    Progid tells which of child1 or child2 we are to look to for the
    exit status.
    Collect the exit status of progid and print an error message if it
    is offensive.
    If interrupted by the user, return NO, else YES.
#endif
static Flag
dowait (progid, child1, child2)
int progid;
Reg1 int child1;
Reg2 int child2;
{
    int tretstat;
#endif
    int retstat;
    Reg4 void (*alarmsig) ();

    alarm (0);
    alarmed = NO;
    alarmsig = signal (SIGALRM, alarmproc);
    /* wait for the program(s) to terminate */
    /* or user to type CCINT, whichever is first */
    for (;;) {
	alarmed = NO;
	(void) signal (SIGALRM, alarmproc);
	alarm (EXECTIM);
#ifdef RUNSAFE
	if (wait (&retstat) == progid)
	    progid = 0;
	if (!alarmed || !progid) {
#else
	do Block {
	    Reg3 int proc;
	    proc = wait (&tretstat);
	    if (proc == child1)
		child1 = 0;
	    else if (proc == child2)
		child2 = 0;
	    if (proc  == progid)
		retstat = tretstat;
	} while (!alarmed && (child1 || child2));
	if (!alarmed || (!child1 && !child2)) {
#endif
	    alarm (0);
	    alarmed = NO; /* in case it just happened before the alarm (0) */
	    break;
	}
	if (sintrup ())
	    break;
    }
    (void) signal (SIGALRM, alarmsig);

    if (alarmed) {
	alarmed = NO;
#ifdef RUNSAFE
	if (progid) {
	    kill (progid, SIGKILL);
	    while (wait (&retstat) != child1)
		continue;
	}
#else
	if (child1) {
	    kill (child1, SIGKILL);
	    while (wait (&tretstat) != child1)
		continue;
	}
	if (child2) {
	    kill (child2, SIGKILL);
	    while (wait (&tretstat) != child2)
		continue;
	}
#endif
	return NO;
    }
    if (sintrup ())
	return NO;

    Block {
	Reg3 int exitstat;
	exitstat = (retstat >> 8) & 0377;
	if (exitstat == CANTEXEC) {
	    mesg (ERRALL + 1, "Can't find program to execute.");
	    return YES;
	}
	if (exitstat || (retstat & 0377))
	    mesg (ERRALL + 1, "Program either failed or was not found");
    }
    return YES;
}

#ifdef COMMENT
Flag
receive (chgend, from, nclose, iqbuf, puflg)
    long    chgend;
    Nlines  from;
    Nlines  nclose;
    Small   iqbuf;
    Flag    puflg;          /* putup when done */
.
    Receive the output from the forked program.
    It will have been written onto the end of the changes file starting at
    chgend.  Parse it, and insert it at 'from' after closing 'nclose' lines
    and putting them into 'iqbuf' if there were any to close.
#endif
Flag
receive (chgend, from, nclose, iqbuf, puflg)
long    chgend;
Nlines  from;
Nlines  nclose;
Small   iqbuf;
Flag    puflg;          /* putup when done */
{
    Nlines nn;
    register Nlines nins;
    Nlines lsize;
    Nlines endgap;

    /*  first order of business after letting another program write on the
     *  end of the changes file is to do an la_tcollect to nail down
     *  the new size of the changes file and collect the lines of text.
     **/
    if (!la_tcollect (chgend)) {
	mesg (ERRALL + 1, "tcollect bug");
	return YES;
    }

    getline (-1);
    if ((endgap = from - (lsize = la_lsize (curlas))) > 0) {
	if (!extend (endgap)) {
	    mesg (ERRALL + 1, "Can't extend the file");
	    return YES;
	}
	else
	    lsize += endgap;
    }

    (void) la_lseek (curlas, from, 0);
    if (nclose <= 0 || from >= lsize) Block {
	static La_linepos zlines = 0;
	nclose = 0;
	nins = la_lrcollect (curlas, &zlines);
    /*? Watch out if collect was aborted */
    }
    else Block {
	register La_stream *tlas;
	register La_stream *dlas;   /* where to put the deleted stuff */
	nclose = min (nclose, lsize - from);
	clean (OLDLFILE);
	nn = la_lsize (dlas = &fnlas[OLDLFILE]);
	(void) la_align (dlas, tlas = &qbuf[iqbuf].buflas);
	la_stayset (tlas);
	nins = la_lrcollect (curlas, &nclose, dlas);
    /*? Watch out if collect was aborted */
	la_setrlines (tlas, nclose);
	la_stayclr (tlas);
	qbuf[iqbuf].ncols = 0;
	redisplay (OLDLFILE, nn, 0, nclose, YES);
    }
    redisplay (curfile, from, min (nins, nclose), nins - nclose, puflg);
    if (puflg)
	poscursor (cursorcol, from - curwksp->wlin);
    return YES;
}

#ifdef COMMENT
/* XXXXXXXXXXXXXXXXXXXXXXXXXX */
void
alarmproc ()
.
    Called from SIGALRM interrupt.  Merely sets the global flag 'alarmed'.
#endif
/* XXXXXXXXXXXXXXXXXXXXXXXXXX */
void
alarmproc ()
{
    alarmed = YES;
    return;
}

static
noexecmesg (progpath)
char *progpath;
{
    mesg (ERRALL + 2, "Can't execute ", progpath);
}

static
nopipemesg ()
{
    mesg (ERRALL + 1, "Can't open a pipe.");
    return;
}

static
noforkmesg ()
{
    mesg (ERRALL + 1, "Can't fork a sub-process.");
    return;
}

static
execabortmesg (name)
char *name;
{
    mesg (ERRALL + 3, "\"", name, "\" command aborted.");
    return;
}

