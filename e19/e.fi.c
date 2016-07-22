#ifdef COMMENT
--------
file e.fi.c
    filter functions, like fill, justify, center
    For use on machines with big instruction address space
    instead of piping to a filter program.
    UNDER CONSTRUCTION
#endif

#ifdef COMMENT
	Copyright abandoned, 1983, The Rand Corporation
#endif

#include "e.h"
#ifdef UNIXV7
#include <ctype.h>
#endif

#ifdef COMMENT

    Handles backspaces and tabs.

	 ##### not yet smart about '\r' ######

#endif

#define MAXINBUF 1024           /* must be at least 512                    */
#define MAXOUTLINE 162
#define MAXINT 32767
#define MAXTIE 10
#define TABCOL 8
#define LINELENGDEFLT 65
#define spcortab(c) (c == ' ' || c == '\t')
#define curindent() (outparline == 1 ? indent[0]: indent[1])

#define NWORDS MAXINBUF/2
#define ENDPAR YES

#ifdef ALLOWNROFF
int usenroff = 0;               /* when set means must use nroff           */
#endif

char inbuf[MAXINBUF];

int justflg = 0;                /* 1 if just, 0 if fill                    */

int indent[2];                  /* first and second line indents           */
int prespace;                   /* num of spaces at last beg of line       */

char dblstr[] = ".?!:";         /* put two spaces after these chars when   */
				/* next word starts with a cap. letter     */

/* info relating to line to be output */
int o_nchars = 0;               /* num of chars accumulated in the line    */
int o_nwords = 0;               /* number of words in line                 */
int o_fullflg = 0;              /* we have a full line already             */
int o_moreflg = 0;              /* there is more after this line is output */
struct lwrd {
    char candidate;             /* candidate for spreading on this pass    */
    char nspaces;               /* num of spaces after word                */
 } oword[MAXOUTLINE/2];

struct wrd {
    int nchars;                 /* num of chars in word                    */
    int strleng;                /* num of chars collected for word         */
    char *chars;                /* where word is in inbuf                  */
    char firstchar;             /* first char of word                      */
    char lastchar;              /* last char of word                       */
    char brkchar;               /* char that caused end-of-word            */
    char nextchar;              /* first char of next word                 */
 };                             /*   or \n or \f or EOF                    */

int firstword;              /* index of first word in wordtbl              */
int lastword;               /* index of last word in wordtbl               */
struct wrd word[NWORDS];    /* the words                                   */

int bksperr = 0;                /* illegal use of backspace encountered    */
int inline;                     /* line number of input                    */
int outparline;                 /* line num within current para output     */
int nextcflg = 0;               /* there was a char pushed back onto input */
int nextc;                      /* this is the char pushed back            */


dofill ()
{
    inline = 1;
    for (;;) {
	/* once for each paragraph */
	Reg1 int c;
	indent[0] = indent[1] = 0;
	outparline = 1;

	for (;;) {                      /* treat blank lines at beginning */
	    firstword = 0;
	    lastword = NWORDS - 1;
	    word[firstword].chars = 0;  /* to detect overflow on first */
					/* two lines in paragraph */
	    word[lastword].chars = inbuf + MAXINBUF;
	    word[lastword].nextchar = 0;
	    word[lastword].brkchar = 0;
	    word[lastword].strleng = 0;
	    o_nchars = o_nwords = o_fullflg = o_moreflg = 0;

	    if ((c = fi_getword ()) == EOF) {
		finish ();
		return;
	    }
	    if (!isprint (c) && word[lastword].nchars == 0)
		filtputc (c);
	    else
		break;
	}
	indent[0] = prespace;           /* set first line indent */
	for (; c != EOF; c = fi_getword ()) {  /* get remainder of first line */
	    if (!isprint (c))
		break;
	}
	if (c == EOF) {
	    finish ();
	    return;
	}
	if ((c = fi_getword ()) == EOF) {    /* get line 2 indent           */
	    finish ();
	    return;
	}
	indent[1] = prespace;
	for (; c != EOF; c = fi_getword ()) {
	    if (!isprint (c)) {
		putwords (!ENDPAR);
		if (word[lastword].nchars == 0) { /* line was blank */
		    putwords (ENDPAR);
		    filtputc ('\n');
		    break;
		}
	    }
	}
	if (c == EOF) {
	    finish ();
	    return;
	}
    }
    return;
}


static int fi_getword ()
{
    Reg4 struct wrd *wp;
    Reg3 char *cp;
    static int col = 0;
    int strtcol, nextcol;
    Reg2 int chr;
    int leadhyphens;
    int backspaced;

    /* wrap around char pointer and word index as necessary */
    wp = &word[lastword];
    if (wp->chars + wp->strleng - inbuf + linewidth < MAXINBUF)
	cp = wp->chars + wp->strleng;
    else {
	if (word[firstword].chars == inbuf)
	    return '\n';    /* inbuf is full! */
	else
	    cp = inbuf;
    }
    if (wp->nextchar == '\n') {
	inline++;
	col = 0;
    }
    lastword = next (lastword);
    wp = &word[lastword];
    wp->chars = cp;
    wp->nchars = wp->strleng = 0;
    wp->firstchar = 0;
    wp->lastchar = 0;

    if (nextcflg) {
	nextcflg = 0;
	chr = nextc;
    }
    else
	chr = getch ();
    if (col == 0) {
	Reg1 int tmp;
	for (tmp = 0; chr != EOF; chr = getch ()) {
	    if (chr == ' ')
		tmp++;
	    else if (chr == '\t')
		tmp += tmp % TABCOL + TABCOL;
	    else if (chr == '\b') {
		if (tmp > 0)
		    tmp--;
		else {
		    fprintf (stderr,
		      "%s: backsp past newline ignored on input line %d.\n",
		      progname, inline);
		    bksperr = 1;
		    chr = getch ();
		    break;
		}
	    }
	    else
		break;
	}
	prespace = col = tmp;
    }
    strtcol = nextcol = col;
    leadhyphens = 1;
    backspaced = 0;
    for (; chr != EOF; chr = getch ()) {
	if (isprint (chr)) {
	    if (col == strtcol) {
		if (!(isupper (wp->firstchar) || wp->firstchar == '-'))
		    wp->firstchar = c;
	    }
	    else if (   !leadhyphens
		     && cp[-1] == '-'
		     && chr != '-'
		     && col == nextcol
		    )
		break;
	    *cp++ = chr;
	    if (chr != '-')
		leadhyphens = 0;
	    if (++col >= nextcol) {
		nextcol = col;
		if (!backspaced || !doublesp (wp->lastchar))
		    wp->lastchar = chr;
		backspaced = 0;
	    }
	}
	else if (chr == '\b') {
	    backspaced = 1;
	    if (col > strtcol) {
		col--;
		*cp++ = chr;
	    }
	    else {
		fprintf (stderr,
		  "%s: backsp past beg of word ignored on input line %d.\n",
		  progname, inline);
		bksperr = 1;
		chr = getch ();
		break;
	    }
	}
	else if (spcortab (chr) && col < nextcol) {
	    if (chr == ' ') {
		*cp++ = chr;
		++col;
	    }
	    else if (chr == '\t'){
		Reg1 Small tmp;
		tmp = (tmp = col % TABCOL) ? TABCOL - tmp : TABCOL;
		do {
		    *cp++ = ' ';
		    col++;
		} while (--tmp);
	    }
	    if (col > nextcol)
		nextcol = col;
	}
	else {
	    for (; col < nextcol; ) {
		*cp++ = ' ';
		col++;
	    }
	    break;
	}
	if (cp >= &inbuf[MAXINBUF-1]) {
	    fprintf (stderr, "%s: line overflow on input line %d!\n",
		progname, inline);
	    break;
	}
    }
    wp->brkchar = chr;
    wp->nchars = nextcol - strtcol;
    wp->strleng = cp - wp->chars;

    for (; chr != EOF; chr = getch ()) {
	if (chr == ' ')
	    ++col;
	else if (chr == '\t') {
	    Reg1 Small tmp;
	    tmp = (tmp = col % TABCOL) ? TABCOL - tmp : TABCOL;
	    do {
		col++;
	    } while (--tmp);
	}
	else
	    break;
    }
    wp->nextchar = chr;
    if (chr != '\n' && chr != '\f' && chr != EOF) {
	nextc = chr;
	nextcflg = 1;
    }
    addword (lastword);
    return bksperr ? EOF : chr;
}


next (wordindex)
{
    return wordindex < NWORDS - 1 ? wordindex + 1 : 0;
}


prev (wordindex)
{
    return wordindex > 0 ? wordindex - 1 : NWORDS - 1;
}


addword (wordindex)
{
    register int i;
    register struct wrd *wp;
    register struct wrd *lwp;
    int space;

    wp = &word[wordindex];
    if (wp->nchars == 0)
	return;
    if (o_fullflg == 0) {
	if (o_nwords == 0)
	    space = 0;
	else {
	    lwp = &word[prev (wordindex)];
	    if (   isupper (wp->firstchar)
		&& doublesp (lwp->lastchar)
	       )
		space = 2;
	    else if (   lwp->lastchar == '-'
		     && (!hyonly (lwp->chars, lwp->strleng))
		     && (   isprint (lwp->brkchar)
			 || lwp->nextchar == '\n'
			 || lwp->nextchar == '\f'
			)
		     && wp->firstchar != '-'
		    )
		space = 0;
	    else
		space = 1;
	    oword[o_nwords-1].nspaces = space;
	}
	if (   (i = o_nchars + space + wp->nchars) <= linewidth - curindent ()
	    || o_nwords == 0
	   ) {
	    o_nchars = i;
	    o_nwords++;
	}
	else
	    o_moreflg = 1;
	if (i > linewidth - curindent ())
	    o_fullflg = 1;
    }
    else
	o_moreflg = 1;
    return;
}


hyonly (cp, n)
register char *cp;
register n;
{
    register c;

    for (; n > 0; n--) {
	c = *cp++;
	if (isprint (c) && c != '-')
	    return 0;
    }
    return 1;
}

putwords (endparflg)
Flag endparflg;
{
    struct wrd *wp;
    register char *cp;
    register int j;
    int n;
    int i;

    for (;;) {
	/* try to put out one line's worth     */
	if (!endparflg && !o_fullflg)
	    break;
	else if (justflg && o_moreflg)
	    justify ();
	n = curindent ();
	for (i = 0; i < n; i++)
	    filtputc (' ');
	for (n = 0, i = firstword; n < o_nwords; n++, i = next (i)) {
	    wp = &word[i];
	    for (cp = wp->chars, j = 0; j < wp->strleng; j++)
		filtputc (*cp++);
	    if (n < o_nwords - 1) {
		for (j = 0; j < oword[n].nspaces; j++)
		    filtputc (' ');
	    }
	}
	filtputc ('\n');
	outparline++;
	firstword = i;
	o_nchars = 0;
	o_nwords = 0;
	o_fullflg = 0;
	if (!o_moreflg)
	    break;
	else {
	    o_moreflg = 0;
	    while (o_moreflg == 0) {
		addword (i);
		if (i == lastword)
		    break;
		else
		    i = next (i);
	    }
	}
    }
    return;
}


justify ()
{
    Reg5 int maxgap;
    Reg4 int ncandidates;
    Reg3 int ems;               /* number of spaces to distribute in the line  */

    if ((ems = linewidth - o_nchars - curindent ()) == 0)
	return;         /* already justified */
    /* mark certain gaps as candidates for expansion                       */
    ncandidates = 0;
    Block {
	Reg2 int tmp2;
	for (tmp2 = 0; tmp2 < o_nwords - 1; tmp2++) {
	    if (oword[tmp2].candidate = oword[tmp2].nspaces > 0)
		ncandidates++;
	}
    }
    if (ncandidates == 0)
	return;         /* no spaces to pad */
    /* if there are more ems than gaps to put them in,                     */
    /*   add equal numbers of spaces to all the gaps                       */
    maxgap = 2;
    Block {
	Reg1 int tmp1;
	Reg2 int tmp2;
	if (tmp1 = ems / ncandidates) {
	    for (tmp2 = 0; tmp2 < o_nwords-1; tmp2++) {
		if (oword[tmp2].candidate)
		    oword[tmp2].nspaces += tmp1;
	    }
	    if ((ems %= ncandidates) == 0)
		return;
	    maxgap += tmp1;
	}
    }

    /* we now have fewer ems to distribute than gaps                       */
    /* mark the smaller gaps as candidates for expansion                   */
    ncandidates = 0;
    Block {
	Reg2 int tmp2;
	for (tmp2 = 0; tmp2 < o_nwords - 1; tmp2++) {
	    if (oword[tmp2].candidate =
		    (word[tmp2].candidate && oword[tmp2].nspaces < maxgap))
		ncandidates++;
	}
    }
    /* if none of the gaps are of the smaller size then they are all the   */
    /* same - mark all non-zero sized gaps as candidates and inc maxgap    */
    if (ncandidates == 0) {
	Reg2 int tmp2;
	for (tmp2 = 0; tmp2 < o_nwords - 1; tmp2++) {
	    if (oword[tmp2].candidate = oword[tmp2].nspaces > 0)
		ncandidates++;
	}
	maxgap++;
    }
    /* else if there are more ems than small gaps, fill up all the small   */
    /*     first                                                           */
    else if (ems >= ncandidates) {
	Reg2 int tmp2;
	ems -= ncandidates;
	ncandidates = 0;
	for (tmp2 = 0; tmp2 < o_nwords - 1; tmp2++) {
	    if (oword[tmp2].candidate) {
		oword[tmp2].nspaces = maxgap;
		oword[tmp2].candidate = 0;
	    }
	    else {
		oword[tmp2].candidate = oword[tmp2].nspaces > 0;
		ncandidates++;
	    }
	}
    }
    if (ems == 0)
	return;         /* all spaces are equal & line is justified */

    /* here is where you have to decide where to fill out a line by        */
    /* distributing leftover spaces among the candidates.                  */

    /* the algorythm presented here starts widening from the right on odd  */
    /* lines in paragraphs and from the left on even lines.                */
    /* N.B. other tricks to sprinkle extra spaces more randomly throughout */
    /* the line are annoying to the reader;  I tried it. - D. Yost         */

    if (outparline & 1) {
	Reg2 int tmp2;
	for (tmp2 = o_nwords - 2; ems; tmp2--) {
	    if (oword[tmp2].candidate) {
		oword[tmp2].nspaces++;
		ems--;
	    }
	}
    }
    else {
	Reg2 int tmp2;
	for (tmp2 = 0; ems; tmp2++) {
	    if (oword[tmp2].candidate) {
		oword[tmp2].nspaces++;
		ems--;
	    }
	}
    }
    return;
}


finish ()
{
    putwords (ENDPAR);
    if (bksperr) {
	fprintf (stderr, "%s: stopped.\n", progname);
	exit (-2);
    }
    return;
}


getch ()
{
    Reg1 chr;

    for (chr = getc (input); ; chr = getc (input)) {
	if (   spcortab (chr)
	    || isprint (chr)
	    || chr == '\b'
	    || chr == '\n'
	    || chr == '\f'
	    || chr == EOF
	   )
	    return chr;
	else
	    fprintf (stderr, "%s: \\%o char ignored.\n", progname, chr);
    }
}


doublesp (c)
Reg2 int c;
{
    Reg1 char *cp;

    for (cp = dblstr; *cp; cp++)
	if (c == *cp)
	    return 1;
    return 0;
}
