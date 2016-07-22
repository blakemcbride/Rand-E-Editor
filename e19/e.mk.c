#ifdef COMMENT
--------
file e.p.c
    cursor marking
#endif

#ifdef COMMENT
	Copyright abandoned, 1983, The Rand Corporation
#endif

#include "e.h"
#include "e.inf.h"
#include "e.m.h"

extern void markprev ();

extern Flag fileticksflags[];
extern struct markenv fileticks[];

Flag reset_impl_tick ();

/* Implicite tick mark : to go back from window move,
 * the implicite tick mark is also call "transiant tick" (in documentation)
 */
Flag impl_tick_flg = NO;
static struct markenv impl_tick;
static char searchkey_ref [256];

static const char attick_msg [] = "At tick mark";

#ifdef COMMENT
void
mark ()
.
    Mark this spot.
#endif
void
mark ()
{
    if (prevmark == 0)
	rand_info (inf_mark, 4, "MARK");
    if (curmark)
	loopflags.flash = YES;
    markprev ();
    exchmark (YES);
    return;
}

#ifdef COMMENT
Flag
unmark ()
.
    Remove marks, if any.
#endif
Flag
unmark ()
{
    Flag flg;

    if ( flg = (curmark != NULL) ) {
	rand_info (inf_mark, 4, "");
	rand_info (inf_area, infoarealen, "");
    }
    curmark = prevmark = 0;
    infoarealen = 0;
    marklines = 0;
    markcols = 0;
    mklinstr[0] = 0;
    mkcolstr[0] = 0;
    return flg;
}

#ifdef COMMENT
Nlines
topmark ()
.
    Return the top line of a marked area.
#endif
Nlines
topmark ()
{
    return min (curwksp->wlin + cursorline,
		curmark->mrkwinlin + curmark->mrklin);
}

#ifdef COMMENT
Ncols
leftmark ()
.
    Return the leftmost column of a marked area.
#endif
Ncols
leftmark ()
{
    return min (curwksp->wcol + cursorcol,
		curmark->mrkwincol + curmark->mrkcol);
}

#ifdef COMMENT
Flag
gtumark (leftflg)
    Flag leftflg;
.
    /*
    Go to upper mark.
    if leftflg is YES, go to upper left corner
    else if lines must be exchanged, then exchange columns also
    else don't exchange columns
    */
#endif
Flag
gtumark (leftflg)
Flag leftflg;
{
    Short tmp;
    Flag exchlines = NO;
    Flag exchcols  = NO;

    /* curmark is the OTHER corner */
    markprev ();

    if (    curmark->mrkwinlin + curmark ->mrklin
	 < prevmark->mrkwinlin + prevmark->mrklin
       )
	exchlines = YES;
    if (   (   leftflg
	    &&    curmark->mrkwincol + curmark ->mrkcol
	       < prevmark->mrkwincol + prevmark->mrkcol
	   )
	|| (!leftflg && exchlines)
       )
	exchcols = YES;
    if (exchlines || exchcols) {
	if (!exchcols) {
	    /* exchange the cols so mark will set them back */
	    tmp = curmark->mrkwincol;
	    curmark->mrkwincol = prevmark->mrkwincol;
	    prevmark->mrkwincol = tmp;
	    tmp = curmark->mrkcol;
	    curmark->mrkcol = prevmark->mrkcol;
	    prevmark->mrkcol = tmp;
	}
	else if (!exchlines) {
	    /* exchange the lines so mark will set them back */
	    tmp = curmark->mrkwinlin;
	    curmark->mrkwinlin = prevmark->mrkwinlin;
	    prevmark->mrkwinlin = tmp;
	    tmp = curmark->mrklin;
	    curmark->mrklin = prevmark->mrklin;
	    prevmark->mrklin = tmp;
	}
	return exchmark (NO) & WINMOVED;
    }
    return 0;
}

/* save current position into mark struct */

void savemark (struct markenv *mark_pt)
{
    if ( ! mark_pt ) return;
    mark_pt->mrkwincol = curwksp->wcol;
    mark_pt->mrkwinlin = curwksp->wlin;
    mark_pt->mrkcol = cursorcol;
    mark_pt->mrklin = cursorline;
}


static Flag at_mark (struct markenv *mark_pt)
{
    Nlines s_lin;
    Ncols  s_col;

    s_lin = mark_pt->mrkwinlin + mark_pt->mrklin;
    s_col = mark_pt->mrkwincol + mark_pt->mrkcol;
    return (   (s_lin) == (curwksp->wlin + cursorline)
	    && (s_col) == (curwksp->wcol + cursorcol) );
}

/* copy mark if the current position is different from what to save */
void copymark (struct markenv *s_mark_pt, struct markenv *d_mark_pt)
{
    void infosetmark ();

    if ( at_mark (s_mark_pt) ) return;

#if 0
    Nlines s_lin;
    Ncols  s_col;

    s_lin = s_mark_pt->mrkwinlin + s_mark_pt->mrklin;
    s_col = s_mark_pt->mrkwincol + s_mark_pt->mrkcol;

    if ( (s_lin) == (curwksp->wlin + cursorline) )
	/* do not save if the cursor was not moved outside the current line */
	return;

    if ( (s_lin <= 0)  || (s_lin >= la_lsize (curlas)) ) {
	/* do not save position at top or bottom of the file */
	if ( DebugVal > 0 ) {
	    mesg (TELALL + 1, (s_lin <= 0) ?
		    " At beginning of file : do not save the position"
		  : " At end of file : do not save the position");

	    loopflags.hold = YES;
	}
	return;
    }
#endif

    /* copy the saved position */
    *d_mark_pt = *s_mark_pt;
    infosetmark ();
}

/* move to the marked position */

Small move2mark (struct markenv *mark_pt, Flag putflg)
{
    Nlines  winlin, lin;
    Ncols   wincol, col;
    Slines  curslin;
    Scols   curscol;
    Small   retval;

    if ( ! mark_pt ) return;
    if ( at_mark (mark_pt) )
	return 0;   /* no move */

    winlin = mark_pt->mrkwinlin;
    wincol = mark_pt->mrkwincol;
    curslin = mark_pt->mrklin;
    curscol = mark_pt->mrkcol;
    lin = winlin + curslin;
    col = wincol + curscol;

    if (   (lin >= curwksp->wlin) && (lin <= (curwksp->wlin + curwin->btext))
	&& (col >= curwksp->wcol) && (col <= (curwksp->wcol + curwin->rtext))
       ) {  /* within the current window, do not move the window */
	winlin = curwksp->wlin;
	wincol = curwksp->wcol;
	curslin = lin - winlin;
	curscol = col - wincol;
    } else {
	if ( curslin > curwin->btext ) {
	    winlin += (curslin - curwin->btext);
	    curslin = curwin->btext;
	}
	if ( curscol > curwin->rtext ) {
	    wincol += (curscol - curwin->rtext);
	    curscol = curwin->rtext;
	}
    }
    retval = movewin (winlin, wincol, curslin, curscol, putflg);
    return retval;
}


#ifdef COMMENT
Small
exchmark (puflg)
    Flag puflg;
.
    Exchange the two marked positions.
#endif
Small
exchmark (puflg)
Flag puflg;
{
    struct markenv *tmp;
    Small retval = 0;

    if ( curmark ) retval = move2mark (curmark, puflg);
    tmp = curmark;
    curmark = prevmark;
    prevmark = tmp;
    return retval;
}


#ifdef COMMENT
void
markprev ()
.
    Copy the current position into the prevmark structure.
#endif
void
markprev ()
{
    static struct markenv mk1, mk2;

    if (prevmark == 0)
	prevmark = curmark == &mk1 ? &mk2 : &mk1;
    savemark (prevmark);
    return;
}


/* file ticks : mark a position in a file, to be used by "goto tick" cmd */
/* --------------------------------------------------------------------- */

static struct markenv * check_tick (AFn fnb, char * msgbuf, Flag msg_flg)
{
    static char notick_msg [] = " No \"tick\" set in the file";

    if ( !(fileflags[fnb] & INUSE) ) fileticksflags [fnb] = NO;
    if ( ! fileticksflags [fnb] ) {
	if ( msgbuf ) strcat (msgbuf, notick_msg);
	if ( msg_flg ) {
	    mesg (TELALL + 1, notick_msg);
	    loopflags.hold = YES;
	}
    }
    return fileticksflags [fnb] ? & fileticks [fnb] : NULL;

}


static Flag get_mark (struct markenv *markpt, int * tkln_pt, int * tkcl_pt)
{
    if ( !markpt ) return NO;
    if ( tkln_pt ) *tkln_pt = markpt->mrkwinlin + markpt->mrklin +1;
    if ( tkcl_pt ) *tkcl_pt = markpt->mrkwincol + markpt->mrkcol +1;
    return YES;
}

struct markenv * get_file_mark (AFn fnb)
{
    struct markenv *markpt;

    if ( !(fileflags[fnb] & INUSE) ) fileticksflags [fnb] = NO;
    if ( ! fileticksflags [fnb] ) return NULL;
    return ( &fileticks[fnb] );
}

void set_file_mark (AFn fnb, Flag tick_flg, struct markenv *my_markpt)
{
    struct markenv *markpt;

    if ( fnb <= 0 ) return;
    fileticksflags [fnb] = tick_flg;
    if ( ! tick_flg ) return;
    markpt = get_file_mark (fnb);
    if ( ! markpt ) return;
    *markpt = *my_markpt;
}

void msg_mark (struct markenv *markpt, char * prefix, char * msgbuf,
	       Flag show_flg, Flag compact_flg)
{
    int tkln, tkcl;
    char * buf;

    if ( !markpt || !msgbuf ) return;
    (void) get_mark (markpt, &tkln, &tkcl);
    buf = &msgbuf [strlen (msgbuf)];
    if ( compact_flg )
	sprintf (buf, "%dx%d", tkln, tkcl);
    else
	sprintf (buf, "%sline %d, column %d", prefix ? prefix : "", tkln, tkcl);
    if ( show_flg ) {
	mesg (TELALL + 1, msgbuf);
	loopflags.hold = YES;
    }
}

static void msg_tick (AFn fnb, char * prefix, char * msgbuf, Flag chk_flg)
{
    struct markenv *markpt;
    int tkln, tkcl;

    markpt = check_tick (fnb, msgbuf, chk_flg);
    if ( ! markpt ) return;
    msg_mark (markpt, prefix, msgbuf, chk_flg, NO);
}

void infotick ()
{
    AFn fnb;
    struct markenv *markpt;
    char msgbuf [128];
    int tkln, tkcl;

    fnb = curwin->wksp->wfile;
    memset (msgbuf, 0, sizeof (msgbuf));
    msg_tick (fnb, " File \"tick\" at ", msgbuf, NO);
    strcat (msgbuf, ",");
    msg_mark (&curwksp->wkpos, " \"prev\" cursor at ", msgbuf, YES, NO);
}


void infosetmark ()
{
    char msgbuf [128];
    Flag flg;

    memset (msgbuf, 0, sizeof (msgbuf));
    flg = ( DebugVal > 0 ); /* DebugVal is set with "set debug n" cmd */
    msg_mark (&curwksp->wkpos, " Save \"prev\" cursor position : ",
	      msgbuf, flg, NO);
}


void gototick ()
{
    AFn fnb;
    struct markenv *markpt;

    fnb = curwin->wksp->wfile;
    markpt = check_tick (fnb, NULL, YES);
    if ( ! markpt ) return;
    markpt = & fileticks [fnb];
    (void) move2mark (markpt, YES);
	mesg (TELALL + 1, attick_msg);
	loopflags.hold = YES;
}

/* Set or reset the file tick mark at current position for the file */
/* ---------------------------------------------------------------- */

static void marktickfile_msg (AFn fnb, Flag set, Flag msg_flg)
{
    struct markenv *markpt;
    char msgbuf [128];
    Flag flg;

    if ( (fnb < 0) || (fnb >= MAXFILES) )
	return;

    fileticksflags [fnb] = NO;
    if ( !(set && (fileflags[fnb] & INUSE)) )
	return;

    markpt = & fileticks [fnb];
    savemark (markpt);
    fileticksflags [fnb] = YES;
    flg = ( fnb == curwin->wksp->wfile );
    if ( msg_flg ) {
	memset (msgbuf, 0, sizeof (msgbuf));
	msg_tick (fnb, " Set file \"tick\" at ", msgbuf, flg);
    }
}

void marktickfile (AFn fnb, Flag set)
{
    marktickfile_msg (fnb, set, YES);
}

void marktick (Flag set)
{
    marktickfile_msg (curwin->wksp->wfile, set, YES);
}

void marktick_msg (Flag set, Flag msg)
{
    marktickfile_msg (curwin->wksp->wfile, set, msg);
}

void goto_tick_or_prev ()
{
    struct markenv *mark_pt, tmpmk;
    char *msg;

    if ( impl_tick_flg ) {
	if ( ! at_mark (&impl_tick) ) {
	    (void) move2mark (&impl_tick, YES);
	}
	(void) reset_impl_tick ();
	return;
    }

    mark_pt = check_tick (curwin->wksp->wfile, NULL, NO);   /* tick mark */
    msg = (char *) attick_msg;
    if ( !mark_pt || at_mark (mark_pt) ) {
	/* no tick or at tick, try previous position */
	mark_pt = &curwksp->wkpos;
	msg = NULL;
    }
    if ( at_mark (mark_pt) ) return;
    savemark (&tmpmk);
    (void) move2mark (mark_pt, YES);
    copymark (&tmpmk, &curwksp->wkpos);
    if ( msg ) {
	mesg (TELALL + 1, msg);
	loopflags.hold = YES;
    }
}

Flag test_at_impl_tick ()
{
    Flag flg;

    if ( ! impl_tick_flg ) return NO;
    if ( flg = at_mark (&impl_tick) ) (void) reset_impl_tick ();
    return flg;
}

static void my_set_impl_tick (Flag force, Flag info_flg)
{
    if ( !force && impl_tick_flg ) {
	return;
    }
    savemark (&impl_tick);
    impl_tick_flg = YES;
    if ( info_flg ) rand_info (inf_at -1, 1, "*");
}

void set_impl_tick (Flag force)
{
    my_set_impl_tick (force, YES);
}

void reset_searchkey_ref ()
{
    if ( *searchkey_ref )
	memset (searchkey_ref, 0, sizeof (searchkey_ref));
}

Flag reset_impl_tick ()
{
    reset_searchkey_ref ();
    if ( ! impl_tick_flg ) return NO;
    impl_tick_flg = NO;
    rand_info (inf_at -1, 1, " ");
    return YES;
}

void save_or_set_impl_tick (Flag test_flg)
/* To be call only during search processing */
{
    static struct markenv saved_impl_tick;

    if ( !test_flg ) {
	if ( ! searchkey ) return;
	if ( *searchkey_ref && (strcmp (searchkey_ref, searchkey) != 0) ) {
	    reset_impl_tick ();
	}
	if ( ! *searchkey_ref ) {
	    /* a new search sequence */
	    strncpy (searchkey_ref, searchkey, sizeof (searchkey_ref) -1);
	    savemark (&saved_impl_tick);
	}
	return;
    }

    if (   (saved_impl_tick.mrkwincol != curwksp->wcol)
	|| (saved_impl_tick.mrkwinlin != curwksp->wlin) ) {
	/* the window was moved, set impl_tick */
	impl_tick = saved_impl_tick;
	impl_tick_flg = YES;
	rand_info (inf_at -1, 1, "*");
    } else if ( impl_tick_flg ) {
	impl_tick_flg = NO;
	rand_info (inf_at -1, 1, " ");
    }
}
