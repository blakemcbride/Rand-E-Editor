#ifdef COMMENT
--------
file e.re.c
    regular expression pattern matching
#endif

#ifdef COMMENT
	Copyright abandoned, 1983, The Rand Corporation
#endif

#ifdef UNIXV7
#include <ctype.h>
#endif
#include "e.h"
#include "e.m.h"
#include "e.cm.h"
#include "e.se.h"

extern char *re_comp (), *re_exec(), *re_replace();

extern Nlines srchline;
extern Ncols srchcol;
extern Flag starthere;
extern Flag first_repl_line;
extern int DebugVal;

static Flag re_zerolen1;        /* we KNOW the length is zero when set */

/* XXXXXXXXXXXXXXXXXXX */
static  int re_advance();

#ifdef COMMENT
Small
patsearch (str, ln, stcol, limit, delta, srch)
    char   *str;
    Nlines  ln;
    Ncols   stcol;
    Nlines  limit;
    Small   delta;
    Flag srch;      /* if NO, then only look at current position */
.
    /*
    Searches through curfile for pat starting at [ln, stcol].
    If 'srch' == 0, merely check to see if current position matches pat;
    don't look further.
    'Delta' is either 1 for forward search or -1 for backward search.
    Don't go beyond 'limit' line.
    Assumes curwksp->wfile == curfile.

    Added Purdue CS 10/18/82 MAB
    */
#endif
Small
patsearch (pat, ln, stcol, limit, delta, srch)
char   *pat;
Nlines  ln;
Ncols   stcol;
Nlines  limit;
Small   delta;
Flag srch;      /* if NO, then only look at current position */
{
    register char  *at;
    register char  *fk;
    char   *at1;
    Nlines  continln;
    Flag    retval;
    char b[BUFSIZ];


    for(fk = pat, at = b; *fk; fk++, at++){
	    if (*fk == ESCCHAR && (fk[1]|0140) == 'j'){
		mesg (ERRALL + 1, "use ^ or $ rather than ^J");
		return NOTFOUND_SRCH;
	    }
	    else
		*at = *fk;
    }
    if (at[-1] == '\\')
	at[-1] = '$';
    *at = '\0';

    retval = NOTFOUND_SRCH;
    intrupnum = 0;

    if ((limit = rangelimit (ln, delta, limit)) == -1) {
	retval = OUTRANGE_SRCH;
	goto pret;
    }

    /*
     *  Handle the two special cases "$" and "^" directly.
     */
    re_zerolen1 = NO;
    if (b[1] == '\0' && (b[0] == '$' || b[0] == '^'))
	return( BegOrEnd( *b, ln, stcol, limit, delta, !srch ));

    if ((at = re_comp(b)) != (char *) 0){
	mesg (ERRALL + 4, pat, ": malformed regular expression (", at, ")");
	return NOTFOUND_SRCH;
    }


    if (ln >= la_lsize (curlas)) {
	if (delta > 0)
	    goto pret;
	ln = la_lsize (curlas) - 1;
	getline (ln);
	at = &cline[ncline];        /* past end (delta is < 0) */
    }
    else {
	getline (ln);
	/*
	 *  If pattern is "^$" and already at a blank line, start at
	 *  next/previous line.  If that line is blank, found it.
	 */
	if (ncline == 1 && b[2] == '\0' && b[0] == '^' && b[1] == '$') {
	    if (delta > 0) {
		if (++ln >= la_lsize (curlas))
		    goto pret;
		getline (ln);
	    }
	    else {
		if (--ln < 1)
		    goto pret;
		getline (ln);
	    }
	    if (ncline == 1) {      /* blank line */
		retval = FOUND_SRCH;
		at = cline;
		goto pret;
	    }
	}
	/*
	 * set starting search position
	 */
	if (delta > 0)
	    at = &cline[stcol+(starthere ? 0 : 1)];
/*          at = &cline[stcol+((stcol+1)<ncline)];      */
	else
	    at = &cline[stcol];

	if (at - cline > ncline - 1)
	    at = &cline[ncline - 1];
    }
    cline[ncline - 1] = '\0';

    /*
     * First line is special if going backwards
     */
    if (delta < 0) {
	at1 = cline - 1;
	/*
	 *  look for last match prior to starting position
	 */
	while ((fk = re_exec(at1+1)) != NULL && fk < at)
	    at1 = fk;
	at = at1;
	/*
	 *  If no occurrence on this line (at == cline -1) get last line.
	 *  Otherwise a match is found.
	 */
	if (at == cline - 1) {
	    if (--ln < limit) {
		retval = NOTFOUND_SRCH;
		goto pret;
	    }
	    getline (ln);
	    cline[ncline - 1] = '\0';
	    at = cline;
	}
	else {
	    retval = FOUND_SRCH;
	    goto pret;
	}
    }

    /*
     *  main searching loop
     */

    for ( ; ; ) {
	if ((fk = re_exec(at)) != NULL && (b[0] != '^' || fk == cline)){
	    retval = FOUND_SRCH;
	    at = fk;
	    if (delta < 0 && b[0] != '^') {
		/*
		 * find last match on line
		 */
		while ((fk = re_exec(at+1)) != NULL)
		    at = fk;
	    }
	    break;
	}
	if (!srch)
	    break;
	ln += delta;
	if (   (delta < 0 && ln < limit)
	    || (delta > 0 && ln > limit)
	    || (   intrupnum++ > SRCHPOL
		&& (retval = sintrup () ? ABORTED_SRCH : NOTFOUND_SRCH)
		   == ABORTED_SRCH
	       )
	   )
	    break;
	getline (ln);
	at = cline;
	cline[ncline - 1] = '\0';
    }
 pret:
    srchline = ln;
    srchcol = at - cline;

    return retval;
}


#ifdef COMMENT
Small
BegOrEnd (ch, ln, stcol, limit, delta, curpos_only)
    int     ch;
    Nlines  ln;
    Ncols   stcol;
    Nlines  limit;
    Small   delta;
    Flag    curpos_only;
.
    /*
    Find the beginning or end of line with the proviso that if
    the current position is already at the end (or beg) of a line, you're
    looking for the end (or beg) of the next or last line (unless
    the repl_first_line flag is set).

    Assumes curwksp->wfile == curfile.
    */
#endif
Small
BegOrEnd (ch, ln, stcol, limit, delta, curpos_only)
int     ch;
Nlines  ln;
Ncols   stcol;
Nlines  limit;
Small   delta;
Flag    curpos_only;
{

	re_zerolen1 = YES;

	if (delta > 0) {        /* forward search */
	    if (ln >= la_lsize (curlas))
		return NOTFOUND_SRCH;
	    getline (ln);
	    if (ch == '$') {
		if (curpos_only) {              /* repl interactive */
		    if (stcol == ncline - 1)
			return FOUND_SRCH;
		    else
			return NOTFOUND_SRCH;
		}

		if (stcol >= ncline - 1) {              /* end of line */
		    if (ln == la_lsize (curlas) - 1)    /*   && last line */
			return NOTFOUND_SRCH;
		    if (++ln > limit)
			return NOTFOUND_SRCH;
		    getline (ln);
		}
		srchline = ln;
		srchcol = ncline - 1;
	    }
	    else {                      /* "^" */
		if (ln == la_lsize (curlas))
		    return NOTFOUND_SRCH;
		    /* repl interactive... */
		if (curpos_only) {
		    if (stcol == 0)
			return FOUND_SRCH;
		    else
			return NOTFOUND_SRCH;
		}

		if (!first_repl_line)
		    if (++ln > limit)
			return NOTFOUND_SRCH;
		first_repl_line = NO;
		getline (ln);
		srchline = ln;
		srchcol = 0;
	    }
	}
	else {
	    if (ln < 0)
		return NOTFOUND_SRCH;
		/*
		 *  'stcol' needs adjusting when past the
		 *  end of the file
		 */
	    else if (ln >= la_lsize (curlas)) {
		ln = la_lsize (curlas) - 1;
		if (!curpos_only) {
		    if (ch == '^')
			stcol = 0;
		    else
			stcol = ncline;
		}
	    }
	    getline (ln);

	    if (ch == '$') {
		if (curpos_only) {              /* repl interactive */
		    if (stcol == ncline - 1)
			return FOUND_SRCH;
		    else
			return NOTFOUND_SRCH;
		}

		if (stcol < ncline) {
		    if (ln == 0 || --ln < limit)
			return NOTFOUND_SRCH;
		    getline (ln);                   /* previous line */
		}
		srchline = ln;
		srchcol = ncline - 1;
	    }
	    else {                      /* "^" */
		if (curpos_only) {
		    if (stcol == 0)
			return FOUND_SRCH;
		    else
			return NOTFOUND_SRCH;
		}

		if (stcol == 0 && !first_repl_line) {
		    if (ln == 0 || --ln < limit)
			return NOTFOUND_SRCH;
		    getline (ln);                   /* previous line */
		}
		first_repl_line = NO;
		srchline = ln;
		srchcol = 0;
	    }
	}

	return FOUND_SRCH;
}




#ifdef COMMENT
/*
   swiped from regex.c (and modified slightly)

   routines to do regular expression matching

   Entry points:

	re_comp(s)
		char *s;
	 ... returns 0 if the string s was compiled successfully,
		     a pointer to an error message otherwise.
	     If passed 0 or a null string returns without changing
	     the currently compiled re (see note 11 below).

	re_exec(s)
		char *s;
	 ... returns 1 if the string s matches the last compiled regular
		       expression,
		     0 if the string s failed to match the last compiled
		       regular expression, and
		    -1 if the compiled regular expression was invalid
		       (indicating an internal error).

   The strings passed to both re_comp and re_exec may have trailing or
   embedded newline characters; they are terminated by nulls.

   The identity of the author of these routines is lost in antiquity;
   this is essentially the same as the re code in the original V6 ed.

   The regular expressions recognized are described below. This description
   is essentially the same as that for ed.

	A regular expression specifies a set of strings of characters.
	A member of this set of strings is said to be matched by
	the regular expression.  In the following specification for
	regular expressions the word `character' means any character but NUL.

	1.  Any character except a special character matches itself.
	    Special characters are the regular expression delimiter plus
	    \ [ . and sometimes ^ * $.
	2.  A . matches any character.
	3.  A \ followed by any character except a digit or ( )
	    matches that character.
	4.  A nonempty string s bracketed [s] (or [^s]) matches any
	    character in (or not in) s. In s, \ has no special meaning,
	    and ] may only appear as the first letter. A substring
	    a-b, with a and b in ascending ASCII order, stands for
	    the inclusive range of ASCII characters.
	5.  A regular expression of form 1-4 followed by * matches a
	    sequence of 0 or more matches of the regular expression.
	6.  A regular expression, x, of form 1-8, bracketed \(x\)
	    matches what x matches.
	7.  A \ followed by a digit n matches a copy of the string that the
	    bracketed regular expression beginning with the nth \( matched.
	8.  A regular expression of form 1-8, x, followed by a regular
	    expression of form 1-7, y matches a match for x followed by
	    a match for y, with the x match being as long as possible
	    while still permitting a y match.
	9.  A regular expression of form 1-8 preceded by ^ (or followed
	    by $), is constrained to matches that begin at the left
	    (or end at the right) end of a line.
	10. A regular expression of form 1-9 picks out the longest among
	    the leftmost matches in a line.
	11. An empty regular expression stands for a copy of the last
	    regular expression encountered.
*/
#endif

/*
 * constants for re's
 */
#define	CBRA	1
#define	CCHR	2
#define	CDOT	4
#define	CCL	6
#define	NCCL	8
#define	CDOL	10
#define	CEOF	11
#define	CKET	12
#define	CBACK	18

#define	CSTAR	01

#define	ESIZE	512
#define	NBRA	9

static  char    expbuf[ESIZE], *braslist[NBRA], *braelist[NBRA], matched[BUFSIZ];
static	char	circf;

#ifdef COMMENT
char *
re_comp(sp)
	register char	*sp;
.
    compile the regular expression argument into a dfa
    re is in sp on entry, expbuf when compiled
    returns NULL on success, pointer to error message on failure

    Added Purdue CS 10/18/82 MAB

#endif
char *
re_comp(sp)
	register char	*sp;
{
	register int	c;
	register char	*ep = expbuf;
	int	cclcnt, numbra = 0;
	char	*lastep = 0;
	char	bracket[NBRA];
	char	*bracketp = &bracket[0];
	static	char	*retoolong = "Regular expression too long";

#define	comerr(msg) {expbuf[0] = 0; numbra = 0; return(msg); }

	if (sp == 0 || *sp == '\0') {
		if (*ep == 0)
			return("No previous regular expression");
		return(0);
	}
	if (*sp == '^') {
		circf = 1;
		sp++;
	}
	else
		circf = 0;
	for (;;) {
		if (ep >= &expbuf[ESIZE])
			comerr(retoolong);
		if ((c = *sp++) == '\0') {
			if (bracketp != bracket)
				comerr("unmatched \\(");
			*ep++ = CEOF;
			*ep++ = 0;
			return(0);
		}
		if (c != '*')
			lastep = ep;
		switch (c) {

		case '.':
			*ep++ = CDOT;
			continue;

		case '*':
			if (lastep == 0 || *lastep == CBRA || *lastep == CKET)
				goto defchar;
			*lastep |= CSTAR;
			continue;

		case '$':
			if (*sp != '\0')
				goto defchar;
			*ep++ = CDOL;
			continue;

		case '[':
			*ep++ = CCL;
			*ep++ = 0;
			cclcnt = 1;
			if ((c = *sp++) == '^') {
				c = *sp++;
				ep[-2] = NCCL;
			}
			do {
				if (c == '\0')
					comerr("missing ]");
				if (c == '-' && ep [-1] != 0) {
					if ((c = *sp++) == ']') {
						*ep++ = '-';
						cclcnt++;
						break;
					}
					while (ep[-1] < c) {
						*ep = ep[-1] + 1;
						ep++;
						cclcnt++;
						if (ep >= &expbuf[ESIZE])
							comerr(retoolong);
					}
				}
				*ep++ = c;
				cclcnt++;
				if (ep >= &expbuf[ESIZE])
					comerr(retoolong);
			} while ((c = *sp++) != ']');
			lastep[1] = cclcnt;
			continue;

		case '\\':
			if ((c = *sp++) == '(') {
				if (numbra >= NBRA)
					comerr("too many \\(\\) pairs");
				*bracketp++ = numbra;
				*ep++ = CBRA;
				*ep++ = numbra++;
				continue;
			}
			if (c == ')') {
				if (bracketp <= bracket)
					comerr("unmatched \\)");
				*ep++ = CKET;
				*ep++ = *--bracketp;
				continue;
			}
			if (c >= '1' && c < ('1' + NBRA)) {
				*ep++ = CBACK;
				*ep++ = c - '1';
				continue;
			}
			*ep++ = CCHR;
			*ep++ = c;
			continue;

		defchar:
		default:
			*ep++ = CCHR;
			*ep++ = c;
		}
	}
}

#ifdef COMMENT
char *
re_exec(p1)
	register char	*p1;
.
    match the argument string against the compiled re
    the string is *p1, the re is in expbuf
    on success, the part of p1 matched goes into matched[],
    anything matching \(...\) is pointed to by braslist[], braelist[]
    returns pointer to beginning of matching part of p1 on success,
	NULL on failure

    Added Purdue CS 10/18/82 MAB

#endif
char *
re_exec(p1)
	register char	*p1;
{
	register char	*p2 = expbuf;
	register int    c;
	register char   *rv;
	register char *m;
	char *last;

	if (!*p1){
	    if ((*p2 == CEOF && circf) || (*p2 == CDOL && p2[1] == CEOF)){
		matched[0] = '\0';
		return(p1);
	    }
	    return(NULL);
	}
	for (c = 0; c < NBRA; c++) {
		braslist[c] = 0;
		braelist[c] = 0;
	}
	if (circf){
		if (re_advance(m = p1, p2, &last))
			goto gotit;
		return(NULL);
	}

	/*
	 * fast check for first character
	 */
	if (*p2 == CCHR) {
		c = p2[1];
		do {
			if (*p1 != c)
				continue;
			if (re_advance(m = p1, p2, &last))
				goto gotit;
		} while (*p1++);
		return(NULL);
	}
	/*
	 * regular algorithm
	 */
	do
		if (re_advance(m = p1, p2, &last)){
gotit:
			while(m < last)	{
				matched[m - p1] = *m;
				m++;
			}
			matched[m - p1] = '\0';
			for(c = 0; c < NBRA; c++){
				if (braslist[c])
					braslist[c] = braslist[c] - p1 + matched;
				if (braelist[c])
					braelist[c] = braelist[c] - p1 + matched;
			}
			return(p1);
		}
	while (*p1++);
	return(NULL);
}

#ifdef COMMENT
static  int
re_advance(lp, ep, last)
	register char   *lp, *ep, **last;
.
    try to match the next thing in the dfa
    returns 1 if re *ep matches (a prefix of) *lp, 0 if not;
    if 1 is returned, **last points to the end of what *ep matches

    Added Purdue CS 10/18/82 MAB

#endif
static  int
re_advance(lp, ep, last)
	register char   *lp, *ep, **last;
{
	register char   *curlp;
	int	ct, i;
	int	rv;

	for (;;)
		switch (*ep++) {

		case CCHR:
			if (*ep++ == *lp++)
				continue;
			return(0);

		case CDOT:
			if (*lp++)
				continue;
			return(0);

		case CDOL:
			if (*lp == '\0')
				continue;
			return(0);

		case CEOF:
			*last = lp;
			return(1);

		case CCL:
			if (re_cclass(ep, *lp++, 1)) {
				ep += *ep;
				continue;
			}
			return(0);

		case NCCL:
			if (re_cclass(ep, *lp++, 0)) {
				ep += *ep;
				continue;
			}
			return(0);

		case CBRA:
			braslist[*ep++] = lp;
			continue;

		case CKET:
			braelist[*ep++] = lp;
			continue;

		case CBACK:
			if (braelist[i = *ep++] == 0)
				return(-1);
			if (re_backref(i, lp)) {
				lp += braelist[i] - braslist[i];
				continue;
			}
			return(0);

		case CBACK|CSTAR:
			if (braelist[i = *ep++] == 0)
				return(-1);
			curlp = lp;
			ct = braelist[i] - braslist[i];
			while (re_backref(i, lp))
				lp += ct;
			while (lp >= curlp) {
				if (rv = re_advance(lp, ep, last))
					return(rv);
				lp -= ct;
			}
			continue;

		case CDOT|CSTAR:
			curlp = lp;
			while (*lp++)
				;
			goto star;

		case CCHR|CSTAR:
			curlp = lp;
			while (*lp++ == *ep)
				;
			ep++;
			goto star;

		case CCL|CSTAR:
		case NCCL|CSTAR:
			curlp = lp;
			while (re_cclass(ep, *lp++, ep[-1] == (CCL|CSTAR)))
				;
			ep += *ep;
			goto star;

		star:
			do {
				lp--;
				if (rv = re_advance(lp, ep, last))
					return(rv);
			} while (lp > curlp);
			return(0);

		default:
			return(-1);
		}
}

#ifdef COMMENT
re_backref(i, lp)
	register int	i;
	register char	*lp;
.
    see if pattern braslist[i] matches
    (a prefix of) *lp;
    return 1 if yes, 0 if no

    Added Purdue CS 10/18/82 MAB

#endif
re_backref(i, lp)
	register int	i;
	register char	*lp;
{
	register char	*bp;

	bp = braslist[i];
	while (*bp++ == *lp++)
		if (bp >= braelist[i])
			return(1);
	return(0);
}

#ifdef COMMENT
int
re_cclass(set, c, af)
	register char	*set, c;
	int	af;
.
    if char c is a member of set *set,
    return af; otherwise, return !af

    Added Purdue CS 10/18/82 MAB

#endif
int
re_cclass(set, c, af)
	register char	*set, c;
	int	af;
{
	register int	n;

	if (c == 0)
		return(0);
	n = *set++;
	while (--n)
		if (*set++ == c)
			return(af);
	return(! af);
}

#ifdef COMMENT
char *
re_replace(lp)
	register char	*lp;
.
    Instantiate the replacement re
    Note that \1 and & are patterns and must be replaced
    by quantities from the matched string

    Added Purdue CS 10/18/82 MAB

#endif
char *
re_replace(lp)
	register char	*lp;
{
	char buf[BUFSIZ];
	register char *b, *m;

	for(b = buf; (*lp)&0377; lp++){
		if (*lp=='&') {
			m = matched;
			while (*m)
				*b++ = *m++;
			continue;
		} else if (*lp == '\\'){
			if ((*++lp &= 0177) >='1' && *lp < NBRA+'1' && braslist[*lp - '1']) {
				m = braslist[*lp-'1'];
				while(m < braelist[*lp-'1'])
					*b++ = *m++;
				continue;
			}
			else
				*b++ = *lp;
		}
		else
			*b++ = (*lp)&0177;
	}
	*b = '\0';
	return(append(buf, ""));
}

#ifdef COMMENT
int
re_len ()
.
    Determine the length of the string matching the re

    Added Purdue CS 10/18/82 MAB

#endif
int
re_len()
{
    register char *m;
    register int len;

    if (re_zerolen1)
	len = 0;
    else for(m = matched, len = 0; *m; m++, len++)
	if ((*m == ESCCHAR && (m[1] == 'J' || m[1] == 'j') || *m =='\n') &&
		m[2] == '\0')
	    break;

    return(len);
}
