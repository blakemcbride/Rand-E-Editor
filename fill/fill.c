#ifdef COMMENT
Copyright       abandoned, 1983, The Rand Corporation
#endif

#ifdef COMMENT
/*
    fill & just

    6/85: changed to not split hyphenated words, by default, and added
    -h flag to enable splitting.  Also, no longer filters out CTRL
    chars.

    +----------------------------------------------+
    | Upgraded to VAX, Sept 18, 1979, by W. Giarla |
    +----------------------------------------------+

    David Yost 2/79

    fill {[-l[65]] -h {filename}}

    An argument consisting of '-' by itself means read the original
      standard input, and is treated as if it were a filename.

    lineleng is the total linelength intended for the output line
    -l sets lineleng to 65
    -l20 sets lineleng of output to 20
    if no -l option specified, use 65
    -h enable breaking lines at hyphens.

    Each input file argument is treated according to the options up to
      that point.

    Default is for fill/just to verify all options before proceeding and
      to abort if any errors.  This behavior can be changed by setting
      the appropriate global flags below.
    Default is to check access to all files and report those which can't
      read, then to go ahead with those that can.  This behavior can
      be changed by setting the appropriate global flags below.

    Handles backspaces and tabs.

	 ##### not yet smart about returns ######

    Decides whether it is fill or just by looking at the last part of the
      pathname in argv[0].

    If any arguments beginning with '.' but not starting with '.ll' are
      encountered, everything is shipped off to nroff like the old days.

    Buffers output and input.

    Uses rw stdio library (-lSRW) +
      new faccess(name,mode) fpipe(files) and intss()

    Exits 0 if done OK.
    Exits -1 if error encountered and nothing touched.
    Exits -2 if error encountered after doing some output.
*/
#endif

/*  * * */
#include <stdio.h>
#include <ctype.h>		/* uptovax WG */
#include <sys/types.h>
/* XXXXXXXXXXXXXXXXXXXXXX
#include <sgtty.h>
   XXXXXXXXXXXXXXXXXXXXXX */
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
#define MAXOUTLINE 162
#define MAXINT 32767
#define MAXTIE 10
#define TABCOL 8
#define LINELENGDEFLT 65
#define spcortab(c) (c == ' ' || c == '\t')
#define printing(c) (c > ' ' && c < 0177)
#define min(a,b) (a<b? a: b)
#define max(a,b) (a>b? a: b)
#define curindent() (outparline == 1? indent[0]: indent[1])

#define NWORDS MAXINBUF/2
#define ENDPAR 1

#ifdef ALLOWNROFF
    int             usenroff = 0;	/* when set means must use nroff           */
#endif

    char            inbuf[MAXINBUF];

int             justflg = 0;	/* 1 if just, 0 if fill                    */
int             hyphenate = 0;	/* set to not break hyphenated words       */

int             indent[2];	/* first and second line indents           */
int             lineleng = LINELENGDEFLT;	/* length of output line                   */
int             npar = MAXINT;	/* number of paragraphs to do              */
int             prespace;	/* num of spaces at last beg of line       */

char            dblstr[] = ".?!:";	/* put two spaces after these chars
					 * when   */
/* next word starts with a cap. letter     */

/* info relating to line to be output */
int             o_nchars = 0;	/* num of chars accumulated in the line    */
int             o_nwords = 0;	/* number of words in line                 */
int             o_fullflg = 0;	/* we have a full line already             */
int             o_moreflg = 0;	/* there is more after this line is output */
struct lwrd {
    char            candidate;	/* candidate for spreading on this pass    */
    char            nspaces;	/* num of spaces after word                */
}               oword[MAXOUTLINE / 2];

struct wrd {
    int             nchars;	/* num of chars in word                    */
    int             strleng;	/* num of chars collected for word         */
    char           *chars;	/* where word is in inbuf                  */
    char            firstchar;	/* first char of word                      */
    char            lastchar;	/* last char of word                       */
    char            brkchar;	/* char that caused end-of-word            */
    char            nextchar;	/* first char of next word                 */
};				/* or \n or \f or EOF                    */

int             firstword;	/* index of first word in wordtbl              */
int             lastword;	/* index of last word in wordtbl               */
struct wrd      word[NWORDS];	/* the words                                   */

int             bksperr = 0;	/* illegal use of backspace encountered    */
int             inpline;         /* line number of input                    */
int             outparline;	/* line num within current para output     */
int             nextcflg = 0;	/* there was a char pushed back onto input */
int             nextc;		/* this is the char pushed back            */


/*  * * */
main(argc, argv)
    int             argc;
    char          **argv;
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
	getoptions(1);		/* check for option errors      */
#ifdef ALLOWNROFF
	if (usenroff)
	    break;
#endif
	filterfiles(1);		/* check for bad files          */
    } while (curarg < numargs);

#ifdef ALLOWNROFF
    if (usenroff) {
	if (curarg < numargs)
	    fprintf(stderr,
	     "%s: when using nroff args, standard input only.\n", progname);
	else
	    feednroff();
    } else
#endif
	dohere();

    exit(0);
}


dohere()
{
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

    if (cmpstr(progname, "just"))
	justflg = 1;
    curarg++;
}


cmpstr(s1, s2)
    register char  *s1, *s2;
{
    for (; *s1 == *s2; s1++, s2++)
	if (*s1 == '\0')
	    return (1);
    return (0);
}


getoptions(check_only)
{
    register char  *p;

    for (; curarg < numargs; curarg++) {
	curargchar = argarray[curarg];
	p = curargchar;
	if (*p == '-') {
	    if (*++p == '\0')	/* arg was '-' by itself */
		return;
	}
#ifdef ALLOWNROFF
	else if (*p == '.') {
	    if (*++p != 'l' || *++p != 'l') {
		usenroff = 1;
		continue;
	    }
	}
#endif
	else
	    return;

	for (; curargchar = p; p++) {
	    switch (*p) {
	    case 'x':		/* ignore */
		break;

	    case 'l':
		for (p++; *p == ' '; p++)
		    continue;
		if (*p == '\0')
		    lineleng = LINELENGDEFLT;
		else {
		    for (lineleng = 0; *p; p++) {
			if (isdigit(*p)) {
			    lineleng *= 10;
			    lineleng += *p - '0';
			} else
			    goto error;
		    }
		}
		break;

	    case 'h':
		hyphenate = 1;
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
		break;
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
		/* f = faccess(curargchar,"r");  uptovax WG */
		f = (FILE *) (!access(curargchar, 4));
	    else
		f = fopen(curargchar, "r");
	    if (f == NULL) {
		struct stat     scratch;

		if (stat(curargchar, &scratch) == -1) {
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



/*  * * */
doit()
{
    register int    c;
    int             np;

    inpline = 1;
    for (np = 0; np < npar; np++) {
	indent[0] = indent[1] = 0;
	outparline = 1;

	for (;;) {		/* treat blank lines at beginning */
	    firstword = 0;
	    lastword = NWORDS - 1;
	    word[firstword].chars = 0;	/* to detect overflow on first */
	    /* two lines in paragraph */
	    word[lastword].chars = inbuf + MAXINBUF;
	    word[lastword].nextchar = 0;
	    word[lastword].brkchar = 0;
	    word[lastword].strleng = 0;
	    o_nchars = o_nwords = o_fullflg = o_moreflg = 0;

	    if ((c = getword()) == EOF) {
		finish();
		return;
	    }
	    if (!printing(c) && word[lastword].nchars == 0)
		putchar(c);
	    else
		break;
	}
	indent[0] = prespace;	/* set first line indent */
	for (; c != EOF; c = getword()) {	/* get remainder of first
						 * line */
	    if (!printing(c))
		break;
	}
	if (c == EOF) {
	    finish();
	    return;
	}
	if ((c = getword()) == EOF) {	/* get line 2 indent           */
	    finish();
	    return;
	}
	indent[1] = prespace;
	for (; c != EOF; c = getword()) {
	    if (!printing(c)) {
		putwords(!ENDPAR);
		if (word[lastword].nchars == 0) {	/* line was blank */
		    putwords(ENDPAR);
		    putchar('\n');
		    break;
		}
	    }
	}
	if (c == EOF) {
	    finish();
	    return;
	}
    }
}


getword()
{
    register int    i;
    register struct wrd *wp;
    register char  *cp;
    static int      col = 0;
    int             strtcol, nextcol;
    int             c;
    int             leadhyphens;
    int             backspaced;

    /* wrap around char pointer and word index as necessary */
    wp = &word[lastword];
    if (wp->chars + wp->strleng - inbuf + lineleng < MAXINBUF)
	cp = wp->chars + wp->strleng;
    else {
	if (word[firstword].chars == inbuf)
	    return '\n';	/* inbuf is full! */
	else
	    cp = inbuf;
    }
    if (wp->nextchar == '\n') {
	inpline++;
	col = 0;
    }
    lastword = next(lastword);
    wp = &word[lastword];
    wp->chars = cp;
    wp->nchars = wp->strleng = 0;
    wp->firstchar = 0;
    wp->lastchar = 0;

    if (nextcflg) {
	nextcflg = 0;
	c = nextc;
    } else
	c = getch();
    if (col == 0) {
	for (i = 0; c != EOF; c = getch()) {
	    if (c == ' ')
		i++;
	    else if (c == '\t')
		i += i % TABCOL + TABCOL;
	    else if (c == '\b') {
		if (i > 0)
		    i--;
		else {
		    fprintf(stderr,
		      "%s: backsp past newline ignored on input line %d.\n",
			    progname, inpline);
		    bksperr = 1;
		    c = getch();
		    break;
		}
	    } else
		break;
	}
	prespace = col = i;
    }
    strtcol = nextcol = col;
    leadhyphens = 1;
    backspaced = 0;
    for (; c != EOF; c = getch()) {
	if (printing(c)) {
	    if (col == strtcol) {
		if (!(isupper(wp->firstchar) || wp->firstchar == '-'))
		    wp->firstchar = c;
	    }
	    /* else if (   (!leadhyphens)  */
	    else if (hyphenate && (!leadhyphens)
		     && cp[-1] == '-'
		     && c != '-'
		     && col == nextcol
		)
		break;
	    *cp++ = c;
	    if (c != '-')
		leadhyphens = 0;
	    if (++col >= nextcol) {
		nextcol = col;
		if (!backspaced || !doublesp(wp->lastchar))
		    wp->lastchar = c;
		backspaced = 0;
	    }
	} else if (c == '\b') {
	    backspaced = 1;
	    if (col > strtcol) {
		col--;
		*cp++ = c;
	    } else {
		fprintf(stderr,
		  "%s: backsp past beg of word ignored on input line %d.\n",
			progname, inpline);
		bksperr = 1;
		c = getch();
		break;
	    }
	} else if (spcortab(c) && col < nextcol) {
	    if (c == ' ') {
		*cp++ = c;
		++col;
	    } else if (c == '\t') {
		i = (i = col % TABCOL) ? TABCOL - i : TABCOL;
		do {
		    *cp++ = ' ';
		    col++;
		} while (--i);
	    }
	    if (col > nextcol)
		nextcol = col;
	} else if (c == ' ' || c == '\t' || c == '\n' || c == '\f') {
	    for (; col < nextcol;) {
		*cp++ = ' ';
		col++;
	    }
	    break;
	} else			/* a CTRL char */
	    *cp++ = c;

	if (cp >= &inbuf[MAXINBUF - 1]) {
	    fprintf(stderr, "%s: line overflow on input line %d!\n",
		    progname, inpline);
	    break;
	}
    }
    wp->brkchar = c;
    wp->nchars = nextcol - strtcol;
    wp->strleng = cp - wp->chars;

    for (; c != EOF; c = getch()) {
	if (c == ' ')
	    ++col;
	else if (c == '\t') {
	    i = (i = col % TABCOL) ? TABCOL - i : TABCOL;
	    do
		col++;
	    while (--i);
	} else
	    break;
    }
    wp->nextchar = c;
    if (c != '\n' && c != '\f' && c != EOF) {
	nextc = c;
	nextcflg = 1;
    }
    addword(lastword);
    return (bksperr ? EOF : c);
}


next(wordindex)
{
    return (wordindex < NWORDS - 1 ? wordindex + 1 : 0);
}


prev(wordindex)
{
    return (wordindex > 0 ? wordindex - 1 : NWORDS - 1);
}


addword(wordindex)
{
    register int    i;
    register struct wrd *wp;
    register struct wrd *lwp;
    int             space;

    wp = &word[wordindex];
    if (wp->nchars == 0)
	return;
    if (o_fullflg == 0) {
	if (o_nwords == 0)
	    space = 0;
	else {
	    lwp = &word[prev(wordindex)];
	    if (isupper(wp->firstchar)
		&& doublesp(lwp->lastchar)
		)
		space = 2;
	    else if (lwp->lastchar == '-'
		     && (!hyonly(lwp->chars, lwp->strleng))
		     && (printing(lwp->brkchar)
			 || lwp->nextchar == '\n'
			 || lwp->nextchar == '\f'
			 )
		     && wp->firstchar != '-'
		)
		space = 0;
	    else
		space = 1;
	    oword[o_nwords - 1].nspaces = space;
	}
	if ((i = o_nchars + space + wp->nchars) <= lineleng - curindent()
	    || o_nwords == 0
	    ) {
	    o_nchars = i;
	    o_nwords++;
	} else
	    o_moreflg = 1;
	if (i > lineleng - curindent())
	    o_fullflg = 1;
    } else
	o_moreflg = 1;
}


hyonly(cp, n)
    register char  *cp;
    register        n;
{
    register        c;

    for (; n > 0; n--) {
	c = *cp++;
	if (printing(c) && c != '-')
	    return (0);
    }
    return (1);
}

putwords(endparflg)
{
    struct wrd     *wp;
    register char  *cp;
    register int    j;
    int             n;
    int             i;

    for (;;) {
	/* try to put out one line's worth     */
	if (!endparflg && !o_fullflg)
	    return;
	else if (justflg && o_moreflg)
	    justify();
	n = curindent();
	for (i = 0; i < n; i++)
	    putchar(' ');
	for (n = 0, i = firstword; n < o_nwords; n++, i = next(i)) {
	    wp = &word[i];
	    for (cp = wp->chars, j = 0; j < wp->strleng; j++)
		putchar(*cp++);
	    if (n < o_nwords - 1) {
		for (j = 0; j < oword[n].nspaces; j++)
		    putchar(' ');
	    }
	}
	putchar('\n');
	outparline++;
	firstword = i;
	o_nchars = 0;
	o_nwords = 0;
	o_fullflg = 0;
	if (o_moreflg) {
	    o_moreflg = 0;
	    while (o_moreflg == 0) {
		addword(i);
		if (i == lastword)
		    break;
		else
		    i = next(i);
	    }
	} else
	    break;
    }
}


justify()
{
    register int    i, n;
    int             ems;	/* number of spaces to distribute in the line  */
    int             maxgap, ncandidates;

    if ((ems = lineleng - o_nchars - curindent()) == 0)
	return;			/* already justified */
    /* mark certain gaps as candidates for expansion                       */
    ncandidates = 0;
    for (n = 0; n < o_nwords - 1; n++) {
	if (oword[n].candidate = oword[n].nspaces > 0)
	    ncandidates++;
    }
    if (ncandidates == 0)
	return;			/* no spaces to pad */
    /* if there are more ems than gaps to put them in,                     */
    /* add equal numbers of spaces to all the gaps                       */
    maxgap = 2;
    if (i = ems / ncandidates) {
	for (n = 0; n < o_nwords - 1; n++) {
	    if (oword[n].candidate)
		oword[n].nspaces += i;
	}
	if ((ems %= ncandidates) == 0)
	    return;
	maxgap += i;
    }
    /* we now have fewer ems to distribute than gaps                       */
    /* mark the smaller gaps as candidates for expansion                   */
    ncandidates = 0;
    for (n = 0; n < o_nwords - 1; n++) {
	if (oword[n].candidate =
	    oword[n].candidate && oword[n].nspaces < maxgap)
	    ncandidates++;
    }
    /* if none of the gaps are of the smaller size then they are all the   */
    /* same - mark all non-zero sized gaps as candidates and inc maxgap    */
    if (ncandidates == 0) {
	for (n = 0; n < o_nwords - 1; n++) {
	    if (oword[n].candidate = oword[n].nspaces > 0)
		ncandidates++;
	}
	maxgap++;
    }
    /* else if there are more ems than small gaps, fill up all the small   */
    /* first                                                           */
    else if (ems >= ncandidates) {
	ems -= ncandidates;
	ncandidates = 0;
	for (n = 0; n < o_nwords - 1; n++) {
	    if (oword[n].candidate) {
		oword[n].nspaces = maxgap;
		oword[n].candidate = 0;
	    } else {
		oword[n].candidate = oword[n].nspaces > 0;
		ncandidates++;
	    }
	}
    }
    if (ems == 0)
	return;			/* all spaces are equal & line is justified */

    /* here is where you have to decide where to fill out a line by        */
    /* distributing leftover spaces among the candidates.                  */

    /* the algorythm presented here starts widening from the right on odd  */
    /* lines in paragraphs and from the left on even lines.                */
    /* N.B. other tricks to sprinkle extra spaces more randomly throughout */
    /* the line are annoying to the reader;  I tried it. - D. Yost         */

    if (outparline & 1) {
	for (n = o_nwords - 2; ems; n--) {
	    if (oword[n].candidate) {
		oword[n].nspaces++;
		ems--;
	    }
	}
    } else {
	for (n = 0; ems; n++) {
	    if (oword[n].candidate) {
		oword[n].nspaces++;
		ems--;
	    }
	}
    }
    return;
}


finish()
{
    putwords(ENDPAR);
    if (bksperr) {
	fprintf(stderr, "%s: stopped.\n", progname);
	exit(-2);
    }
}


getch()
{

    return (getc(input));

#ifdef OUT
    register int    c;

    for (c = getc(input);; c = getc(input)) {
	if (spcortab(c)
	    || printing(c)
	    || c == '\b'
	    || c == '\n'
	    || c == '\f'
	    || c == EOF
	    )
	    return (c);
	else
	    fprintf(stderr, "%s: \\%o char ignored.\n", progname, c);
    }
#endif
}


doublesp(c)
    register int    c;
{
    register char  *cp;

    for (cp = dblstr; *cp; cp++)
	if (c == *cp)
	    return (1);
    return (0);
}

intss()
{
    return isatty(fileno(stdin));
}




#ifdef ALLOWNROFF

#ifdef COMMENT
/*  * * */
/***************************************************************************
 *
 *  The following is the old fill/just program modified to work here
 *
 ***************************************************************************
 *
 *  fill/just - source code for preprocessors used to run nroff
 *          from the Rand Editor (ned).
 *
 *  Original:
 *  13 Sep 76   Walt Bilofsky
 *
 *  1 Oct 77    Dave Crocker    Rewritten to fully buffer output,
 *                pipe into nroff, rather than use
 *                a temporary file, and minimize
 *                number of .in commands given.
 *
 *  4 Oct 77    Dave Crocker    Really re-written.  Not so  eager
 *                to remove extra spaces.
 *
 *  19 Feb 79   Dave Yost   use stdio where possible
 *
 *  Unless the "x" argument  is  given,  may  replaces  multiple
 *  blanks/tabs with fewer blanks; leaves indentation alone on the
 *  first line of paragraph,  and  indents  all  subsequent  lines
 *  according to the indentation on the second line.
 *
 **/
#endif

/* #define DBG  */
/**/
#define LBUF 512

int             i;
int             tmp;
int             childid;	/* nroff                        */
int             rflag;
FILE           *out[2];		/* input and output pipe        */
FILE           *output;		/* output pipe set to = out[1]  */
int             nchar;		/* data chars on output line    */
int             line;		/* output paragraph line number */
int             inpline;         /* input line number            */
int             outcol;		/* output line column           */
int             ind;		/* indent for current par       */
int             flinind;	/* indent for 1st line of par   */
int             oflind;		/* flinind for last par         */
int             oldind;		/* indent for last par          */
int             spaces;		/* current block of white space */
int             incol;		/* column of current input line */
int             fwordflg;	/* now saving first word of par */
int             fwlen;		/* length of first word         */
int             curbuf;		/* current linbuf being built   */
char            linbuf[2][200];	/* */
char           *args[50];	/* for nroff                    */
char            fword[100];	/* first word of current par    */
char            oldchar;	/* last character read          */

/*  * * */
feednroff()
{
    char          **d, **c;	/* only used for parms          */
    register char   e, *p;

    /* Copy argument list either to roff or to roff file */
    c = args;

    rflag = 0;
    *c++ = "/usr/bin/nroff";

    d = &argarray[1];
    for (curarg = 1; curarg < numargs; curarg++) {
	if (**d == 'x')
	    rflag = 1;
	else if (**d != '.')
	    *c++ = *d;
	d++;
    }
    *c++ = 0;

    if (intss())
	fprintf(stderr, "%c%s: start typing.\n", BELL, progname);
#ifdef DBG
    output = stdout;
#endif
#ifndef DBG
    plumb();
#endif

    if (!justflg)
	fputs(".na\n", output);
    fputs(".hy 0\n.pl 32767\n", output);	/* hyphenate off                */

    d = argarray + 1;
    fprintf(output, ".ll %d\n", lineleng);
    d = &argarray[1];
    for (curarg = 1; curarg < numargs; curarg++) {
	if (**d == '.')
	    fprintf(output, "%s\n", *d);
	d++;
    }
    fputs(".rs\n.c2 ~\n.cc ~\n~ec ~\n", output);
    inpline = line = 1;

    p = linbuf[0];
    e = getchar();		/* take care of first-  */
    switch (e) {		/* line blank           */
    case EOF:
	endit();
	return;

    case ' ':
	spaces++;
	incol++;
	break;

    case '\t':
	incol = spaces = 8;
	break;

    case '\n':
	fputs("~~sp\n", output);
	break;

    default:
	*p++ = e;
	outcol++;
	nchar++;
	fwordflg = 1;
	fword[fwlen++] = e;
    };

    /*      * * */
    for (; (e = getchar()) != EOF;) {
	incol++;

	switch (e) {
	case ' ':
	    spaces++;
	    continue;

	case '\t':
	    tmp = 0177770 & (--incol + 8);
	    spaces += tmp - incol;
	    incol = tmp;
	    continue;

	case '\n':
	    if (nchar == 0) {	/* blank line           */
		parmark();
		putc('\n', output);
	    } else {
		*p++ = '\n';
		*p = '\0';
		line++;
		plaslin();
		curbuf ^= 1;
		p = linbuf[curbuf];
	    }
	    inpline = incol = nchar = outcol = 0;
	    continue;
	}

	marganl();

	if (spaces) {
	    if (nchar > 0) {
		for (tmp = ((fwordflg || rflag)
			    ? spaces
			    : ((oldchar == '.') && (spaces > 1))
			    ? 2
			    : 1);
		     tmp--; nchar++) {
		    *p++ = ' ';
		    outcol++;
		}

		if (fwordflg)
		    fwordflg = fword[fwlen] = 0;
	    }
	    spaces = 0;
	}
	*p++ = e;
	outcol++;
	nchar++;

	if (fwordflg)
	    fword[fwlen++] = e;

	switch (e) {		/* preserve multiple    */
	    /* spaces?              */
	case '!':
	case '?':
	case ':':
	    oldchar = '.';
	    break;

	case ')':
	case ']':
	case '}':
	    if (oldchar == '.')
		break;
	default:
	    oldchar = e;
	};
    }
    endit();
}
/*  * * */
parmark()
{				/* delimit paragraphs           */
    marganl();

    plaslin();
    oldind = ind;
    oflind = flinind;
    flinind = 0;
    line = 1;
}


plaslin()
{
    register char  *linpt;

    linpt = linbuf[curbuf ^ 1];

    if (*linpt) {
	fputs(linpt, output);
	*linpt = 0;
    }
}


marganl()
{				/* analyze margins for crnt par */
    if (nchar == 0) {
	switch (line) {
	case 1:
	    flinind = spaces;
	    fwordflg = 1;
	    fwlen = 0;
	    break;

	case 2:
	    ind = spaces;

	    if (ind != oldind) {
		if (oldind > ind)
		    fprintf(output, "~~in -%d\n", (oldind - ind));
		else
		    fprintf(output, "~~in +%d\n", (ind - oldind));
	    };

	    if (flinind != ind) {
		if (flinind > ind)
		    fprintf(output, "~~ti +%d\n", flinind - ind);
		else
		    fprintf(output, "~~ti -%d\n", ind - flinind);
	    }
	    plaslin();
	}
    }
}

endit()
{
    int             j;
    int             wi;

    parmark();

    fputs("~~pl 1\n", output);	/* suppress terminal blank lines */
    fflush(output);

#ifndef DBG
    fclose(out[1]);
    while ((j = wait(&wi) != childid) && (j != -1)) {
    }
#endif
};


plumb()
{
    register int    m;
    FILE           *fpipe();

    if (fpipe(out) == NULL) {
	fprintf(stderr, "%s: can't pipe.\n", progname);
	exit(-2);
    }
    output = out[1];

    while ((childid = fork()) == -1)
	sleep(10);

    if (childid == 0) {
	close(0);
	dup(fileno(out[0]));
	for (m = 2; m < 16; m++)
	    close(m);
	execv(args[0], args);
	close(0);
	exit(0);
    };

    fclose(out[0]);
};

/* The following code was added because fpipe does not live in any  */
/* standard include file.  fpipe can be found in Dave Yost's stdio. */
/* WG  */
FILE           *
fpipe(pps)
    FILE           *pps[];
{
    int             fds[2];

    if (pipe(fds) == -1)
	return NULL;
    pps[0] = fdopen(fds[0], "r");
    pps[1] = fdopen(fds[1], "w");
    return pps[0];
}

#endif
