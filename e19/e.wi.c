#ifdef COMMENT
--------
file e.wi.c
    Window mainpulation code.
#endif

#ifdef COMMENT
	Copyright abandoned, 1983, The Rand Corporation
#endif

#include "e.h"
#include "e.cm.h"
#include "e.inf.h"
#include "e.tt.h"
#include "e.wi.h"

extern S_window *setupwindow ();

/* defined in e.t.c, set outside putup (), looked at by putup () */
extern Flag entfstline;     /* says write out entire first line */

extern void removewindow ();
extern void chgwindow ();
extern void drawborders ();
       void switchwindow ();
#ifdef LMCMARG
extern void drawmarg ();
#endif
extern void drawsides ();
extern void draw1side ();
extern void infotrack ();

#ifdef COMMENT
Cmdret
makewindow (file)
char   *file;
.
    Do the "window" command.
    This routine commits itself to making the new window before it
    calls editfile ().  This is not the way it should be.  It should
    verify that a window can be made here, then it should call some
    routine that is simliar to the present editfile () and if editfile
    says OK, it should draw the window, otherwise return with no new
    window made.
#endif
Cmdret
makewindow (file)
char   *file;
{
    Reg2 S_window *oldwin;
    Reg3 S_window *newwin;
    Flag    horiz;                /* 1 if margin horiz, 0 if vert */

    if (nwinlist >= MAXWINLIST) {
	mesg (ERRALL + 1, "Can't make any more windows.");
	return CROK;
    }
    if (   cursorcol == 0
	&& cursorline > 0
	&& cursorline < curwin->btext
       )
	horiz = YES;
    else if (   cursorline == 0
	     && cursorcol > 0
	     && cursorcol < curwin->rtext - 1
	    )
	horiz = NO;
    else {
	mesg (ERRALL + 1, "Can't put a window there.");
	return CROK;
    }

    oldwin = curwin;
    if (!(newwin = setupwindow ((S_window *) 0,
				horiz ? oldwin->lmarg
				      : oldwin->lmarg + cursorcol + 1,
				horiz ? oldwin->tmarg + cursorline + 1
				      : oldwin->tmarg,
				oldwin->rmarg,
				oldwin->bmarg, 0, YES)
       ) )
	return NOMEMERR;
    winlist[nwinlist] = newwin;

    if (horiz) {
	/* newwin is below oldwin on the screen  */
	Reg1 Slines i;
	oldwin->bmarg = oldwin->tmarg + cursorline + 1;
	oldwin->btext = oldwin->bedit = cursorline - 1;
	if ((i = (newwin->btext + 1) * sizeof oldwin->firstcol[0]) > 0) {
	    move (&oldwin->firstcol[cursorline + 1],
		   newwin->firstcol, (Uint) i);
	    move (&oldwin->lastcol[cursorline + 1],
		   newwin->lastcol, (Uint) i);
	}
    }
    else {
	/* newwin is to the right of oldwin on the screen  */
	Reg1 Slines i;
	oldwin->rmarg = oldwin->lmarg + cursorcol + 1;
	oldwin->rtext = oldwin->redit = cursorcol - 1;
	for (i = newwin->btext; i >= 0; i--) {
	    if (oldwin->lastcol[i] > oldwin->rtext + 1) {
		newwin->firstcol[i] = 0;
		newwin->lastcol[i] = oldwin->lastcol[i] - cursorcol - 1;
		oldwin->lastcol[i] = oldwin->rtext + 1;
		oldwin->rmchars[i] = MRMCH;
	    }
	}
    }

    {
	Small winnum;
	/* the number of curwin is new prevwin */
	for (winnum = 0; winlist[winnum] != curwin; winnum++)
	    continue;
	newwin->prevwin = winnum;
    }
    nwinlist++;

    savewksp (curwksp);
    drawborders (oldwin, WIN_INACTIVE | WIN_DRAWSIDES);
    drawborders (newwin, WIN_ACTIVE);

    infotrack (NO);
    inforange (NO);

    {
	Fn ocurfile, oaltfile;
	ocurfile = curwksp->wfile;
	oaltfile = curwin->altwksp->wfile;

	switchwindow (newwin);
	poscursor (0,0);

	if (file) {
	    if (strcmp (file, names[ocurfile]))
		editfile (names[ocurfile], (Ncols) -1, (Nlines) -1, 0, NO);
	    if (editfile (file, (Ncols) -1, (Nlines) -1, 1, YES) <= 0)
		eddeffile (1);
	}
	else {
	    if (oaltfile)
		editfile (names[oaltfile], (Ncols) -1, (Nlines) -1, 0, NO);
	    editfile (names[ocurfile],
		      (Ncols) (horiz ? -1
			       : oldwin->wksp->wcol + oldwin->rtext + 2),
		      (Nlines) (horiz
			       ? oldwin->wksp->wlin + oldwin->btext + 2 : -1),
		      0, YES);  /* puflg is YES to initalize border chars */
	    poscursor (0, 0);
	}
    }

    return CROK;
}

#ifdef COMMENT
S_window *
setupwindow (win, cl, lt, cr, lb, wf, editflg)
    S_window *win;
    Scols   cl;
    Scols   cr;
    Slines  lt;
    Slines  lb;
    Aflag   wf;
    Flag    editflg;
.
    Initialize the window using 'cl', 'cr', 'lt', 'lb' as the left, right,
    top, and bottom.
    If 'editflg' == YES then 'win' is an editing window -- i.e. borders, etc.
    If 'win' == 0, then alloc a new window.
    Return 'win' if no alloc failure, else return 0.


#endif
S_window *
setupwindow (win, cl, lt, cr, lb, wf, editflg)
Reg3 S_window *win;
Scols   cl;
Scols   cr;
Slines  lt;
Slines  lb;
AFlag   wf;
Flag    editflg;
{
    /* Warning : okalloc, salloc ... routines must initialize to 0 the allocated memory */
    if ( !win && !(win = (S_window *) okalloc (SWINDOW)) )
	return 0;

#ifdef LMCSTATE
    win->winflgs = wf;
#endif
    win->lmarg = cl;
    win->tmarg = lt;
    win->rmarg = cr;
    win->bmarg = lb;
    if (editflg) {
	win->ltext = cl + 1;
	win->ttext = lt + 1;
	win->rtext = cr - cl - 2;   /* 2 : this is NHORIZBORDERS */
	win->btext = lb - lt - 2;
    }
    else {
	win->ltext = cl;
	win->ttext = lt;
	win->rtext = cr - cl;
	win->btext = lb - lt;
    }
    win->ledit = 0;
    win->tedit = 0;
    win->redit = win->rtext;
    win->bedit = win->btext;
    win->miline = defmiline;
    win->plline = defplline;
    win->mipage = defmipage;
    win->plpage = defplpage;
    win->rwin = defrwin;
    win->lwin = deflwin;

    if ( !win->wksp )
	win->wksp = (S_wksp *) okalloc (sizeof (S_wksp));
    if ( win->wksp ) {
	if ( ! editflg )
	    return win;

	/* this is an editing window */
	if ( ! win->altwksp )
	    win->altwksp = (S_wksp *) okalloc (sizeof (S_wksp));
	if ( win->altwksp ) {
	    Slines size;
	    int i, sz;
	    AScols ascl;
	    size = term.tt_height - NPARAMLINES - NHORIZBORDERS;
	    if ( size < 80 ) size = 80;     /* min allocation */
	    if ( size > win->size ) {
		if ( win->firstcol ) sfree (win->firstcol);
		if ( win->lmchars )  sfree (win->lmchars);
		win->firstcol = win->lastcol = NULL;
		win->lmchars = win->rmchars = NULL;
		win->size = size;
	    }
	    size = win->size;
	    sz = 2 * size * sizeof (*win->firstcol);
	    if ( ! win->firstcol ) win->firstcol = (AScols *) okalloc (sz);
	    if ( win->firstcol ) {
		win->lastcol = &win->firstcol[size];
		ascl = win->rtext + 1;
		for ( i = size -1 ; i >= 0 ; i-- ) {
			win->firstcol[i] = ascl;
			win->lastcol[i] = 0;
		}
		sz = 2 * size * sizeof (*win->lmchars);
		if ( !win->lmchars )  win->lmchars  = okalloc (sz);
		if ( win->lmchars ) {
		    memset (win->lmchars, ' ', sz);
		    win->rmchars = &win->lmchars[size];
		    /* evry thing ok */
		    return win;
		}
		sfree ((char *) win->firstcol);
	    }

#if 0
	    Slines size;
	    size = term.tt_height - NINFOLINES - NENTERLINES - NHORIZBORDERS;
	    if ( ! win->firstcol ) sfree (win->firstcol);
	    win->firstcol = (AScols *) okalloc (2 * size * (sizeof *win->firstcol));
	    if ( win->firstcol ) {
		win->lastcol = &win->firstcol[size];
		if ( ! win->lmchars ) sfree (win->lmchars);
		win->lmchars = okalloc (2 * size * (sizeof *win->lmchars));
		if ( win->lmchars ) {
		    int i;
		    win->rmchars = &win->lmchars[size];
		    /* can't use bfill here because firstcol may not be a char */
		    for (i = 0; i < size; i++) {
			win->firstcol[i] = win->rtext + 1;
			win->lastcol[i] = 0;
		    }
		    return win;
		}
		sfree ((char *) win->firstcol);
	    }
#endif

	    sfree ((char *) win->altwksp);
	}
	sfree ((char *) win->wksp);
    }
    return NULL;
}

#ifdef COMMENT
void
removewindow ()
.
    Eliminates the last made window by expanding its ancestor
#endif
void
removewindow ()
{
    Scols   stcol;              /* start col for putup  */
    Slines  stlin;              /* start lin for putup  */
    Small   ppnum;              /* prev window number   */
    register S_window *thewin;  /* window to be removed */
    register S_window *pwin;    /* previous window      */

    if (nwinlist == 1) {
	mesg (ERRALL + 1, "Can't remove remaining window.");
	return;
    }
    thewin = winlist[--nwinlist];
    ppnum = thewin->prevwin;
    pwin = winlist[ppnum];
    savewksp (thewin->wksp);

    if (pwin->bmarg != thewin->bmarg) {
	/* thewin is below pwin on the screen  */
	Slines j;
	register Slines tmp;
	pwin->firstcol[j = pwin->btext + 1] = 0;
	pwin->lastcol[j++] = pwin->rtext + 1;
	if ((tmp = (thewin->btext + 1) * sizeof *thewin->firstcol) > 0) {
	    move (&thewin->firstcol[0],
		  &pwin->firstcol[j], (Uint) tmp);
	    move (&thewin->lastcol[0],
		  &pwin->lastcol[j], (Uint) tmp);
	}
	stcol = 0;
	stlin = pwin->btext + 1;
	pwin->bmarg = thewin->bmarg;
	pwin->btext = pwin->bmarg - pwin->tmarg - 2;
	pwin->bedit = pwin->btext;
    }
    else {
	/* thewin is to the right of pwin on the screen  */
	register Slines tmp;
	for (tmp = 0; tmp <= pwin->btext; tmp++) {
	    pwin->lastcol[tmp] = thewin->lastcol[tmp] +
		thewin->lmarg - pwin->lmarg;
	    if (pwin->firstcol[tmp] > pwin->rtext)
		pwin->firstcol[tmp] = pwin->rtext;
	}
	stcol = pwin->rtext + 1;
	stlin = 0;
	pwin->rmarg = thewin->rmarg;
	pwin->rtext = pwin->rmarg - pwin->lmarg - 2;
	pwin->redit = pwin->rtext;
    }
    chgwindow (ppnum);
    putup (stlin, curwin->btext, stcol, MAXWIDTH);
    poscursor (pwin->wksp->ccol, pwin->wksp->clin);

    sfree ((char *) thewin->firstcol);
    sfree (thewin->lmchars);
    if (thewin->altwksp->wfile != NULLFILE)
	(void) la_close (&thewin->altwksp->las);
    sfree ((char *) thewin->altwksp);
    (void) la_close (&thewin->wksp->las);
    sfree ((char *) thewin->wksp);
    sfree ((char *) thewin);
    winlist [nwinlist] = NULL;
    return;
}

#ifdef COMMENT
void
chgwindow (winnum)
    Small winnum;
.
    Moves current window to another window.
    if 'winnum' < 0 means go to next higher window in winlist[].
#endif
void
chgwindow (winnum)
Small winnum;
{
    register S_window *newwin;
    register S_window *oldwin;

    oldwin = curwin;
    if (winnum < 0) {
	winnum = 0;
	while (winnum < nwinlist && oldwin != winlist[winnum++])
	    continue;
    }
    curwksp->ccol = cursorcol;
    curwksp->clin = cursorline;
    newwin = winlist[winnum % nwinlist];    /* wrap back to window 0 */
    if (newwin == oldwin)   /* ALWAYS rewrite first line */
	entfstline = YES;   /* don't skip over blanks */
    else
	drawborders (oldwin, WIN_INACTIVE | WIN_DRAWSIDES);
    drawborders (newwin, WIN_ACTIVE | WIN_DRAWSIDES);
    infotrack (newwin->winflgs & TRACKSET);
    inforange (newwin->wksp->wkflags & RANGESET);
    switchwindow (newwin);
    limitcursor ();
    poscursor (curwksp->ccol, curwksp->clin);
    return;
}

#ifdef COMMENT
void
drawborders (window, how)
    S_window *window;
    Small how;
.
    Draw borders for active or inactive window with or without drawing sides.
#endif
void
drawborders (window, how)
S_window *window;
Small how;
{
    S_window *oldwin;
    register Short i;
    register Short j;
#ifdef LMCMARG
    Ncols k, k1;
#endif

    oldwin = curwin;
    switchwindow (&wholescreen);

    j = window->rmarg;
    poscursor (i = window->lmarg, window->tmarg);
#ifdef LMCMARG
    for (k=0; k < ntabs && tabs[k] < window->wksp->wcol; k++)
	{}
    k1 = k;
#endif
    if (!(how & WIN_ACTIVE))
	for (; i <= j; i++)
	    putch (INMCH, NO);
    else {
	putch (TLCMCH, NO);
	for (i++; i < j; i++)
#ifdef LMCMARG
	    if (tabs[k] == i - window->lmarg - 1 + window->wksp->wcol) {
		/* no tab mark in col 0 */
		if( tabs[k] != 0 )
		    putch (TTMCH, NO);
		else
		    putch (TMCH, NO);
		if (k < ntabs) k++;
	    } else
#endif
		putch (TMCH, NO);
	putch (TRCMCH, NO);
/*****   no left/rt  marg indicators on top border *******************
#ifdef LMCMARG
#ifdef LMCAUTO
	drawmarg (window, autolmarg, window->tmarg, BTMCH);
#endif
	drawmarg (window, linewidth, window->tmarg, BTMCH);
#endif
*****************************************************************/

    }

    poscursor (i = window->lmarg, window->bmarg);
    if (!(how & WIN_ACTIVE))
	for (; i <= j; i++)
	    putch (INMCH, NO);
    else {
	putch (BLCMCH, NO);
	for (i++; i < j; i++)
/***************  no tab marks on bottom margin ************************
#ifdef LMCMARG
	    if (tabs[k1] == i - window->lmarg - 1 + window->wksp->wcol) {
		putch (BTMCH, NO);
		if (k1 < ntabs) k1++;
	    } else
#endif
**********************************************************************/
		putch (BMCH, NO);
	putch (BRCMCH, NO);
#ifdef LMCMARG
#ifdef LMCAUTO
	/* display left mar char, if one is set */
	if( autolmarg )
	    drawmarg (window, autolmarg, window->bmarg, BTMCH);
#endif
	drawmarg (window, linewidth, window->bmarg, BTMCH);
#endif
    }

    drawsides (window, how);
    switchwindow (oldwin);
    return;
}

#ifdef LMCMARG
#ifdef COMMENT
void
drawmarg (window, col, line, chr)
    S_window *window;
    Nlines line;
    Ncols col;
    char chr;
.
    This routine will draw the margin indicator character chr in the window
    at line, col if it is within the current window frame.
#endif
void
drawmarg (window, col, line, chr)
    S_window *window;
    Nlines line;
    Ncols col;
    char chr;

{
    col -= (window->wksp->wcol - 1);
    if (col >= 0 && col <= window->rtext) {
	poscursor (col + window->lmarg, line);
	putch (chr, NO);
    }
}
#endif

#ifdef COMMENT
void
drawsides (window, how)
    S_window *window;
    Small how;
.
    Draw the side borders of window.
#endif
void
drawsides (window, how)
S_window *window;
Small how;
{
    if (!(how & WIN_DRAWSIDES))
	return;

    if (window->rmarg - window->lmarg < term.tt_width - 1) {
	draw1side (window, window->lmarg, how);
	draw1side (window, window->rmarg, how);
    }
    else {
	/* this way is faster when window is full-width */
	Reg3 Slines line;
	Reg4 Slines bottom;
	bottom = window->bmarg - 1;
	line = window->tmarg + 1;
	if (how & WIN_ACTIVE) {
	    Reg1 char *lmcp;
	    Reg2 char *rmcp;
	    lmcp = window->lmchars;
	    rmcp = window->rmchars;
	    for (; line <= bottom; line++) {
		poscursor (window->lmarg, line);
		putch (*lmcp++, NO);
		poscursor (window->rmarg, line);
		putch (*rmcp++, NO);
	    }
	}
	else {
	    for (; line <= bottom; line++) {
		poscursor (window->lmarg, line);
		putch (INMCH, NO);
		poscursor (window->rmarg, line);
		putch (INMCH, NO);
	    }
	}
    }
    return;
}

#ifdef COMMENT
void
draw1side (window, border, how)
    S_window *window;
    register Scols border;
    Small how;
.
    Draw each side border of window.
#endif
void
draw1side (window, border, how)
S_window *window;
register Scols border;
Small how;
{
    Reg3 Slines line;
    Reg4 Slines bottom;

    bottom = window->bmarg - 1;
    line = window->tmarg + 1;
    if (how & WIN_ACTIVE) {
	register char *bchrp;
	bchrp = border == window->lmarg ? window->lmchars : window->rmchars;
	for (; line <= bottom; line++) {
	    poscursor (border, line);
	    putch (*bchrp++, NO);
	}
    }
    else {
	for (; line <= bottom; line++) {
	    poscursor (border, line);
	    putch (INMCH, NO);
	}
    }
    return;
}

#ifdef COMMENT
void
switchwindow (win)
    S_window *win;
.
    Adjust all the 'cur' globals like curfile, etc.
    Changes cursorline, cursorcol to be relative to new upper lefthand
    corner.
    You must do a poscursor after switchwindow.
#endif
void
switchwindow (win)
register S_window *win;
{
    register S_window *cwin;

    cwin = curwin;
    cwin->ccol = cursorcol;
    cwin->clin = cursorline;
    cursorcol  += cwin->ltext - win->ltext;
    cursorline += cwin->ttext - win->ttext;
    if (curwksp = (curwin = win)->wksp) {
	curfile = curwksp->wfile;
	curlas = &curwksp->las;
    }
    if (!curwin || !curwksp)
	fatal (FATALBUG, "switchwindow %o %o", curwin, curwksp);
/*  defplline = defmiline = win->btext / 4 + 1;                         */


    /*
     *  If a "set +line" or "set -line" cmd has been issued for this
     *  window, honor the values.  Otherwise, setup the default values
     *  for +line and -line for the window.
     */

    if (win->winflgs & PLINESET)
	defplline = win->plline;
    else
	defplline = win->btext / 4 + 1;

    if (win->winflgs & MLINESET)
	defmiline = win->miline;
    else
	defmiline = win->btext / 4 + 1;

    defplpage = win->plpage;
    defmipage = win->mipage;
    defrwin = win->rwin;
    deflwin = win->lwin;

    return;
}

#ifdef COMMENT
void
infotrack (onoff)
    Flag onoff;
.
    Update the TRACK display on the info line
#endif
void
infotrack (onoff)
Reg1 Flag onoff;
{
    static Flag wason;

    if ((onoff = onoff ? YES : NO) ^ wason)
	rand_info (inf_track, 3, (wason = onoff) ? "TRK" : "");
    return;
}
