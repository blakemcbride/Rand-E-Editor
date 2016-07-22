#ifdef COMMENT
--------
file e.tb.c
    Tab handling routines
#endif

#ifdef COMMENT
	Copyright abandoned, 1983, The Rand Corporation
#endif

#include "e.h"
#include "e.m.h"
#include "e.cm.h"

ANcols *ptabs;          /* new tabstops requested (each is +1 from actual) */
Short   pntabs;         /* num of new tabstops requested */
Short   pstabs;         /* num of new tabstops alloced */

extern void alcptabs ();
extern void tabevery ();
       void stclptabs ();
extern void sctab ();

#ifdef COMMENT
Cmdret
dotab (setclr)
    Flag setclr;
.
    Do the "tab" command.
#endif
Cmdret
dotab (setclr)
Flag setclr;
{
    if ((pntabs = getptabs ()) == -1)
	return CRBADARG;
    if (pntabs == 0) {
	ptabs[0] = curwksp->wcol + cursorcol + 1;
	pntabs = 1;
    }

    stclptabs (setclr);
    if (ptabs)
	sfree ((char *) ptabs);
    return CROK;
}

#ifdef COMMENT
Cmdret
dotabs (setclr)
    Flag setclr;
.
    Do the "tabs" command.
#endif
Cmdret
dotabs (setclr)
Flag setclr;
{
    Cmdret retval;

    if ((pntabs = getptabs ()) == -1)
	return CRBADARG;
    if (pntabs == 0 && setclr)
	Goret (CRNEEDARG);
    if (pntabs > 1)
	Goret (CRTOOMANYARGS);

    if (curmark) {
	if (marklines > 1 || markcols == 0) {
	    mesg (ERRALL + 1,
		  "\"tabs\" only works with a marked rectangle 1x...");
	    Goret (CROK);
	}
	if (gtumark (YES))
	    putupwin ();
	tabevery (pntabs == 0? (Ncols) 1: ptabs[0], curwksp->wcol + cursorcol,
		      curwksp->wcol + cursorcol + markcols, setclr);
	unmark ();
    }
    else {
	if (pntabs == 0) {
#ifdef LMCMARG
	    int i;
	    for (i = Z; i < ntabs; i++)
		scvtab (tabs[i], YES);
#endif
	    ntabs = 0;
	} else
	    tabevery (ptabs[0], (Ncols) 0, (NTABS / 2 - 1) * ptabs[0],
		      setclr);
    }
    retval = CROK;
ret:
    if (ptabs)
	sfree ((char *) ptabs);
    return retval;
}

#ifdef COMMENT
Small
getptabs ()
.
    Get the arguments to a "tab" or "tabs" command.
    Set the global 'pntabs' to the number of tab arguments collected
    and return that number or -1 if syntax error.
#endif
Small
getptabs ()
{
    int tmpi;
    char *cp;
    char *cp1;

    ptabs = 0;
    pntabs = 0;
    cp = cmdopstr;
    alcptabs ();

    for (;;) {
	for (; *cp && *cp == ' '; cp++)
	    continue;
	cp1 = cp;
	cp = s2i (cp, &tmpi);
	if (cp == cp1 || tmpi <= 0)
	    break;
	if (pntabs >= pstabs) {
	    ptabs = (ANcols *)
		    gsalloc ((char *) ptabs,
			     pntabs * sizeof *ptabs,
			     (pstabs = ((3 * pntabs) / 2)) * sizeof *ptabs,
			     YES);
	}
	ptabs[pntabs++] = tmpi;
    }
    if (*cp != '\0') {
	sfree ((char *) ptabs);
	return -1;
    }
    return pntabs;
}

#ifdef COMMENT
Cmdret
tabfile (setclr)
    Flag setclr;
.
    Do the "tabfile" command.
#endif
Cmdret
tabfile (setclr)
Flag setclr;
{
    if (*opstr == '\0')
	return CRNEEDARG;
    if (*nxtop != '\0')
	return CRTOOMANYARGS;

    gettabs (opstr, setclr);
    return CROK;
}

#define TNDIG 6
#ifdef COMMENT
Flag
gettabs (filenam, setclr)
    char   *filenam;
    Flag setclr;
.
    Sets or clears tabs as specified in a file.  The spec in the
    file is just a list of decimal numbers separated by spaces.
    All existing tabs are left set, and new tabstops are set according
    to the tabfile.
    Called only by tabfile.
#endif
Flag
gettabs (filenam, setclr)
char   *filenam;
Flag setclr;
{
    /* XXXXXXXXXXXXXXXXXXXXXXXXX */
    extern char * xdir_dir;
    int retval = YES;
    register char  *cp;
    register Short  gc;
    char    ts[TNDIG],
            nambuf[128];
    Short   i;
    FILE  *iob;

    if (   (iob = fopen (filenam, "r")) == NULL
	&& (   *filenam == '/'
	    || (   (copy (filenam, copy ("/etc/",copy (getmypath (),nambuf))),
		    (iob = fopen (nambuf, "r"))) == NULL
		|| (copy (filenam, copy (xdir_dir, nambuf)),
		    (iob = fopen (nambuf, "r"))) == NULL
	       )
	   )
       ) {
	mesg (ERRALL + 1, "Can't open file");
	return NO;
    }
    alcptabs ();
    for (gc = getc (iob); ; gc = getc (iob)) {  /* once for each tab stop */
	if (gc == EOF)
	    break;
	for (cp = ts; ;gc = getc (iob)) {
	    if (gc >= '0' && gc <= '9') {
		if (cp > &ts[TNDIG-1])
		    goto bad;
		*cp++ = gc;
	    }
	    else
		break;
	}
	if (gc == ' ' || gc == '\n' || gc == EOF) {
	    if (cp == ts) {
		if (gc == EOF)
		    break;
		continue;
	    }
	}
	else {
 bad:       mesg (ERRALL + 1, "Bad tabstop file format");
	    Goret (NO);
	}
	*cp = 0;
	s2i (ts, &i);
	if (i <= 0)
	    goto bad;
	if (pntabs >= pstabs) {
	    ptabs = (ANcols *)
		    gsalloc ((char *) ptabs,
			     pntabs * sizeof *ptabs,
			     (pstabs = ((3 * pntabs) / 2)) * sizeof *ptabs,
			     YES);
	}
	ptabs[pntabs++] = i;
    }
    stclptabs (setclr);

 ret:
    if (ptabs)
	sfree ((char *) ptabs);
    fclose (iob);
    return retval;
}

#ifdef COMMENT
void
alcptabs ()
.
    Allocate the ptabs array and initialize pntabs and pstabs.
#endif
void
alcptabs ()
{
    ptabs = (ANcols *) salloc (NTABS * sizeof *ptabs, YES);
    pntabs = 0;
    pstabs = NTABS;
    return;
}

#ifdef COMMENT
void
tabevery (interval, stcol, endcol, setclr)
    register Ncols interval;
    Ncols stcol;
    register Ncols endcol;
    Flag setclr;
.
    Set/clear tabs every 'interval' columns from 'stcol' through 'endcol' - 1.
    If 'setclr' is non-0 then set else clear.
#endif
void
tabevery (interval, stcol, endcol, setclr)
register Ncols interval;
Ncols stcol;
register Ncols endcol;
Flag setclr;
{
    register Ncols col;

    for (col = stcol; col <= endcol; col += interval)
	sctab (col,setclr);
    return;
}

#ifdef COMMENT
void
stclptabs (setclr)
    Flag setclr;
.
    Use the global 'ptabs' array as a template to set/clear real tabs.
    If 'setclr' is non-0 then set else clear.
#endif
void
stclptabs (setclr)
Flag setclr;
{
    register Ncols i;

    for (i = Z; i < pntabs; )
	sctab (ptabs[i++] - 1, setclr);
    return;
}

#ifdef COMMENT
void
sctab (col, setclr)
    Ncols col;
    Flag setclr;
.
    Set/clear a tab in 'col'.
    If 'setclr' is non-0 then set else clear.
#endif
void
sctab (col, setclr)
Ncols col;
Flag setclr;
{
    register Ncols i1;
    register Ncols i2;

    if (setclr) {
	if (ntabs + 1 > stabs)
	    tabs = (ANcols *) gsalloc ((char *) tabs,
				       ntabs * sizeof *tabs,
				       (stabs += NTABS / 2) * sizeof *tabs,
				       YES);

	for (i1 = Z; ; i1++) {
	    if (i1 >= ntabs) {
 setit:         ntabs++;
		tabs[i1] = col;
		break;
	    }
	    if ((i2 = col - tabs[i1]) == 0)
		break;
	    if (i2 < 0) {
		if ((i2 = (ntabs - i1) * sizeof *tabs) > 0)
		    move ((char *) &tabs[i1], (char *) &tabs[i1 + 1],
			  (Uint) i2);
		goto setit;
	    }
	}
#ifdef LMCMARG
	scvtab (col, NO);
#endif
    }
    else {
#ifdef LMCMARG
	scvtab (col, YES);
#endif
	for (i1 = Z; i1 < ntabs; i1++) {
	    if ((i2 = col - tabs[i1]) < 0)
		break;
	    if (i2 == 0) {
		if ((i2 = (--ntabs - i1) * sizeof *tabs) > 0)
		    move ((char *) &tabs[i1 + 1], (char *) &tabs[i1],
			  (Uint) i2);
		break;
	    }
	}
    }
    return;
}
