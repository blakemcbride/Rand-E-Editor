#ifdef COMMENT
Copyright       abandoned, 1983, The Rand Corporation
#endif

#ifdef COMMENT
/*
    center

    +----------------------------------------------+
    | Upgraded to VAX, Sept 18, 1979, by W. Giarla |
    +----------------------------------------------+

    David Yost 2/79, 4/80

    center {[-s[1]    ]  {filename}}
	    [-n[65535]]
	    [-i[0]    ]
	    [-l[65]   ]

    An argument consisting of '-' by itself means read the original
      standard input, and is treated as if it were a filename.

    start is the starting line for centering.
      lines before start are passed through unchanged.
    -s sets the starting line to 1
    -s20 sets it to 20

    nlines is the number of line to be centered.
      remaining lines are passed through unchanged.
    -n sets the number of lines to be centered to the whole input
    -n20 sets num of lines centered to 20

    lineleng is the total linelength intended for the output line
    -l sets lineleng to 65
    -l20 sets lineleng of output to 20
    if no -l option specified, use linelength of 65

    indent is the indent prepended to output lines
    -i sets indent to 0
    -ii sets indent to first printing column of first line centered
    -i20 sets it to 20

    Each input file argument is treated according to the options up to
      that point.

    Default is for center to verify all options before proceeding and
      to abort if any errors.  This behavior can be changed by setting
      the appropriate global flags below.
    Default is to check access to all files and report those which can't
      read, then to go ahead with those that can.  This behavior can
      be changed by setting the appropriate global flags below.

    Buffers output and input.

    Uses rw stdio library (-lSRW) + new faccess(name,mode) call.

    Exits 0 if done OK.
    Exits -1 if error encountered and nothing touched.
    Exits -2 if error encountered after doing some output.
*/

#endif

    /*      * */
#define RWSTDIO
#include <stdio.h>
#include <ctype.h>		/* uptovax  WG */
#include <sys/types.h>
/* XXXXXXXXXXXXXXXXXXXX
#include <sgtty.h>
*/
#include <sys/stat.h>

#define BELL 07

    int             numargs,	/* global copy of argc                             */
                    curarg;	/* current arg to look at                          */
    char          **argarray,	/* global copy of argv                             */
                   *curargchar,	/* current arg character to look at                */
                   *progname;	/* global copy of argv[0] which is program
				 * name    */

    int             opterrflg = 0,	/* option error encountered                          */
                    badfileflg = 0;	/* bad file encountered                            */

    int             opt_abort_flg = 1,	/* abort entirely if any option
					 * errors encountered */
                    opt_stop_flg = 0;	/* stop processing at first option
					 * error           */
    /* do not turn this on if opt_abort_flg is on      */

    int             fil_abort_flg = 0,	/* abort entirely if any bad files
					 * encountered     */
                    fil_stop_flg = 0;	/* stop processing at first bad file               */
    /* do not turn this on if fil_abort_flg is on      */

    int             show_errs_flg;	/* print error diagnostics to stderr               */
    int             output_done = 0;	/* set to one if any output done                   */

    FILE           *input;
    /********************* end of standard filter declarations *****************/
#define MAXINBUF 1024		/* must be at least 512                    */
#define MAXINT 32767
#define MAXUNS 65535
#define TABCOL 8
#define INDENTDEFLT 0
#define LINELENGDEFLT 65
#define spcortab(c) (c == ' ' || c == '\t')
#define printing(c) (c > ' ' && c < 0177)
#define min(a,b) (a<b? a: b)
#define max(a,b) (a>b? a: b)

    char            inbuf[MAXINBUF];
int             indent = INDENTDEFLT;	/* indent is part of lineleng              */
int             indent_flg = 0;	/* use first printing col of 1st line proc. */
int             lineleng = LINELENGDEFLT;	/* lineleng of centering area              */
unsigned int    start = 1;	/* line number to start with (first is 1)  */
unsigned int    nlines = MAXUNS;/* number of lines to do                   */
int             frstcol = 0;	/* first col of input line                 */
int             inlinel = 0;	/* line length of printing text on inp line */

/*  * */
main(argc, argv)
    int             argc;
    char          **argv;
{
    numargs = argc;
    curarg = 0;
    argarray = argv;
    /* curargchar = *argarray[curarg];     uptovax WG */
    curargchar = *(argarray + curarg);

    input = stdin;
    getprogname();

    opterrflg = badfileflg = 0;
    show_errs_flg = 0;

    do {
	getoptions(1);		/* check for option errors      */
	filterfiles(1);		/* check for bad files          */
    } while (curarg < numargs);

    if ((opterrflg && !opt_stop_flg) || (badfileflg && !fil_stop_flg)) {
	curarg = 1;
	opterrflg = badfileflg = 0;
	show_errs_flg = 1;
	do {
	    getoptions(1);	/* check for option errors      */
	    filterfiles(1);	/* check for bad files          */
	} while (curarg < numargs);
	if ((opterrflg && opt_abort_flg) || (badfileflg && fil_abort_flg)) {
	    fprintf(stderr, "%s: not performed.\n", progname);
	    exit(-2);
	}
    }
    curarg = 1;
    opterrflg = badfileflg = 0;

    if (intss())
	setbuf(stdout, NULL);
    do {
	opterrflg = 0;
	show_errs_flg = opt_stop_flg;
	getoptions(0);
	if (opterrflg && opt_stop_flg) {
	    fprintf(stderr, "%s: stopped.\n", progname);
	    exit(-1 - output_done);
	}
	badfileflg = 0;
	show_errs_flg = fil_stop_flg;
	filterfiles(0);
	if (badfileflg && fil_stop_flg) {
	    fprintf(stderr, "%s: stopped.\n", progname);
	    exit(-1 - output_done);
	}
    } while (curarg < numargs);

    exit(0);
}


getprogname()
{
    register char  *cp;
    register char   lastc = '\0';

    progname = cp = argarray[0];
    for (; *cp; cp++) {
	if (lastc == '/')
	    progname = cp;
	lastc = *cp;
    }
    curarg++;
}


getoptions(check_only)
{
    register char  *p;

    for (; curarg < numargs; curarg++) {
	curargchar = argarray[curarg];
	if (curargchar[0] != '-')	/* not an option arg */
	    return;
	if (curargchar[1] == '\0')	/* arg was '-' by itself */
	    return;

	p = ++curargchar;
	for (;; p++, curargchar++) {
	    switch (*p) {
	    case 'x':
		break;

	    case 'l':
		if (*++p == '\0') {
		    if (!check_only)
			lineleng = LINELENGDEFLT;
		} else
		    for (lineleng = 0; *p; p++) {
			if (isdigit(*p)) {
			    if (!check_only) {
				lineleng *= 10;
				lineleng += *p - '0';
			    }
			} else
			    goto error;
		    }
		break;

	    case 'i':
		if (*++p == '\0') {
		    if (!check_only)
			indent = INDENTDEFLT;
		} else if (*p == 'i') {
		    indent_flg = 1;
		    if (*++p)
			goto error;
		} else
		    for (indent = 0; *p; p++) {
			if (isdigit(*p)) {
			    if (!check_only) {
				indent *= 10;
				indent += *p - '0';
			    }
			} else
			    goto error;
		    }
		break;

	    case 's':
		if (*++p == '\0') {
		    if (!check_only)
			start = 1;
		} else
		    for (start = 0; *p; p++) {
			if (isdigit(*p)) {
			    if (!check_only) {
				start *= 10;
				start += *p - '0';
			    }
			} else
			    goto error;
		    }
		break;

	    case 'n':
		if (*++p == '\0') {
		    if (!check_only)
			nlines = MAXUNS;
		} else
		    for (nlines = 0; *p; p++) {
			if (isdigit(*p)) {
			    if (!check_only) {
				nlines *= 10;
				nlines += *p - '0';
			    }
			} else
			    goto error;
		    }
		break;

	    case '\0':		/* done */
		break;

	    default:		/* unknown option or other errors */
	error:
		opterrflg = 1;
		if (show_errs_flg) {
		    fprintf(stderr, "%s: option error: -%s\n",
			    progname, curargchar);
		    fflush(stderr);
		}
	    }
	    break;
	}
    }
}

filterfiles(check_only)
{
    register FILE  *f;

    if (curarg >= numargs && !check_only) {
	input = stdin;
	filter();
	return;
    }
    for (; curarg < numargs; curarg++) {
	curargchar = argarray[curarg];
	if (curargchar[0] == '-') {
	    if (curargchar[1] == '\0')	/* "-" arg */
		f = stdin;
	    else
		return;
	} else {
	    if (check_only)
		/* f = faccess(curargchar,"r");    uptovax WG */
		f = (FILE *) (!access(curargchar, 4));
	    else
		f = fopen(curargchar, "r");
	    if (f == NULL) {
		/* XXXXXXXXXXXXXXXXXXXXXXX */
		struct stat     scratch[64];
		if (stat(curargchar, scratch) == -1) {
		    if (show_errs_flg) {
			fprintf(stderr, "%s: can't find %s\n",
				progname, curargchar);
			fflush(stderr);
		    }
		} else {
		    if (show_errs_flg) {
			fprintf(stderr, "%s: not allowed to read %s\n",
				progname, curargchar);
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

filter()
{
    if (input == stdin && intss())
	fprintf(stderr, "%c%s: start typing.\n", BELL, progname);
    doit();
    if (input != stdin)
	fclose(input);
    else
	rewind(stdin);
    fflush(stdout);
}


doit()
{
    register int    c;
    register unsigned int n;

    for (n = 1; n < start;) {
	if ((c = readline()) != EOF) {
	    fputs(inbuf, stdout);
	    if (c == '\n')
		n++;
	} else
	    break;
    }
    if (c == EOF)
	goto done;
    for (n = 0; n < nlines; n++) {
	if ((c = getline()) != EOF)
	    centline();
	else
	    break;
    }
    if (c == EOF)
	goto done;
    for (;;) {
	if ((c = readline()) != EOF)
	    fputs(inbuf, stdout);
	else
	    break;
    }
done:
    finish();
}


getline()
{
    register int    i, c;
    register char  *cp;
    int             thislinel;
    int             j;
    static          eof_flg = 0;

    if (eof_flg) {
	eof_flg = 0;
	return (EOF);
    }
    frstcol = MAXINT;
    inlinel = 0;
    cp = inbuf;
    for (;;) {
	thislinel = 0;
	for (i = 0; (c = getch()) != EOF;) {
	    if (c == ' ') {
		*cp++ = c;
		i++;
	    } else if (c == '\t') {
		j = (j = i % TABCOL) ? TABCOL - j : TABCOL;
		do {
		    *cp++ = ' ';
		    i++;
		} while (--j);
	    } else
		break;
	}
	frstcol = min(frstcol, i);
	for (; c != EOF; c = getch()) {
	    if (cp >= &inbuf[MAXINBUF - 1]) {
		fprintf(stderr, "%s: line overflow!\n", progname);
		break;
	    }
	    if (c == '\t') {
		j = (j = i % TABCOL) ? TABCOL - j : TABCOL;
		do {
		    *cp++ = ' ';
		    i++;
		} while (--j);
	    } else {
		if (c == '\n' || c == '\f' || c == '\r') {
		    i = max(thislinel, i);
		    for (cp--; cp >= inbuf; cp--) {
			if (*cp == ' ')
			    i--;
			else if (*cp == '\b') {
			} else
			    break;
		    }
		    *++cp = c;
		    *++cp = '\0';
		    thislinel = i;
		    break;
		}
		*cp++ = c;
		if (c == '\b') {
		    if (i > thislinel)
			thislinel = i;
		    i--;
		} else
		    i++;
	    }
	}
	inlinel = max(inlinel, thislinel);
	if (c != '\r')
	    break;
    }
    if (indent_flg) {
	indent = frstcol;
	indent_flg = 0;
    }
    if (c == EOF) {
	if (inlinel == 0)
	    return (EOF);
	else {
	    eof_flg = 1;
	    return (inlinel);
	}
    }
    return (inlinel);
}


getch()
{
    register        c;

    for (c = getc(input);; c = getc(input)) {
	if (spcortab(c) || printing(c)
	    || c == '\n' || c == '\f' || c == '\b' || c == '\r' || c == EOF)
	    return (c);
	else
	    fprintf(stderr, "%s: \"\\%o\" char ignored.\n", progname, c);
    }
}


centline()
{
    register int    i, j;
    register char  *cp;
    int             cenwidth;
    int             spaceover;

    cenwidth = inlinel - frstcol;
    if (cenwidth > lineleng - indent)
	spaceover = 0;
    else
	spaceover = (lineleng - cenwidth) / 2;
    if (cenwidth > 0) {
	cp = inbuf;
	for (;;) {
	    for (i = 0; i < indent; i++)
		putchar(' ');
	    for (j = 0; j < spaceover; j++)
		putchar(' ');
	    for (cp += frstcol; *cp; cp++) {	/* ### buggy */
		putchar(*cp);
		if (*cp == '\r')
		    break;
	    }
	    if (*cp++ != '\r')
		break;
	}
    } else
	fputs(inbuf, stdout);
}


readline()
{
    register int    i, c;
    register char  *cp;
    static          eof_flg = 0;

    if (eof_flg) {
	eof_flg = 0;
	return (EOF);
    }
    for (i = 0, cp = inbuf; (c = getc(input)) != EOF; i++) {
	if (i < MAXINBUF - 1) {
	    *cp++ = c;
	    if (c == '\n')
		break;
	} else
	    break;
    }
    *cp = '\0';
    if (c == EOF) {
	if (i == 0)
	    return (EOF);
	else {
	    eof_flg = 1;
	    return (c);
	}
    }
    return (c);
}


finish()
{
}

intss()
{
    return ( isatty(0) );
    /* XXXXXXXXXXXXXXXXXXXXXXXXXXXX
    struct sgttyb   buf;
    return (gtty(0, &buf) != -1);
       XXXXXXXXXXXXXXXXXXXXXXXXXXXX */
}
