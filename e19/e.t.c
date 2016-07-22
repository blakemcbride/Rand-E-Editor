#ifdef COMMENT
--------
file e.t.c
    terminal-independent input and output routines
#endif

#ifdef COMMENT
	Copyright abandoned, 1983, The Rand Corporation
#endif

#include "e.h"
#include "e.inf.h"
#include "e.m.h"
#include "e.cm.h"
#include "e.tt.h"
#ifdef LMCMARG
#include "e.wi.h"
#endif

#include <errno.h>

#ifdef  SYSSELECT
#include <sys/time.h>
#else
#include SIG_INCL
#endif

#ifdef __sun
#undef EMPTY
#endif


#ifdef TESTRDNDELAY
# undef EMPTY
# define RDNDELAY
# define O_RDONLY 0
# define O_NDELAY 0
# define F_SETFL  0
# define fcntl(a,b,c) 0;
#endif

#ifdef RDNDELAY
#include <fcntl.h>
#else
# define xread(a,b,c) read (a, b, c)
# ifdef EMPTY
#   define xempty(fd) empty(fd)
# else
#   define xempty(fd) 1
# endif
#endif

#ifdef _AIX
#include <sys/select.h>
#endif

#define EINTR   4       /* should be #included but for problems ... */

extern void testandset_resize ();
extern char *getparam (char *str, int *beg_pos, int *end_pos);
extern int check_keyword (char *, int, S_looktbl *, int, int *,
			  char **, int *, int *, Flag);

/* set outside putup (), looked at by putup () */
Flag entfstline = NO;           /* says write out entire first line */
Flag needputup; /* because the last one was aborted */
Flag atoldpos;
Slines  newcurline = -1;        /* what cursorline will be set to after */
Scols   newcurcol  = -1;        /* similar to newcurline                */
Flag    freshputup;             /* ignore previous stuff on these lines */
Flag    nodintrup;              /* disallow display interruption */
Scols putupdelta;               /* nchars to insert (delete) in putup() */

static Flag entering = NO;      /* set if e is in the param () routine. */
				/* looked at by getkey1() and quote_process */
static Flag ignore_quote_flg = NO;  /* set when the quote processing must no be apply */

extern void putupwin ();
extern void putup ();
extern void multchar ();
extern void dobullets ();
#ifdef LMCMARG
extern void tm_unbullet ();
extern void bm_unbullet ();
extern void setmarg ();
extern void stmarg0 ();
#endif
extern void setbul ();
extern void clrbul ();
extern void gotomvwin ();
extern void savecurs ();
extern void restcurs ();
extern void poscursor ();
extern void movecursor ();
extern void writekeys ();
extern void putch ();
extern void param ();
extern void exchgcmds ();
extern void getarg ();
extern void limitcursor ();
extern void rand_info ();
extern void mesg ();
extern void credisplay ();
extern void redisplay ();
extern void screenexit ();
       void tglinsmode ();
extern Flag vinsdel ();

static unsigned Short getkey1 ();
static char * history_get (Flag forward);


/* command line history handling */

static char history [2048];
static char * history_end = NULL;
static char * hist_write = NULL;    /* write into history pointer */
static char * hist_get = NULL;      /* where to get the next entry */

void history_init ()
{
    if ( hist_write ) return;
    memset (history, 0, sizeof (history));
    hist_write = &history[0];
    hist_get = &history [0];
    history_end = &history[sizeof (history) -1];
}


void history_dump (FILE *stfile)
{
    int sz;
    char *cmd1, *cmd2;

    if ( ! stfile ) return;
    sz = hist_write - history;
    if ( sz == 0 ) {
	putlong  ((long) sz, stfile);
	return;
    }

    hist_get = hist_write;
    cmd1 = history_get (NO);
    cmd2 = history_get (NO);
    if ( cmd1 && cmd2 ) {
	if ( strcmp (cmd1, cmd2) == 0 ) {
	    /* ignore the last command if it is already in history list */
	    cmd1 = history_get (YES);
	    hist_write = hist_get;
	    *hist_write = '\0';
	    sz = hist_write - history;
	}
    }
    putlong  ((long) sz, stfile);
    if ( sz > 0 ) {
	(void) fwrite (history, sizeof (history [0]), sz, stfile);
    }
}


void history_reload (FILE *stfile, Flag dump_state)
{
    int sz, i;
    char * cmd, *str;

    hist_write = NULL;
    history_init ();
    if ( ! stfile ) return;

    sz = (int) getlong (stfile);
    if ( sz <= 0 ) {
	if ( dump_state ) puts ( sz < 0 ? "No history data in state file"
					: "Empty history list");
	return;
    }
    if ( dump_state ) printf ("History buffer size : %d\n", sz);
    if ( sz > sizeof (history) ) {
	if ( dump_state ) printf ("    Too large history buffer (max %d), ignore it\n", sizeof (history));
	return;
    }
    (void) fread (history, sizeof (history [0]), sz, stfile);
    hist_write = history + sz;
    hist_get = &history [0];

    if ( dump_state ) {
	hist_get = hist_write;
	for ( i = 0 ; ; i++ ) {
	    str =  history_get (NO);
	    if ( ! str || ! *str ) break;
	    cmd = str;
	}
	for ( ;  i > 0 ; i-- ) {
	    printf ("  %3d : \"%s\"\n", i, cmd);
	    cmd = history_get (YES);
	    if ( ! cmd || ! *cmd ) break;
	}
    }
}


static history_store (char *cmd)
{
    unsigned int size, sz, pos;
    char *hpt;

    if ( ! cmd ) return;

    size = strlen (cmd);
    if ( (size <= 0) || (size >= (sizeof (history) -1)) ) return;

    /* do not duplicate the same command */
    hpt = hist_write;
    if ( hpt != history ) {
	for ( hpt -=2 ; *hpt ; hpt-- ) if ( hpt < history ) break;
	hpt++;
	if ( strcmp (cmd, hpt) == 0 ) return;
    }

    /* something to be pushed in history */
    if ( (hist_write + size) >= history_end ) {
	/* not enough room in history to store cmd */
	sz = hist_write - history;  /* used space */
	if ( size < sz ) {
	    /* need less room that used space */
	    sz = strlen (&history [size]) +1;
	    pos = sz + size;    /* what will be removed */
	    sz = hist_write - &history [pos];
	    (void) memmove (history, &history [pos], sz);
	    hist_write -= pos;
	    memset (hist_write, 0, pos);
	    hist_get -= pos;
	    if ( hist_get < &history [0] ) hist_get = &history [0];
	} else {
	    memset (history, 0, sizeof (history));
	    hist_write = &history[0];
	    hist_get = &history [0];
	}
    }
    memcpy (hist_write, cmd, size);
    hist_write += size;
    *hist_write = '\0';
    hist_write++;
    hist_get = hist_write;
}

static char * history_get (Flag forward)
{
    int sz;

    if ( forward ) {
	/* move forward in the command history list */
	sz = strlen (hist_get);
	if ( !sz ) return NULL;
	hist_get += sz +1;
    } else {
	/* move backward */
	if ( hist_get == history ) {
	    putchar ('\007');
	    return NULL;
	}
	for ( hist_get -=2 ; *hist_get ; hist_get-- )
	    if ( hist_get < history ) break;
	hist_get++;
    }
    return hist_get;
}

/* version 19.57 */
static void check_history_get (char * ccmdp)
{
    char *saved_hist_get, *hist_strg;

    if ( !ccmdp || !*ccmdp ) return;

    saved_hist_get = hist_get;
    hist_strg = history_get (NO);   /* get previous entry */
    if ( ! hist_strg ) return;

    if ( strcmp (ccmdp, hist_strg) != 0 )
	hist_get = saved_hist_get;  /* restaure history get pointer */
	/* else a mouve backward will not duplicate the command */
}

/* edited file list navigation handling */
/* ------------------------------------ */

static char fnavig_cmd [PATH_MAX +32];
static char *fnavig_buf = NULL;
static AFn fnavig_fn = 0;


static char * file_list_navigation (S_window * cwin)
{
    AFn delta, sv_fn;
    Flag down_flg;

    down_flg = ( key != CCMOVEUP ); /* Move down or file navigation */
    sv_fn = fnavig_fn;
    delta = down_flg ? 1 : -1;
    for ( fnavig_fn += delta ; ; fnavig_fn += delta ) {
	if ( (fnavig_fn < FIRSTFILE + NTMPFILES) || (fnavig_fn >= MAXFILES) ) {
	    if ( key != CCFNAVIG ) break;
	    /* else go around in the list */
	    if ( fnavig_fn >= MAXFILES )
		fnavig_fn = FIRSTFILE + NTMPFILES;  /* go around */
	    else    /* must be < mini */
		fnavig_fn = MAXFILES -1;
	}
	if ( ! (fileflags [fnavig_fn] & INUSE) ) continue;
	if ( fnavig_fn == cwin->wksp->wfile ) {
	    /* hide the active window edited file */
	    if ( key == CCFNAVIG ) {
		putchar ('\007');
		if ( fnavig_fn == sv_fn ) break; /* nothing can be found */
	    }
	    continue;
	}
#if 0
	/* hide also the active window alterneted file */
	if ( fnavig_fn == cwin->altwksp->wfile ) continue;
#endif
	if ( !names [fnavig_fn] || !*names [fnavig_fn] ) continue;
	strcpy ( fnavig_buf, names [fnavig_fn]);
	return (fnavig_cmd);
    }
    fnavig_fn = sv_fn;
    putchar ('\007');
    return NULL;
}



#ifdef COMMENT
void
putupwin ()
.
    Do a putup() of the whole current window.
#endif
void
putupwin ()
{
    char *mstrg;

    mstrg = NULL;
    savecurs ();
    putup (0, curwin->btext, 0, MAXWIDTH);

    if ( ! (fileflags[curfile] & FWRITEABLE) ) {
            mstrg = "Read Only file";
    }
    else if ( ! (fileflags[curfile] & CANMODIFY) ) {
	if ( ! loopflags.hold ) /* do not overwrite some error message */
            mstrg = "File cannot be modified";
    }
    if ( mstrg ) {
        mesg (TELALL + 1, mstrg);
	loopflags.hold = YES;
    }

    restcurs ();
    return;
}

#ifdef COMMENT
void
putup (ls, lf, strtcol, ncols)
    Slines  ls;
    Slines  lf;
    Scols   strtcol;    /* starting col for partial line redraw */
    Scols   ncols;      /* number of columns for partial line redraw */
.
    /*
    Puts up (displays) lines from 'ls' to 'lf'.
    'ls' and 'lf' are line numbers relative to the top of the current window,
    where 0 would be the first line of the window.
    If 'ls' is negative,
    only puts up line 'lf' and takes it from cline instead of disk.
    Start at column 'strtcol' (rel. to left column of window) on each line.
    If 'ncols' != MAXWIDTH then it is assumed that the line has not gotten
    shorter or longer since last putup, and you know what you are doing,
    and you only want to putup from 'strtcol' to 'strtcol' + 'ncols'.
    Caller must do a poscursor after calling putup.
    If the caller doesn't want the display of the borders to change, he
    clears the global "chgborders" before calling putup (..); If he wants
    dot borders for this putup, he sets chgborders to 2.
    If the caller sets the global `freshputup' flag,
    it means that the line on the screen is blank, and we will
    not need to put out any leading or trailing blanks, and we will have
    to redraw the borders.
    */
#endif
void
putup (ls, lf, strtcol, ncols)
Slines  ls;
Slines  lf;
Scols   strtcol;    /* starting col for partial line redraw */
Scols   ncols;      /* number of columns for partial line redraw */
{
    Reg3 Nlines ln;     /* the line worked on within the for loop */
    Reg6 Ncols  fc;
    Reg5 Ncols  lcol;   /* = curwksp->wcol */
    Reg8 Scols  rlimit; /* = curwin->rtext + 1 */
    Reg7 Scols  lstcol; /* rightmost col of text on line              */
    Reg4 Ncols  col;    /* variable col, reset to strtcol for each line */
    int     lmc;        /* left  margin char that should be on screen */
    int     rmc;        /* right margin char that should be on screen */
    Flag    usecline;   /* ls was < 0, so use cline */
    Flag    offendflg;  /* off the end of the file */

    if (strtcol > (lcol = curwksp->wcol) + (rlimit = curwin->rtext + 1))
	goto ret;
    /* if the caller knows that after the putup he will move the cursor
     * to someplace other than where it is, then he will set newcurline
     * and/or newcurcol to the new position
     * newcurline is always set to -1 on return from putup.
     **/
    if (newcurline == -1)
	newcurline = cursorline;
    if (newcurcol == -1)
	newcurcol = cursorcol;
    atoldpos = NO;
    if (!nodintrup && dintrup ()) {
	needputup = YES;
	goto ret;
    }
    else
	needputup = NO;

    if (lf > curwin->btext)
	lf = curwin->btext;
    if (usecline = ls < 0)
	ls = lf;
    lmc = lcol == 0 ? LMCH : MLMCH;

    for (ln = ls;  ln <= lf; ln++) {
	rmc = RMCH;
	offendflg = NO;

	if (!usecline) {        /* get the line into cline */
	    if (   ln != lf
		&& !entfstline
		&& ( replaying || (++intrupnum > DPLYPOL) )
		&& !nodintrup
		&& dintrup ()
	       ) {
		needputup = YES;
		break;
	    }
	    getline (curwksp->wlin + ln);
	    if (xcline) {
		offendflg = YES;
		lmc = ELMCH;
	    }
	    if (ln >= newcurline)
		atoldpos = YES;
	}

	/* deal with the left border */
	/* if this is the first line of the putup, back up and fix left    */
	/*   border as far up as necessary. */
	if (freshputup) {
	    poscursor (-1, ln);
	    putch (chgborders == 1 ? lmc : INMCH, NO);
	    curwin->lmchars[ln] = lmc;
	} else {
	    Reg1 Slines lt;
	    Slines lb;
	    if (ln == ls && lmc != ELMCH)
		lb = 0;
	    else
		lb = ln;
	    for (lt = ln; lt >= lb; lt--) {
		if ((curwin->lmchars[lt] & CHARMASK) == lmc)
		    break;
		if (   chgborders
		    && (   !borderbullets
			|| chgborders == 2
			|| lt != newcurline
		       )
		   ) {
		    poscursor (-1, lt);
		    putch (chgborders == 1 ? lmc : INMCH, NO);
		}
		curwin->lmchars[lt] = lmc;
	    }
	}

	/*  determine fc: first non-blank column in cline
	 *  if portion of line to be displayed is blank, then set fc
	 *  to rlimit
	 **/
	if (   offendflg
	    || (fc = fnbcol (cline, lcol, min (ncline, lcol + rlimit))) < lcol
	   )
	    fc = rlimit;
	else
	    fc -= lcol;

	/* how many leading blanks do we need to put out */
	if (entfstline) {
	    entfstline = NO;
	    col = 0;
	} else {
	    if (freshputup)
		col = fc;
	    else
		col = min (fc, curwin->firstcol[ln]);
	    if (col < strtcol)
		col = strtcol;
	}
	curwin->firstcol[ln] = fc;
	poscursor ((Scols) col, ln);
	/* if (stdout->_cnt < 100) */  /* smooth any outputting breaks  */
	    d_put (0);
	{
	    Reg1 Scols tmp;
	    if ((tmp = fc - col) > 0) {
		multchar (' ', tmp);
		col = fc;
	    }
	}

	/* determine the rightmost col of the new text to put up */
	/* this should do better on lines that go past right border */
	if (offendflg)
	    lstcol = 0;
	else {
	    Reg1 Ncols tmp;
	    tmp = (ncline - 1) - lcol;
	    if (tmp < 0)
		tmp = 0;
	    else if (tmp > rlimit) {
		tmp = rlimit;
		rmc = MRMCH;
	    }
	    lstcol = tmp;
	}
	if (lstcol < col)
	    lstcol = col;
	{
	    Reg1 Scols ricol;   /* rightmost col of text to put up */
	    ricol = min (lstcol, min (rlimit, strtcol + ncols));
	    /* put out the text */
	    if (ricol - col > 0) {
		/* ---------------------------
		if (putupdelta && col >= strtcol) {
		    d_hmove (putupdelta, (Short) (ricol - col),
			     &cline[lcol + col]);
		} else
		------------------------------ */
		    d_write (&cline[lcol + col], (Short) (ricol - col));
		cursorcol += (ricol - col);
	    }
	}
	/* determine how many trailing blanks we need to put out */
	{
	    Reg1 Scols tmp;
	    if (   !freshputup
		&& (tmp = (curwin->lastcol[ln] - lstcol)) > 0
	       )
		multchar (' ', tmp);
	}
	curwin->lastcol[ln] = (lstcol == fc) ? 0 : lstcol;

	/* what to do about the right border */
	if (freshputup) {
	    poscursor (curwin->rmarg - curwin->ltext, ln);
	    putch (chgborders == 1 ? rmc : INMCH, NO);
	    curwin->rmchars[ln] = rmc;
	} else if ((curwin->rmchars[ln] & CHARMASK) != rmc) {
	    if (  chgborders
		&& (   !borderbullets
		    || chgborders == 2
		    || ln != newcurline
		   )
	       ) {
		poscursor (curwin->rmarg - curwin->ltext, ln);
		putch (chgborders == 1 ? rmc : INMCH, NO);
	    }
	    curwin->rmchars[ln] = rmc;
	}
    }
ret:
    putupdelta = 0;
    newcurline = -1;
    newcurcol = -1;
    return;
}

#ifdef COMMENT
int
fnbcol (line, left, right)
    char *line;
    Ncols left;
    Ncols right;
.
    Look for a non-blank character in line between line[left] and line[right]
    If right < left, or a newline or non-blank is encountered,
	return left - 1
    else
	return index into array line of the non-blank char.
#endif
int
fnbcol (line, left, right)
Reg4 char *line;
Reg5 Ncols left;
Ncols right;
{
    Reg1 char *cp;
    Reg2 char *cpr;
    Reg3 char *cpl;

    if (right < left)
	return left - 1;

    /* this code is a little tricky because it looks for a premature '\n' */
    cpl = &line[left];
    cpr = &line[right];
    for (cp = line; ; cp++) {
	if (   *cp == '\n'
	    || cp >= cpr
	   )
	    return left - 1;
	if (   *cp != ' '
	    && cp >= cpl
	   )
	    return cp - line;
    }
    /* returns from inside for loop */
    /* NOTREACHED */
}

#ifdef COMMENT
void
multchar (chr, num)
    Char chr;
    Scols num;
.
    Write 'num' 'chr's to d_write.
    Advance cursorcol by 'num' columns.
#endif
void
multchar (chr, num)
char chr;
Scols num;
/*
Reg4 Char chr;
Reg1 Scols num;
*/
{
    Reg2 Small cnt;
    Reg3 char *buf;
    char cbuf[3];

    if (num <= 0)
	return;
    cursorcol += num;
    buf = cbuf;
    while (num > 0) {
	cnt = Z;
	if (num > 3) {
	    buf[cnt++] = VCCARG;
	    if (num > 95) {
		buf[cnt++] = 95 + 040;
		num -= 95;
	    }
	    else {
		buf[cnt++] = num + 040;
		num = Z;
	    }
	}
	else {
	    if (num > 2) {
		buf[cnt++] = chr;
		num--;
	    }
	    if (num > 1) {
		buf[cnt++] = chr;
		num--;
	    }
	    num--;
	}
	buf[cnt++] = chr;
	d_write (buf, cnt);
    }
    return;
}

static Scols  oldccol;
static Slines oldcline;

#ifdef COMMENT
void
offbullets ()
.
    Turn off border bullets on the sides of the window.
#endif
void
offbullets ()
{
    Reg1   Scols  occ;
    Reg2   Slines ocl;

    if (!borderbullets)
	return;
    occ = oldccol;
    oldccol = cursorcol;
    ocl = oldcline;
    oldcline = cursorline;
    poscursor (curwin->rtext + 1, ocl);
    putch (curwin->rmchars[ocl], NO);
    poscursor (-1, ocl);
    putch (curwin->lmchars[ocl], NO);
    poscursor (oldccol, oldcline);
}

#ifdef COMMENT
void
dobullets (allbullets, newbullets)
    Flag allbullets;
    Flag newbullets;
.
    Update the border bullets to the current cursor position.
    To minimize terminal cursor addressing, this routine has more
    code than would be necessary without the optimization.
#endif
void
dobullets (allbullets, newbullets)
Reg3 Flag allbullets;
Flag newbullets;
{
    Reg1   Scols  occ;
    Reg2   Slines ocl;

    if (!borderbullets)
	return;
    occ = oldccol;
    oldccol = cursorcol;
    ocl = oldcline;
    oldcline = cursorline;
    if (newbullets || (oldccol != occ)) {
	/* do top */
#ifdef LMCMARG
	if (occ < oldccol)
	    tm_unbullet (allbullets, occ, -1, TMCH, BTMCH, NO);
#else
	if (!allbullets && occ < oldccol) {
	    poscursor (occ, -1);
	    putch (TMCH, NO);
	}
#endif
	poscursor (oldccol, -1);
	putch (BULCHAR, NO);
#ifdef LMCMARG
	if (occ > oldccol)
	    tm_unbullet (allbullets, occ, -1, TMCH, BTMCH, NO);
#else
	if (!allbullets && occ > oldccol) {
	    poscursor (occ, -1);
	    putch (TMCH, NO);
	}
#endif
	/* do bottom */
#ifdef LMCMARG
	if (occ < oldccol)
	    bm_unbullet (allbullets, occ, curwin->btext + 1, BMCH, TTMCH, NO);
#else
	if (!allbullets && occ < oldccol) {
	    poscursor (occ, curwin->btext + 1);
	    putch (BMCH, NO);
	}
#endif
	poscursor (oldccol, curwin->btext + 1);
	putch (BULCHAR, NO);
#ifdef LMCMARG
	if (occ > oldccol)
	    bm_unbullet (allbullets, occ, curwin->btext + 1, BMCH, TTMCH, NO);
#else
	if (!allbullets && occ > oldccol) {
	    poscursor (occ, curwin->btext + 1);
	    putch (BMCH, NO);
	}
#endif
    }

    if (newbullets || (oldcline != ocl)) {
	/* do right side */
	if (!allbullets) {
	    poscursor (curwin->rtext + 1, ocl);
	    putch (curwin->rmchars[ocl], NO);
	}
	poscursor (curwin->rtext + 1, oldcline);
	putch (BULCHAR, NO);
	/* do left side */
	if (!allbullets) {
	    poscursor (-1, ocl);
	    putch (curwin->lmchars[ocl], NO);
	}
	poscursor (-1, oldcline);
	putch (BULCHAR, NO);
    }
    poscursor (oldccol, oldcline);
    d_put (0);
    return;
}

/*
 * trw - redo these two
 */
#ifdef LMCMARG
#ifdef COMMENT
void
tm_unbullet (allbullets, occ, tb, ordch, xordch, igntab)
    Flag allbullets, igntab;
    Ncols occ, tb;
    char ordch, xordch;
.
    tm_unbullet is called to move the top bullets. It essentially
    rewrites the margin character, based on the current tab data.
    If igntab is true, any tab at the character is ignored.
#endif
void
tm_unbullet (allbullets, occ, tb, ordch, xordch, igntab)
    Flag allbullets, igntab;
    Ncols occ, tb;
    char ordch, xordch;

{
    Ncols i;

    if (!allbullets) {
	    /*
	     * want occ > 0, so we don't display the tab mark at the
	     * first position
	     */
	if (!igntab && occ > 0) {
	    for (i = Z; i < ntabs && tabs[i] < curwksp->wcol + occ; i++) ;
	    if (tabs[i] - curwksp->wcol == occ && ntabs > 0 && i < ntabs)
		ordch = ordch + (BTMCH - BMCH);
	}
	poscursor (occ, tb);
	putch (ordch, NO);
    }
}

#ifdef COMMENT
void
bm_unbullet (allbullets, occ, tb, ordch, xordch, igntab)
    Flag allbullets, igntab;
    Ncols occ, tb;
    char ordch, xordch;
.
    bm_unbullet is called to move the bottom bullets. It essentially
    rewrites the margin character, based on current margin data.
    If igntab is true, any tab at the character is ignored.
#endif
void
bm_unbullet (allbullets, occ, tb, ordch, xordch, igntab)
    Flag allbullets, igntab;
    Ncols occ, tb;
    char ordch, xordch;

{
    Ncols i;

    if (!allbullets) {
	poscursor (occ, tb);
	putch (occ == linewidth - curwksp->wcol
#ifdef LMCAUTO
	       || (autolmarg && (occ == autolmarg - curwksp->wcol))
#endif
	       ? xordch : ordch, NO);
    }
}

#ifdef COMMENT
void
scvtab (pos, sc)
    Ncols pos;
    Flag sc;
.
    scvtab is called to set or reset a tab mark on the top
    margins. If sc is YES, the tab is reset.
#endif
void
scvtab (pos, sc)
    Ncols pos;
    Flag sc;

{
    if (pos > 0 ) {
	savecurs();
	stmarg0 (pos, sc, TOPMAR);
	restcurs();
    }
    return;
}

#ifdef COMMENT
void
stmarg0 (pos, igntab, marflag)
    Ncols pos;
    Flag igntab;
    Flag marflag;
.
    stmarg0 rewrites the top and/or bottom edges of a window at column pos. If
    igntab is YES then the existance of a tab on that column is ignored,
    thus effectively resetting it.  Marflag indicates which margins to
    update (1=top, 2=bot, 3=both);
#endif
void
stmarg0 (pos, igntab, marflag)
    Ncols pos;
    Flag igntab;
    Flag marflag;
{
    if (pos >= 0 && pos <= curwin->rtext)
    {
	if (marflag & TOPMAR)
	    tm_unbullet (NO, curwin->lmarg + pos, -1, TMCH, TTMCH, igntab);
	if (marflag & BOTMAR)
	    bm_unbullet (NO, curwin->lmarg + pos, curwin->bmarg - 1,
		    BMCH, BTMCH, igntab);
    }
    return;
}

#ifdef COMMENT
void
setmarg (old, new)
    Ncols *old, new;
.
    setmarg moves the left or right margin indicator by first removing the
    old, setting the new, and then setting the old to the new.
#endif
void
setmarg (old, new)
    Ncols *old, new;

{
    Ncols temp;

    savecurs();
    temp = *old;
    *old = new;
/*  stmarg0 (temp, NO, BOTMAR);
    stmarg0 (new, NO, BOTMAR);          */
    stmarg0 (temp, YES, BOTMAR);
    stmarg0 (new, YES, BOTMAR);
    restcurs();
    return;
}
#endif

#ifndef SYSSELECT
#ifdef COMMENT
int
bulalarm ()
.
    A null routine to catch the SIGALRM signal which may come in as part
    of the 1 second timeout for a bullet.
#endif
int
bulalarm ()
{
    return;
}
#endif

static int bulsave = -1;    /* saved character from setbul */

#ifdef COMMENT
void
setbul (wt)
    Flag wt;
.
    Set a bullet at the current cursor postion.
    If 'wt' is non-0, set the 1-second alarm to hold the bullet in place.
#endif
void
setbul (wt)
Flag wt;
{
    if (!term.tt_bullets)
	return;
    if (curfile != NULLFILE) {
	Reg1 Ncols charpos;
	getline (cursorline + curwksp->wlin);
	bulsave = (charpos = cursorcol + curwksp->wcol) < ncline - 1
		  ? (cline[charpos] & CHARMASK)
		  : ' ';
    }
    else
	bulsave = ' ';

    putch (BULCHAR, YES);
    movecursor (LT, 1);
    d_put (0);
    if (wt) {
#ifdef  SYSSELECT
	{
	    static struct timeval onesec = { 1, 0 };

	    getkey (WAIT_PEEK_KEY, &onesec);
	    /* either this read will time out
	     * or a key will have come in, in which case we want to clear
	     * bullet right away and reset the alarm.
	     **/
#else
	if (getkey (PEEK_KEY) == NOCHAR) {
	    (void) signal (SIGALRM, bulalarm);
	    alarm (1);
	    if (getkey (WAIT_PEEK_KEY) != NOCHAR)
		alarm (0);
	    /* either this read will be interrupted by the alarm,
	     * or a key will have come in, in which case we want to clear
	     * bullet right away and reset the alarm.
	     **/
#endif
	}
	clrbul ();
    }
    return;
}

#ifdef COMMENT
void
clrbul ()
.
    Clear the bullet that was placed at the current cursor position.
#endif
void
clrbul ()
{
    if (bulsave == -1)
	return;
    putch (bulsave, NO);
    movecursor (LT, 1);
    bulsave = -1;
    return;
}

#ifdef COMMENT
    All three of the following functions: gotomvwin(), vertmvwin(), and
    horzmvwin() must not return until they have called movewin, because
    movewin looks to see if a putup needs to be done after one was aborted.
#endif

#ifdef COMMENT
void
gotomvwin (number)
    Nlines  number;
.
    Move the window so that line 'number' is the distance of one +LINE
    down from the top of the window.
#endif
void
gotomvwin (number)
Reg1 Nlines number;
{
    Reg3 Scols  cc;
    Reg4 Nlines cl;
    Reg2 Small  defpl;
    Reg5 Ncols wincol;

    number = max (0, number);
    wincol = curwksp->wcol;
    if (curwin->btext > 1)
	defpl = defplline;
    else
	defpl = Z;
    if (number == 0) {
	wincol = 0;
	cc = Z;
    }
    else
	cc = cursorcol;
    cl = min (number, defpl);
    movewin (number - defpl, wincol, cl, cc, 1);
    return;
}

#ifdef COMMENT
Small
vertmvwin (nl)
    Nlines nl;
.
    Moves window nl lines down.  If nl < 0 then move up.
#endif
Small
vertmvwin (nl)
Reg2 Nlines nl;
{
    Reg3 Nlines winlin;
    Reg1 Nlines cl;

    winlin = curwksp->wlin;
    if (winlin == 0 && nl <= 0)
	nl = 0;
    if ((winlin += nl) < 0) {
	nl -= winlin;
	winlin = Z;
    }
    cl = cursorline - nl;

    if (abs (nl) > curwin->btext)
	cl = cursorline;
    else if (cl < 0)
	cl = Z;
    else if (cl > curwin->btext)
	cl = curwin->btext;

    return movewin (winlin, curwksp->wcol, cl, cursorcol, 1);
}

#ifdef COMMENT
Small
horzmvwin (nc)
    Ncols nc;
.
    Moves window 'nc' columns right.  If nc < 0 then move left.
#endif
Small
horzmvwin (nc)
Reg2 Ncols nc;
{
    Reg1 Ncols  cc;

    cc = cursorcol;
    if ((curwksp->wcol + nc) < 0)
	nc = -curwksp->wcol;
    cc -= nc;
    cc = max (cc, curwin->ledit);
    cc = min (cc, curwin->redit);
    return movewin (curwksp->wlin, curwksp->wcol + nc, cursorline, (Scols) cc,
			YES);
}

#ifdef COMMENT
Small
movewin (winlin, wincol, curslin, curscol, puflg)
    Nlines  winlin;
    Ncols   wincol;
    Slines  curslin;
    Scols   curscol;
    Flag    puflg;
.
    /*
    Move the window so that winlin and wincol are the upper left corner,
    and curslin and curscol are the cursor position.
    If TRACKSET in curwin, move altwksp too, but don't disturb its
    cursor postion.
    */
#endif
Small
movewin (winlin, wincol, curslin, curscol, puflg)
Reg4 Nlines  winlin;
     Ncols   wincol;
Reg5 Slines  curslin;
Reg6 Scols   curscol;
     Flag    puflg;
{
    Reg3 Small newdisplay;
    Reg4 Nlines vdist;      /* vertical distance to move */
    Reg4 Ncols  hdist;      /* horizontal distance to move */
    Reg2 S_wksp *cwksp = curwksp;

    winlin = max (0, winlin);
    wincol = max (0, wincol);
#ifdef LA_LONGFILES
    if (winlin + defplline < 0)
#else
    if ((long) winlin + defplline > LA_MAXNLINES)
#endif
	winlin = LA_MAXNLINES - defplline;
    curslin = max (0, curslin);
    curslin = min (curslin, curwin->btext);
    curscol = max (0, curscol);
    curscol = min (curscol, curwin->rtext);
    newdisplay = Z;

    {
	Reg1 S_wksp *awksp;
	awksp = curwin->altwksp;
	if (vdist = winlin - cwksp->wlin) {
	    if (curwin->winflgs & TRACKSET) {
		if ((awksp->wlin += winlin - cwksp->wlin) < 0)
		    awksp->wlin = 0;
#ifdef LA_LONGFILES
		else if (awksp->wlin + defplline < 0)
#else
		else if ((long) awksp->wlin + defplline > LA_MAXNLINES)
#endif
		    awksp->wlin = LA_MAXNLINES - defplline;
	    }
	    cwksp->wlin = winlin;
	    newdisplay |= WLINMOVED;
	}
	if (hdist = wincol - cwksp->wcol) {
	    if (   (curwin->winflgs & TRACKSET)
		&& (awksp->wcol += wincol - cwksp->wcol) < 0
	       )
		awksp->wcol = 0;
	    cwksp->wcol = wincol;
	    newdisplay |= WCOLMOVED;
	}
	if (   (curwin->winflgs & TRACKSET)
	    && (newdisplay & (WLINMOVED | WCOLMOVED))
	   ) {
	    awksp->clin = curslin;
	    awksp->ccol = curscol;
	}
    }

    if (newdisplay || needputup) {
	newcurline = curslin;
	newcurcol  = curscol;
	if (puflg) {
	    if (needputup)
		putupwin ();
	    else {
		if (   hdist == 0
		    && abs(vdist) <= curwin->btext
		    && curwksp->wlin < la_lsize (curlas)
		    && curwksp->wlin - vdist < la_lsize (curlas)
		   )
		    (void) vinsdel (0, -vdist, YES);
		else
		    putupwin ();
	    }
	}
    }
    if (cursorline != curslin)
	newdisplay |= LINMOVED;
    if (cursorcol != curscol)
	newdisplay |= COLMOVED;
#ifdef LMCMARG
    if (newdisplay & WCOLMOVED)
	drawborders (curwin, WIN_ACTIVE);
#endif
    poscursor (curscol, curslin);
    return newdisplay;
}

struct svcs {
    struct svcs *sv_lastsv;
    AScols  sv_curscol;
    ASlines sv_cursline;
} *sv_curs;


/* Update a saved cursor position (called from screen resize) */
void update_saved_curs (struct svcs *curs, AScols ccol, ASlines clin)
{
    if ( ! curs ) return;
    curs->sv_curscol  = ccol;
    curs->sv_cursline = clin;
}


#ifdef COMMENT
void
savecurs ()
.
    Save current cursor position.
    See restcurs().
#endif
void
savecurs ()
{
    Reg1 struct svcs *lastsv;

    lastsv = sv_curs;
    sv_curs = (struct svcs *) salloc (sizeof *sv_curs, YES);
    sv_curs->sv_lastsv = lastsv;
    sv_curs->sv_curscol = cursorcol;
    sv_curs->sv_cursline = cursorline;
    return;
}

#ifdef COMMENT
void
restcurs ()
.
    Save current cursor position.
    See savecurs().
#endif
void
restcurs ()
{
    Reg1 struct svcs *lastsv;

    poscursor (sv_curs->sv_curscol, sv_curs->sv_cursline);
    lastsv = sv_curs->sv_lastsv;
    sfree ((char *) sv_curs);
    sv_curs = lastsv;
    return;
}

#ifdef COMMENT
void
poscursor (col, lin)
    Scols  col;
    Slines lin;
.
    Position cursor.
    col is relative to curwin->ltext
    lin is relative to curwin->ttext
#endif
void
poscursor (col, lin)
Reg1 Scols  col;
Reg2 Slines lin;
{
    if (cursorline == lin) {
	/* only need to change column?  */
	switch (col - cursorcol) {  /* fortran arithmetic if!!      */
	case -1:
	    d_put (VCCLEF);
	    --cursorcol;
	case 0:               /* already in the right place   */
	    return;
	case 1:
	    d_put (VCCRIT);
	    ++cursorcol;
	    return;
	}
    }
    else if (cursorcol == col) {   /* only need to change line?    */
	switch (lin - cursorline) {
	    /* fortran arithmetic if!!  zero taken care of above     */
	case -1:
	    d_put (VCCUP);
	    --cursorline;
	    return;
	case 1:
	    d_put (VCCDWN);
	    ++cursorline;
	    return;
	}
    }
    cursorcol = col;
    cursorline = lin;

    /* the 041 below is for the terminal simulator cursor addressing */
    putscbuf[0] = VCCAAD;
    putscbuf[1] = 041 + curwin->ltext + col;
    putscbuf[2] = 041 + curwin->ttext + lin;
    d_write (putscbuf, 3);
    return;
}

#ifdef COMMENT
void
movecursor (func, cnt)
    Small   func;
    Nlines cnt;
.
    Move cursor within boundaries of current window.
    Type of motion is specified by 'func'.
#endif
void
movecursor (func, cnt)
Small func;
Reg4 Nlines cnt;
{
    Reg3 Slines lin;
    Reg2 Ncols col;

    lin = cursorline;
    col = cursorcol;
    switch (func) {
    case 0:                     /* noop                             */
	break;
    case HO:                    /* upper left-hand corner of screen */
	col = Z;
	lin = Z;
	break;
    case UP:                    /* up one line                      */
	lin -= cnt;
	break;
    case RN:                    /* left column and down one */
	if (autofill)
	    col = autolmarg - curwksp->wcol;
	else col = curwin->ledit;
    case DN:                    /* down one line */
	lin += cnt;
	break;
    case RT:                    /* forward one space */
	col += cnt;
	break;
    case LT:                    /* back space (non-destructive) */
	col -= cnt;
	break;
    case TB:                    /* tab forward to next stop */
	{
	    Reg1 Short  i;
	    for (i = Z, col += curwksp->wcol; i < ntabs; i++)
		if (tabs[i] > col) {
		    if (tabs[i] <= curwksp->wcol + curwin->rtext)
			col = tabs[min (ntabs - 1, i + cnt - 1)];
		    break;
		}
	}
	col -= curwksp->wcol;
	break;
    case BT:                    /* tab back to previous stop        */
	{
	    Reg1 Short  i;
	    for (i = ntabs - 1, col += curwksp->wcol; i >= 0; i--)
		if (tabs[i] < col) {
		    if (tabs[i] >= curwksp->wcol)
			col = tabs[max (0, i - (cnt - 1))];
		    break;
		}
	}
	col -= curwksp->wcol;
	break;
    }

    {
	Reg5 Nlines ldif;
	Reg6 Ncols  cdif;
	if ((cdif = col - curwin->ledit) < 0)
	    col = curwin->ledit;
	else if ((cdif = col - curwin->redit) > 0)
	    col = curwin->redit;
	else
	    cdif = NO;

	if ((ldif = lin - curwin->tedit) < 0)
	    lin = curwin->tedit;
	else if ((ldif = lin - curwin->bedit) > 0)
	    lin = curwin->bedit;
	else
	    ldif = NO;

	if ((ldif || cdif) && func && (curwin->lmchars))
	    movewin (curwksp->wlin + ldif,
		     curwksp->wcol + cdif,
		     lin, (Scols) col, YES);
	else
	    poscursor ((Scols) col, lin);
    }
    return;
}

/* GetCccmd : found a Ctrl char allocated to <cmd> key function */
/* ------------------------------------------------------------ */

static int cccmd_ctrl = -1;

int GetCccmd ()     /* to be called during initialisation */
{
    char ch, cmd[256];
    int lnb, nb;

    if ( cccmd_ctrl >= 0 ) return (cccmd_ctrl);

    for ( ch = ('Z' & '\037') ; ch > 0 ; ch-- ) {
	cmd[0] = ch; cmd[1] = '\0';
	lnb = 1;
	nb = (*kbd.kb_inlex) (cmd, &lnb);
	if ( (nb == 1) && (cmd[0] == CCCMD) ) {
	    cccmd_ctrl = ch;
	    break;
	}
    }
    return (cccmd_ctrl);
}

/* capture a quoted char : a control char or any printing char */
#ifdef  SYSSELECT
static unsigned Short quote_process (Flag *knockdown_pt, unsigned Short *quoted_key_pt, Flag peekflg, struct timeval *timeout)
#else
static unsigned Short quote_process (Flag *knockdown_pt, unsigned Short *quoted_key_pt, Flag peekflg)
#endif
{
    char *endptr, *mesg_str;
    int i, base;
    long int ch_code;
    unsigned Short rkey;
    unsigned char val [6];

    if ( !entering && loopflags.hold ) {
	loopflags.hold = NO;
	mesg (TELALL);
    }

    mesg_str = NULL;
    memset (val, 0, sizeof (val));
    i = 0;
    for ( ; ; ) {
#ifdef  SYSSELECT
	rkey = getkey1 (peekflg, timeout);
#else
	rkey = getkey1 (peekflg);
#endif

	if ( rkey == CCINT ) {
	    /* interrupt the quoted char processing */
	    *knockdown_pt = NO;
	    break;
	} else {
	    rkey &= 0377;
	    if ( (i == 0) && (rkey >= 0100) && (rkey < 0177) ) {
		/* a control char */
		*quoted_key_pt = rkey & 0337;
		rkey = CCCTRLQUOTE;
		break;
	    } else if ( isxdigit (rkey) || (rkey == 'x') || (rkey == 'X') ) {
		/* capture a char by his code */
		val [i++] = rkey;
		if ( i >= 4 ) break;    /* no more room */
		if ( (i >= 3) && (val [0] != '0') ) break;  /* completed decimal value */
		continue;
	    } else if ( (i > 0) && ((rkey == ' ') || (rkey == CCRETURN)) ) {
		/* complete the capture */
		break;
	    } else {
		val [i] = rkey;
		mesg_str = " : Invalid : alpha char or octal, decimal or hexadecimal number";
		goto valerr;
	    }
	}
    }
    if ( (rkey != CCCTRLQUOTE) && (rkey != CCNULL)  && (rkey != CCINT) ) {
	if ( ! isdigit (val [0]) ) {
	    mesg_str = " : Invalid char by code, must be like : 0123 83 or 0x53";
	    goto valerr;
	} else {
	    if ( val [0] != '0' ) base = 10;
	    else base = (toupper (val [1]) == 'X') ? 16 : 8;
	    ch_code = strtol (val, &endptr , base);
	    if ( *endptr == '\0' ) {
#ifdef CHAR7BITS
		if ( (ch_code < 0) || (ch_code >= 0177) ) {
		    mesg_str = "Invalid ASCII character code (out of range)";
#else
		if ( (ch_code < 0) || (ch_code >= 0400)
		     || ((ch_code >= 0177) && (ch_code < 0240)) ) {
		    mesg_str = "Invalid ISO 8859 character code (out of range)";
#endif
		    goto valerr;
		}
		rkey = (unsigned Short) ch_code;
		if ( rkey <= 037 ) {
		    /* a control char */
		    *quoted_key_pt = rkey | 0100;
		    rkey = CCCTRLQUOTE;
		} else {
		    /* a printable char */
		    *knockdown_pt = NO;
		}
	    } else {
		mesg_str = "Invalid ISO 8859 character code, must be like : 0123 83 or 0x53";
		goto valerr;
	    }
	}
    }
    if ( mesg_str ) {
valerr:
	if ( entering ) mesg (TELDONE);
	if ( val[0] )
	    mesg (ERRALL + 4, "\"", val, "\"", mesg_str);
	else
	    mesg (ERRALL + 1, mesg_str);

	loopflags.hold = YES;
	loopflags.clear = NO;
	rkey = entering ? CCINT : CCNULL;
	*knockdown_pt = NO;
    }
    return rkey;
}

void ignore_quote ()
{
    ignore_quote_flg = YES;
}

#ifdef  SYSSELECT
#ifdef COMMENT
unsigned Short
getkey (peekflg, tiemout)
    Flag peekflg;
    struct timeval *timeout;
.
    Read another character from the input stream.  If the last character
    wasn't used up (keyused == NO) don't read after all.
.
    peekflg is one of
      WAIT_KEY      wait for a character, ignore interrupted read calls.
      PEEK_KEY      peek for a character
      WAIT_PEEK_KEY wait for a character, then peek at it;
		    if read times out, return NOCHAR.
#endif
unsigned Short
getkey (peekflg, timeout)
Flag peekflg;
struct timeval *timeout;
#else
#ifdef COMMENT
unsigned Short
getkey (peekflg)
    Flag peekflg;
.
    Read another character from the input stream.  If the last character
    wasn't used up (keyused == NO) don't read after all.
.
    peekflg is one of
      WAIT_KEY      wait for a character, ignore interrupted read calls.
      PEEK_KEY      peek for a character
      WAIT_PEEK_KEY wait for a character, then peek at it;
		   if read is interrupted, return NOCHAR.
#endif
unsigned Short
getkey (peekflg)
Flag peekflg;
#endif
{
    static unsigned Short quoted_key;
    static Flag knockdown = NO;
    unsigned Short rkey;
    Flag qp_flg;

    qp_flg = ignore_quote_flg;
    ignore_quote_flg = NO;

    if ( (peekflg == WAIT_KEY) && (keyused == NO) )
	return key; /* then getkey is really a no-op */

    if ( knockdown && (peekflg == WAIT_KEY)) {
	knockdown = NO;
	keyused = NO;
	key = quoted_key;
	return key;
    }

#ifdef  SYSSELECT
    rkey = getkey1 (peekflg, timeout);
#else
    rkey = getkey1 (peekflg);
#endif

#if 0
    /* ------- old version <= E19.57 ------------- */
    if (knockdown && rkey < 040)
	rkey |= 0100;
    if (peekflg != WAIT_KEY)
	return rkey;
    knockdown = rkey == CCCTRLQUOTE;
    keyused = NO;
    return key = rkey;
    /* ------- old version <= E19.57 ------------- */
#endif

    if ( peekflg != WAIT_KEY ) {
	return rkey;
    }

    keyused = NO;
    knockdown = (rkey == CCCTRLQUOTE) && !qp_flg;
    if ( knockdown ) {
#ifdef  SYSSELECT
	rkey = quote_process (&knockdown, &quoted_key, peekflg, timeout);
#else
	rkey = quote_process (&knockdown, &quoted_key, peekflg);
#endif
    }
    return key = rkey;
}

#ifdef  SYSSELECT
#ifdef COMMENT
static unsigned Short
getkey1 (peekflg, timeout)
    Small peekflg;
    struct timeval *timeout;
.
    Return the next key from the input stream.
    Write the key to the keystroke file unless it is CCINT, which will NOT
    be written to the keyfile by this routine.  The caller will write it
    there if and only if it actually interrupted something.
    See getkey() for the function of peekflg.
    if peekflg is WAIT_PEEK_CHAR, then wait for timeout microseconds.
#endif
static unsigned Short
getkey1 (peekflg, timeout)
Small peekflg;
struct timeval *timeout;
#else
#ifdef COMMENT
static unsigned Short
getkey1 (peekflg)
    Small peekflg;
.
    Return the next key from the input stream.
    Write the key to the keystroke file unless it is CCINT, which will NOT
    be written to the keyfile by this routine.  The caller will write it
    there if and only if it actually interrupted something.
    See getkey() for the function of peekflg.
#endif
static unsigned Short
getkey1 (peekflg)
Small peekflg;
#endif
{
    static Short lcnt;
    static int   lexrem;        /* portion of lcnt that is still raw */
    static Uchar chbuf[NREAD];
    static Uchar *lp;

    char fmsg [128];

    if ( replaying ) {
	static Small replaydone = 0;
	if ( replaydone ) {
	    extern void resize_screen ();
 finishreplay:
	    close (inputfile);
	    inputfile = STDIN;
	    if (silent) {
		silent = NO;
		(*term.tt_ini1) (); /* not d_put(VCCICL) because fresh() */
				    /* follows */
		windowsup = YES;
		fresh ();
	    }
	    mesg (ERRALL + 1,
		  recovering
		  ? "Recovery completed, please exit and restart the editor."
		  : replaydone == 1
		    ? "Replay completed, please exit and restart the editor."
		    : "Replay aborted, please exit and restart the editor."
		 );
	    d_put (0);
	    replaying = NO;
	    recovering = NO;
	    replaydone = 0;
	    /* Resize to the current screen size, and allows resizing */
	    resize_screen ();
	    testandset_resize (YES, NULL, NULL);
	    goto nonreplay;
	}
	if (   !recovering
	    && !xempty (STDIN) /* any key stops replay */
	   ) {
	    lcnt = 0;
	    replaydone = 2;
	    while (xread (STDIN, chbuf, NREAD) > 0 && !xempty (STDIN))
		continue;
	    goto endreplay;
	}
	if (lcnt <= 0) {
	    static Uchar charsaved;
	    static Uchar svchar;
	    d_put (0);
	    for (;;) {
		Reg1 Uchar *cp;
		/* note that keystroke file is in canonical format and
		   doesn't have to go through inlex */
		cp = lp = chbuf;
		if (charsaved)
		    *cp++ = svchar;
		if ((lcnt = xread (inputfile, (char *) cp, NREAD - charsaved))
		    >= 0) {
		    if (lcnt > 0) {
			svchar = cp[lcnt - 1];
			if (!charsaved) {
			    charsaved = 1;
			    if (--lcnt == 0)
				continue;
			}
		    }
		    break;
		}
		if (errno != EINTR) {
		    sprintf (fmsg, "Error reading input (errno=%d).", errno);
		    fatal (FATALIO, fmsg);
		}
	    }
	}
	if (   lcnt == 0
#ifdef UNSCHAR
	    || *lp == CCSTOP
#else
	    || (*lp & CHARMASK) == CCSTOP
#endif
	   ) {
	    replaydone = 1;
 endreplay:
	    if ( ! entering )
		goto finishreplay;
	    else {
		*chbuf = CCINT;
		lp = chbuf;
		lcnt = 1;
	    }
	}
    }
    else {
 nonreplay:
	if (lcnt < 0)
	    fatal (FATALBUG, "lcnt < 0");
	if (   (lcnt - lexrem == 0 && peekflg != PEEK_KEY)
	    || (lcnt < NREAD && !xempty (inputfile))
	   ) {
	    /* read some more */
	    if (lcnt == 0)
		lp = chbuf;
	    else if (lp > chbuf) {
		if (lcnt == 1)
		    /* handle frequent case more efficiently */
		    *chbuf = *lp;
		else
		    move (lp, (char *) chbuf, (Uint) lcnt);
		lp = chbuf;
	    }
	    d_put (0);
#ifdef  SYSSELECT
	    {
		/* XXXXXXXXXXXXXXXXXXXXXXXXXXXX */
		fd_set readmask, z;
		FD_ZERO (&z);
		FD_ZERO (&readmask);
		FD_SET (inputfile, &readmask);

		/*
		int z = 0;
		int readmask = 1 << inputfile;
		*/
		if (   peekflg == WAIT_PEEK_KEY
		    && select (inputfile + 1, &readmask, &z, &z, timeout) <= 0
		   )
		    return NOCHAR;
	    }
#endif
	    do {
		Reg3 int nread;
		nread = NREAD - lcnt;
		if ((nread = xread (inputfile, (char *) &chbuf[lcnt], nread))
		    > 0) {
		    Reg4 Uchar *stcp;
		    stcp = &chbuf[lcnt -= lexrem];
		    lexrem += nread;
		    if ((nread = (*kbd.kb_inlex) (stcp, &lexrem)) > 0) {
			Reg1 Uchar *cp;
			Reg2 int nr;
			cp = &stcp[nread];
			/* look ahead for interrupts */
			nr = nread;
			do {
			    if (*--cp == CCINT)
				break;
			} while (--nr);
			if ((nr = cp - stcp) > 0) {
			    /* found CCINT character in chars just read */
			    /* and some characters in front of it are to be */
			    /* thrown away */
			    lp += lcnt + nr;
			    lcnt = nread - nr + lexrem;
			}
			else
			    lcnt += nread + lexrem;
		    }
		    else
			lcnt += lexrem;
		}
		else if (nread < 0) {
		    if (errno != EINTR) {
			sprintf (fmsg, "Error reading input (errno=%d).", errno);
			fatal (FATALIO, fmsg);
		    }
		    if (peekflg == WAIT_PEEK_KEY)
			break;
		}
		else
		    fatal (FATALIO, "Unexpected EOF in key input.");
	    } while (lcnt - lexrem == 0);
	}
    }

    if (   lcnt - lexrem <= 0
	&& peekflg != WAIT_KEY
       )
	return NOCHAR;

    if (peekflg != WAIT_KEY)
#ifdef UNSCHAR
	return *lp;
#else
	return *lp & CHARMASK;
#endif

    {
	Reg1 unsigned Short rchar;
#ifdef UNSCHAR
	rchar = *lp++;
#else
	rchar = *lp++ & CHARMASK;
#endif
	lcnt--;
	if (keyfile != NULL && rchar != CCINT) {
	    putc (rchar, keyfile);
	    if (numtyp > MAXTYP) {
		flushkeys ();
		numtyp = 0;
	    }
	}
	return rchar;
    }
}

#ifdef RDNDELAY

/*  These routines adapt the late-model read() with O_NDELAY
 *  to look like an empty call and a blocking read call.
 *  The implementation assumes that at most two fd's will be used this way.
 **/

struct rdbuf {
    int  rd_nread;
    int  rd_ndelay;
    int  rd_fd;
    char rd_char;
 };
static struct rdbuf rd0, rdx;
extern struct rdbuf *getrdb ();

#ifdef COMMENT
int
xempty (fd)
    int fd;
.
    return YES if read on fd would block, else NO.
#endif
int
xempty (fd)
int fd;
{
    Reg1 struct rdbuf *rdb;

    rdb = getrdb (fd);

    if (rdb->rd_nread > 0)
	return NO;
    if (!rdb->rd_ndelay) {
	if (fcntl (fd, F_SETFL, O_NDELAY) == -1)
	    fatal(FATALBUG, "Can't set ndelay");
	rdb->rd_ndelay = YES;
    }
    return (rdb->rd_nread = read (fd, &rdb->rd_char, 1)) <= 0;
}

#ifdef COMMENT
int
xread (fd, buf, count)
    int fd;
    char *buf;
    int count;
.
    blocking read.
#endif
int
xread (fd, buf, count)
int fd;
char *buf;
int count;
{
    int retval;
    Reg1 struct rdbuf *rdb;

    rdb = getrdb (fd);

    if (rdb->rd_nread > 0) {
	buf[0] = rdb->rd_char;
	count--;
    }
    else if (rdb->rd_ndelay) {
	if (fcntl (fd, F_SETFL, 0) == -1)
	    fatal (FATALBUG, "Can't clr ndelay");
	rdb->rd_ndelay = NO;
    }
    retval = rdb->rd_nread + read (fd, &buf[rdb->rd_nread], count);
    rdb->rd_nread = 0;
    return retval;
}

#ifdef COMMENT
struct rdbuf *
getrdb (fd)
    int fd;
.
    Return a pointer to the appropriate rdbuf structure for the fd.
#endif
struct rdbuf *
getrdb (fd)
int fd;
{
    Reg1 struct rdbuf *rdb;

    if (fd == STDIN) {
	rdb = &rd0;
	rd0.rd_fd = fd;
    }
    else {
	if (fd != rdx.rd_fd && rdx.rd_fd != 0)
	    fatal (FATALBUG, "empty bug in e.t.c");
	rdb = &rdx;
	rdx.rd_fd = fd;
    }
    return rdb;
}

#endif

#ifdef COMMENT
void
writekeys (code1, str, code2)
    Char    code1;
    char   *str;
    Char    code2;
.
    Write code1, then str, then code2 to the keys file.
#endif
void
writekeys (code1, str, code2)
Char    code1;
char   *str;
Char    code2;
{
    putc (code1, keyfile);
    fputs (str, keyfile);
    putc (code2, keyfile);
    flushkeys ();
    return;
}

#ifdef COMMENT
Flag
dintrup ()
.
    Look ahead for certain keys which will interrupt a putup.
    Return YES if such a key is found, else NO.
#endif
Flag
dintrup ()
{
    Reg1 unsigned Short ichar;
     static struct timeval nulsec = { 0, 0 };

    intrupnum = 0;

    ichar = getkey (PEEK_KEY, nulsec);
    switch (ichar) {
	case CCOPEN:
	case CCCLOSE:
	    return atoldpos;

	/* NOTE: so far, all functions that appear here call horzmvwin(),
	 *       vertmvwin() when it is their turn to actually be performed,
	 *       and movewin does a putup if the last one was aborted.
	 **/
	case CCLWINDOW:
	case CCRWINDOW:
	case CCMIPAGE:
	case CCPLPAGE:
	case CCMILINE:
	case CCPLLINE:
	    return YES;

	case CCMOVEUP:
	    if (newcurline == curwin->tedit)
		return YES;
	    break;

	case CCMOVEDOWN:
	    if (newcurline == curwin->bedit)
		return YES;
	    break;

	case CCMOVELEFT:
	    if (newcurcol == curwin->ledit)
		return YES;
	    break;

	case CCMOVERIGHT:
	    if (newcurcol == curwin->redit)
		return YES;
	    break;

	case NOCHAR:
	case CCDEL:     /* should this be here? */
	case CCDELCH:
	case CCCMD:
	case CCINSMODE:
	case CCMISRCH:
	case CCPLSRCH:
	case CCSETFILE:
	case CCCHWINDOW:
	case CCPICK:
	case CCUNAS1:
	case CCREPLACE:
	case CCMARK:
	case CCINT:
	case CCTABS:
	case CCCTRLQUOTE:
	case CCBACKSPACE:
	default:
	    break;
    }
    return NO;
}

#ifdef COMMENT
Flag
la_int()
.
    If intok == 0, return 0, else return sintrup ().
#endif
Flag
la_int()
{
    if (intok)
	return sintrup ();
    else
	return 0;
}

#ifdef COMMENT
Flag
sintrup ()
.
    Look ahead for certain keys which will interrupt a search or any
    subprogram, e.g. a "run", "fill", etc.
    Return YES if such a key is found, else NO.
#endif
Flag
sintrup ()                        /* Whether to intrup search/exec      */
{
     static struct timeval nulsec = { 0, 0 };

    intrupnum = 0;                /* reset the counter                  */
    if (getkey (PEEK_KEY, nulsec) == CCINT) {
	putc (CCINT, keyfile);
	keyused = YES;
	getkey (WAIT_KEY, nulsec);
	keyused = YES;
	return YES;
    }
    return NO;
}

#ifdef COMMENT
void
putch (chr, flg)
#ifdef UNSCHAR
    Uchar chr;
#else
    Uint chr;
#endif
    Flag    flg;
.
    /*
    Put a character up at current cursor position.
    The character has to be a space, a printing char, a bell or an 0177.
    Can't be negative.
    If 'flg' is non-0, then 'chr' is being put into a display window,
    and its position may be before or after existing printing characters
    on the line.  This must be noted for putup ().
    'flg' is is only YES in 3 places: 2 in printchar and 1 in setbul ().
    */
#endif
void
putch (chr, flg)
#ifdef UNSCHAR
Uchar chr;
#else
Reg1 int chr;
#endif
Flag    flg;
{
#ifndef UNSCHAR
    chr &= CHARMASK;
#endif
    if (chr < 040) {
	d_put (chr);
	return;
    }
    if (flg && chr != ' ') {
	if (curwin->firstcol[cursorline] > cursorcol)
	    curwin->firstcol[cursorline] = cursorcol;
	if (curwin->lastcol[cursorline] <= cursorcol)
	    curwin->lastcol[cursorline] = cursorcol + 1;
    }

    /*  Adjust cursorcol, cursorline for edge effects of screen.
	Sets cursorcol, cursorline to correct values if they were
	positioned past right margin.
	If cursor was incremented from bottom right corner of screen do not
	put out a character since screen would scroll, but home cursor.
     **/
    if (curwin->ltext + ++cursorcol > term.tt_width) {
	cursorcol = -curwin->ltext;  /* left edge of screen */
	if (curwin->ttext + ++cursorline >= term.tt_height) {
	    cursorline = -curwin->ttext;
	    d_put (VCCHOM);
	}
	else
	    d_put (chr);
	poscursor (cursorcol < curwin->ledit
		   ? curwin->ledit
		   : cursorcol > curwin->redit
		     ? curwin->redit
		     : cursorcol,
		   cursorline < curwin->tedit
		   ? curwin->tedit
		   : cursorline > curwin->bedit
		     ? curwin->bedit
		     : cursorline);
    }
    else
	d_put (chr);
    return;
}

/* static variables used for the command line display and edit */
/* ----------------------------------------------------------- */ 

extern char *pkarg ();

static S_window *msowin;
static Scols  msocol;
static Slines msolin;

#define  LPARAM 512     /* length of free increment of command buffer */
#define  CMDEND 1024    /* large enought cursor displacement to be at the end of command */

static char *ccmdp;
static Short ccmdl = 0; /* allocated space in *ccmdp */
static Short ccmdlen;

static Short ccmdpos;   /* insertion position */
static Scols ccmdbg;    /* begining of the command line string */
static int   ccmdoffset;/* offset in cmd string of the 1st visible char */


static char *lcmdp;
static Short lcmdl = 0; /* allocated space in *lcmdp */
static Short lcmdlen;

/* get a copy of the current command line string */
/* --------------------------------------------- */

int getccmd (char *strg, int strg_sz, int *pos_pt)
{
    int sz;

    if ( !strg ) return 0;
    if ( strg_sz <= 0 ) return 0;
    memset (strg, 0, strg_sz);

    ccmdp [ccmdlen] = '\0';
    strncpy (strg, ccmdp, strg_sz -1);
    if ( pos_pt ) *pos_pt = ccmdpos;
    sz = strlen (strg);
    return sz;
}


static const char cmd_prompt [] = "CMD: ";
static const char cmds_prompt[] = "CMDS:";
static int cmd_prompt_sz = sizeof (cmd_prompt) -1;
/* the size of cmd_prompt must be compatible with the size of "CMDS:" */

void cmds_prompt_mesg ()
{
    mesg (TELALL + 1, cmds_prompt);
}


#ifdef WIN32
/* generate a clear command line control character string */
char *clr_ccmd () {
    static char cmd_clear[] = "\01 \01\00\01\00";

    extern char cmdCharacter ();

    if ( ! cmd_clear[3] ) {
        cmd_clear[0] = cmd_clear[2] = cmd_clear[4] = cmdCharacter (CCCMD);
        cmd_clear[3] = cmdCharacter (CCBACKSPACE);
        cmd_clear[5] = cmdCharacter (CCDELCH);
    }
    return (cmd_clear);
}

/* Error in paste from clipboard defered handling */

static char *ccpaste_err_msg = NULL;

void clear_pasteErrorMsg () {
    ccpaste_err_msg = NULL;
}

void pasteErrorMsg () {
    if ( ccpaste_err_msg ) mesg (ERRALL +1, ccpaste_err_msg);
    ccpaste_err_msg = NULL;
}
#endif /* WIN32 */


/* routines to display and input within the current command display line */
/* --------------------------------------------------------------------- */

static void move_cursor (delta, refresh)   
    int delta;      /* cursor displacement */
    Flag refresh;   /* force redisplay of command string */
{
    Scols colini, col;
    Flag redisplay;
    char *str;
    int nb, nbsp, offset;

    redisplay = NO;
    ccmdp[ccmdlen] = '\0';
    colini = cursorcol;
    col = colini + delta;

    if ( col >  enterwin.redit ) {
        col = enterwin.redit;
    }
    else if ( col < ccmdbg ) {
        col = ccmdbg;
    }

    if ( ccmdpos > ccmdlen ) {
        ccmdpos = ccmdlen;
    }
    else if ( ccmdpos < 0 ) {
        ccmdpos = 0;
        col = ccmdbg;
    }

    offset = ccmdpos - (col - ccmdbg);
    if ( offset >= ccmdlen ) {
        offset = ccmdlen - (enterwin.redit - ccmdbg);    /* can be < 0 */    
    }
    if ( offset < 0 ) {
        offset = 0;
    }

    col = ccmdpos - offset + ccmdbg;
    redisplay = ( offset != ccmdoffset);

    if ( refresh || redisplay ) {
        ccmdoffset = offset;
        str = ccmdp + offset;
        nb = ccmdlen - offset;
        if ( (nb + ccmdbg) > (enterwin.redit +1) ) nb = enterwin.redit +1 - ccmdbg;
        nbsp = term.tt_width - nb - ccmdbg;
        poscursor (ccmdbg, cursorline);
        for ( ; nb > 0 ; nb -- ) putch (*str++, NO);
        if ( nbsp > 0 ) multchar (' ', nbsp);
    }
    poscursor (col, cursorline);
}


/* display the command string and put the cursor at the end */
static void putmsg () 
{
    move_cursor (CMDEND, YES);   
}


static void putch_cmd (chr) 
    char chr;
{
    ccmdp[ccmdlen] = '\0';
    if ( cursorcol < enterwin.redit )
        putch (chr, NO);
    else 
        move_cursor (1, YES);   
}

/* cmd_insert : insert a string in the command string at current position */
/* ---------------------------------------------------------------------- */

int cmd_insert (int wlen, char *name_expantion)
{
    int i;

    if ( wlen <= 0 ) return 0;

    /* make sure allocated space is big enough */
    if (ccmdlen + wlen >= ccmdl)
	ccmdp = gsalloc (ccmdp, ccmdlen, ccmdl = ccmdlen + wlen + LPARAM, YES);
    /* make room for insertion */
    if ((i = ccmdlen - ccmdpos) > 0)
	memmove (&ccmdp[ccmdpos + wlen], &ccmdp[ccmdpos], i);
    /* insert the string and update the screen */
    memmove (&ccmdp[ccmdpos], name_expantion, wlen);
    ccmdlen += wlen;
    ccmdp[ccmdlen] = '\0';
    ccmdpos += wlen;
    move_cursor (wlen, YES);
    return wlen;
}

/* cmd_fname_expantion : insert a file name extension (do not double '/') */
/* ---------------------------------------------------------------------- */

int cmd_fname_expantion (int wlen, char *name_expantion)
{
    int wl;

    if ( wlen <= 0 ) return 0;

    wl = wlen;
    if ( (name_expantion[wl-1] == '/') && (ccmdp[ccmdpos] == '/') ) wl--;
    return (cmd_insert (wl, name_expantion));
}


/* cmd_delete : remove char from the command string from current position */
/* ---------------------------------------------------------------------- */

static void cmd_delete (int wlen)
{
    int nb;

    if ( wlen <= 0 ) return;

    nb = ccmdlen - ccmdpos;
    ccmdpos -= wlen;
    if ( nb <= 0 ) {        /* end of the string */
	ccmdlen = ccmdpos;
    }
    else {
	memmove (&ccmdp[ccmdpos], &ccmdp[ccmdpos+wlen], nb);
	ccmdlen -= wlen;
    }
    ccmdp[ccmdlen] = '\0';
    move_cursor (wlen, YES);
}

/* insert the key value in the command line string */

static void push_in_cmdline (Flag clean_msg)
{
    if ( (ccmdpos > 0) && (ccmdp [ccmdpos -1] == ESCCHAR) ) {
	if ( (key >= '\100') && (key < '\177') ) {
	    key &= '\137';
	} else {
	    return;
	}
    }
    if ( insmode && (ccmdpos < ccmdlen) ) {
	memmove (&ccmdp[ccmdpos +1], &ccmdp[ccmdpos], (ccmdlen++ - ccmdpos));
	ccmdp[ccmdpos++] = key;
	move_cursor (1, YES);   /* move cursor and refresh display */
    }
    else {
	if ( ccmdpos == ccmdlen ) ccmdlen++;
	ccmdp[ccmdpos++] = key;
	ccmdp[ccmdlen] = '\0';
	if ( clean_msg ) move_cursor (1, YES);
	else putch_cmd (key);
    }
}

/* Expand the given keyword in editor command line */
/* ----------------------------------------------- */

static int expd_keyword_name (int pos, S_looktbl *table, int *val_pt, Flag max_flg)
/*
 *  pos is the index of the 1st char of key word in the command line "ccmdp"
 *  if max_flg is true : return the longest keyword
 *  return :
 *      -2 : ambiguous keyword
 *      -1 : keyword not found in table
 *       0 : nothing done
 *       1 : keyword expanded
 *    *val_pt : the keyword value (from table)
 */
{
    char * name_expantion;
    char strg[256];     /* must be large enough : see MAXLUPN in e.pa.c */
    char * keyword;
    int cc, wlen, idx, val;
    int delta_pos, pos1, pos2;

    /* outside command line ? */
    if ( (pos < 0) || (pos >= ccmdlen) ) return 0;

    cc = check_keyword (NULL, pos, table, 1, &pos2, &name_expantion,
			&wlen, &idx, max_flg);
    if ( cc == -2 ) return -2;      /* ambiguous keyword */
    if ( cc == -1 ) return -1;      /* keyword not found */
    val = table[idx].val;
    if ( val_pt ) *val_pt = val;
    if ( (cc <= 0) || (wlen == 0) ) return 0;   /* nothing to do */

    /* move the cursor at end of command name abreviation */
    delta_pos = ccmdpos - pos2;
    if ( delta_pos ) {
	ccmdpos -= delta_pos;
	move_cursor (-delta_pos, NO);
    }

    /* now expand the keyword in command line*/
    (void) cmd_insert (wlen, name_expantion);

    /* move back the cursor at the same relative position */
    if ( delta_pos < 0 ) delta_pos -=wlen;
    if ( delta_pos ) {
	ccmdpos += delta_pos;
	move_cursor (delta_pos, NO);
    }
    return 1;   /* done */
}

int expand_keyword_name (int pos, S_looktbl *table, int *val_pt)
{
    return expd_keyword_name (pos, table, val_pt, NO);
}

int expand_max_keyword_name (int pos, S_looktbl *table, int *val_pt)
{
    return expd_keyword_name (pos, table, val_pt, YES);
}


/* Expand the editor command name (1st param) in command window */

static int expand_cmd_line_para ()
{
    extern S_looktbl cmdtable [];
    int cc;

    cc = expand_keyword_name (0, cmdtable, NULL);
    return cc;
}


#ifdef COMMENT
void
param ()
.
    Take input from the Command Line.
    For historical reasons, this is spoken of as 'getting a parameter'.

    !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    There are three types of parameters:
	    paramtype = -1 -- cursor defined
	    paramtype = 0  -- just <arg> <function>
	    paramtype = 1  -- string, either integer or text:
    !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    On return the value of paramtype global variable is set to : if the command line 
	 *  0: was empty
	 *  1: contained only a number
	 *  2: contained only a rectangle spec (e.g. "12x16")
	 *  3: contained a single-word string
	 *  4: contained a multiple-word string

    Returns pointer to the text string entered.
    The pointer is left in the global variable paramv, and
    its alloced length in bytes (not string length) in ccmdl.
#endif

void param ()
{
    void re_param (Flag);
    re_param (NO);
}


void re_param (Flag uselast)
/*  uselast */              /* re-use the last command string */
{
    extern void clear_namelist ();
    extern void clear_ambiguous_param ();
    extern int get_next_file_name ();
    extern void key_resize ();
    extern int command_class ();

    long seek_point;        /* position on entry in param in keyfile */
    int old_wlen;           /* insert file name expension */
    Flag cmd_file_flg;      /* set when file expantion is active */
    Flag used_file_flg;     /* set if file expansion was uesed */
    Flag resize_entry_flg;  /* begining of resize sequence */
    Flag cmdenter_flg;      /* already in command when entring resize */
    char resize_chr [8];    /* screen size : col_chr, lin_chr */
    int resize_chr_idx;     /* current write position into resize_chr */
    unsigned int key_cnt, previous_key_cnt; /* main for loop counter */
    char * file_name_expantion;     /* expanded file name */
    char * hist_strg;       /* cmd string from history list */

    S_window * cwin;            /* current active window */
    struct svcs * csv_curs;     /* saved cursor position */

    Flag all_cmdflg, previous_all_cmdflg;   /* only CCCMD keys */
    Flag cmdflg, previous_cmdflg;   /* previous and pervious previous key was CCCDM */
    Flag clean_msg;                 /* hold cmd line until next key input */
    Flag prchar_flg;                /* put key value in the command line */
    Flag done_flg;                  /* completion of the main for loop */
    Flag parmstrt_flg;              /* a full new command line */
    Flag keep_previous_flg;         /* keep the previous state variable in main loop (for resize) */
    Flag hist_flg, previous_hist_flg;   /* in history processing */
    Flag hist_cont_flg;                 /* continue history processing */
    Flag file_navig_flg;                /* in file list navigation */
    Flag file_navig_cont_flg;           /* continue file navigation */
    Flag previous_file_navig_flg;
    Flag file_navig_enter_flg;          /* to enter in file navigation */
    Flag ambiguous_flg;                 /* tab processing found something ambiguous */
    int cmd_class;                      /* command class for ambiguous processing */

    /* for file navigation */
    char * file_cmd_strg;               /* cmd string for file navigation */
    char * file_para;
    Short cmdval;
    static char fcmd [32];      /* command line on entry in file navigation */

    static struct timeval nulsec = { 0, 0 };

    resize_entry_flg = cmdenter_flg = NO;
    entering = YES;     /* set this flag so that getkey1 () knows you are in
			   this routine. */

    /* save the cursor position */
    savecurs ();
    cwin = curwin;
    csv_curs = sv_curs;

    setbul (NO);

    if (ccmdl == 0) {  /* first time only */
	ccmdp = salloc (ccmdl = LPARAM, YES);
	lcmdp = salloc (lcmdl = LPARAM, YES);
    }

    exchgcmds ();

    parmstrt_flg = YES;
    used_file_flg = NO;
    ambiguous_flg = NO;
    key_cnt = previous_key_cnt = 0;
    previous_all_cmdflg = all_cmdflg = YES;
    cmdflg = previous_cmdflg = cmd_file_flg = NO;
    keep_previous_flg = YES;
    hist_get = hist_write;
    hist_flg = previous_hist_flg = hist_cont_flg = NO;
    file_navig_flg = file_navig_cont_flg = previous_file_navig_flg = NO;
    file_navig_enter_flg = ( key == CCFNAVIG );
    cmd_class = NOT_CMD_CLASS;

    /* Main get command line loop */
    for ( done_flg = NO ; ! done_flg ; keyused = YES ) {
	if ( key != CCTAB ) ambiguous_flg = NO;
	if ( ! keep_previous_flg ) {
	    cmdflg = ((unsigned) key == CCCMD);
	    all_cmdflg = all_cmdflg && cmdflg;
	    hist_flg = hist_flg && hist_cont_flg;
	    file_navig_flg = file_navig_flg && file_navig_cont_flg;
	}
	keep_previous_flg = hist_cont_flg = file_navig_cont_flg = NO;

	if ( parmstrt_flg ) {
	    /* Get a fully new command line */
	    parmstrt_flg = NO;

	    /* display command prompt */
	    clean_msg = loopflags.hold;
	    if ( cmdmode ) {
		if ( ! loopflags.hold )
		    mesg ((TELSTRT|TELCLR) + 1, "");
		else
		    mesg ((TELSTRT) + 1, "");
		poscursor (sizeof(cmds_prompt)- 1, 0);
	    }
	    else {
		mesg ((TELSTRT|TELCLR) + 1, cmd_prompt);
	    }
	    loopflags.hold = NO;

	    /* init the command line storage */
	    ccmdbg = cmd_prompt_sz;
	    memset (ccmdp, 0, ccmdl);
	    ccmdpos = 0;
	    ccmdlen = 0;
	    ccmdoffset = 0;
	    cmdflg = previous_cmdflg = NO;
	    cmd_file_flg = NO;
	    clear_namelist ();
	    old_wlen = 0;

	    if ( uselast ) {
		/* re-display and re-use the last (or saved) command string */
		char * cmdstrg;
		Short cmdl;

		uselast = NO;
		cmdl = lcmdlen;
		cmdstrg = lcmdp;
		if ( cmdl == 0 ) {
		    /* if the last command is empty, try the last cmd in history */
		    /* version 19.57 */
		    cmdstrg = history_get (NO); /* get last command */
		    if ( cmdstrg ) cmdl = strlen (cmdstrg);
		    else cmdstrg = lcmdp;
		}
		if (cmdl >= ccmdl)
		    ccmdp = gsalloc (ccmdp, 0, ccmdl = cmdl + LPARAM, YES);
		memmove (ccmdp, cmdstrg, cmdl);
		ccmdpos = ccmdlen = cmdl;
		ccmdp[cmdl] = '\0';
		key_cnt = ccmdpos +2;   /* for the <CMD> <ALT> keys */
		putmsg ();
		all_cmdflg = NO;
		if ( ccmdlen ) check_history_get (ccmdp);   /* version 19.57 */

		/* rebuild ambiguous info and the file names list if any */
		cmd_class = process_tab_in_cmd (cmdflg, &old_wlen,
						&done_flg,
						&used_file_flg,
						&cmd_file_flg,
						&ambiguous_flg);
		if ( ambiguous_flg && (old_wlen > 0) &&
		     ((cmd_class == FILE_CMD_CLASS) ||
		      (cmd_class == DIR_CMD_CLASS)) ) {
		    /* if something added, remove it */
		    extern void reset_namelist_idx ();
		    cmd_delete (old_wlen);
		    old_wlen = 0;
		    reset_namelist_idx ();
		}
		ambiguous_flg = done_flg = NO;
	    }
	    else {
		/* keyfile write position, for file expension and CCINT */
		seek_point = ftell (keyfile);
	    }
	}

	if ( file_navig_enter_flg ) {
	    /* simulate an 'edit' command */
	    Char saved_key;
	    saved_key = key;
	    key = 'e';
	    push_in_cmdline (clean_msg);
	    done_flg = NO;
	    clean_msg = NO;
	    key = saved_key;
	    file_navig_enter_flg = NO;
	} else {
	    /* Get a key value from input stream */
	    testandset_resize (YES, csv_curs, cwin);
	    getkey (WAIT_KEY, nulsec);
	    testandset_resize (NO, csv_curs, cwin);
	    key_cnt++;
	    if ( key_cnt == INT_MAX ) key_cnt = 2;  /* must be > 1 */
	}

	prchar_flg = YES;   /* default : put key in the command line */
	done_flg = NO;

	/* check for <CMD> <key> command which are main editing command */
	if ( all_cmdflg && !cmdmode ) {
	    if (    ((unsigned) key == CCDELCH)
		 || ((unsigned) key == CCMOVELEFT)
		 || ((unsigned) key == CCMOVERIGHT)
		 || ((unsigned) key == CCBACKSPACE)
	       ) {
		 done_flg = YES;
		 break;     /* done, exit from main loop */
	    }
	}

	/* check command line storage */
	if ( ccmdlen <= 0 ) ccmdlen = ccmdpos = 0;
	if  (ccmdlen >= ccmdl )
	    ccmdp = gsalloc (ccmdp, ccmdlen, ccmdl += LPARAM, YES);
	ccmdp [ccmdlen] = '\0';

	if ( ((unsigned) key == CCMOVEUP) || ((unsigned) key == CCMOVEDOWN) ) {
	    /* look for navigation in expanded file name list */
	    int delta, wlen;
	    Flag beep, cont;

	    cont = beep = YES;  /* default beep and go back in the for loop */
	    delta = ((unsigned) key == CCMOVEUP) ? -1 : 1;
	    wlen = get_next_file_name (delta, &file_name_expantion);
	    if ( wlen == -3 ) {
		/* only one name in the list */
		if ( delta > 0 ) {
		    key = CCTAB;  /* simulate a Tab */
		    beep = cont = NO;
		}
	    } else if ( wlen >= 0 ) {
		/* something in the list */
		beep = NO;
		if ( old_wlen > 0 ) {
		    cmd_delete (old_wlen);
		    old_wlen = 0;
		}
		if ( wlen > 0 ) {
		    old_wlen = cmd_fname_expantion (wlen, file_name_expantion);
		}
	    } else if ( wlen == -5 ) {
		/* nothing in file navigation list */
		cont = beep = cmd_file_flg;
	    }

	    if ( beep ) putchar ('\007');
	    if ( cont ) {
		/* in navigation, continue to get command */
		clean_msg = NO;
		continue;
	    }
	} else {
	    /* no navigation, clear list */
	    if ( ((unsigned) key != CCTAB) || !ambiguous_flg ) {
		clear_ambiguous_param ();
		clear_namelist ();
		old_wlen = 0;
	    }
	}

	if ( CTRLCHAR ) {
	    /* key value is an editor key function value.
	     *   Make sure that all codes for which <CMD><key>
	     *   is undefined are are ignored by this processing.
	     */

	    prchar_flg = NO;   /* do not put key in the command line */
	    done_flg = YES;    /* for loop completed */

	    switch ((unsigned) key) {

		case CCCMD:
		    previous_cmdflg = cmdflg;
		    previous_all_cmdflg = all_cmdflg;
		    previous_key_cnt = key_cnt;
		    previous_hist_flg = hist_flg;
		    previous_file_navig_flg = file_navig_flg;

		    done_flg = NO;
		    break;

		case CCRETURN:
		    if ( resize_entry_flg ) {
			extern void replaying_resize (char *resize_chr);
			resize_chr [sizeof (resize_chr) -1] = '\0';
			if ( replaying ) replaying_resize (resize_chr);
			resize_entry_flg = NO;
			/* restaure value on entry */
			cmdflg = previous_cmdflg;
			if ( previous_cmdflg ) key = CCCMD;
			all_cmdflg = previous_all_cmdflg;
			hist_flg = previous_hist_flg;
			file_navig_flg = previous_file_navig_flg;
			key_cnt = previous_key_cnt;
			keep_previous_flg = YES;
			if ( cmdenter_flg ) done_flg = NO;  /* some command is to be entered */
		    }
		    break;

		case CCINT:
		    /* do this whether or not preceded by <CMD> key */
#if 0
		    if ( (ccmdpos > 0) && (ccmdp [ccmdpos -1] == ESCCHAR) ) {
			/* remove interrupted quoted char */
			cmd_delete (1);
		    }
#endif
		    if ( ccmdlen > 0 ) {
			/* we have generated something we may want to call back */
			if ( used_file_flg ) {
			    /* a file para could have been expanded,
			     *  simulate the input in keyfile to save data
			     */
			    int i;
			    (void) fseek (keyfile, seek_point, SEEK_SET);
			    for ( i = 0 ; i < ccmdlen ; i++ ) putc (ccmdp[i], keyfile);
			    key_resize ();  /* for the case where a resize was done during file xpension */
			    clear_namelist ();
			    old_wlen = 0;
			    used_file_flg = NO;
			}
			exchgcmds ();
		    }
		    putc (CCINT, keyfile);
		    keyused = YES;
		    if ( cmdmode ) {
			mesg (TELSTOP);
			uselast = NO;
			done_flg = NO;
			parmstrt_flg = YES;
			break;
		    }
		    /* to force that what was just saved will be use as
		     *  last command during the done processing.
		     *  If ccmdlen is not set to 0, this is not true
		     */
		    ccmdlen = 0;
		    break;

		case CCRESIZE:  /* resize the screen (in replaying only) */
		    resize_chr_idx = 0;
		    memset (resize_chr, 0, sizeof (resize_chr));
		    resize_entry_flg = YES;
		    cmdenter_flg = cmdflg || (ccmdlen > 0); /* already in command */
		    cmdenter_flg = cmdflg || (ccmdlen > 0);
		    done_flg = NO;
		    break;

		case CCTAB:     /* expand file name or command ? */
		    {
			extern int process_tab_in_cmd (
			    Flag cmdflg, int *old_wlen_pt, Flag *done_flg_pt,
			    Flag *used_file_flg_pt, Flag *cmd_file_flg_pt,
			    Flag *ambiguous_flg_pt);

			static int cmd_class = NOT_CMD_CLASS;
			Flag break_flg;

			if ( ambiguous_flg ) {
			    /* previous key (tab) had detected an ambiguous parameter */
			    break_flg = NO;
			    switch (cmd_class) {
				case -1 :
				case -2 :
				    /* ambiguous command */
				    done_flg = YES;
				    key = CCHELP;
				    break_flg = YES;
				    break;

				case NOT_CMD_CLASS :
				    break;

				case FILE_CMD_CLASS :
				case DIR_CMD_CLASS  :
				case HELP_CMD_CLASS :
				case ARG_CMD_CLASS :
				    /* simulate a HELP key stroke to display
				     *  the list of matching keywords.
				     */
				    done_flg = YES;
				    key = CCHELP;
				    break_flg = YES;
				    if ( (old_wlen > 0) &&
					 ((cmd_class == FILE_CMD_CLASS) ||
					  (cmd_class == DIR_CMD_CLASS)) )
					cmd_delete (old_wlen);
				    break;

				default :
				    ;
			    }
			    ambiguous_flg = NO;
			    if ( break_flg ) break;
			}

			/* try to expand the command name and call processing */
			(void) expand_cmd_line_para ();
			ccmdp [ccmdlen] = '\0';
			old_wlen = 0;
			cmd_class = process_tab_in_cmd (cmdflg, &old_wlen,
							&done_flg,
							&used_file_flg,
							&cmd_file_flg,
							&ambiguous_flg);
			if ( cmd_class < 0 ) {
			    ambiguous_flg = YES;
			    done_flg = NO;
			}
			break;
		    }

		case CCBACKSPACE:
		    done_flg = NO;
		    if ( ccmdpos <= 0 ) break;  /* beginig of the command string */

		    if ( cmdflg ) {
			int i;

			i = ccmdpos;
			if ( insmode ) {
			    int nb;

			    nb = ccmdlen - ccmdpos;
			    if ( nb > 0 )
				memmove (ccmdp, &ccmdp[ccmdpos], nb);
			    else nb = 0;
			    ccmdlen = nb;
			}
			else {
			    memset (ccmdp, ' ', ccmdpos);
			}
			ccmdp[ccmdlen] = '\0';
			ccmdpos = 0;
			move_cursor (-i, YES);
		    }
		    else {
			int nb;
			nb = ccmdlen - ccmdpos;
			ccmdpos--;
			if ( nb <= 0 ) {        /* end of the string */
			    ccmdlen--;
			}
			else {
			    if ( insmode ) {
				memmove (&ccmdp[ccmdpos], &ccmdp[ccmdpos+1], nb);
				ccmdlen--;
			    }
			    else
				ccmdp[ccmdpos] = ' ';
			}
			ccmdp[ccmdlen] = '\0';
			move_cursor (-1, YES);
		    }
		    break;

		case CCDELCH:
		    done_flg = NO;
		    if ( ccmdpos >= ccmdlen ) break;   /* at the end of command */

		    if ( cmdflg ) {
			ccmdlen = ccmdpos;
		    } else {
			int nb;
			nb = ccmdlen - ccmdpos -1;
			if ( nb > 0 )
			    memmove (&ccmdp[ccmdpos], &ccmdp[ccmdpos+1], nb);
			ccmdlen--;
		    }
		    ccmdp[ccmdlen] = '\0';
		    move_cursor (0, YES);
		    break;

		case CCINSMODE:
		    /* do this whether or not preceded by <CMD> key */
		    tglinsmode ();
		    done_flg = NO;
		    break;

		case CCMOVELEFT:
		    if ( ccmdpos ) {
			int  i, nb;
			nb = cmdflg ? -ccmdpos : -1;
			ccmdpos += nb;
			for ( i = ccmdlen ; i > ccmdpos ; i-- ) {
			    if ( ccmdp[i-1] != ' ' ) break;
			}
			ccmdlen = i;
			ccmdp[ccmdlen] = '\0';
			move_cursor (nb, NO);
		    }
		    done_flg = NO;
		    break;

		case CCMOVERIGHT:
		    done_flg = NO;
		    if ( cmdflg ) {
			int nb;
			nb = ccmdlen - ccmdpos;
			if ( nb <= 0 ) break;

			ccmdpos = ccmdlen;
			move_cursor (nb, NO);
		    }
		    else {
			if ( ccmdpos == ccmdlen ) {
			    ccmdp[ccmdpos] = ' ';
			    ccmdlen++;
			}
			ccmdp[ccmdlen] = '\0';
			ccmdpos++;
			move_cursor (1, NO);
		    }
		    break;

		case CCPICK:
		    if ( ! cmdflg )
			break;
		    {
			int wlen; /* must be int because of pkarg argument */
			char *wcp;
			done_flg = NO;
			if ((wcp = pkarg (msowin->wksp,
					   msowin->wksp->wlin + msolin,
					    msowin->wksp->wcol + msocol,
					     &wlen)) == NULL)
			    break;
			if ( wlen <= 0 ) break;
			(void) cmd_insert (wlen, wcp);
		    }
		    break;


		case CCSETFILE:
		    if ( ! cmdflg )
			break;

		    /* <CMD> <alt file> : redisplay the previous command string */
		    /*      and enter in history navigation (19.57) */
		    mesg (TELSTOP);
		    uselast = YES;
		    keyused = YES;
		    parmstrt_flg = YES;
		    done_flg = NO;
		    hist_flg = hist_cont_flg = YES; /* new in version 19.57 */
		    break;

		case CCCTRLQUOTE:
		    /* do this whether or not preceded by <CMD> key */
		    key = ESCCHAR;
		    prchar_flg = YES;
		    done_flg = NO;
		    break;

		case CCMOVEUP:
		case CCMOVEDOWN:
		case CCFNAVIG:
		    if ( !cmdflg && !hist_flg && !file_navig_flg ) {
			if ( ! ccmdlen ) break;
			if ( command_class (ccmdp, &file_para, &cmdval) != 1 )
			    break;
			if ( (cmdval != CMDEDIT) || (file_para && *file_para) )
			    break;

			/* entering in edited file list navigation */
			for ( ; ccmdp[ccmdlen -1] == ' ' ; ccmdlen-- ) ;
			ccmdp[ccmdlen++] = ' ';
			ccmdp[ccmdlen] = '\0';
			memset (fnavig_cmd, '\0', sizeof (fnavig_cmd));
			strcpy (fnavig_cmd, ccmdp);
			fnavig_buf = &fnavig_cmd [ccmdlen];
			fnavig_fn = cwin->wksp->wfile;
			file_navig_flg = YES;
		    }

		    if ( file_navig_flg ) {
			/* in edited file list navigation */
			file_cmd_strg = file_list_navigation (cwin);

			if ( file_cmd_strg ) {
			    ccmdpos = ccmdlen = strlen (file_cmd_strg);
			    memmove (ccmdp, file_cmd_strg, ccmdlen);
			    ccmdp[ccmdlen] = '\0';
			    key_cnt = ccmdpos +1;
			    putmsg ();
			    all_cmdflg = NO;
			} else if ( key == CCFNAVIG ) {
			    /* only a single file in file edited list */
			    ccmdpos = ccmdlen = 0;
			    ccmdp[ccmdpos] = '\0';
			    done_flg = YES;
			    break;
			}
			file_navig_cont_flg = YES;
			done_flg = NO;
			break;
		    }

		    if ( !hist_flg ) hist_get = hist_write;
		    hist_strg = history_get ( (unsigned) key == CCMOVEDOWN );
		    if ( hist_strg ) {
			ccmdpos = ccmdlen = strlen (hist_strg);
			memmove (ccmdp, hist_strg, ccmdlen);
			ccmdp[ccmdlen] = '\0';
			key_cnt = ccmdpos +2;   /* for the <CMD> <CCMOVExx> keys */
			putmsg ();
			all_cmdflg = NO;
		    }
		    hist_flg = hist_cont_flg = YES;
		    done_flg = NO;
		    break;

		case CCHELP:
		    /* in any case do help processing */
		    done_flg = YES;
		    break;

#if 0
/* can be an alternative */
		case CCMISRCH:
		case CCPLSRCH:
		case CCLWINDOW:
		case CCRWINDOW:
		case CCHOME:
		case CCBACKTAB:
		case CCMIPAGE:
		case CCPLPAGE:
		case CCMILINE:
		case CCPLLINE:
		case CCCHWINDOW:
		case CCLWORD:
		case CCRWORD:
		case CCOPEN:
		case CCCLOSE:
		case CCERASE:
		    /* key function which use parameters for command line */
		    break;

		default:

		    /* do not return if a command line editing is in progress */
		    done_flg = all_cmdflg || ccmdlen || (cmdflg && (ccmdlen == 0));

		    /* this is the previous version behaviour */
		    done_flg = ! cmdflg;

#endif

		default:
		    done_flg = all_cmdflg || ccmdlen || (cmdflg && (ccmdlen == 0));

	    }   /* end of switch (key) */

	}   /* end of if ( CTRLCHAR ) */

	if ( prchar_flg ) {
	    /* A normal char or something to be put in the command line */

	    if ( resize_entry_flg ) {
		/* replay resize command */
		if ( resize_chr_idx < sizeof (resize_chr) )
		    resize_chr [resize_chr_idx++] = key;
		done_flg = NO;
		continue;   /* go back in the for loop */
	    }

	    push_in_cmdline (clean_msg);
#if 0
	    if ( insmode && (ccmdpos < ccmdlen) ) {
		memmove (&ccmdp[ccmdpos +1], &ccmdp[ccmdpos], (ccmdlen++ - ccmdpos));
		ccmdp[ccmdpos++] = key;
		move_cursor (1, YES);   /* move cursor and refresh display */
	    }
	    else {
		if ( ccmdpos == ccmdlen ) ccmdlen++;
		ccmdp[ccmdpos++] = key;
		ccmdp[ccmdlen] = '\0';
		if ( clean_msg ) move_cursor (1, YES);
		else putch_cmd (key);
	    }
#endif
	    done_flg = NO;
	    clean_msg = NO;
	}
    }       /* end of main for loop */


    /* The get command line input is completed */
    move_cursor (CMDEND, NO);   /* move cursor to the end of command string */
    mesg (TELSTOP);
    if ( ccmdlen ) {    /* remove trailling space */
	for ( ; ccmdlen && (ccmdp[ccmdlen -1] == ' ') ; ccmdlen-- ) ;
    }
    ccmdp[ccmdlen] = '\0';

    if ( used_file_flg && (key != CCINT) ) {
	/* a file para could have been expanded,
	 * simulates in the keyfile what is the current command string
	 */
	int i;
	(void) fseek (keyfile, seek_point, SEEK_SET);
	for ( i = 0 ; i < ccmdlen ; i++ ) putc (ccmdp[i], keyfile);
	key_resize ();  /* for the case where a resize was done during file expansion */
	putc (key, keyfile);
    }

    /* get the parameter type */
    if ( ccmdlen == 0 ) paramtype = 0;
    else {
	/* in most cases, the interest is whether the arg string
	 *  0: was empty
	 *  1: contained only a number
	 *  2: contained only a rectangle spec (e.g. "12x16")
	 *  3: contained a single-word string
	 *  4: contained a multiple-word string
	 */
	char *c2;

	c2 = ccmdp;
	paramtype = getpartype (&c2, YES, NO, curwksp->wlin + cursorline);

	switch ( paramtype ) {
	    char *cp;
	    case 1:
	    case 2:
		for ( cp = c2 ; *cp && *cp == ' ' ; cp++) continue;
		if ( *cp )
		    paramtype = 4; /* more than one string */
		break;

	    case 3:
		for ( cp = ccmdp ; *cp && *cp == ' ' ; cp++) continue;
		for ( ; *cp && *cp != ' ' ; cp++) continue;
		for ( ; *cp && *cp == ' ' ; cp++) continue;
		if ( *cp )
		    paramtype = 4;
	}
    }

    paramv = ccmdp;
    if ( ccmdlen == 0 )
	/* exchange back so that we don't have empty string as last cmd */
	exchgcmds ();
    if ( ! hist_write ) history_init;
    history_store (ccmdp);
    restcurs ();
    clrbul ();
    entering = NO;
    return;
}

#ifdef COMMENT
void
exchgcmds ()
.
    Exchange the current command and last command.
#endif
void
exchgcmds ()
{
    Reg1 int i;
    Reg2 char *cp;

    cp = lcmdp;
    lcmdp = ccmdp;
    ccmdp = cp;
    i = lcmdl;
    lcmdl = ccmdl;
    ccmdl = i;
    i = lcmdlen;
    lcmdlen = ccmdlen;
    ccmdlen = i;
    return;
}

#ifdef COMMENT
void
getarg ()
.
    Get the argument from the edit window - all characters up to space.
    Called from mainloop ().
#endif
void
getarg ()
{
    Reg1 char  *cp;
    int len;

    if (ccmdl == 0)
	ccmdp = salloc (ccmdl = LPARAM, YES);

    if ((cp = pkarg (curwksp, curwksp->wlin + cursorline,
		     curwksp->wcol + cursorcol, &len)) == NULL) {
	ccmdp[0] = '\0';
	return;
    }

    if (ccmdl < len + 1)
	ccmdp = gsalloc (ccmdp, 0, ccmdl = len + 1, YES);
    /* arg = rest of "word" */
    strncpy (ccmdp, cp, len);
    ccmdp[len] = '\0';
    ccmdlen = len;
    paramv = ccmdp;
    return;
}

#ifdef COMMENT
char *
pkarg (wksp, line, col, len)
    S_wksp *wksp;
    Nlines line;
    Ncols col;
    int *len;
.
    Get the argument from wksp [line, col] i.e. all characters up to space.
    Return pointer to string within cline.
    Return length of string in len.
#endif
char *
pkarg (wksp, line, col, len)
Reg4 S_wksp *wksp;
Reg5 Nlines line;
Reg3 Ncols col;
int *len;
{
    Reg6 La_stream *olas;
    Reg2 int rlen;
    Reg1 char *cp;

    olas = curlas;
    curlas = &wksp->las;
    getline (line);
    curlas = olas;
    if (col >= ncline - 1 || cline[col] == ' ')
	return NULL;
    for (rlen = 0, cp = &cline[col]; *cp != ' ' && *cp != '\n'; rlen++, cp++)
	continue;
    *len = rlen;
    return &cline[col];
}

#ifdef COMMENT
void
limitcursor ()
.
    Limit cursor to be within current window.
    Used after a new window has been made to be sure that the cursor
    in the parent window stays within that window.
#endif
void
limitcursor ()
{
    curwksp->ccol = min (curwksp->ccol, curwin->rtext);
    curwksp->clin = min (curwksp->clin, curwin->btext);
    return;
}


static void my_put_char (Short chr)
{
    if ( ISCTRLCHAR(chr) ) {
	putch (ESCCHAR, NO);
	if (chr != ESCCHAR)
	    putch ((chr & 037) | 0100, NO);
    }
    else putch (chr, NO);
}


/* image of the full info line */
static char info_line [MAXWIDTH +1];

#ifdef COMMENT
void
rand_info (column, ncols, msg)
    Scols column;
    Scols ncols;
    char *msg;
.
    Put 'msg' up on the Info Line at 'column'.
    'ncols' is the length of the previous message that was there.
    If 'ncols' is longer than the length of 'msg', the remainder of
    the old message is blanked out.
#endif
void
rand_info (column, ncols, usr_msg)
Scols column;
Scols ncols;
char *usr_msg;
{
    char *msg;
    Short chr;
    S_window *oldwin;
    Scols  oldcol;
    Slines oldlin;
    char * infl;
    int sz;

    msg = usr_msg ? usr_msg : "";
    if ( column >= sizeof (info_line) ) return;

    if ( ! info_line [0] ) memset (info_line, ' ', sizeof (info_line));

    /* update the info line image */
    sz = (column + ncols) <= sizeof (info_line) ? ncols : sizeof (info_line) - column;
    infl = &info_line [column];
    memset (infl, ' ', sz);
    sz = strlen (msg);
    if ( (column + sz) > sizeof (info_line) ) sz = sizeof (info_line) - column;
    memcpy (infl, msg, sz);
    info_line [sizeof (info_line) -1] = '0';

    if ( column > infowin.redit ) return;

    /* save old window info   */
    oldwin = curwin;
    oldcol = cursorcol;
    oldlin = cursorline;

    switchwindow (&infowin);
    if ( ncols > (infowin.redit - column) ) ncols = infowin.redit - column;
    poscursor (column, 0);
    for ( ; cursorcol < infowin.redit && (chr = *msg++) ; ncols-- ) {
	my_put_char (chr);
    }
    if ( ncols > 0 ) multchar (' ', ncols);
    switchwindow (oldwin);
    poscursor (oldcol, oldlin);
    return;
}

/* refresh the info line on the screen */
void refresh_info ()
{
    info_line [sizeof (info_line) -1] = '\0';
    rand_info (0, 0, info_line);
}

#ifdef COMMENT
void
mesg (parm, msgs)
    Small parm;
    char *msgs;
.
    Put a message on the Command Line.
    The least significant 3 bits of 'parm' tell how many string arguments
    follow.  The rest of the bits are used to control the workings of
    'mesg'.
.
      TELSTRT 0010  Start a new message
      TELSTOP 0020  End of this message
      TELCLR  0040  Clear rest of Command Line after message
      TELDONE 0060  (TELSTOP | TELCLR)
      TELALL  0070  (TELSTRT | TELCLR)
      TELERR  0100  This is an error message.
    The following are analogous to TEL...
      ERRSTRT 0110
      ERRSTOP 0120            /* no more to write     */
      ERRCLR  0140            /* clear rest of line   */
      ERRDONE 0160
      ERRALL  0170

JUST INCREMENTING A POINTER ON THE STACK DOES NOT WORK ON A RISC MACHINE.
Workaround coded by Dietrich 870612
#endif

/* VARARGS 1 */
void
mesg (parm, msg1,msg2,msg3,msg4,msg5,msg6,msg7)
Small parm;
unsigned char *msg1,*msg2,*msg3,*msg4,*msg5,*msg6,*msg7;
{
    char nb_msg;
    Scols lastcol, col;
    Small nmsg;
    Flag to_image;         /* YES = to screen image, NO = putchar */
    unsigned char *msgarray[7];

    memset (msgarray, 0, sizeof (msgarray));
    nb_msg = parm & 07;
    switch (nb_msg) {
        case 7:
            msgarray[6] = msg7;
        case 6:
            msgarray[5] = msg6;
        case 5:
            msgarray[4] = msg5;
        case 4:
            msgarray[3] = msg4;
        case 3:
            msgarray[2] = msg3;
        case 2:
            msgarray[1] = msg2;
        case 1:
            msgarray[0] = msg1;
        case 0:
            ;
    }

    to_image = windowsup || replaying;
    if (to_image) {
	if (parm & TELSTRT) {
	    msowin = curwin;        /* save old window info   */
	    msocol = cursorcol;
	    msolin = cursorline;
	    switchwindow (&enterwin);
            /* old version : no clear of the command line
	    if (cmdmode)
		poscursor (sizeof(cmds_prompt)- 1, 0);
	    else {
		poscursor (0, 0);
	    };
            */
            col = (cmdmode) ? cmd_prompt_sz : 0;
	    poscursor (col, 0);
	    if ( !loopflags.hold ) {
		/* clear the command line */
		multchar (' ', term.tt_width - cursorcol);
		poscursor (col, 0);
	    }
	}
	else
	    if (curwin != &enterwin)
		return;

	lastcol = cursorcol;
    }
    else {
	if (parm & TELSTRT) {
	    putchar ('\n');
	    fflush (stdout);
	}

    }

    if ((parm & ERRSTRT) == ERRSTRT) {
	Reg1 char *cp;
	cp = "\007 *** ";
	if (to_image)
	    for (; cursorcol < enterwin.redit && *cp; )
		putch (*cp++, NO);
	else
	    for (; cursorcol < enterwin.redit && *cp; )
		putchar (*cp++);
    }

    if ( nb_msg > 0 ) {
        for (nmsg = 0; nmsg < nb_msg ; nmsg++ ) {
	    unsigned char *cp;
	    unsigned char chr;
	    cp = (unsigned char *) msgarray[nmsg];
            if ( ! cp ) continue;
	    if (to_image) {
	        for ( ; cursorcol < enterwin.redit && (chr = *cp) ; cp++ ) {
		    my_put_char (chr);
	        }
	    }
	    else {
	        while (*cp)
		    putchar (*cp++);
	    }
        }
    }

    if ((parm & ERRSTOP) == ERRSTOP) {
	if (to_image) {
	    putch ('\007', NO);
	    loopflags.hold = YES;
	}
	else
	    putchar ('\007');
    }

    if (to_image) {
	Reg1 Scols tmp;
	static Scols lparamdirt;       /* for edit part        */
	static Scols rparamdirt;       /* for info part        */
	lastcol = (cursorcol < lastcol) ? enterwin.redit : cursorcol;

	/* Did we wrap around? */
	tmp = (lastcol <= term.tt_width - 1) ? lparamdirt : rparamdirt;
	/* how much dirty from before? */
	if (lastcol > tmp || parm & TELCLR) {
	    if (lastcol <= term.tt_width - 1)
		lparamdirt = lastcol;
	    else
		rparamdirt = lastcol;
	}

	if ((parm & TELCLR) && (lastcol < enterwin.redit)) {
	    /* wipe out rest of what was there */

	    tmp = min (tmp, (enterwin.redit +1));
	    multchar (' ', tmp - cursorcol);
	    if (!(parm & TELALL +1))
		poscursor (lastcol, cursorline);
	}
    }

    if (parm & TELSTOP) {         /* remember last position and.. */
	if (to_image) {
	    if ( curwin != msowin ) {
		/* cursor position can be changed by resizing */
		msocol = msowin->ccol;
		msolin = msowin->clin;
		switchwindow (msowin); /* go back to original window */
	    }
	    poscursor (msocol, msolin);
	}
	else
	    putchar (' ');
    }
    return;
}

#ifdef COMMENT
void
credisplay (cwkspflg)
Flag cwkspflg;
.
    Redisplay the current line if it has changed since that last time
    credisplay was called.
    Only called from one place: mainloop ().

#endif
void
credisplay (cwkspflg)
Flag cwkspflg;
{
    /* this is a little tricky.  Everywhere else in the editor, fcline
     * is treated as if it is either 0 or non-0, thus it is given YES
     * or NO values.  Here, if it is YES, we set it to 2, so that we can
     * tell if it has been set to YES since the last time we were called.
     * Only if it is YES do we need to do a redisplay.
     **/
    if (fcline == YES) {
	redisplay (curfile, curwksp->wlin + cursorline, 1, 0, cwkspflg);
	fcline = 2;
    }
    return;
}

extern Nlines readjtop ();

#ifdef COMMENT
void
redisplay (fn, from, num, delta, cwkspflg)
    Fn      fn;
    Nlines  from;
    Nlines  num;
    Nlines  delta;
    Flag    cwkspflg;
.
    /*
    Redisplay is called after a change has been made in file 'fn',
    starting at line 'from',
    with a total change of 'delta' in the length of the file.
    If `delta' is negative, `num' lines must be redisplayed after
    deleting `delta' lines.
    If `delta' is positive, `delta' + `num' lines must be redisplayed.
    If `delta' is 0, dislplay `num' lines.
    We are supposed to:
    1. Redisplay any workspaces which are actually changed by this
	tampering, including curwksp if 'cwkspflg' is non-0
    2. Adjust the current line number of any workspace which may be pointing
	further down in the file than the change, and doesn't want
	to suffer any apparent motion.
    */
#endif
void
redisplay (fn, from, num, delta, cwkspflg)
Fn      fn;
Reg5 Nlines  from;
Reg8 Nlines  num;
Reg9 Nlines  delta;
Flag    cwkspflg;
{
    Reg1 S_wksp *tw;
    Reg4 Small  win;
    Reg3 Slines winfirst;   /* from - wksp->wlin */
    Reg6 Slines first;      /* first line of area in window to be changed */
    Reg2 Slines endwin;     /* height of window -1 */

    for (win = Z; win < nwinlist; win++) {
	if ((tw = winlist[win]->altwksp)->wfile == fn)
	    tw->wlin += readjtop (tw->wlin, from, num, delta,
				  winlist[win]->btext + 1);
	if ((tw = winlist[win]->wksp)->wfile == fn) {
	    Nlines wmove;
	    Reg7 S_window *oldwin;
	    wmove = readjtop (tw->wlin, from, num, delta,
			      winlist[win]->btext + 1);
	    winfirst = from - (tw->wlin += wmove);
	    if (tw == curwksp && !cwkspflg)
		continue;
	    first = winfirst > 0 ? winfirst : 0;
	    endwin = winlist[win]->btext;
	    /* are the changes below the bottom of the window? */
	    if (first > endwin)
		continue;
	    /* are the changes above the top of the window? */
	    if (delta == 0 && winfirst + num <= 0)
		continue;
	    if (   delta > 0
		&& winfirst < 0         /* for insert on first line */
		&& winfirst + num <= 0
	       )
		continue;
	    if (delta < 0 && winfirst - delta + num <= 0)
		continue;
	    /* something in the window will have to change */
	    oldwin = curwin;
	    savecurs ();
	    switchwindow (winlist[win]);
	    if (curwin != oldwin)
		chgborders = 0;
	    if (delta == 0) {
		/* wmove will also be 0 at this point */
		putup (first, winfirst + num - 1, 0, MAXWIDTH);
	    } else if (delta > 0) {
		if (winfirst + delta + num > endwin)
		    putup (first, endwin, 0, MAXWIDTH);
		else if (from >= la_lsize (curlas) - delta)
		    putup (first, winfirst + delta + num - 1, 0, MAXWIDTH);
		else if (  !vinsdel (first, delta, curwin == oldwin)
			 && num > 0
			)
		    putup (winfirst + delta, winfirst + delta + num - 1,
			   0, MAXWIDTH);
	    } else { /* delta < 0 */
		if (   first - (delta - wmove) > endwin
		    || (   num > 0
			&& winfirst - delta + num > endwin
		       )
		   )
		    putup (first, endwin, 0, MAXWIDTH);
		else if (from >= la_lsize (curlas))
		    putup (first, winfirst - delta - 1, 0, MAXWIDTH);
		else if (   !vinsdel (first, delta - wmove, curwin == oldwin)
			 && num > 0
			)
		    putup (first, winfirst + num - 1, 0, MAXWIDTH);
	    }
	    chgborders = 1;
	    switchwindow (oldwin);
	    restcurs ();
	}
    }
    return;
}

#ifdef COMMENT
Nlines
readjtop (wlin, from, num, delta)
    Nlines wlin;
    Nlines  from;
    Nlines num;
    Nlines delta;
.
    Called by redisplay () to determine how far to move the top of 'wksp'.
    Returns the distance to move the window.
.
    The logic in redisplay () and the algorithm
    implemented here are closely interdependent.
#endif
Nlines
readjtop (wlin, from, num, delta, height)
Reg1 Nlines wlin;
Reg3 Nlines from;
Reg4 Nlines num;
Reg2 Nlines delta;
Slines height;
{
    /* adjust line num of top of wksp, if necessary  */

    if (delta == 0)
	return 0;
    if (delta > 0) {
	if (from >= wlin)
	    return 0;
	if (from + num <= wlin)
	    return delta;
	return 0;
    }
    /* delta < 0 */
	if (from > wlin)
	    return 0;
	if (from - delta + num <= wlin)
	    /* no effect on screen */
	    return delta;
	if (num == 0)
	    return from - wlin;
	return 0;
}

#ifdef COMMENT
void
screenexit (scroll)
    Flag scroll;
.
    Reset terminal if required.
    Put cursor at lower left of screen
    Scroll the screen if scroll is non-0.
#endif
void
screenexit (scroll)
Flag scroll;
{
#ifdef LMCLDC
    if (line_draw)                      /* if in line-drawing mode, */
	(*term.tt_xlate) (' ');         /* send printable to exit mode. */
#endif
#ifdef CLEARONEXIT
    /*  When a terminal simulator is used that keeps its own duplicate image
     *  of the screen, screenexit is only used upon exiting, so it doesn't
     *  need to reinitialize that image.
     */
    d_put (VCCCLR);
    (*kbd.kb_end) ();
    d_put (VCCEND);
#else
    putscbuf[0] = VCCAAD;
    putscbuf[1] = 041 + 0;
    putscbuf[2] = 041 + term.tt_height - 1;
    putscbuf[3] = 0;
    d_write (putscbuf, 4);
    (*kbd.kb_end) ();
    d_put (VCCEND);
    if (scroll) {
	putchar ('\n');
	fflush (stdout);
    }
#endif
    windowsup = NO;
    return;
}

#ifdef COMMENT
void
tglinsmode ()
.
    Toggle INSERT mode.
#endif
void
tglinsmode ()
{
    rand_info (inf_insert, 6, (insmode = !insmode) ? "INSERT" : "");
    return;
}

#ifdef COMMENT
void
tglpatmode ()
.
    Toggle PATTERN mode.
#endif
void
tglpatmode ()                           /* MAB */
{
    rand_info (inf_pat, 2, (patmode = !patmode) ? "RE" : "  ");
    return;
}


#ifdef COMMENT
Flag
vinsdel (start, delta, mainwin)
    Slines start;
    Slines delta;
    Flag mainwin;
.
    /*
    Insert `delta' lines at line `start' in the window.
    This means delete if `delta' is negative, of course.
    Return YES if we had to do a putup for the whole thing,
    i.e. couldn't use ins/del terminal capability, else NO.
    */
#endif
Flag
vinsdel (start, delta, mainwin)
Reg1 Slines start;
Reg3 Slines delta;
Flag mainwin;
{
    Reg5 S_wksp *cwksp = curwksp;

    if (mainwin) {
	clrbul ();
	offbullets ();
    }
    if (delta > 0) {
	Reg2 int num;
	/* insert lines */
	do {
	    Reg4 int stplnum;   /* start + num */
	    Reg5 Uint nmove;
	    num = d_vmove (curwin->ttext + start,
			   curwin->lmarg,
			   curwin->btext + 1 - (start + delta),
			   curwin->rmarg + 1 - curwin->lmarg,
			   delta,
			   YES);
	    if (num <= 0) {
 doputup:
		savecurs ();
		putup (start, curwin->btext, 0, MAXWIDTH);
		restcurs ();
		return YES;
	    }
	    nmove = curwin->btext + 1 - (stplnum = start + num);
	    move (&curwin->firstcol[start], &curwin->firstcol[stplnum],
		  nmove * sizeof curwin->firstcol[0]);
	    fill (&curwin->firstcol[start],
		  num * sizeof curwin->firstcol[0], 0);
	    move (&curwin->lastcol[start], &curwin->lastcol[stplnum],
		  nmove * sizeof curwin->lastcol[0]);
	    fill (&curwin->lastcol[start],
		  num * sizeof curwin->lastcol[0], 0);
	    move (&curwin->lmchars[start], &curwin->lmchars[stplnum], nmove);
	    move (&curwin->rmchars[start], &curwin->rmchars[stplnum], nmove);
	    savecurs ();
	    freshputup = YES;
	    nodintrup = YES;
	    cwksp->wlin += delta - num;
	    putup (start, start + num - 1, 0, MAXWIDTH);
	    cwksp->wlin -= delta - num;
	    nodintrup = NO;
	    freshputup = NO;
	    restcurs ();
	} while ((delta -= num) > 0);
    } else { /* delta < 0 */
	Reg2 int num;
	delta = -delta;
	/* delete lines */
	do {
	    Reg4 int stplnum;   /* start + num */
	    Reg5 Uint nmove;
	    num = d_vmove (curwin->ttext + start + delta,
			   curwin->lmarg,
			   curwin->btext + 1 - start - delta,
			   curwin->rmarg - curwin->lmarg + 1,
			   -delta,
			   YES);
	    if (num <= 0)
		goto doputup;
	    nmove = curwin->btext + 1 - (stplnum = start + num);
	    move (&curwin->firstcol[stplnum], &curwin->firstcol[start],
		  nmove * sizeof curwin->firstcol[0]);
	    fill (&curwin->firstcol[curwin->btext + 1 - num],
		  num * sizeof curwin->firstcol[0], 0);
	    move (&curwin->lastcol[stplnum], &curwin->lastcol[start],
		  nmove * sizeof curwin->lastcol[0]);
	    fill (&curwin->lastcol[curwin->btext + 1 - num],
		  num * sizeof curwin->firstcol[0], 0);
	    move (&curwin->lmchars[stplnum], &curwin->lmchars[start], nmove);
	    move (&curwin->rmchars[stplnum], &curwin->rmchars[start], nmove);
	    savecurs ();
	    freshputup = YES;
	    nodintrup = YES;
	    cwksp->wlin -= delta - num;
	    putup (curwin->btext + 1 - num, curwin->btext, 0, MAXWIDTH);
	    cwksp->wlin += delta - num;
	    nodintrup = YES;
	    freshputup = NO;
	    restcurs ();
	} while ((delta -= num) > 0);
    }
    return NO;
}
