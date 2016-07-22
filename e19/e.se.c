#ifdef COMMENT
--------
file e.se.c
    search and replace
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

extern Flag save_or_set_impl_tick (Flag test_flg);
extern void reset_searchkey_ref ();

Flag rplinteractive, rplshow;
static char *rpls1;
static char *rpls2;
static Ncols rpls1len;
static Ncols rpls2len;
Flag starthere;

Nlines srchline;
Ncols srchcol;

       void doreplace ();
       void replkey ();
       void rplit ();
       void dosearch ();
extern void aborted ();
extern char *re_comp (), *re_exec(), *re_replace();
extern int DebugVal;
Flag first_repl_line;

static Small dsplsearch ();

#ifdef COMMENT
int
zaprpls ()
.
    Clear the strings rpl1 and rpl2

    Added Purdue CS 10/18/82 MAB

#endif
int
zaprpls ()
{
    int retval;

    retval = 0;
    if (rpls1){
	sfree(rpls1);
	rpls1 = (char *) 0;
	rpls1len = 0;
	retval++;
    }
    if (rpls2){
	sfree(rpls2);
	rpls2 = (char *) 0;
	rpls2len = 0;
	retval++;
    }
    return retval;
}

#ifdef COMMENT
Cmdret
replace (delta)
    Small delta;
.
    Do the "replace" command.
    'Delta' is either 1 for forward replace or -1 for backward replace.
#endif
Cmdret
replace (delta)
Small delta;
{
    static S_looktbl rpltable[] = {
	"interactive" , 0,
	"show"        , 0,
	0
    };
    register char *cp;
    char *s1;
    char *s2;
    char *fcp;
    Char delim;
    int nl;
    char b[BUFSIZ], *tmps, *tmpb;       /* Added 10/18/82 MAB */
    int tmp;
    Ncols s1len;
    Ncols s2len;
    Flag moved;
    int rplopts ();

    if (*(fcp = cmdopstr) == '\0')
	return CRNEEDARG;
    moved = NO;
    nl = 1;
    rplshow = NO;
    rplinteractive = NO;
    tmp = scanopts (&fcp, NO, rpltable, rplopts);
/*       3 marked area        \
 *       2 rectangle           \
 *       1 number of lines      > may have stopped on an unknown option
 *       0 no area spec        /
 *      -2 ambiguous option     ) see parmlines, parmcols for type of area
 *   <= -3 other error
 **/
    if (tmp <= -2)
	return tmp;
    if (*(cp = fcp) == '\0') {
	mesg (ERRALL + 1, "no search string");
	return CROK;
    }
    switch (tmp) {
    case 0:
	if (delta > 0)
	    nl = max (0, la_lsize (curlas)
			 - (curwksp->wlin + cursorline)) + 1;
	else
	    nl = curwksp->wlin + cursorline + 1;
	break;

    case 1:
	nl = parmlines;
	break;

    case 2:
	return NORECTERR;
	break;

    case 3:
	if (markcols)
	    return NORECTERR;
	nl = marklines;
	break;
    }

#ifdef UNIXV6
#define ispunct(c) (isprint(c) && !isalpha(c) && !isdigit(c))
#endif
    if (!ispunct (*cp)) {
	mesg (ERRALL + 1, "invalid string delimiter");
	return CROK;
    }
    delim = *cp++;
    s1 = cp;
    for (; *cp && *cp != delim; cp++)
	continue;
    if (*cp == '\0') {
    /*  mesg (ERRALL + 1, "unterminated search string");  */
	mesg (ERRALL + 4, "unterminated search string: ", "\"", s1, "\"");
	return CROK;
    }
    if (cp++ - s1 == 0) {
	mesg (ERRALL + 1, "null search string");
	return CROK;
    }
					/* Begin adding 10/18/82 MAB */
    for (tmps = s1, tmpb = b; tmps < cp; tmpb++, tmps++)
	    if (*tmps == ESCCHAR && (tmps[1]|0140) == 'j' && patmode){
		mesg (ERRALL + 1, "use ^ or $ rather than ^J");
		return CROK;
	    }
	    else
		*tmpb = *tmps;
    if (tmpb[-1] == '\\')
	tmpb[-1] = '$';
    *tmpb = '\0';
#if 0
|    if (b[0] == '^' && b[1] == '\0'){
|        b[1] = '.';
|        b[2] = '\0';
|    }
|    else if (b[0] == '$' && b[1] == '\0'){
|        b[0] = '.';
|        b[1] = '$';
|        b[2] = '\0';
|    }
#endif
    if (patmode && (s2 = re_comp(b)) != (char *) 0){
	mesg (ERRALL + 3, b, ": ", s2);
	return CROK;
    }
					/* End adding 10/18/82 MAB */
    s2 = cp;
    for (; *cp && *cp != delim; cp++)
	continue;
    if (*cp == '\0') {
	mesg (ERRALL + 1, "unterminated replacement string");
	return CROK;
    }
    fcp = cp++;
    for (; *cp && *cp == ' '; cp++)
	continue;
    if (*cp != '\0') {
	mesg (ERRALL + 1, "extraneous stuff after replacement string");
	return CROK;
    }
    s2[-1] = '\0';
    if ((s1len = skeylen (s1, YES, NO, NO)) == -1) {
	mesg (ERRALL + 1, "imbedded newline in search string");
	if (moved)
	    putupwin ();
	goto r2;
    }
    /* By the way, it's OK if s1len == 0 at this point */
    *fcp = '\0';
    if ((s2len = skeylen (s2, NO, NO, NO)) == -1) {
	mesg (ERRALL + 1, "newline in replacement string");
	if (moved)
	    putupwin ();
	goto r1;
    }

    if (rpls1) {
	sfree (rpls1);
	sfree (rpls2);
    }
    rpls1 = append (s1, "");
    rpls2 = append (s2, "");
    rpls1len = s1len;
    rpls2len = s2len;

    if (curmark) {
	moved = gtumark (YES);
	(void) unmark ();
    }
    if (moved && rplshow || rplinteractive)
	putupwin ();
    if (rplinteractive) {
	if (searchkey)
	    sfree (searchkey);
	reset_searchkey_ref ();
	searchkey = append (rpls1, "");
	starthere = YES;
	dosearch (delta);
	starthere = NO;
    }
    else {
	doreplace (nl, delta, !moved || rplshow);
	if (moved && !rplshow)
	    putupwin ();
    }
r1: *fcp = delim;
r2: s2[-1] = delim;

    return CROK;
}

#ifdef COMMENT
rplopts (cp, str, tmp, equals)
    char *cp;
    char **str;
    Small tmp;
    Flag equals;
.
    Get the options to the "replace" command.
    For now the only options are "interactive" and "show".
#endif
rplopts (cp, str, tmp, equals)
register char *cp;
char **str;
Small tmp;
Flag equals;
{
    if (equals)
	return CRBADARG;
    switch (tmp) {
    default:
	return -1;

    case 0:
	rplinteractive = YES;
	break;

    case 1:
	rplshow = YES;
	break;
    }
    for (; *cp && *cp == ' '; cp++)
	continue;
    *str = cp;
    return 1;
}

#ifdef COMMENT
void
doreplace (nl, delta, puflg)
    Nlines nl;
    Small delta;
    Flag puflg;
.
    Actually do the replace at the current position.
    'Delta' is either 1 for forward replace or -1 for backward replace.
    Any newlines in s1 must be at the beginning and/or end of the string
    s1len is exclusive of newlines
    s2 must not have any newlines in it.
#endif
void
doreplace (nl, delta, puflg)
Nlines nl;
Small delta;
Flag puflg;
{
    Nlines limit;
    Small srchret;
    Slines svline;
    Scols svcol;

    srchline = curwksp->wlin + (svline = cursorline);
    srchcol = curwksp->wcol + (svcol = cursorcol);

    if (delta > 0) {
	limit = min (la_lsize (curlas) - 1, srchline + nl - 1);
	starthere = YES;
    }
    else
	limit = max (0, srchline - (nl - 1));

    if ((limit = rangelimit (srchline, delta, limit)) == -1)
	goto ret;

    if (patmode)
       first_repl_line = YES;

    for (;;) {
	if (delta > 0) {
	    if (srchline > limit)
		break;
	}
	else {
	    if (srchline < limit)
		break;
	}
	if (sintrup ())
	    goto abort;
	if (rplshow) {
	    if ((srchret = dsplsearch (rpls1, srchline, srchcol,
				       limit, delta, NO, YES))
		!= FOUND_SRCH
	       )
		break;
	    svline = cursorline;
	    svcol  = cursorcol;
	    dobullets (NO, NO);
	    setbul (YES);
	}
	else {
	    if (patmode)                /* Added 10/18/82 MAB */
		srchret = patsearch (rpls1, srchline, srchcol, limit,
					  delta, YES);
	    else
		srchret = strsearch (rpls1, srchline, srchcol, limit,
					  delta, YES);
	    if (srchret == NOTFOUND_SRCH)
		break;
	    else if (srchret == OUTRANGE_SRCH)
		break;
	    else if (srchret == ABORTED_SRCH) {
 abort:         aborted (srchline);
		loopflags.flash = NO;
		break;
	    }
	}
	rpls1len = patmode ? re_len() : skeylen (rpls1, YES, NO, NO);  /* Added 10/18/82 MAB */
	rplit (srchline, srchcol, puflg);
	if (delta > 0)
	    srchcol += rpls2len + (rpls1len == 0);
    }
    poscursor (svcol, svline);
 ret:
    starthere = NO;
    if (patmode)
       first_repl_line = NO;

    return;
}

#ifdef COMMENT
void
replkey ()
.
    Do the REPLACE key.
#endif
void
replkey ()
{
    Nlines slin;
    Ncols  scol;
    Small   srchret;

    if (rpls1 == 0 || *rpls1 == 0) {
	mesg (ERRALL + 1, "no replacement to do");
	return;
    }
    slin = curwksp->wlin + cursorline;
    scol = curwksp->wcol + cursorcol;
    starthere = YES;
				/* Changed 10/18/82 MAB */
    if (patmode)
	srchret = patsearch(rpls1, slin, scol, slin, 1, NO);
    else
	srchret = strsearch (rpls1, slin, scol, slin, 1, NO);
    switch (srchret) {
    case FOUND_SRCH:
	Block {
	    Scols svcol;
	    rpls1len = patmode ? re_len() : skeylen (rpls1, YES, NO, NO);  /* Added 10/18/82 MAB */
	    svcol = cursorcol;
	    rplit (slin, scol, YES);
	    poscursor (svcol, cursorline);
	}
	break;
    case NOTFOUND_SRCH:
    case ABORTED_SRCH:
	mesg (ERRALL + 1, "cursor is not on the string to be replaced");
	break;
    case OUTRANGE_SRCH:
	mesg (ERRALL + 1, "out of range");
	break;
    }
    starthere = NO;
    return;
}

#ifdef COMMENT
void
rplit (slin, scol, puflg)
    Nlines slin;
    Ncols scol;
    Flag puflg;
.
    Replace it.
    That is, do the actual change at 'slin', 'scol',
#endif
void
rplit (slin, scol, puflg)
Nlines slin;
Ncols scol;
Flag puflg;
{
    Ncols wdelta;
    Nlines lin;
    char *tmprpls2;                             /* Added 10/18/82 MAB */

    /*
     * this is got added 10/18/82 by MAB
     * the idea here is to pretend that the pattern
     * match was a real honest-to-goodness string match
     * so we switch rpls2 and evaluate rpls2len here
     * (since rpls2 may be a pattern itself)
     * we have to compute the length here
     */
    if (patmode){
	tmprpls2 = rpls2;                       /* save replacement pattern */
	rpls2 = re_replace(tmprpls2);           /* instantiate it */
	rpls2len = strlen(rpls2);               /* get the length */
    }

    getline (slin); /* we DO need this */
    /* adjust cline to accept the replacement string */
    if ((wdelta = rpls2len - rpls1len) > 0)
	putbks (scol, wdelta);
    else if (wdelta < 0) {
	if ((ncline += wdelta) - scol > 0)
	    move (&cline[scol - wdelta], &cline[scol],
		    (Uint) (ncline - scol));
    }
    if (rpls2len > 0)
	move (rpls2, &cline[scol], (Uint) rpls2len);
    fcline = YES;
    if (   puflg
	&& (lin = slin - curwksp->wlin) >= 0
	&& lin <= curwin->btext
       )
	putup (-1, lin, (Scols) max (0, scol - curwksp->wcol),
	       (Scols) (wdelta == 0 ? rpls2len : MAXWIDTH));
    redisplay (curfile, slin, 1, 0, NO);
    if (patmode){
	sfree(rpls2);                               /* Added 10/18/82 MAB */
	rpls2 = tmprpls2;                           /* Added 10/18/82 MAB */
    }
    return;
}

#ifdef COMMENT
void
dosearch (delta)
    Small delta;
.
    Search for the next occurrence of searchkey.
    'Delta' is either 1 for forward search or -1 for backward search.
#endif
void
dosearch (delta)
Small delta;
{
    (void) save_or_set_impl_tick (NO);
    (void) dsplsearch (searchkey,
		       curwksp->wlin + cursorline,
		       curwksp->wcol + cursorcol,
		       delta > 0 ? la_lsize (curlas) - 1 : 0,
		       delta, YES, YES);
    (void) save_or_set_impl_tick (YES);
    return;
}

#ifdef COMMENT
static Small
dsplsearch (str, ln, stcol, limit, delta, delay, puflg)
    char   *str;
    Nlines  ln;
    Ncols   stcol;
    Nlines  limit;
    Small   delta;
    Flag    delay;
    Flag    puflg;
.
    'Display-search'
    searches through curfile for str starting at [ln, stcol].
    Stop short of 'limit' line.
    'Delta' is either 1 for forward search or -1 for backward search.
    If key is not on current page, positions
    window with key on top line.  Leaves cursor under key.
    If 'delay' is non-0 put up a bullet for one second.
#endif
static Small
dsplsearch (str, ln, stcol, limit, delta, delay, puflg)
char   *str;
Nlines  ln;
Ncols   stcol;
Nlines  limit;
Small   delta;
Flag    delay;
Flag    puflg;
{
    Small   srchret;
    Flag    newputup;
    Nlines  winlin;
    Ncols   wincol;
    Nlines  lin;
    Ncols   col;
    Ncols   lkey;
    char *tmp;                  /* Added 10/18/82 MAB */

    if (str == 0 || *str == 0) {
	mesg (ERRALL + 1, "Nothing to search for.");
	return NOTFOUND_SRCH;
    }

    if (puflg) {
	setbul (NO);
	mesg (TELALL + 3, delta > 0 ? "+" : "-", "SEARCH: ", str);
	d_put (0);
    }

    if (patmode)
	srchret =  patsearch (str, ln, stcol, limit, delta, YES);
    else
	srchret = strsearch (str, ln, stcol, limit, delta, YES);

    switch (srchret) {

    case FOUND_SRCH:
	if (puflg) {
	    winlin = curwksp->wlin;
	    wincol = curwksp->wcol;

	    newputup = NO;
	    lin = srchline - winlin;
	    if (lin < 0 || lin > curwin->btext) {
		newputup = YES;
		if (curwin->btext > 1)
		    lin = defplline;
		else
		    lin = 0;
		if ((winlin = srchline - lin) < 0) {
		    lin += winlin;
		    winlin = 0;
		}
	    }

       /*   if ((lkey = skeylen (str, YES, YES, YES)) == 0)     */
	    lkey = patmode ? re_len() : skeylen (str, YES, YES, YES);
	    if (lkey == 0)
		lkey = 1;
	    col = srchcol;
	    if (   col < wincol
		|| col > wincol + curwin->rtext + 1 - lkey
	       ) {
		newputup = YES;
		if (col < curwin->rtext + 1 - lkey)
		    wincol = 0;
		else {
		    wincol = col - (curwin->rtext + 1 - lkey);
		    col = curwin->rtext - lkey + 1;
		}
	    }
	    else
		col -= wincol;
	    if (newputup)
		movewin (winlin, wincol, (Slines) lin, (Scols) col, YES);
	    else {
		clrbul ();
		poscursor ((Scols) col, lin);
	    }
	    if (delay)
		setbul (YES);
	}
	break;

    case NOTFOUND_SRCH:
    case ABORTED_SRCH:
    case OUTRANGE_SRCH:
	if (puflg) {
reterr:
	    clrbul ();
	    if (srchret == NOTFOUND_SRCH)
		mesg (ERRALL + 1, "Search key not found.");
	    else if (srchret == OUTRANGE_SRCH)
		mesg (ERRALL + 1, "Search key not found. (out of range)");
	    else
		aborted (srchline);
	    loopflags.flash = NO;
	}
	break;
    }
    return srchret;
}

#ifdef COMMENT
Small
strsearch (str, ln, stcol, limit, delta, srch)
    char   *str;
    Nlines  ln;
    Ncols   stcol;
    Nlines  limit;
    Small   delta;
    Flag srch;      /* if NO, then only look at current position */
.
    /*
    Searches through curfile for str starting at [ln, stcol].
    If 'srch' == 0, merely check to see if current position matches str;
    don't look further.
    'Delta' is either 1 for forward search or -1 for backward search.
    Don't go beyond 'limit' line.
    Assumes curwksp->wfile == curfile.
    */
#endif
Small
strsearch (str, ln, stcol, limit, delta, srch)
char   *str;
Nlines  ln;
Ncols   stcol;
Nlines  limit;
Small   delta;
Flag srch;      /* if NO, then only look at current position */
{
    register char  *at;
    register char  *sk;
    register char  *fk;
    char   *atcol;
    Nlines  continln;
    Ncols   lkey;
    Flag    nextline;
    Flag    mustmatch;
    Flag    retval;

    retval = NOTFOUND_SRCH;
    intrupnum = 0;
    lkey = skeylen (str, YES, YES, YES);

    if ((limit = rangelimit (ln, delta, limit)) == -1) {
	retval = OUTRANGE_SRCH;
	goto ret;
    }

    if (ln >= la_lsize (curlas)) {
	if (delta > 0)
	    goto ret;
	ln = la_lsize (curlas);
	at = cline;
	getline (ln);
    }
    else {
	getline (ln);
	at = &cline[min (ncline - lkey, stcol)];
    }
    if (starthere)
	at -= delta;

    nextline = NO;
    mustmatch = NO;
    sk = str;
    for (;;) {
	/* get a line that is long enough that it could contain str */
#ifdef lint
	continln = 0;
#endif
	for ( at += delta
	    ; nextline || at < cline || at >= &cline[ncline - lkey]
	    ;) {
	    if (!srch)
		goto ret;
	    if (nextline)
		continln++;
	    else
		continln = ln += delta;
	    if (   (delta < 0 && continln < limit)
		|| (delta > 0 && continln > limit && !nextline)
		|| (   intrupnum++ > SRCHPOL
		    && (retval = sintrup () ? ABORTED_SRCH : NOTFOUND_SRCH)
		       == ABORTED_SRCH
		   )
	       )
		goto ret;
	    getline (continln);
	    at = cline;
	    if (nextline)
		break;
	    if (delta < 0)
		at += ncline - 1 - lkey;
	    mustmatch = NO;
	    sk = str;
	}
	fk = at;
	do {
	    if (    sk[0] == ESCCHAR
		&& (sk[1] | 0140) == 'j'
	       ) {
		if (sk == str && sk[2]) {
		    if (fk == cline) {
			sk += 2;
			mustmatch = NO;
			continue;
		    }
		    else
			break;
		}
		else if (*fk == '\n') {
		    sk += 2;
		    if (*sk == 0)
			break;
		    else {
			mustmatch = YES;
			if ( !nextline ) {
			    atcol = at;
			    nextline = YES;
			}
			    goto contin_search;
		    }
		}
	    }
	    else if (*sk == *fk) {
		sk++;
		fk++;
		mustmatch = NO;
		continue;
	    }
	    if (mustmatch) {
		if (delta > 0)
		    at = cline + ncline - lkey;
		else
		    at = cline;
	    }
	    break;
	} while (*sk);
	if (nextline) {
	    nextline = NO;
	    at = atcol;
	}

	if (*sk == 0) {  /* found it */
	    retval = FOUND_SRCH;
	    break;
	}
	mustmatch = NO;
	sk = str;
 contin_search:
	if (!srch)
	    break;
    }
 ret:
    srchline = ln;
    srchcol = at - cline;
    return retval;
}

#ifdef COMMENT
Ncols
skeylen (str, nlok, imbed, stopatnl)
    char *str;
    Flag nlok;      /* ok for newlines in string */
    Flag imbed;     /* imbedded newlines are OK */
    Flag stopatnl;  /* stop counting at first newline after other text */
.
    get length of non-newline characters in str
#endif
Ncols
skeylen (str, nlok, imbed, stopatnl)
char *str;
Flag nlok;      /* ok for newlines in string */
Flag imbed;     /* imbedded newlines are OK */
Flag stopatnl;  /* stop counting at first newline after other text */
{
    register char *cp;
    register Ncols lkey;

    /* get length of searchkey */
    /* if there are imbedded newlines, length is first part */

    /* skip over initial newlines */
    for (cp = str; ; cp += 2)
	if (    cp[0] == ESCCHAR
	    && (   cp[1] == 'j'
		|| tolower (cp[1]) == 'j'
	       )
	   ) {
	    if (!nlok)
		return -1;
	}
	else
	    break;

    /* stop at imbedded newline */
    for (lkey = Z; *cp; cp++, lkey++)
	if (   cp[0] == ESCCHAR
	    && (   cp[1] == 'j'
		|| tolower (cp[1]) == 'j'
	       )
	   ) {
	    if (!nlok)
		return -1;
	    if (!imbed) {
		if (cp[2] != '\0')
		    return -1;
		else
		    return lkey;
	    }
	    if (stopatnl)
		break;
	    cp++;
	    lkey--;
	}
    return lkey;
}


#ifdef COMMENT
void
aborted ()
.
    Tell the user where he aborted and sleep 1.
#endif
void
aborted (line)
Nlines line;
{
    char lstr[16];
#ifdef LA_LONGFILES
    sprintf (lstr, "%D", srchline + 1);
#else
    sprintf (lstr, "%d", srchline + 1);
#endif
    mesg (ERRALL + 2, "Search aborted at line ", lstr);
    d_put (0);
    sleep (1);
    return;
}

