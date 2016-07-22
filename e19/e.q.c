#ifdef COMMENT
--------
file e.q.c
    exit command and related stuff
#endif

#ifdef COMMENT
	Copyright abandoned, 1983, The Rand Corporation
#endif

#include <sys/utsname.h>

#include "e.h"
#include "e.m.h"
#include "e.cm.h"
#include "e.fn.h"
#include "e.tt.h"
#include "e.it.h"

#include SIG_INCL

extern char *getenv ();
extern Flag is_eddeffile (Fn fn);
extern void close_dbgfile (char *str);
extern char *command_msg;

static void docall ();
static void reenter ();

/* Quit argument */

#define EXABORT         0
#define EXDUMP          1
#define EXNORMAL        2
#define EXNOSAVE        3
#define EXQUIT          4

S_looktbl exittable [] = {
    "abort"   , EXABORT    ,
    "dump"    , EXDUMP     ,
    "nosave"  , EXNOSAVE   ,
    "quit"    , EXQUIT     ,
    0, 0
};


#ifdef COMMENT
Cmdret
shell ()
.
    Do the "shell" command.
#endif
Cmdret
shell ()
{
    register char *cp;
    static char *args[] = {"-", (char *)0};

    for (cp = cmdopstr; *cp && *cp == ' '; cp++)
	continue;

    if (*cp != '\0')
	return call ();

    if (saveall () == NO)
	return CROK;

    docall (NO, args);
    /* never returns */
    /* NOTREACHED */
}

#ifdef COMMENT
Cmdret
call ()
.
    Do the "call" command.
#endif
Cmdret
call ()
{
    register char *cp;
    static char *args[] = {"sh", "-c", (char *)0, (char *)0};

    for (cp = cmdopstr; *cp && *cp == ' '; cp++)
	continue;

    if (*cp == '\0')
	return CRNEEDARG;

    if (saveall () == NO)
	return CROK;

    args[2] = cp;
    docall (YES, args);
    /* never returns */
    /* NOTREACHED */
}

#ifdef COMMENT
static void
docall (waitflg, args)
    Flag waitflg;
    char *args[];
.
    Code called by shell() and call().
    Do all the cleanup as you would on exit.
    Fork a shell with 'args' and when it exits, re-exec editor.
#endif
static void
docall (waitflg, args)
    Flag waitflg;
    char *args[];
{
    extern char **argarray;
    char *ename;
    int child;
    int retstat;
    register Small j;

    savestate ();               /* to work from where it  left off    */
    cleanup (YES, YES);           /* restore terminal modes; free tmp   */
    for (j = 1; j <= NSIG ; j++) {
	if (j == SIGINT || j == SIGQUIT)
	    signal (j, SIG_IGN);
	else
	    signal (j, SIG_DFL);
    }
    for (j = 2; j < 25 /*HIGHFD*/; )
	close (j++);
    dup (STDOUT);

#ifdef  ENVIRON
    shpath = append (getenv ("SHELL"), "");
#else
    getpath ("sh", &shpath, NO);
#endif
    args[0] = shpath;
    if ((child = fork ()) != -1) {
	if (child == 0) {
	    execv (shpath, args);
	    printf ("Can't exec shell\n");
	    fflush (stdout);
#ifdef PROFILE
	    monexit (-1);
#else
	    exit (-1);
#endif
	    /* NOTREACHED */
	}
	else {
	    while (wait (&retstat) != child)
		continue;
	}
    }
    else
	printf ("Can't fork a shell\n");

    reenter (waitflg);

/* What is needed here is some way of re-execing this same image to get a
 * fresh data space, but using the same text.  Re-execing it by name is
 * not a reliable way to do this.
 **/

#ifdef old_version
#ifdef  ENVIRON
    if ((ename = getenv ("PROGPATH")) == NULL)
#endif
#ifdef  UNIXV6
	ename = ENAME;
#else
	ename = "/usr/local/unix/e";            /* CERN mod */
#endif
#endif

    ename = argarray[0];
    fflush(stderr);     /* without this we get "Can't reenter.. */
    retstat = execlp (ename, append (loginflg? "-": "", ename), 0);
    perror("EXEC");
    printf ("Can't reenter editor %s, stat = %d\n",ename, retstat);
#ifdef PROFILE
    monexit (-1);
#else
    exit (-1);
#endif
    /* never returns */
    /* NOTREACHED */
}

#ifdef COMMENT
static void
reenter (waitflg)
    Flag waitflg
.
    Ask the user to "Hit <RETURN> when ready to resume editing. ".
    When he has typed a <RETURN> return to caller.
#endif
static void
reenter (waitflg)
    Flag waitflg;
{
    register int j;

    putc (CCSTOP, keyfile);     /* stop replay at this point */
    if ( waitflg ) {
	printf ("Hit <RETURN> when ready to resume editing. ");
	fflush (stdout);
	while ((j = getchar ()) != EOF  && (j & 0177) != '\n')
	    continue;
    }
    return;
}

#ifdef COMMENT
Cmdret
eexit ()
.
    Exit from the editor.
    Do appropriate saves, cleanups, etc.
    Only returns if there is a syntax error in the exit command or an error
    happened while saving files.  Otherwise it ends with a call to exit().
.
		  saves   updates      deletes     deletes
		  files  state file   keystroke    'changes'
			   (.es1)    file (.ek1)  file (.ec1)
		  -----  ----------  -----------  -----------
    exit           YES      YES          YES         YES
    exit nosave     -       YES          YES         YES
    exit quit       -        -           YES         YES
    exit abort      -        -            -          YES
    exit dump       -        -            -           -
#endif
Cmdret
eexit ()
{
    extern char default_workingdirectory[];
    extern int lookup_abv (char *, S_looktbl *, int);

    Small extblind;
    Cmdret retval;

    if (opstr[0] == '\0')
	extblind = EXNORMAL;
    else {
	if (*nxtop)
	    return CRTOOMANYARGS;
	extblind = lookup_abv (opstr, exittable, 1);
	if ( extblind == -1 ) {
	    extern Cmdret help_cmd_arguments (char *str, S_looktbl *table);
	    /* check for "exit ?" */
	    retval = help_cmd_arguments (opstr, exittable);
	    if ( retval != CRUNRECARG )
		return retval;
	    mesg (ERRSTRT + 1, opstr);
	    return (Cmdret) extblind;
	}
	if ( extblind == -2 ) {
	    ;
	}
	extblind = exittable[extblind].val;
    }

    /* close debugging output file */
    close_dbgfile (opstr);

    (void) chdir (default_workingdirectory);
    switch (extblind) {
    case EXNORMAL:
	if (saveall () == NO)
	    return CROK;
	break;

    case EXABORT:
    case EXDUMP:
    case EXNOSAVE:
    case EXQUIT:
	d_put (0);
	fixtty ();
	screenexit (YES);
	break;

    default:
	return CRBADARG;
    }

    switch (extblind) {
    case EXDUMP:
	fatal (FATALEXDUMP, "Aborted");

    case EXNORMAL:
    case EXNOSAVE:
#if 0
	if (!notracks)      /* before version 19.58 */
#endif
	    savestate ();   /* we must save the current session state */
    case EXQUIT:
    case EXABORT:
	cleanup (YES, extblind != EXABORT);
	if (extblind == EXNORMAL) {
#ifdef PROFILE
	    monexit (0);
#else
	    exit (0);
#endif
	    /* NOTREACHED */
	}
    }
#ifdef PROFILE
    monexit (1);
#else
    exit (1);
#endif
    /* NOTREACHED */
}

#ifdef COMMENT
Flag
saveall ()
.
    Fix tty modes, put cursor at lower left.
    Do the actual modifcations to disk files:
      Save all the files that were modified and for which UPDATE is set.
      Rename files that were renamed.
      Delete files that were deleted.
    Return YES if all went OK, else NO.
#endif
Flag
saveall ()
{
    Fn i;
    int j;

    d_put (0);
    fixtty ();
    screenexit (NO);

    /* The strategy here is to stave off all permanent actions until as
     * late as possible.  Deletes and renames are not done until necessary.
     * On the other hand, according to this precept, all of the modified
     * files should be saved to temp files first, and then linked or
     * copied to backups, etc.  But this would take too much disk space.
     * So the saves go one at a time, with some deleting and renaming along
     * the way.  If we bomb during a save and any deletes or renames have
     * happened, we're probably screwed if we want to replay
     **/

    /* delete all backup files to make disk space
     * then do the saves
     **/
    for (j = 1; ;) {
	for (i = FIRSTFILE + NTMPFILES; i < MAXFILES; i++) {
	    if (   (fileflags [i] & (INUSE | UPDATE | DELETED))
		   == (INUSE | UPDATE)
		/* the modified test is done by savefile now !
		 *  && la_modified (&fnlas[i])
		 **/
		&& savefile ((char *) 0, i, fileflags [i] & INPLACE, j, YES) == NO
	       ) {
 err:           putchar ('\n');
		fflush (stdout);
		reenter (YES);
		setitty ();
		setotty ();
		windowsup = YES;
		fresh ();
		return NO;
	    }
	}
	if (j-- == 0)
	    break;
	putline (YES);
	la_sync (NO);   /* flush all cached buffers out */
    }

    /* do any deleting as necessary
     * if any deleted names conflicted with NEW names or RENAMED names,
     *  they were already deleted by the save loop above
     **/
    for (i = FIRSTFILE + NTMPFILES; i < MAXFILES; i++) {
	if ((fileflags [i] & (UPDATE | DELETED)) == (UPDATE | DELETED)) {
	    mesg (TELALL + 2, "DELETE: ", names[i]);
	    unlink (names[i]);
	}
    }

    /* do any renaming as necessary
     * if any oldnames conflict with NEW names,
     * or if any saved names wre RENAMED,
     *  they were already renamed by the save loop above
     **/
    for (i = FIRSTFILE + NTMPFILES; i < MAXFILES; i++) {
	if (   (fileflags [i] & (UPDATE | RENAMED)) == (UPDATE | RENAMED)
	    && !svrename (i)
	   )
	    goto err;
    }

    putchar ('\n');
    fflush (stdout);
    return YES;
}

static void put_strg (char *str, FILE *stfile)
{
    if ( str ) {
	putshort ((short) (strlen (str) + 1), stfile);
	fputs (str, stfile);
	putc ('\0', stfile);
    }
    else putshort (0, stfile);   /* not defined */
}

static void put_fname (Fn fn, FILE *stfile)
{
    extern char *cwdfiledir [];
    extern char default_workingdirectory[];
    int sz;
    char *fname, *fdir;
    char ffpath [PATH_MAX];

    fname = names[fn];
    if ( !fname ) return;

    fdir = cwdfiledir [fn];
    if ( (fname[0] != '/') && fdir
	  && (strcmp (fdir, default_workingdirectory) != 0) ) {
	memset (ffpath, 0, sizeof (ffpath));
	strcpy (ffpath, fdir);
	sz = strlen (ffpath);
	if ( (sz > 0) && (ffpath [sz -1] != '/') ) ffpath [sz] = '/';
	strncat (ffpath, fname, sizeof (ffpath) - sz);
	ffpath [sizeof (ffpath) -1] = '\0';
	fname = ffpath;
    }
    put_strg (fname, stfile);
#if 0
    putshort ((short) (strlen (fname) + 1), stfile);
    fputs (fname, stfile);
    putc ('\0', stfile);
#endif
}

static void put_wksp (S_wksp *wksp_pt, FILE *stfile)
{
    if ( ! wksp_pt ) return;
    if ( ! names[wksp_pt->wfile] ) {
	/* no file attached to this work space : null file name size */
	putshort ((short) 0, stfile);
	return;
    }
    put_fname ((int)  wksp_pt->wfile, stfile);
    putlong  ((long)  wksp_pt->wlin,  stfile);
    putshort ((short) wksp_pt->wcol,  stfile);
    putc     (        wksp_pt->clin,  stfile);
    putshort ((short) wksp_pt->ccol,  stfile);
}

static void put_mark (struct markenv *markpt, FILE *stfile)
{
    if ( ! markpt ) return;
    putlong  ((long)  markpt->mrkwinlin, stfile);
    putshort ((short) markpt->mrkwincol, stfile);
    putlong  ((long)  markpt->mrklin, stfile);
    putshort ((short) markpt->mrkcol, stfile);
}

static void put_wksp_cursor (S_wksp *wksp, FILE *stfile)
{
    struct markenv mark;
    S_wksp *lwksp;

    if ( ! wksp ) return;
    mark.mrkwinlin = wksp->wlin;
    mark.mrkwincol = wksp->wcol;
    mark.mrklin    = wksp->clin;
    mark.mrkcol    = wksp->ccol;
    put_mark (&mark, stfile);
    put_mark (&wksp->wkpos, stfile);
}

#ifdef COMMENT
Flag
savestate ()
.
    Update the state file.
    See State.fmt for a description of the state file format.
#endif
Flag
savestate ()
{
    extern int get_info_level ();
    extern void history_dump (FILE *stfile);
    extern Flag get_graph_flg ();
    extern Flag inputcharis7bits;
    extern const char build_date [];
    extern char *myHost, *fromHost, *fromDisplay, *fromHost_alias;
    extern char *ssh_ip, *sshHost, *sshHost_alias;
    void delchar_dump (FILE *);

    Flag savfile_flg [MAXFILES];
    int cc, fi, wi, wn, i0, nb, finb;
    Short i, fflag;
    char *fname;
    S_window *window;
    char stbuf[BUFSIZ];
    FILE *stfile;
    struct utsname utsnm;
    char msg [256];

    curwksp->ccol = cursorcol;
    curwksp->clin = cursorline;

    unlink (rfile);
    if ((stfile = fopen (rfile, "w")) == NULL)
	return NO;
    setbuf (stfile, stbuf);
    chmod (rfile,0600);

    /*  Put out a flag word which can't match revision number.
     *  Later, if all is OK, we'll seek back to beginning and put out the
     *  revision number.
     **/
    putshort ((short) 0, stfile);

    /* terminal type name */
    put_strg (tname, stfile);
#if 0
    putshort ((short) (strlen (tname) + 1), stfile);
    fputs (tname, stfile);
    putc ('\0', stfile);
#endif

    putc (term.tt_height, stfile);
    putc (term.tt_width, stfile);

    /* time of start of session */
    putlong (strttime, stfile);

    /* tabstops */
    putshort ((short) ntabs, stfile);
    if (ntabs > 0) Block {
	Reg1 int ind;
	for (ind = 0; ind < ntabs; ind++)
	    putshort ((short) tabs[ind], stfile);
    }

    putshort ((short) linewidth, stfile);

    put_strg (searchkey, stfile);

    putc (insmode, stfile);
    putc (patmode, stfile);     /* Added Purdue CS 10/8/82 MAB */

    putc (curmark != 0, stfile);
    if (curmark) {
	putlong  ((long) curmark->mrkwinlin, stfile);
	putshort ((short) curmark->mrkwincol, stfile);
	putc     (curmark->mrklin, stfile);
	putshort ((short) curmark->mrkcol, stfile);
    }

#ifdef LMCV19
#ifdef LMCAUTO
    putc (autofill, stfile);
    putshort (autolmarg, stfile);
#else
    putc ((char) 0, stfile);
    putshort (0, stfile);
#endif
#endif

    putc (nwinlist, stfile);
    for (i = 0; i < nwinlist; i++)
	if (winlist[i] == curwin)
	    break;
    putc (i, stfile);   /* index of current window */
    for (i = 0; i < nwinlist; i++) {
	window = winlist[i];
	putc     (window->prevwin, stfile);
	putc     (window->tmarg, stfile);
	putshort ((short) window->lmarg, stfile);
	putc     (window->bmarg, stfile);
	putshort ((short) window->rmarg, stfile);
#ifdef LMCV19
	putc     (window->winflgs, stfile);
#endif
	putshort ((short) window->plline, stfile);
	putshort ((short) window->miline, stfile);
	putshort ((short) window->plpage, stfile);
	putshort ((short) window->mipage, stfile);
	putshort ((short) window->lwin, stfile);
	putshort ((short) window->rwin, stfile);
	/* workspace data */
	put_wksp (window->altwksp, stfile);
	put_wksp (window->wksp,    stfile);
	savewksp (window->wksp);    /* save cursor position in lastlook */

#if 0
	/* old fation */
	if (fname = names[window->altwksp->wfile]) {
	    put_fname ((int) window->altwksp->wfile, stfile);
	    putlong  ((long) window->altwksp->wlin, stfile);
	    putshort ((short) window->altwksp->wcol, stfile);
	    putc     (window->altwksp->clin, stfile);
	    putshort ((short) window->altwksp->ccol, stfile);
	}
	else
	    putshort ((short) 0, stfile);   /* no alt file, file name size is 0 */
	put_fname ((int) window->wksp->wfile, stfile);
	/*
	fname = names[window->wksp->wfile];
	putshort ((short) (strlen (fname) + 1), stfile);
	fputs (fname, stfile);
	putc ('\0', stfile);
	*/
	putlong  ((long) window->wksp->wlin, stfile);
	putshort ((short) window->wksp->wcol, stfile);
	putc     (window->wksp->clin, stfile);
	putshort ((short) window->wksp->ccol, stfile);
	/* -- old fation */
#endif
    }
    if (ferror (stfile)) {
	fclose (stfile);
	return NO;
    }

    /* Dump the history buffer */
    history_dump (stfile);

    /* Dump the list of edited files and there flags (exepted deleted and
     *  renamed), the end of the list is flaged with an empty string.
     */
    memset (savfile_flg, 0, sizeof(savfile_flg));
    finb = 0;
    for ( fi = FIRSTFILE + NTMPFILES ; fi < MAXFILES ; fi++ ) {
	fflag = fileflags [fi];
	if ( !(fflag & INUSE) ) continue;
	if ( fflag & (DELETED | RENAMED) ) continue;
	if ( is_eddeffile (fi) ) continue;
	put_fname (fi, stfile);
	putshort (fileflags [fi], stfile);
	savfile_flg [fi] = YES;
	finb++;
    }
    putshort (0, stfile);  /* end of file list */

    /* Dump the key function assigned to <Del> character */
    delchar_dump (stfile);

    /* Various status info */

    /* session status */
    putc ((char) notracks, stfile);
    putc ((char) norecover, stfile);
    putc ((char) replaying, stfile);
    putc ((char) recovering, stfile);

    /* char set and info line */
    putc ((char) get_graph_flg (), stfile); /* graphic char for window edges */
    putc ((char) inputcharis7bits, stfile); /* 7 or 8 bits characters */
    putshort ((short) get_info_level (), stfile);   /* current info line level */

    /* number of edited file */
    putshort ((short) (FIRSTFILE + NTMPFILES), stfile); /* first user file id */
    putshort ((short) finb, stfile);    /* number of edited file in the session */

    /* Dump the files cursor positions and tick marks */
    for ( fi = FIRSTFILE + NTMPFILES ; fi < MAXFILES ; fi++ ) {
	extern struct markenv * get_file_mark (Fn fnb);
	struct markenv *markpt, mark;
	S_wksp *lwksp;

	if ( ! savfile_flg [fi] ) continue;
	/* Dump cursor positions and tick */
	lwksp = &lastlook [fi];
	put_wksp_cursor (lwksp, stfile);
	markpt = get_file_mark (fi);
	putc ((markpt != NULL), stfile);
	/* Dump tick mark if any */
	put_mark (markpt, stfile);
    }
    putshort (0xAAAA, stfile);  /* end of section flag */

    /* Dump the window cursor positions */
    for ( i = 0 ; i < nwinlist ; i++ ) {
	short fn;
	S_wksp *wksp;

	window = winlist[i];
	wksp = window->altwksp;
	fn = (short) wksp->wfile;
	putshort (fn, stfile);
	if ( fn != NULLFILE ) put_wksp_cursor (wksp, stfile);
	wksp = window->wksp;
	fn = (short) wksp->wfile;
	putshort (fn, stfile);
	if ( fn != NULLFILE ) put_wksp_cursor (wksp, stfile);
    }
    putshort (0xAAAA, stfile);  /* end of section flag */

    /* file system and Rand version, revision and build date */
    putlong (PATH_MAX, stfile);
    putshort (-revision, stfile);
    putshort (subrev, stfile);
    putshort ((short) (strlen (build_date) + 1), stfile);
    fputs (build_date, stfile);  /* Rand build date */
    putc ('\0', stfile);

    /* OS info */
    cc = uname (&utsnm);
    if ( cc >= 0 ) {    /* on Sun uname returns 1 if ok !?! */
	sprintf (msg, "%s %s, Hostname \"%s\", user \"%s\"", utsnm.sysname,
		 utsnm.release, utsnm.nodename, myname);
	put_strg (msg, stfile);
    } else {
	putshort (0, stfile);   /* error in uname call */
    }

    /* client session info */
    put_strg (fromHost, stfile);
    put_strg (fromHost_alias, stfile);
    put_strg (fromDisplay, stfile);
    put_strg (ssh_ip, stfile);
    put_strg (sshHost, stfile);
    put_strg (sshHost_alias, stfile);

    putshort (0xAAAA, stfile);  /* end of section flag */

    if (ferror (stfile)) {
	fclose (stfile);
	return NO;
    }

    putshort (0xFFFF, stfile);  /* end of file flag */

    fseek (stfile, 0L, 0);
    putshort (revision, stfile);   /* state file is OK */
    fclose (stfile);
    return  YES;
}

void delchar_dump (FILE *stfile)
{
    extern char * itgetvalue (char *);
    extern char del_strg [];
    char *val_pt;

    val_pt = itgetvalue (del_strg);
    if ( val_pt ) {
	putc (del_strg [0], stfile);
	putc (*val_pt, stfile);
    } else {
	/* not assigned */
	putc ('\0', stfile);
	putc ('\0', stfile);
    }
    putshort (0xAAAA, stfile);  /* end of section flag */
}
