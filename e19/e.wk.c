#ifdef COMMENT
--------
file e.wk.c
    workspace manipulation code.
#endif

#ifdef COMMENT
	Copyright abandoned, 1983, The Rand Corporation
#endif

#include "e.h"
#include "e.inf.h"

       void switchfile ();
extern void exchgwksp ();
       void savewksp ();
extern void releasewk ();
extern void inforange ();
extern void infoprange ();
extern Flag reset_impl_tick ();

#ifdef COMMENT
void
switchfile ()
.
    Switch to alternate wksp if there is one, else do error mesg.
#endif
void
switchfile ()
{
    if (curwin->altwksp->wfile == OLDLFILE)
	getline (-1);   /* so that the last line modified will show up */
    if (!swfile ())
	mesg (ERRALL + 1, "No alternate file");
    return;
}

#ifdef COMMENT
Flag
swfile ()
.
    Return NO if no alt wksp else switch to alt wksp and return YES.
#endif
Flag
swfile ()
{
    if (curwin->altwksp->wfile == NULLFILE)
	return NO;
    (void) reset_impl_tick ();
    exchgwksp (NO);
    putupwin ();
    if (curwin->winflgs & TRACKSET)
	poscursor (curwin->altwksp->ccol, curwin->altwksp->clin);
    else
	poscursor (curwksp->ccol, curwksp->clin);
    return YES;
}

#ifdef COMMENT
void
exchgwksp ()
.
    /*
    Exchange wksp and altwksp.  If silent is set, don't turn on lights.
    */
#endif
void
exchgwksp (silent)
Flag silent;
{
    S_wksp *cwksp;

    cwksp = curwksp;
    savewksp (cwksp);

    cwksp->ccol = cursorcol;
    cwksp->clin = cursorline;

    curwin->wksp    = curwin->altwksp;
    curwin->altwksp = cwksp;
    curwksp = curwin->wksp;
    if (!silent) inforange (curwksp->wkflags & RANGESET);

    curfile = curwksp->wfile;
    curlas = &curwksp->las;
    limitcursor ();

    return;
}

#ifdef COMMENT
void
savewksp (S_wksp *pwk)
.
    Save workspace position in in lastlook[pwk->wfile].
#endif
void
savewksp (S_wksp *pwk)
{
    S_wksp *lwksp;

    if ( ! pwk ) return;
    if ( curwksp == pwk ) {
	pwk->ccol = cursorcol;
	pwk->clin = cursorline;
    }
    if ( pwk->wfile == NULLFILE )
	return;
    lwksp = &lastlook[pwk->wfile];

    /* save cursor position of the current work space */
    (void) la_align (&pwk->las, &lwksp->las);
    lwksp->wcol = pwk->wcol;
    lwksp->wlin = pwk->wlin;
    lwksp->ccol = pwk->ccol;
    lwksp->clin = pwk->clin;
    /* save also the previous cursor position */
    lwksp->wkpos = pwk->wkpos;

    return;
}

#ifdef COMMENT
void
releasewk (wk)
    register S_wksp *wk;
.
    /*
    Release the file in workspace.
    La_close the range streams, if any, for workspace 'wk'.
    */
#endif
void
releasewk (wk)
register S_wksp *wk;
{
    if ( wk->wfile != NULLFILE ) {
	savewksp (wk);
	(void) la_close (&wk->las);
	wk->wfile = NULLFILE;
    }
    wk->wkflags &= ~RANGESET;
    if (wk->brnglas) {
	(void) la_close (wk->brnglas);
	wk->brnglas = (La_stream *) 0;
	(void) la_close (wk->ernglas);
    }
    return;
}

static char *lastwhere = " ";

#ifdef COMMENT
void
inforange (onoff)
    Flag onoff;
.
    Update the RANGE display on the info line
#endif
void
inforange (onoff)
Reg1 Flag onoff;
{
    static Flag wason;

    if ((onoff = onoff ? YES : NO) ^ wason) {
	if (wason = onoff)
	    rand_info (inf_range + 1, 2, "RG");
	else {
	    rand_info (inf_range, 2, "");
	    lastwhere = " ";
	}
    }
    return;
}

#ifdef COMMENT
void
infoprange (line)
    Nlines line;
.
    Update the position portion of the RANGE display on the info line
#endif
void
infoprange (line)
Reg2 Nlines line;
{
    Reg1 char *where;

    if (line < la_lseek (curwksp->brnglas, 0, 1))
	where = "<";
/*  else if (line > la_lseek (curwksp->ernglas, 0, 1)) */
    else if (line >= la_lseek (curwksp->ernglas, 0, 1))
	where = ">";
    else
	where = "=";
    if (   *lastwhere == ' '
	|| *where != *lastwhere
       ) {
	rand_info (inf_range, 1, where);
	lastwhere = where;
    }
    return;
}
