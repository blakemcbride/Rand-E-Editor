#ifdef NOTYET

#ifdef COMMENT
--------
file e.dif.c
    diff command
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

#define FOUND_SRCH    0
#define NOTFOUND_SRCH 1
#define ABORTED_SRCH  2
#define OUTRANGE_SRCH 4

Nlines srchline;
Ncols srchcol;

extern Nlines rangelimit ();
extern void showrange ();
extern void aborted ();

#ifdef COMMENT
Cmdret
diff (delta)
    Small delta;
.
    Do the "diff" command.
    'Delta' is either 1 for forward diff or -1 for backward diff.
#endif
Cmdret
diff (delta)
Small delta;
{
    static S_looktbl diftable[] = {
	0
    };
    register char *cp;
    char *s1;
    char *s2;
    char *fcp;
    Char delim;
    int nl;
    int tmp;
    Ncols s1len;
    Ncols s2len;
    int rplopts ();

    if (*cmdopstr != '\0')
	return CRTOOMANYARGS;
    if (curwin->altwksp->wfile == NULLFILE) {
	mesg (ERRALL + 1, "No alternate file");
	return CROK;
    }
    curwin->winflgs |= TRACKSET;
    dodiff (curwksp->wlin + cursorline + delta,
	    delta > 0 ? la_lsize (curlas) - 1 : 0,
	    delta);
    return CROK;
}

#ifdef COMMENT
Small
dodiff (ln, limit, delta)
    Nlines  ln;
    Nlines  limit;
    Small   delta;
.
    'dodiff'
    searches through curfile for a difference starting at ln.
    Stop short of 'limit' line.
    'Delta' is either 1 for forward search or -1 for backward search.
    If difference is not on current page, positions
    window with difference on standard line.  Leaves cursor under difference
#endif
Small
dodiff (ln, limit, delta)
Nlines  ln;
Nlines  limit;
Small   delta;
{
    Small   srchret;
    Flag    newputup;
    Nlines  winlin;
    Ncols   wincol;
    Nlines  lin;
    Ncols   col;
    Ncols   lkey;

    setbul (NO);
    mesg (TELALL + 2, delta > 0 ? "+" : "-", "DIFF");
    d_put (0);

    switch (srchret = diffsearch (ln, limit, delta)) {
    case FOUND_SRCH:
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
	break;

    case NOTFOUND_SRCH:
    case ABORTED_SRCH:
    case OUTRANGE_SRCH:
	clrbul ();
	if (srchret == NOTFOUND_SRCH)
	    mesg (ERRALL + 1, "Difference not found.");
	else if (srchret == OUTRANGE_SRCH)
	    mesg (ERRALL + 1, "Difference not found. (range is set)");
	else
	    aborted (srchline);
	loopflags.flash = NO;
	break;
    }
    return srchret;
}

#ifdef COMMENT
Small
diffsearch (ln, limit, delta)
    Nlines  ln;
    Nlines  limit;
    Small   delta;
.
    /*
    Searches through curfile and altfile for difference starting at ln.
    'Delta' is either 1 for forward search or -1 for backward search.
    Don't go beyond 'limit' line.
    ln must be >= 0
    Assumes curwksp->wfile == curfile.
    */
#endif
Small
diffsearch (ln, limit, delta)
Nlines  ln;
Nlines  limit;
Small   delta;
{
    register char *i, *j;
    Flag    retval;
    char    hardline[100], *altline, *alline;
    int     lenalloc;

    lenalloc = 0;
    retval = NOTFOUND_SRCH;
    intrupnum = 0;

    if ((limit = rangelimit (ln, delta, limit)) == -1) {
	retval = OUTRANGE_SRCH;
	goto ret;
    }

    if (ln >= la_lsize (curlas)) {
	if (delta > 0)
	    goto ret;
	ln = la_lsize (curlas);
    }
    getline (ln);
    if (ncline > 100) {
	altline = salloc (ncline,YES);
	lenalloc = ncline;
    } else {
	altline = hardline;
	lenalloc = 0;
    }
    mv (cline, altline, ncline);
    exchgwksp (YES);
    if (ln >= la_lsize (curlas)) {
	if (delta > 0) {
	    exchgwksp (YES);
	    goto ret;
	}
	ln = la_lsize (curlas);
    }
    getline (ln);
    exchgwksp (YES);
    for (;;) {
	i = altline;
	j = cline;
	do {
	    if (*i != *j) goto found;
	    j++;
	} while (*i++ != '\n');
	ln += delta;
	if (   (delta < 0 && ln < limit)
	    || (delta > 0 && ln > limit)
	    || (   intrupnum++ > SRCHPOL
		&& (retval = sintrup () ? ABORTED_SRCH : NOTFOUND_SRCH)
		   == ABORTED_SRCH
	       )
	   )
	    goto ret;
	if (ln >= la_lsize (curlas)) {
	    goto ret;
	}
	getline (ln);
	if (ncline > 100) {
	    if (lenalloc < ncline) {
		if (lenalloc = 0)
		    sfree (alline);
		altline = alline = salloc (ncline,YES);
		lenalloc = ncline;
	    }
	} else
	    altline = hardline;
	mv (cline, altline, ncline);
	exchgwksp (YES);
	if (ln >= la_lsize (curlas)) {
	    if (delta > 0) {
		exchgwksp (YES);
		goto ret;
	    }
	}
	getline (ln);
	exchgwksp (YES);
    }
found:
    retval = FOUND_SRCH;
ret:
    if (lenalloc)
	sfree (alline);
    srchline = ln;
    srchcol = i - altline;
    return retval;
}

#endif
