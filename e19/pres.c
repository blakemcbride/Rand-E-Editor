#ifdef COMMENT

    pres - print E state file

    See State.fmt for a description of the latest state file format.

    David Yost 3/79 and later

    pres [file]...

    An argument consisting of '-' by itself means read the original
      standard input, and is treated as a file argument, not an option.

    If no files are specified, use stdin as input.
    If files are specified, use them as input.

    Default is to check access to all files and report those which can't
      read, then to go ahead with those that can.  This behavior can
      be changed by setting the appropriate global flags below.
    Buffers output and input.

    Exits 0 if done OK.
    Exits -1 if error encountered and nothing touched.
    Exits -2 if error encountered after doing some output.

#endif

/******************* standard filter declarations ************************/
#include "e.h"
/*  #include <stdio.h>  is already included by e.h above */
#ifdef SUN
char _sobuf[BUFSIZ];
#else
extern char _sobuf[];
#endif SUN
FILE *tfopen ();

#define STATSIZE 100     /* max num of characters returned by stat call */
#define BELL 07
/* #define RAWMODE      /* define this if raw mode is needed               */
			/* NOT FIXED FOR UNIXV7 yet */
#ifdef UNIXV7
#include <sgtty.h>
#else
#include <sys/sgtty.h>
#endif
#ifdef RAWMODE
struct sgttyb instty;
#endif
int rawflg = 0;         /* input is set to raw mode                        */
int flushflg = 0;       /* means a user is sitting there waiting to see    */
			/*   the output, so give it to him as soon as it   */
			/* is ready                                        */

int numargs,            /* global copy of argc                             */
    curarg;             /* current arg to look at                          */
char **argarray,        /* global copy of argv                             */
     *curargchar,       /* current arg character to look at                */
     *progname;         /* global copy of argv[0] which is program name    */

int opterrflg = 0,      /* option error encountered                          */
    badfileflg = 0;     /* bad file encountered                            */

int opt_abort_flg = 1,  /* abort entirely if any option errors encountered */
    opt_stop_flg = 0;   /* stop processing at first option error           */
			/* do not turn this on if opt_abort_flg is on      */

int fil_abort_flg = 0,  /* abort entirely if any bad files encountered     */
    fil_stop_flg = 0;   /* stop processing at first bad file               */
			/* do not turn this on if fil_abort_flg is on      */

int show_errs_flg;      /* print error diagnostics to stderr               */
int output_done = 0;    /* set to one if any output done                   */

FILE *input;
/********************* end of standard filter declarations *****************/


/**/
main(argc, argv)
int argc;
char **argv;
{
    numargs = argc;
    curarg = 0;
    argarray = argv;
    curargchar = argarray[curarg];

    input = stdin;
    getprogname();

    opterrflg = badfileflg = 0;
    show_errs_flg = 0;

    do {
	getoptions(1);          /* check for option errors      */
	filterfiles(1);         /* check for bad files          */
    } while (curarg < numargs);
    if ( (opterrflg && !opt_stop_flg) || (badfileflg && !fil_stop_flg) ) {
	curarg = 1;
	opterrflg = badfileflg = 0;
	show_errs_flg = 1;
	do {
	    getoptions(1);          /* check for option errors      */
	    filterfiles(1);         /* check for bad files          */
	} while (curarg < numargs);
	if ( (opterrflg && opt_abort_flg) || (badfileflg && fil_abort_flg) ) {
	    fprintf(stderr,"%s: not performed.\n",progname);
	    getout (-2);
	}
    }

    curarg = 1;
    opterrflg = badfileflg = 0;

    if (!intss())
	setbuf(stdout, _sobuf);
    do {
	opterrflg = 0;
	show_errs_flg = opt_stop_flg;
	getoptions(0);
	if (opterrflg && opt_stop_flg)
	    stop ();
	badfileflg = 0;
	show_errs_flg = fil_stop_flg;
	filterfiles(0);
	if (badfileflg && fil_stop_flg)
	    stop ();
    } while (curarg < numargs);
    getout (0);
}


getprogname()
{
    register char *cp;
    register char lastc = '\0';

    progname = cp = argarray[0];
    for (; *cp; cp++) {
	if (lastc == '/')
	    progname = cp;
	lastc = *cp;
    }
    curarg++;

    /******************************************************************/
    /**/
    /**/    /* determine what to do depending on prog name here */
    /**/
    /******************************************************************/
}


getoptions(check_only)
{
    register char *p, *c;

    for (; curarg<numargs; curarg++) {
	curargchar = argarray[curarg];
	if (curargchar[0] != '-')           /* not an option arg */
	    return;
	if (curargchar[1] == '\0')          /* arg was '-' by itself */
	    return;

	p = ++curargchar;
	for (;; p++,curargchar++) {
	    switch (*p) {
	    case 'x':
		if (!check_only) {
		    /******************************************************/
		    /**/
		    /**/    /* process single char option, such as "-x" */
		    /**/
		    /******************************************************/
		}
		continue;

	    case 'l':
		/******************************************************/
		/**/
		/**/    /* process option with modifier, such as "-l65" */
		/**/    /*   checking for syntax errors               */
		/**/
		/******************************************************/
		if (!check_only)
		/******************************************************/
		/**/
		/**/    /* process option with modifier, such as "-l65" */
		/**/    /*   and actually set the appropriate         */
		/**/    /*   global variables                         */
		/**/
		/******************************************************/
		break;

	    case '\0':                    /* done */
		break;

	    default:                      /* unknown option or other errors */
	    error:
		opterrflg = 1;
		if (show_errs_flg) {
		    fprintf(stderr,"%s: option error: -%s\n",
			progname,curargchar);
		    fflush(stderr);
		}
	    }
	    break;
	}
    }
}

filterfiles(check_only)
{
    register FILE *f;

    if (curarg >= numargs && !check_only) {
	input = stdin;
	filter();
	return;
    }

    for (; curarg<numargs; curarg++) {
	curargchar = argarray[curarg];
	if (curargchar[0] == '-') {
	    if (curargchar[1] == '\0')    /* "-" arg */
		f = stdin;
	    else
		return;
	}
	else {
	    if (check_only)
		f = tfopen(curargchar,"r");
	    else
		f = fopen(curargchar,"r");
	    if (f == NULL) {
		char scratch[STATSIZE];

		if (stat(curargchar,scratch) == -1) {
		    if (show_errs_flg) {
			fprintf(stderr,"%s: can't find %s\n",
			    progname,curargchar);
			fflush(stderr);
		    }
		}
		else {
		    if (show_errs_flg) {
			fprintf(stderr,"%s: not allowed to read %s\n",
			    progname,curargchar);
			fflush(stderr);
		    }
		}
		badfileflg = 1;
	    }
	}
	if (!check_only && f != NULL) {
	    input = f;
	    filter();
	}
    }
}

filter ()
{
    if ( input == stdin && intss() )
	fprintf(stderr,"%c%s: start typing.\n",BELL,progname);

    if (xintss ())
	setraw ();
    doit();
    fixtty ();

    if (input != stdin)
	fclose(input);
    else
	rewind(stdin);
    fflush(stdout);
}


doit()
{
    int n, revision, nwinlist;
    Char chr, majdev, mindev;

    printf ("Revision %d\n", revision = - getshort (input));
    output_done = 1;
    if (revision <= 0)
	badstart();

    switch (revision) {
    case 9:
    case 10:
    case 11:
	if (revision >= 10)
	    printf ("Terminal type: %d\n", getshort (input));
	majdev = getc (input);
	mindev = getc (input);
	printf ("Working Directory device: %d, %d; inode: %d\n",
	    majdev, mindev, getshort (input));
	goto contcase;

    case 12:
    case 14:
	doscrsize();
contcase:
	dotime();
	dotabs();
	dofill();
	dosrch();
	doinmode();
	dooldmark();
	nwinlist = dowindow();
	for (n = 0; n < nwinlist; n++) {
	    sepwindows();
	    domargins();
	    dooldalt();
	    dooldfile();
	}
	break;

    case 13:
    case 15:
    case 17:
    case 18:
    case 19:
	dotermtype();
	doscrsize();
	dotime();
	dotabs();
	dofill();
	dosrch();
	doinmode();
	if (revision >= 18)
		dorexp();
	domark();
	if (revision >= 19)
		doauto();
	nwinlist = dowindow ();
	for (n = 0; n < nwinlist; n++) {
		sepwindows();
		domargins(n);
		if (revision >= 19)
			dotrack();
		if (revision >= 18)
			dosets();
		doalt();
		dofile();
	}
	break;
    default:
	fputs ("Don't know how to interpret that version.\n", stdout);
	return;
    }
    chr = getc(input);
    if (!feof (input))
	fputs ("\nBad startup file.  Too long (wrong format?)\n", stdout);
    return;
}

badstart()
{
    int nerr;

    if (nerr = ferror (input))
	printf ("\nBad startup file.  Read error %d.\n", nerr);
    else if (feof (input))
	fputs ("\nBad startup file.  Premature EOF.\n", stdout);
    else
	fputs ("\nBad startup file.\n", stdout);
}


setraw ()
{
    register regi;

#ifdef RAWMODE
    if (gtty (INSTREAM, &instty) != -1) {
	regi = instty.sg_flags;
	instty.sg_flags = RAW | (instty.sg_flags & ~(ECHO | CRMOD));
	stty (INSTREAM, &instty);        /* set tty raw mode */
	instty.sg_flags = regi;             /* all set up for cleanup */
	rawflg = 1;
    }
#endif
    flushflg = 1;
}

stop ()
{
    fprintf(stderr,"%s: stopped.\n",progname);
    getout(-1 - output_done);
}

getout (status)
{
    fixtty ();
    exit (status);
}

void
fixtty ()
{
#ifdef RAWMODE
    if (rawflg)
	stty (0, &instty);
#endif
    flushflg = 0;
}

xintss()
{
    struct sgttyb buf;

    if (gtty (fileno (input), &buf) != -1)
	return 1;
    return gtty (fileno (stdin), &buf) != -1;
}

dotermtype ()
{
	short nletters;

	printf ("Terminal type: \"");
	if (nletters = getshort (input)) {
	    while (--nletters > 0)
		putchar (getc (input));
	    if (getc (input))
		badstart();
	}
	printf ("\"\n");
}

doscrsize()
{
	char nlin, ncol;

	nlin = getc (input) & 0377;
	ncol = getc (input) & 0377;
	printf ("Screen size: %d x %d\n", nlin, ncol);
}

dotime()
{
	long tmpl;

	tmpl = getlong (input);
	printf ("Time of start of session: %s", ctime (&tmpl) );
}

dotabs()
{
	short n;

	if ((n = getshort (input)) > 0) {
	    printf ("%d tabstops: ", n);
	    do {
		printf ("%d, ", getshort (input));
		if (feoferr (input))
		    badstart();
	    } while (--n);
	}
	else
	    fputs ("No tabstops.", stdout);
	putchar ('\n');
}

dofill()
{
	printf ("Width for fill, etc. = %d\n", getshort (input));
}

dosrch()
{
	short nletters;

	if (nletters = getshort (input)) {
	    printf ("Search string is \"");
	    while (--nletters > 0)
		putchar (getc (input));
	    if (getc (input))
		badstart();
	    printf ("\"\n");
	}
	else
	    printf ("No search string.\n");
}

doinmode()
{
	printf ("INSERT mode ");
	if (getc (input))
	    printf ("on\n");
	else
	    printf ("off\n");
}

dorexp ()
{
	printf ("RE mode ");
	if (getc (input))
	    printf ("on\n");
	else
	    printf ("off\n");
}

domark()
{
	long winlin;
	short col, wincol;
	char lin;

	if (getc (input)) {
	    printf ("MARK in effect:\n");
	    winlin = getlong  (input);
	    wincol = getshort (input);
	    lin    = getc     (input);
	    col    = getshort (input);
	    printf ("  window at (%D, %d); cursor at (%d, %d)\n",
		     winlin, wincol, lin, col);
	}
	else
	    printf ("MARK not in effect\n");
}

dooldmark()
{
	short col, wincol, winlin;
	char lin;

	if (getc (input)) {
	    printf ("MARK in effect:\n");
	    winlin = getshort (input);
	    wincol = getshort (input);
	    lin    = getc     (input);
	    col    = getshort (input);
	    printf ("  window at (%d, %d); cursor at (%d, %d)\n",
		     winlin, wincol, lin, col);
	}
	else
	    printf ("MARK not in effect\n");
}

doauto()
{
	if (getc (input))
		printf ("AUTOfill is on.\n");
	else
		printf ("AUTOfill is off.\n");
	printf ("Left margin set on column %d.\n", getshort (input));
}

dowindow()
{
	char nwinlist, winnum;

	nwinlist = getc (input);
	if (ferror(input) || nwinlist > MAXWINLIST)
	    badstart();
	printf ("Number of windows: %d\n", nwinlist);
	winnum = getc (input);
	printf ("Current window: %d\n", winnum);
	return nwinlist;
}

domargins(n)
	int n;
{
	char tmarg, bmarg;
	short lmarg, rmarg;

	printf ("Window %d:\n", n);
	printf ("  Previous window: %d\n", getc (input));
	tmarg = getc     (input);
	lmarg = getshort (input);
	bmarg = getc     (input);
	rmarg = getshort (input);
	printf ("  (%d, %d, %d, %d) = (t, l, b, r) window margins\n",
	    tmarg, lmarg, bmarg, rmarg);
}

dotrack ()
{
	if (getc (input))
	    printf ("  TRACK set.\n");
	else
	    printf ("  TRACK not set.\n");
}

dosets ()
{
	short plline, miline, plpage;
	short mipage, lwin, rwin;

	plline = getshort (input);
	miline = getshort (input);
	plpage = getshort (input);
	mipage = getshort (input);
	lwin = getshort (input);
	rwin = getshort (input);
	printf ("  (+l, -l, +p, -p, wl, wr) = (%d, %d, %d, %d, %d, %d)\n",
		     plline, miline, plpage, mipage, lwin, rwin);
}

doalt()
{
	short nletters, wincol;
	long winlin;

	if (nletters = getshort (input)) {
	    if (feoferr (input))
		badstart();
	    fputs ("  Alternate file: ", stdout);
	    while (--nletters > 0)
		putchar (getc (input));
	    if (getc (input))
		badstart();
	    putchar ('\n');
	    winlin = getlong (input);
	    wincol = getshort (input);
	    printf ("    (%D, %d) = (lin, col) window upper left\n",
		winlin, wincol);
	    printf ("    (%d, ", getc (input));
	    printf ("%d) = (lin, col) cursor position\n", getshort (input));
	} else
		fputs ("  No alt wksp\n", stdout);
	if (feoferr (input))
		badstart();
}

dooldalt()
{
	short nletters, wincol, winlin;

	if (nletters = getshort (input)) {
	    if (feoferr (input))
		badstart();
	    fputs ("  Alternate file: ", stdout);
	    while (--nletters > 0)
		putchar (getc (input));
	    if (getc (input))
		badstart();
	    putchar ('\n');
	    winlin = getshort (input);
	    wincol = getshort (input);
	    printf ("    (%d, %d) = (lin, col) window upper left\n",
		winlin, wincol);
	    printf ("    (%d, ", getc (input));
	    printf ("%d) = (lin, col) cursor position\n", getshort (input));
	}
	else
	    fputs ("  No alt wksp\n", stdout);
	if (feoferr (input))
	    badstart();
}

dofile()
{
	short nletters, wincol;
	long winlin;

	fputs ("  File: ", stdout);
	nletters = getshort (input);
	while (--nletters > 0)
	    putchar (getc (input));
	if (getc (input))
	    badstart();
	putchar ('\n');
	winlin = getlong (input);
	wincol = getshort (input);
	printf ("    (%D, %d) = (lin, col) window upper left\n",
	    winlin, wincol);
	printf ("    (%d, ", getc     (input));
	printf ("%d) = (lin, col) cursor position\n", getshort (input));
}

dooldfile()
{
	short nletters, wincol, winlin;

	fputs ("  File: ", stdout);
	    nletters = getshort (input);
	    while (--nletters > 0)
		putchar (getc (input));
	    if (getc (input))
		badstart();
	    putchar ('\n');
	    winlin = getshort (input);
	    wincol = getshort (input);
	    printf ("    (%d, %d) = (lin, col) window upper left\n",
		winlin, wincol);
	    printf ("    (%d, ", getc     (input));
	    printf ("%d) = (lin, col) cursor position\n", getshort (input));
}

sepwindows ()
{
	fputs ("============================================\n", stdout);
}
