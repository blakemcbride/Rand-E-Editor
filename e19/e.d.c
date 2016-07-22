#ifdef COMMENT
--------
file e.d.c
    /*
    Display manipulation code.
    Here is where the internal screen image is maintained and output
    minimizing takes place.
    There is code here which is not currently used.  It is ifdef'ed out
    with DUMB and is not supported.

	Copyright abandoned, 1983, The Rand Corporation
    */
#endif


#include "e.h"
#include "e.tt.h"
#ifdef  KBFILE
#include "e.it.h"
#endif

#define MINIMIZE     /* define to minimize output    */
#undef  WINDOWSTUFF
#define DUMB    /* define this until e is smart about insert/delete and  */
		/* windowing, etc. */

/* these are the functions expected of the physical terminal */
#define MINI0()         (*term.tt_ini0)  ()
#define MINI1()         (*term.tt_ini1)  ()
#define MEND()          (*term.tt_end)   ()
#define MLEFT()         (*term.tt_left)  ()
#define MRIGHT()        (*term.tt_right) ()
#define MDN()           (*term.tt_dn)    ()
#define MUP()           (*term.tt_up)    ()
#define MCRET()         (*term.tt_cret)  ()
#define MNL()           (*term.tt_nl)    ()
#define MCLEAR()        (*term.tt_clear) ()
#define MHOME()         (*term.tt_home)  ()
#define MBSP()          (*term.tt_bsp)   ()
#define MADDR(l,c)      (*term.tt_addr)  (l,c)
#define MLAD(l)         (*term.tt_lad)   (l)
#define MCAD(c)         (*term.tt_cad)   (c)
#define MXLATE(c)       (*term.tt_xlate) (c)
#define MWL()           term.tt_wl
#define MCWR()          term.tt_cwr
#define MPWR()          term.tt_pwr
#define MAXIS()         term.tt_axis
#define MPRTOK()        term.tt_prtok

extern Scols tabright ();
extern Scols tableft ();

static AFlag   dtabs[MAXWIDTH]; /* array of tabstops */

static Uchar  *image = NULL;    /* allocated memory array - screen image */
static int     imagesize = 0;   /* size of the allocated image memory array */
static Short   icursor;         /* used as index into image */
static Short   ocursor;         /* where the terminal cursor is */

static Short   lincurs;         /* pos of beginning of current line*/

static Scols   winl;            /* left column of window */
static Scols   winr;            /* right column of window */
static Slines  wint;            /* top line of window */
static Slines  winb;            /* bottom line of window */
static Short   winul;           /* upper-left corner of window */
static Scols   width;           /* width of current window */
static Slines  height;          /* height of current window */
static Scols   argcol;          /* marked column */
static Slines  argline;         /* marked line */

static Flag    redraw;          /* we're redrawing screen */
static Small   arg;             /* repeat count for repeatable commands */
static Small   state;           /* state the char is to be analyzed in */
static Flag    noclear;         /* do not clear out opened area in next move */
static Flag    rollmode;        /* roll at bottom of window */
static Flag    wrapmode;        /* wrap at end of line */
static Flag    cwrapmode;       /* cursor wraparound */

Slines  ilin;                   /* current internal image line */
Scols   icol;                   /* current internal image column */
Scols   ocol;                   /* column position of the terminal */
Slines  olin;                   /* line   position of the terminal */

static void d_init ();
       void d_write ();
       void putmult ();
       void rollifnec ();
       void scroll ();
       void clwindow ();
       void dbfill ();
       void dbmove ();
       void putscr ();
       void fresh ();
/* these flag are defined in e.x.c
       Flag NoBell;
#ifdef LMCVBELL
       Flag VBell;
#endif
*/

static void d_new_image (Flag clearmem, Flag clearscr)
{
    Short i;

    if ( ! image ) return;

    ocursor = 0;
    icursor = 0;
    lincurs = 0;
    ilin = 0;
    icol = 0;
    if (clearmem) dbfill (' ', 0, screensize, NO, YES);
    if (clearscr) {
	MCLEAR ();
	MHOME ();
    }
    fflush (stdout);
    state = 0;
    if (redraw) return;

    winl = 0;
    winr = term.tt_width - 1;
    wint = 0;
    winb = term.tt_height - 1;
    winul = 0;
    width = term.tt_width;
    height = term.tt_height;
    argcol = argline = 0;

    arg = 1;
    rollmode = 1;
    wrapmode = 1;
    cwrapmode = 1;
    noclear = 0;
}



/* build a new image storage */
void new_image ()
{
    Uchar * old_image;

    old_image = image;
    screensize = term.tt_width * term.tt_height;
    if ( image && (screensize > imagesize) ) {
	sfree (image);
	imagesize = 0;
	image = NULL;
    }
    if ( ! image ) {
	image = (Uchar *) salloc (screensize, YES);
	imagesize = screensize;
    }
    memset (image, 0, imagesize);
    redraw = NO;
    d_new_image (YES, YES);
}


#ifdef COMMENT
static void
d_init (clearmem, clearscr)
    Flag clearmem;
    Flag clearscr;
.
    Initialize the screen manager.
    NOTE: The first character in the first call to d_write must be VCCINI.
#endif
static void
d_init (clearmem, clearscr)
Flag clearmem;
Flag clearscr;
{
    Short i;

    if ( ! image ) new_image ();

    d_new_image (clearmem, clearscr);

    /* init the tabs stop array to default default */
    i = (sizeof dtabs / sizeof dtabs[0]) - 1;
    do  dtabs[i] = ((i & 7) ? 0 :1);
	while (i--);
}

#ifdef COMMENT
void
d_write (chp, count)
    Uchar *chp;
    Short count;
.
    Write characters to the display.
    Chp points to a mixture of data and control characters.
    The control characters are VCC*** and are defined in e.tt.h.
#endif
void
d_write (chp, count)
#ifdef UNSCHAR
Uchar *chp;
#else
char *chp;
#endif
Short count;
{
    register Short j;
    register int chr;

    for (; count > 0; chp++, count--) {
/* XXXXXXXXXXXXXXXXXXXXXXXXXXX
#ifdef UNSCHAR
	chr = *chp;
#else
	chr = *chp & 0377;
#endif
   XXXXXXXXXXXXXXXXXXXXXXXXXXX */
	chr = *chp & 0377;
	for (;;) {
	    icol = icursor - lincurs;
	    if (chr == VCCNUL) {
		putscr (0);
		goto nextchar;
	    }
	    switch (state) {
	    case 0:
		if (chr >= 040) {
		    if (arg == 1) {
			if ((chr != image[icursor]) && !redraw)
			{
			    putscr (chr);
			    image[icursor] = chr;
			}
			if (redraw && chr != ' ')
			    putscr (chr);
			icursor++;
			if (icol >= winr) {
			    if (wrapmode) {
				icursor = lincurs + winl;
				state = 1;
			    }
			    else
				state = 10;
			}
		    }
		    else  putmult (chr);
		}
		else switch (chr) {
		case VCCARG:
		    state = 2;
		    break;

		case VCCBEL:
		    if (!silent && !NoBell)
#ifdef LMCVBELL
			if (VBell)
			    (*term.tt_vbell) ();
			else
#endif
			     putchar (7);
/* I've got a bad feeling about this! Should be using a terminal routine. */
		    goto nextchar;

		case VCCAAD:
		    state = 3;
		    break;

		case VCCINI:
		    MINI0 ();
		    d_init (YES, NO);
		    goto nextchar;

		case VCCEND:
		    MEND ();
#ifdef  KBFILE
		    fwrite (kbendstr, sizeof (char), kbendlen, stdout);
#endif
		    fflush (stdout);
		    goto nextchar;

		case VCCICL:
		    MINI1 ();
		    d_init (NO, YES);
#ifdef  KBFILE
		    fwrite (kbinistr, sizeof (char), kbinilen, stdout);
#endif
		    fflush (stdout);
		    goto nextchar;

#ifndef DUMB
		case VCCCLR:
		    clwindow ();
		    break;

		case VCCRES:
		    d_init (NO, NO);
		    goto nextchar;

		case VCCRAD:
		    state = 17;
		    break;

		case VCCNCL:
		    noclear = 1;
		    goto nextchar;

		case VCCESC:
		    state = 6;
		    goto nextchar;     /* save arg where it is */

#endif
		case VCCRET:
		    icursor = lincurs + winl;
		    break;

#ifndef DUMB
		case VCCNWL:
		    icursor = lincurs + winl;
		    rollifnec (arg);
		    break;

		case VCCSTB:
		case VCCCTB:
		    chr = (chr == VCCSTB? 1: 0);
		    dtabs[icol - winl] = chr;
		    if (icol < winr)
			icursor++;
		    break;

		case VCCFTB:
		    icursor = tabright (arg) + lincurs + winl;
		    break;

		case VCCBTB:
		    icursor = tableft (arg) + lincurs + winl;
		    break;
#endif

		case VCCLEF:
		    for (;;) {
			j = icol - winl;
			if (j > 0) {
			    j = min (j, arg);
			    icursor -= j;
			    arg -= j;
			}
			if (!cwrapmode)
			    break;
			if (arg <= 0)
			    break;
			icursor += width;
			icol = winr + 1;
		    }
		    break;

		case VCCRIT:
		    for (;;) {
			j = winr - icol;
			if (j > 0) {
			    j = min (j, arg);
			    icursor += j;
			    arg -= j;
			}
			if (!cwrapmode)
			    break;
			if (arg <= 0)
			    break;
			icursor -= width;
			icol = winl - 1;
		    }
		    break;

		case VCCUP:
		    for (;;) {
			j = ilin - wint;
			if (j > 0) {
			    j = min (arg, j);
			    ilin -= j;
			    arg -= j;
			    icursor -= (j *= term.tt_width);
			    lincurs -= j;
			}
			if (!cwrapmode)
			    break;
			if (arg <= 0)
			    break;
			lincurs += (Short) (height - 1) * (Short) term.tt_width;
			icursor = lincurs + icol;
			ilin = winb;
			arg--;
		    }
		    break;

		case VCCDWN:
		    for (;;) {
			j = winb - ilin;
			if (j > 0) {
			    j = min (arg, j);
			    ilin += j;
			    arg -= j;
			    icursor += (j *= term.tt_width);
			    lincurs += j;
			}
			if (!cwrapmode)
			    break;
			if (arg <= 0)
			    break;
			lincurs -= (Short) (height - 1) * (Short) term.tt_width;
			icursor = lincurs + icol;
			ilin = wint;
			arg--;
		    }
		    break;

		case VCCBKS:
		    if ((j = icol - winl) > 0) {
			arg = j = min (arg, j);
			putscr (0); /* update terminal cursor */
			icursor -= j;
			do {
			    MBSP ();
			    ocursor--;
			} while (--j);
			dbfill (' ', icursor, arg, NO, YES);
		    }
		    break;

#ifndef DUMB
		case VCCEOL:
		    dbfill (' ', icursor, winr - icol + 1, 1, 1);
		    break;
#endif

		case VCCHOM:
		    icursor = winul;
		    lincurs = icursor - winl;
		    ilin = wint;
		    break;

#ifndef DUMB
		case VCCWIN:
		    if ((icol >= argcol) && (ilin >= argline)) {
			winl = argcol;
			winr = icol;
			wint = argline;
			winb = ilin;
			width = winr - winl + 1;
			height = winb - wint + 1;
			winul = wint * term.tt_width + winl;
		    }
		    break;

		case VCCDLL:
		    scroll (ilin, arg, 1);
		    break;

		case VCCINL:
		    scroll (ilin, arg, 0);
		    break;

		case VCCDLC:
		    i = winr - icol + 1;
		    j = min (arg, i);
		    dbmove (icursor + j, icursor, i - j, 1, 1);
		    if (noclear)
			noclear = 0;
		    else
			dbfill (' ', icursor + i - j, j, 1, 1);
		    break;

		case VCCINM:
		    state = 8;
		    break;

		case VCCINC:
		    i = winr - icol + 1;
		    j = min (arg, i);
		    dbmove (icursor, icursor + j, i - j, 1, 1);
		    if (noclear)
			noclear = 0;
		    else
			dbfill (' ', icursor, j, 1, 1);
		    break;

		case VCCMVL:
		    j = min (arg, width);
		    i = winb - wint + 1;
		    to = winul;
		    do {
			dbmove (to + j, to, width - j, 1, 1);
			if (!noclear)
			    dbfill (' ', to + width - j, j, 1, 1);
			to += term.tt_width;
		    } while (--i);
		    noclear = 0;
		    icursor -= min (j, icol - winl);
		    break;

		case VCCMVR:
		    j = min (arg, width);
		    i = winb - wint + 1;
		    to = winul;
		    do {
			dbmove (to, to + j, width - j, 1, 1);
			if (!noclear)
			    dbfill (' ', to, j, 1, 1);
			to += term.tt_width;
		    } while (--i);
		    noclear = 0;
		    icursor += min (j, winr - icol);
		    break;
#endif

		default:
		    fatal (FATALBUG, "Illegal command to terminal simulator");
		}

		arg = 1;
		goto nextchar;

	    case 1:                         /* prev char spilled over */
		if (chr >= 040) {
		    rollifnec (1);
		    if (state == 13)
			goto nextchar;
		}
#ifndef DUMB
		else if (chr == VCCESC) {
		    state = 7;
		    goto nextchar;
		}
#endif
		else if (chr == VCCARG) {
		    state = 16;
		    goto nextchar;
		}
		state = 0;
		break;

	    case 2:                         /* get argument */
		state = 0;
		if (chr == 040) {
		    argcol = icol;
		    argline = ilin;
		    arg = 1;
		    goto nextchar;
		}
		else if (chr > 040) {
		    arg = chr - 040;
		    goto nextchar;
		}
		arg = 1;
		break;

	    case 3:                         /* get column addr */
		if (chr > 040) {
		    icursor = lincurs + min (chr - 040, term.tt_width) - 1;
		    state = 5;
		}
		else if (chr == 040)
		    state = 5;
		else if (chr == 127)
		    state = 4;
		else {
		    state = 0;
		    break;
		}
		goto nextchar;

	    case 4:                         /* get oversize column addr */
		icursor = lincurs + min (127 - 040 + chr - 040, term.tt_width) -1;
		state = 5;
		goto nextchar;

	    case 5:                         /* get line addr */
		state = 0;
		if (chr > 040) {
		    ilin = min (chr - 040, term.tt_height) - 1;
		    lincurs = (Short) ilin * (Short) term.tt_width;
		    icursor = lincurs + icol;
		}
		else if (chr < 040)
		    break;
		if (   (icol < winl) || (icol > winr)
		    || (ilin < wint) || (ilin > winb)
		   ) {
		    winl = wint = 0;
		    winr = term.tt_width - 1;
		    winb = term.tt_height - 1;
		    winul = 0;
		    width = term.tt_width;
		    height = term.tt_height;
		}
		goto nextchar;

#ifndef DUMB
	    case 6:                        /* prev char was esc */
		state = 0;
		switch (chr) {
		case VECSWR:
		    wrapmode = 1;
		    break;

		case VECCWR:
		    wrapmode = 0;
		    break;

		case VECSRL:
		    rollmode = 1;
		    break;

		case VECCRL:
		    rollmode = 0;
		    break;

		case VECSWC:
		    cwrapmode = 1;
		    break;

		case VECCWC:
		    cwrapmode = 0;
		    break;
		}
		arg = 1;
		goto nextchar;

	    case 7:                         /* prev char was esc after */
		state = 6;                  /*  line wrapped around    */
		break;

	    case 8:                         /* insert mode in effect */
		if (chr >= 040) {         /*! no multichar inserts as yet */
		    j = winr - icol;
		    dbmove (icursor, icursor + 1, j, 1, 1);
		    if (chr != image[icursor])
		    {
			putscr (chr);
			image[icursor] = chr;
		    }
		    if (icol >= winr) {
			if (wrapmode) {
			    icursor = lincurs + winl;
			    state = 1;
			}
			else
			    state = 10;
		    }
		    else
			icursor++;

		    goto nextchar;
		}
		else if (chr == VCCESC) {
		    state = 9;
		    goto nextchar;
		}
		else
		    state = 0;
		break;

	    case 9:                         /* prev char was esc while */
		state = 6;
		break;

	    case 10:                        /* throwing away characters */
		if (   chr == VCCAAD            /*  off end of line         */
		    || chr == VCCRAD
		    || chr == VCCRET
		    || chr == VCCNWL
		    || chr == VCCHOM
		   ) {
		    state = 0;
		    break;
		}
		if (chr == VCCESC) {
		    state = 11;
		    arg = 1;
		}
		else if (chr == VCCARG)
		    state = 12;
		goto nextchar;

	    case 11:                        /* esc from state 10 */
		state = 10;
		goto nextchar;

	    case 12:                        /* arg from state 10 */
		if (chr > 040)
		    arg = chr - 040;
		state = 10;
		goto nextchar;

	    case 13:                        /* throw away everything */
		if (   chr == VCCHOM             /*  after an attempted */
		    || chr == VCCAAD             /*  newline at winb */
		    || chr == VCCRAD
		   ) {
		    state = 0;
		    break;
		}
		else if (chr == VCCESC)
		    state = 14;
		else if (chr == VCCARG)
		    state = 15;
		goto nextchar;

	    case 14:                        /* esc from state 13 */
		state = 13;
		goto nextchar;

	    case 15:                        /* arg from state 13 */
		state = 13;
		goto nextchar;
#endif

	    case 16:                        /* arg from state 1 */
		if (chr > 040)
		    arg = chr - 040;
		state = 1;
		goto nextchar;

#ifndef DUMB
	    case 17:                        /* get rel column */
		if (chr <= 040)
		    state = 19;
		else if (chr == 127)
		    state = 18;
		else {
		    icursor = lincurs + winl + min (chr - 040, width) - 1;
		    state = 19;
		}
		goto nextchar;

	    case 18:                        /* get oversize rel col */
		icursor = lincurs + winl + min (127 - 040 + chr - 040, width) - 1;
		state = 19;
		goto nextchar;

	    case 19:                        /* get rel line addr */
		if (chr != 0) {
		    ilin = wint + min (chr - 040, height) - 1;
		    lincurs = ilin * term.tt_width;
		    icursor = lincurs + icol;
		}
		state = 0;
		goto nextchar;
#endif

	    default:
		fatal (FATALBUG, "Illegal state in terminal simulator");
	    }
	}
 nextchar:
	{}
    }
    return;
}


#ifdef  COMMENT
  /*
  move characters horizontally within a line at the current position.
  If `delta' > 0, then insert characters, else delete them.
  The character positions vacated by the move are filled from buf.
  */
#endif
int
d_hmove (delta, num, buf)
Scols delta;
Scols num;
char *buf;
{
    if (term.tt_inschar && term.tt_delchar)
	; /* how nice */
    Block {
	Reg2 Uchar *rto;
	rto = &image[icursor];
	do {
	    if (*rto != *buf)
		putscr (*rto++ = *buf++);
	    else {
		++rto;
		++buf;
	    }
	    icursor++;
	    icol++;
	} while (--num);
    }
    return;
}


#ifdef  COMMENT
  /*
  move a rectangle of text vertically
  If `clearok' == YES, then it is ok to use ins/del line which will
  clear the vacated lines.
  If can't do insert/delete, return -1.
  Else if multiple lines can't be moved in one operation, return 1.
  Else return number of lines moved.
  */
#endif
int
d_vmove (line, col, aheight, awidth, num, clearok)
Slines line;
Scols col;
Slines aheight;
register Scols awidth;
int num;
Flag clearok;
{
    register Short i;
    register Short nch;
    register Short to;
    Flag useinsdel
	=  clearok
	&& awidth == term.tt_width
	&& (term.tt_vscroll || (term.tt_insline && term.tt_delline));

    if (!useinsdel)
	return -1;
    if (num == 0)
	return 0;
    if (col + awidth > width)
	fatal (FATALBUG, "vmove width to big");
    if (line + aheight > height)
	fatal (FATALBUG, "vmove move area too high");
    if (num < 0 && line < num)
	fatal (FATALBUG, "vmove move too far up");
    if (num > 0 && line + aheight + num > height)
	fatal (FATALBUG, "vmove move too far down");


    if (num < 0) {
	num = -num;

	if (smoothscroll) {
	    line -= num - 1;
	    aheight += num - 1;
	    num = 1;
	}

	nch = num * term.tt_width;
	to = winul + col + (Short) (line - num) * (Short) term.tt_width;
	if (useinsdel) {
	    /* do move with insline & delline */
	    if (term.tt_vscroll)
		(*term.tt_vscroll) (wint + line - num,
				    wint + line + aheight - 1, -num);
	    else if (smoothscroll || singlescroll) {
		i = num;
		do {
		    MADDR (wint + line - num, 0);
		    (*term.tt_delline) (1);
		    MADDR (wint + line + aheight - 1, 0);
		    (*term.tt_insline) (1);
		} while (--i);
	    } else {
		i = num;
		MADDR (wint + line - num, 0);
		do {
		    i -= (*term.tt_delline) (i);
		} while (i);
		i = num;
		MADDR (wint + line + aheight - num, 0);
		do {
		    i -= (*term.tt_insline) (i);
		} while (i);
	    }
	    MADDR (olin, ocol);
	    i = aheight;
	    do {
		dbmove (to + nch, to, awidth, NO, 1);
		to += term.tt_width;
	    } while (--i);
	    i = num;
	    do {
		dbfill (' ', to, awidth, NO, 1);
		to += term.tt_width;
	    } while (--i);
	} else {
	    i = aheight;
	    do {
		dbmove (to + nch, to, awidth, YES, 1);
		to += term.tt_width;
	    } while (--i);
	}
    } else {
	if (smoothscroll) {
	    aheight += num - 1;
	    num = 1;
	}

	nch = num * term.tt_width;
	if (useinsdel) {
	    /* do move with insline & delline */
	    if (term.tt_vscroll)
		(*term.tt_vscroll) (wint + line,
				    wint + line + aheight - 1 + num, num);
	    else if (smoothscroll || singlescroll) {
		i = num;
		do {
		    MADDR (wint + line + aheight + num - 1, 0);
		    (*term.tt_delline) (1);
		    MADDR (wint + line, 0);
		    (*term.tt_insline) (1);
		} while (--i);
	    } else {
		i = num;
		MADDR (wint + line + aheight, 0);
		do {
		    i -= (*term.tt_delline) (i);
		} while (i);
		i = num;
		MADDR (wint + line, 0);
		do {
		    i -= (*term.tt_insline) (i);
		} while (i);
	    }
	    MADDR (olin, ocol);
	    to = winul + col
	       + (Short) (line + aheight + num) * (Short) term.tt_width;
	} else {
	    /* first display the change from top to bottom */
	    to = winul + col
		+ (Short) (line + num) * (Short) term.tt_width;
	    i = aheight;
	    do {
		dbmove (to - nch, to, awidth, 1, 0);
		to += term.tt_width;
	    } while (--i);
	}

	/* then update image from bottom to top */
	i = aheight;
	do {
	    to -= term.tt_width;
	    dbmove (to - nch, to, awidth, 0, 1);
	} while (--i);
	if (useinsdel) {
	    i = num;
	    do {
		to -= term.tt_width;
		dbfill (' ', to, awidth, 0, 1);
	    } while (--i);
	}
    }
    return num;
}

#ifndef DUMB
Scols
tabright (n)
register Short n;
{
    register Scols wincol, i;

    wincol = icol - winl;
    if (n == 0)
	return wincol;
    do {
	if (wincol == width - 1)
	    return wincol;
	for (i = wincol + 1; i < width; i++)
	    if (dtabs[i]) {
		wincol = i;
		break;
	    }
    } while (--n);
    return wincol;
}

Scols
tableft (n)
register Short n;
{
    register Scols wincol, i;

    wincol = icol - winl;
    if (n == 0)
	return wincol;
    do {
	if (wincol == 0)
	    return wincol;
	for (i = wincol - 1; i >= 0; i--)
	    if (dtabs[i]) {
		wincol = i;
		break;
	    }
    } while (--n);
    return wincol;
}
#endif

#ifdef COMMENT
void
putmult (chr)
    Char chr;
.
    Puts multiple characters to the display.
#endif
void
putmult (chr)
register Char chr;
{
    register Short i, j;

    for (;;) {
	i = winr - (icursor - lincurs) + 1;
	j = min (i, arg);
	arg -= j;
	dbfill (chr, icursor, j, 1, 1);
	if (j < i) {
	    icursor += j;
	    arg = 1;
	    return;
	}
	if (wrapmode == 0) {
	    icursor = lincurs + winr;
	    state = 10;
	    arg = 1;
	    return;
	}
	icursor = lincurs + winl;
	if (arg == 0 || ilin == winb) {
	    state = 1;
	    arg = 1;
	    break;
	}
	icursor += term.tt_width;
	lincurs += term.tt_width;
	ilin++;
    }
    return;
}

#ifdef COMMENT
void
rollifnec (nn)
    Slines nn;
.
    Put the cursor at the beginning of the next line and scroll the screen
    if necessary.
#endif
void
rollifnec (nn)
register Slines nn;
{
    register Short i;

    if (ilin < winb) {
	i = min (nn, winb - ilin);
	nn -= i;
	ilin += i;
	icursor += (i *= term.tt_width);
	lincurs += i;
    }
#ifdef  WINDOWSTUFF
    if (nn > 0) {
	if (rollmode)
	    scroll(wint, nn, 1);
	else
	    state = 13;
    }
#endif
    return;
}

#ifdef  WINDOWSTUFF

void
scroll (lin, nn, upflg)
Slines lin, nn;
Flag upflg;
{
    register Short to;
    register Short i, n;

    if (nn == 0)
	return;
    n = min (nn, winb - lin + 1);
    nn = n;
    n *= term.tt_width;
    if (upflg) {
	to = (Short) lin * (Short) term.tt_width + winl;
	if (i = winb - lin + 1 - nn) {
	    do {
		dbmove (to + n, to, width, 1, 1);
		to += term.tt_width;
	    } while (--i);
	}
	i = nn;
	if (noclear)
	    noclear = 0;
	else
	    do {
		dbfill (' ', to, width, 1, 1);
		to += term.tt_width;
	    } while (--i);
    }
#ifndef DUMB
    else {
	/* first display the change from top to bottom */
	to = lin * term.tt_width + winl;
	i = nn;
	if (noclear)
	    to += term.tt_width * i;
	else
	    do {
		dbfill (' ', to, width, 1, 0);
		to += term.tt_width;
	    } while (--i);
	i = winb - lin + 1 - nn;
	if (i)
	    do {
		dbmove (to - n, to, width, 1, 0);
		to += term.tt_width;
	    } while (--i);

	/* then update image from bottom to top */
	to = winb * term.tt_width + winl;
	i = winb - lin + 1 - nn;
	if (i)
	    do {
		dbmove (to - n, to, width, 0, 1);
		to -= term.tt_width;
	    } while (--i);
	i = nn;
	if (noclear)
	    noclear = 0;
	else
	    do {
		dbfill (' ', to, width, 0, 1);
		to -= term.tt_width;
	    } while (--i);
    }
    return;
#endif
}

void
clwindow ()
{
    register Short i;
    register Short to;

    icursor = winul;
    lincurs = winul - winl;
    ilin = wint;
    to = icursor;
    i = winb - wint + 1;
    do {
	dbfill (' ', to, width, 1, 1);
	to += term.tt_width;
    } while (--i);
    return;
}
#endif

#ifdef COMMENT
void
dbfill (chr, to, nchars, displflg, wrtflg)
    Uchar chr;
    Short to;
    Short nchars;
    Flag displflg;
    Flag wrtflg;
.
    Fill the screen with nchars characters (chr) starting at image[to].
    If displflg, update the terminal screen.
    If wrtflg, update the internal image.
    Nchars must not be enough to go beyond the right edge of the screen.
    Try to use tt_erase if chr is a blank.
#endif
void
dbfill (chr, to, nchars, displflg, wrtflg)
Uchar chr;
Short to;
Reg1 Short nchars;
Flag displflg;
Flag wrtflg;
{

    /* XXXXXXXXXXXXXXXXX */
    chr &= 0377;

    if (nchars <= 0)
	return;
    if (displflg) {
	if (chr == ' ' && nchars > 5) {
	    if (term.tt_erase) {
		putscr (0); /* update terminal cursor */
		(*term.tt_erase) (nchars);
	    }
	    else if (term.tt_clreol) {
		Reg6 Uchar *cp;
		Block {
		    Reg2 Uchar *rto;
		    Reg3 Uchar *lim;
		    rto = &image[to + nchars];
		    lim = &image[lincurs + term.tt_width];
		    cp = (Uchar *) 0;
		    while (rto < lim)
			if (*rto++ != ' ') {
			    if (cp)
				goto punt;
			    cp = &rto[-1];
			}
		}
		Block {
		    Reg3 Short savicursor;
		    Reg4 Scols savicol;
		    Reg5 Slines savilin;

		    savicol = icol;
		    savilin = ilin;
		    savicursor = icursor;
		    icursor = to;
		    ilin = to / term.tt_width;
		    icol = to % term.tt_width;
		    putscr (0); /* update terminal cursor */
		    (*term.tt_clreol) ();
		    if (cp) {
			icol = (icursor = cp - image) - lincurs;
			putscr (*cp);
		    }
		    icursor = savicursor;
		    ilin = savilin;
		    icol = savicol;
		}
	    }
	    else
		goto punt;
	}
	else Block {
	    Reg3 Uchar rchr;
	    Reg4 Short savicursor;
	    Reg5 Scols savicol;
	    Reg5 Slines savilin;
	    Reg6 Short savnchars;
	    Reg2 Uchar *rto;
punt:
	    rto = &image[to];
	    rchr = chr;
	    savicursor = icursor;
	    savilin = ilin;
	    savicol = icol;
	    icursor = to;
	    ilin = to / term.tt_width;
	    icol = to % term.tt_width;
	    savnchars = nchars;
	    do {
		if (*rto++ != rchr)
		    putscr (rchr);
		icursor++;
		icol++;
	    } while (--nchars);
	    icursor = savicursor;
	    ilin = savilin;
	    icol = savicol;
	    nchars = savnchars;
	}
    }
    if (wrtflg)
	fill ((char *) &image[to], (Uint) nchars, chr);
    return;
}

void
dbmove (from, to, nchars, displflg, wrtflg)
Short from, to; /* indexes into the image[] array */
Short nchars;
Flag displflg;
Flag wrtflg;
{

    if (nchars <= 0 || from == to)
	return;
    if (displflg) {
	register Uchar *rfrom;
	register Short rnchars;
	Short savicursor;
	Slines savilin;
	Scols savicol;
	register Uchar *rto;
	rto = &image[to];
	rfrom = &image[from];
	rnchars = nchars;
	savicursor = icursor;
	savicol = icol;
	savilin = ilin;
	icursor = to;
	ilin = icursor / term.tt_width;
	icol = icursor % term.tt_width;
	do {
	    if (*rto++ != *rfrom)
		putscr (*rfrom++);
	    else
		rfrom++;
	    icursor++;
	    icol++;
	} while (--rnchars);
	icursor = savicursor;
	ilin = savilin;
	icol = savicol;
    }
    if (wrtflg)
	move ((char *) &image[from], (char *) &image[to],
	      (Uint) nchars);
    return;
}

/* ----------------------------------------------------
    Old version, MPRTOK seem to be not any more usefull for
	new class of terminal
#define putx(c) if ((c) < FIRSTSPCL && MPRTOK ()) \
		putchar (c); else MXLATE (c)
------------------------------------------------------- */
static void putx (int chr)
{
#ifdef CHAR7BITS
    if ( chr >= FIRSTSPCL )
#else
    if ( chr >= FIRSTSPCL && chr <= 0237 )
#endif
	MXLATE (chr);
    else putchar (chr);
}

#ifdef COMMENT
void
putscr (chr)
    Uint chr;
.
    Put out chr at icursor.
    Should only be called with printing characters, or 0 which forces
    the cursor to be where it is supposed to be.
    This routine would be better structured if it called two routines:
    one to get us in the right position, and one to put out the character.
    However, since it is called once per output character, that would
    cost a bit in performance.
#endif
/* XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
	Modif in processing of the character paramter : to be
	sure that in any case (signed or unsigned char ...) the
	value is in the correct range : see chr = chrxx & 0377
	F. Perriollat : Dec 1996
   XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX */
void
putscr (chrxx)
int chrxx;
{
    static Flag wrapflg;
    register Slines lin;
    register Scols col;
    int chr;

    if (silent)
	return;
    col = icol;
    lin = ilin;
    if (ocursor == icursor)
	wrapflg = NO;
    else {
	if (wrapflg) {
	    wrapflg = NO;
	    olin--;
	    if (icursor == (ocursor -= term.tt_width)) {
		MCRET ();   /* if we got this far, then MCRET will work */
		goto there;
	    }
	}
	if (col == 0) {
	    switch (lin - olin) {
	    case -1:
		if (!term.tt_cret || !term.tt_nup)
		    goto addr;
		MCRET ();
		MUP ();
		break;

	    case 0:
		if (!term.tt_cret)
		    goto addr;
		MCRET ();
		break;

	    case 1:
		if (MCWR () == 1 && icursor - ocursor == 1)
		    goto wrap;
		if (!term.tt_nl)
		    goto addr;
		MNL ();
		break;

	    default:
		if (icursor == 0) {
		    MHOME ();
		    break;
		}
		goto try1;
	    }
	}
	else {
 try1:      switch (icursor - ocursor) {
	    case -3:
		if (   !tt_lt3
		    || (col >= term.tt_width - 3 && MWL () != 1)
		   )
		    goto addr;
		MLEFT ();
		MLEFT ();
		MLEFT ();
		break;

	    case -2:
		if (   !tt_lt2
		    || (col >= term.tt_width - 2 && MWL () != 1)
		   )
		    goto addr;
		MLEFT ();
		MLEFT ();
		break;

	    case -1:
		if (   term.tt_nleft == 0
		    || (col >= term.tt_width - 1 && MWL () != 1)
		   )
		    goto addr;
		MLEFT ();
		break;

	    case 3:
		if (col <= 2 && MCWR () != 1)
		    goto addr;
		if (!tt_rt3) {
		    if (   image[ocursor]     != ' '
			|| image[ocursor + 1] != ' '
			|| image[ocursor + 2] != ' '
		       )
			goto addr;
		    fwrite("   ", 1, 3, stdout);
		} else {
		    MRIGHT ();
		    MRIGHT ();
		    MRIGHT ();
		}
		ocursor += 3;
		break;

	    case 2:
		if (col <= 1 && MCWR () != 1)
		    goto addr;
		if (!tt_rt2) {
		    if (   image[ocursor]     != ' '
			|| image[ocursor + 1] != ' '
		       )
			goto addr;
		    fwrite("  ", 1, 2, stdout);
		} else {
		    MRIGHT ();
		    MRIGHT ();
		}
		ocursor += 2;
		break;

	    case 1:
		if (col <= 0 && MCWR () != 1)
		    goto addr;
		if (term.tt_nright != 1) {
		    if (image[ocursor] == ' ')
			putchar(' ');
		    else if (term.tt_nright)
			goto wrap;
		    else
			goto addr;
		} else
 wrap:              MRIGHT ();
		ocursor++;
		break;

	    default:
	    addr:
		if (MAXIS ()) {
		    if (lin == olin) {
			if (MAXIS () & 2)
			    MCAD (col);
			else
			    goto coordinate;
		    }
		    else if (col == ocol) {
			if (-2 <= lin - olin && lin - olin <= 2) {
			    switch (lin - olin) {
			    case -2:
				if (term.tt_nup == 0)
				    goto coordinate;
				MUP ();
				MUP ();
				break;

			    case -1:
				if (term.tt_nup == 0)
				    goto coordinate;
				MUP ();
				break;

			    case 0:
				/* imossible */
				break;

			    case  2:
				if (term.tt_ndn == 0)
				    goto coordinate;
				MDN ();
				MDN ();
				break;

			    case  1:
				if (term.tt_ndn == 0)
				    goto coordinate;
				MDN ();
				break;
			    }
			}
			else if (MAXIS () & 1)
			    MLAD (lin);
			else
			    goto coordinate;
		    }
		    else
			goto coordinate;
		}
		else
 coordinate:        MADDR (lin, col);
		break;
	    }
	}
	ocursor = icursor;
	olin = lin;
	ocol = col;
    }

 there:
    chr = chrxx & 0377;
    if (chr < 040) {
	if (wrapflg) {
	    wrapflg = NO;
	    MNL ();     /* if we got this far, then MNL will work */
	}
	fflush (stdout);
	return;
    }

    if (col != term.tt_width - 1) {
	ocursor++;
	ocol++;
	putx (chr);
    }
    else {
	switch (MPWR ()) {
	case 0:
	    if (ocursor != screensize - 1) {
		olin = term.tt_height + 10;
		ocol = term.tt_width + 10;
		ocursor = olin * ocol;
		putx (chr);
	    }
	    break;

	case 1:
	    if (ocursor != screensize - 1) {
		ocursor++;
		olin++;
		ocol = 0;
		putx (chr);
	    }
	    break;

	case 2:
	    ocursor -= term.tt_width - 1;
	    ocol = 0;
	case 3:
	    putx (chr);
	    break;

	case 4:
	    wrapflg = YES;
	    ocursor++;
	    olin++;
	    ocol = 0;
	    putx (chr);
	    break;
	}
    }
    return;
}

#ifdef  SIGNALS
#include SIG_INCL
#ifdef SIGTSTP
#ifdef COMMENT
void
dostop ()
.
    Do the stop command.  Fix tty modes, stop, and fix them back on resume.
    Same function is used for both of these:
      do the stop command
      catch a stop signal
#endif
/* XXXXXXXXXXXXXXXXXXXXXXXXXXXXXX */
void dostop ()
{
    fixtty ();                          /* restore tty modes */
    screenexit (YES);                   /* clean up screen as if exiting */
    (void) signal (SIGTSTP, SIG_DFL);
    kill (0, SIGTSTP);                  /* stop us */
    /* we reenter here after stop */
    (void) signal (SIGTSTP, (void (*)()) dostop);
    setitty ();                         /* set tty modes */
    setotty ();                         /* set tty modes */
    fputs (kbinistr, stdout);           /* re-init the keyboard mode */
    (*kbd.kb_init) ();                  /* initialize keyboard */
    (*term.tt_ini1) ();                 /* initialize the terminal */
    fresh ();                           /* redraw the screen */
    windowsup = YES;                    /* windows are set up */
    return;
}
#endif
#endif

#ifdef COMMENT
void
fresh ()
.
    Put up a fresh screen.
#endif
void
fresh ()
{
    savecurs ();
    redraw = YES;
    d_init (NO, YES);
    d_write (image, screensize);
    redraw = NO;
    restcurs ();
    return;
}

#if 0
void new_screen ()
{
    savecurs ();
    redraw = YES;
    d_init (YES, YES);
    d_write (image, screensize);
    redraw = NO;
    restcurs ();
    return;
}
#endif

/* save / restaure on line from the screen image */
int svrs_image_line (char *saved,  int lnb, int sz, Flag save_flg)
{
    int pos, lsz;

    if ( save_flg ) {
	memcpy (saved, (char *) &image[icursor], sz);
    }
    else {
	memcpy ((char *) &image[icursor], saved, sz);
    }
    return icursor;
}
