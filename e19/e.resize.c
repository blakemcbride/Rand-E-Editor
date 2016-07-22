#ifdef COMMENT
file : e.resize.c
-----------------
Resize according to the actual screen size
    can be done if there is only 1 editing window in use

Initial version : by Fabien Perriollat, Fev 2000, <Fabien.Perriollat@cern.ch>
#endif

#include <string.h>
#include <signal.h>

#include "e.h"
#include "e.cm.h"
#include "e.inf.h"
#include "e.tt.h"
#include "e.wi.h"

extern S_window *setupwindow ();
extern int svrs_image_line ();

static void resize_now (Flag msg_flg, int height, int width);
static void (* alt_resize) (int, int, void *) = NULL;
static void * alt_resize_para = NULL;

static Flag resize_requested = NO;
static Flag resize_allowed = NO;


/* Saved cursor and window for which the cursor is saved */
void *saved_curs = NULL;
S_window * saved_curs_win = NULL;


static void save_win_image (S_window * win, char * saved_buf, int entsz,
			    int * ln_nb, int * ln_sz)
{
    int i, lnb, lsz, sz;
    int icur;
    char * saved;
    S_window * oldwin;

    *ln_nb = lnb = win->bmarg - win->tmarg +1;
    *ln_sz = lsz = win->rmarg - win->lmarg +1;
    sz = lnb * lsz;
    if ( sz > entsz ) {
	sz = entsz;
	lnb = sz / lsz;
	}
    saved  = saved_buf;
    oldwin = curwin;
    switchwindow (win);
    for ( i = 0 ; i < lnb ; i++ ) {
	poscursor (0, i);
	icur = svrs_image_line (saved + (lsz * i), i, lsz, YES);
    }
    switchwindow (oldwin);
    poscursor (0, 0);
}

static void restaure_win_image (S_window * win, char * saved,
				int ln_nb, int ln_sz)
{
    int i, lnb, lsz, nb, sz;
    int icur;
    S_window * oldwin;
    char buf [MAXWIDTH +1]; /* the screen size cannot be larger than 255 */

    if ( ! saved ) return;

    lsz = win->rmarg - win->lmarg +1;
    sz = ( ln_sz > lsz ) ? lsz : ln_sz;
    lnb = win->bmarg - win->tmarg +1;
    nb = ( ln_nb > lnb ) ? lnb : ln_nb;
    oldwin = curwin;
    switchwindow (win);
    for ( i = 0 ; i < nb ; i++ ) {
	memset (buf, ' ', sizeof (buf));
	memcpy (buf, saved + (ln_sz * i), ln_sz);
	poscursor (0, i);
	icur = svrs_image_line (buf, i, lsz, NO);
    }
    switchwindow (oldwin);
    poscursor (0, 0);
}

/* Write into keystroke file a resize command
 *  NB : param () in e.t.c is strongly dependant on the
 *       detail of the char and control char which are
 *       written by this routine.
 */
void key_resize ()
{
    extern void writekeys (Char code1, char * str, Char code2);
    char str[8];

    memset (str, 0, sizeof (str));
    str [0] = CCRESIZE;
    sprintf (&str[1], "%dx%d", term.tt_width, term.tt_height);
    writekeys (CCCMD, str, CCRETURN);
}

void replaying_resize (char *resize_chr)
{
    int height, width;
    char *str;

    if ( ! resize_chr ) return;
    str = strchr (resize_chr, 'x');
    if ( !str ) return;
    *str = '\0';
    width =  atoi (resize_chr);
    height = atoi (str+1);
    resize_now (NO, height, width);
}

static void resize_now (Flag msg_flg, int height, int width)
#ifdef VBSTDIO
{
    /* To support this case, the iobuf on stdout must be resized accord to the
     *  new screen size ...
     */
    if ( msg_flg )
	mesg (ERRALL + 1, "Not allowed when Rand editor is build with VBSTDIO defined");
    resize_requested = NO;
    return;
}

#else /* -VBSTDIO */
{
    extern void set_tt_size (S_term *, int, int);
    extern void get_tt_size (int *, int *);
    extern void utility_setupwindow ();
    extern void edit_setupwindow ();
    extern void new_image ();
    extern void fresh ();
    extern void refresh_info ();
    extern void update_saved_curs ();

    int old_height, old_width;
    int delta_height, delta_width;
    Short old_screensize;
    S_window * win0, * oldwin;
    AFlag old_winflgs;
    int col, lin;
    int colw0, linw0;
    char enter_saved [MAXWIDTH +1];
    int enter_ln_nb, enter_ln_sz;
    int hsz, vsz;
    Nlines  nwinlin;
    Ncols   nwincol;
    Slines  ncurslin;
    Scols   ncurscol;
    Flag    movewin_flg;

    char buf [128];

    old_width  = term.tt_width;
    old_height = term.tt_height;
    delta_width = width - old_width;
    delta_height = height - old_height;

    if ( (old_height == height) && (old_width  == width) ) {
	resize_requested = NO;  /* nothing to do */
	return;
    }

    if (nwinlist != 1) {
	fresh ();
	if ( msg_flg )
	    mesg (ERRALL + 1, "Can't resize when more than one window is in use.");
	fflush (stdout);
	return;
    }

    resize_requested = NO;  /* resize will be processed */

    /* Save some initial value */
    oldwin = curwin;
    old_screensize = screensize;
    win0 = winlist[0];
    old_winflgs = win0->winflgs;
    col = cursorcol;
    lin = cursorline;
    colw0 = ( curwin == win0 ) ? cursorcol  : win0->ccol;
    linw0 = ( curwin == win0 ) ? cursorline : win0->clin;


    /* Save the content of entry window */
    save_win_image (&enterwin, &enter_saved[0], sizeof (enter_saved), &enter_ln_nb, &enter_ln_sz);

    /* Set the new screen size */
    set_tt_size (&term, width, height);
    key_resize ();

    /* Build the new screen image, and update window descriptor */
    switchwindow (win0);
    poscursor (0, 0);
    new_image ();
    utility_setupwindow ();
    edit_setupwindow (win0, old_winflgs);

    /* Now refresh the screen */
    switchwindow (win0);
    poscursor (0, 0);
    limitcursor ();
    drawborders (win0, WIN_ACTIVE | WIN_DRAWSIDES);
    putupwin ();
    fflush (stdout);
    restaure_win_image (&enterwin, enter_saved, enter_ln_nb, enter_ln_sz);
    fresh ();
    refresh_info ();
    putscr (0);

    /* Force the cursor to be within the window boundary */
    nwinlin = curwksp->wlin;
    nwincol = curwksp->wcol;
    ncurslin = linw0;
    ncurscol = colw0;
    movewin_flg = NO;
    /* 1 - horizontal move  ? */
    hsz = colw0 - (win0->rmarg - win0->lmarg -2);
    if ( hsz > 0 ) {
	/* cursor over the rigth end of the windowt */
	nwincol += hsz;
	ncurscol -= hsz;
	movewin_flg = YES;
    } else if ( nwincol && (hsz != 0) ) {
	/* cursor within the window, move the window to the left ? */
	if ( delta_width >= curwksp->wcol ) {
	    /* put the window to left end */
	    nwincol = 0;
	    ncurscol += curwksp->wcol;
	    movewin_flg = YES;
	} else {
	    /* move to the left */
	    nwincol -= delta_width;
	    ncurscol += delta_width;
	    movewin_flg = YES;
	}
    }
    /* 2 - vertical move ? */
    vsz = linw0 - (win0->bmarg - win0->tmarg -2);
    if ( vsz > 0 ) {
	/* cursor bellow the bottom of the window */
	nwinlin += vsz;
	ncurslin -= vsz;
	movewin_flg = YES;
    }
    /* 3 - position window and cursor now */
    hsz = win0->rmarg - win0->lmarg -2;
    if ( ncurscol > hsz ) ncurscol = hsz;
    if ( ncurscol < 0 ) ncurscol = 0;
    vsz = win0->bmarg - win0->tmarg -2;
    if ( ncurslin > vsz ) ncurslin = vsz;
    if ( ncurslin < 0 ) ncurslin = 0;
    if ( movewin_flg ) movewin (nwinlin, nwincol, ncurslin, ncurscol, YES);
    poscursor (ncurscol, ncurslin);
    curwin->ccol = cursorcol;
    curwin->clin = cursorline;
    if ( saved_curs && ( saved_curs_win == curwin) ) {
	update_saved_curs (saved_curs, cursorcol, cursorline);
    }

#if 0
    if ( oldwin == win0 || 1 ) {
	sprintf (buf, "movewin %d, cursorcol %d, cursorline %d, ncurscol %d, ncurslin %d",
		      movewin_flg, cursorcol, cursorline, ncurscol, ncurslin);
	mesg (ERRALL + 1, buf);
    }
#endif

    /* Switch to initial window if needed */
    if ( oldwin != win0 ) {
	switchwindow (oldwin);
	poscursor (col, lin);
    }
    putscr (0);
}
#endif /* VBSTDIO */


static get_size (int * width, int * height, Flag chk_min)
{
    extern void get_tt_size   (int *, int *);
    extern void get_term_size (int *, int *);

    *width  = term.tt_width;
    *height = term.tt_height;
    if ( chk_min ) get_tt_size (width, height);
    else get_term_size (width, height);
}

static void do_resize_screen (Flag msg_flg)
{
    int height, width;

    get_size (&width, &height, YES);
    resize_now (msg_flg, height, width);
}


void resize_screen ()
{
    resize_requested = YES;
    do_resize_screen (YES);
}

void set_alt_resize (void (* my_resize) (int, int, void *), void *para)
{
    alt_resize = my_resize;
    alt_resize_para = para;
}

void sig_resize (int num)
{
    int height, width;

    if ( replaying || recovering ) resize_allowed = NO;
    resize_requested = YES;
    if ( resize_allowed ) {
	do_resize_screen (!silent);
    } else {
	if ( alt_resize ) {
	    get_size (&width, &height, NO);
	    (*alt_resize) (width, height, alt_resize_para);
	}
    }
    (void) signal (num, sig_resize);
}

void testandset_resize (Flag set_flg, void *csv_curs, S_window * cwin)
{
    if ( replaying || recovering ) {
	resize_allowed = NO;
	saved_curs = NULL;
	saved_curs_win = NULL;
	return;
    }

    if ( set_flg ) {
	saved_curs = csv_curs;
	saved_curs_win = cwin;
    }
    if ( set_flg && resize_requested ) do_resize_screen (!silent);
    resize_allowed = set_flg;
    if ( ! set_flg ) {
	saved_curs = NULL;
	saved_curs_win = NULL;
    }
}
