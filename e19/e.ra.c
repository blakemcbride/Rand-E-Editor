#ifdef COMMENT
--------
file e.ra.c
    range
#endif

#ifdef COMMENT
	Copyright abandoned, 1983, The Rand Corporation
#endif


#include "e.h"
#include "e.m.h"
#include "e.cm.h"


extern Nlines rangelimit ();
extern void showrange ();

#ifdef COMMENT
Nlines
rangelimit (line, delta, limit)
    Nlines  line;
    Small delta;
    Nlines  limit;
.
    If RANGESET, limit the limit to the end of the range.
    If line is less than range, return -1.
#endif
Nlines
rangelimit (line, delta, limit)
Nlines  line;
Small delta;
Reg1 Nlines  limit;
{
    Reg2 Nlines brange;
    Reg3 Nlines erange;

    if (!(curwksp->wkflags & RANGESET))
	return limit;
    brange = la_lseek (curwksp->brnglas, 0, 1);
    erange = la_lseek (curwksp->ernglas, 0, 1);
    if (delta > 0) {
	if (line < brange)
	    return -1;
	return min (limit, erange-1);
    }
    else {
    /*  if (line > erange)  */
	if (line >= erange)
	    return -1;
	return max (limit, brange);
    }
}

#ifdef COMMENT
Cmdret
rangecmd (type)
    Small type;
.
    Do the 'range', '-range', 'range ?' and '?range' commands.
#endif
Cmdret
rangecmd (type)
Small type;
{
    if ( type != CMDRANGE && opstr[0] ) {
	return CRTOOMANYARGS;
    }
    if ( type == CMDRANGE && (opstr[0] == '?') ) type = CMDQRANGE;

    switch (type) {
    case CMDQRANGE:
	if (curwksp->brnglas) {
	    showrange ();
	    return CROK;
	}
	else
	    return NORANGERR;

    case CMDRANGE:
	if (   opstr[0] == '\0'
	    && !curmark
	   ) {
	    if (curwksp->brnglas) {
#ifdef LMCTRAK
		if (curwksp->wkflags & RANGESET)
		    curwksp->wkflags &= ~RANGESET;
		else
#endif
		    curwksp->wkflags |= RANGESET;
		showrange ();
		break;
	    }
	    else
		return NORANGERR;
	}
	Block {
	    Reg1 Nlines nlines;
	    if (opstr[0] != '\0') {
		if (*nxtop)
		    return CRTOOMANYARGS;
		Block {
		    char *str;
		    str = opstr;
		    /* arg string
		     *  0: was empty
		     *  1: contained only a number
		     *  2: contained only a rectangle spec (e.g. "12x16")
		     *  3: contained a single-word string
		     *  4: contained a multiple-word string
		     **/
		    switch (getpartype (&str, YES, NO, curwksp->wlin + cursorline)) {
		    case 1: Block {
			    Reg1 char *cp;
			    for (cp = str; *cp && *cp == ' '; cp++)
				continue;
			    if (*cp != 0) {
		    default:
				mesg (ERRSTRT + 1, opstr);
				return CRUNRECARG;
			    }
			}
			nlines = parmlines;
			break;

		    case 2:
			return NORECTERR;

		    }
		}
		nlines = parmlines;
	    }
	    else {
		if (markcols)
		    return NORECTERR;
		if (gtumark (NO)) {
		    savecurs ();
		    putupwin ();
		    restcurs ();
		}
		nlines = marklines;
		unmark ();
	    }
	    if (!curwksp->brnglas) {
		if (!(curwksp->brnglas = la_clone (curlas, (La_stream *) 0)))
		    return NOMEMERR;
		if (!(curwksp->ernglas = la_clone (curlas, (La_stream *) 0))) {
		    (void) la_close (curwksp->brnglas);
		    return NOMEMERR;
		}
		/*new (optional)*/
		la_stayset( curwksp->brnglas );
	    }
	    (void) la_lseek (curwksp->brnglas, curwksp->wlin + cursorline, 0);
	    (void) la_align (curwksp->brnglas, curwksp->ernglas);
    /*      (void) la_lseek (curwksp->ernglas, nlines - 1, 1); */
	    (void) la_lseek (curwksp->ernglas, nlines, 1);
	}
	curwksp->wkflags |= RANGESET;
	break;

    case CMD_RANGE:
	if (curwksp->wkflags & RANGESET)
	    curwksp->wkflags &= ~RANGESET;
	else
	    return NORANGERR;
	break;
    }
    inforange (type == CMDRANGE);
    return CROK;
}

#ifdef COMMENT
void
showrange ()
.
    Tell the user what the current range is.
#endif
void
showrange ()
{
    char msgbuf[80];
    Reg1 Nlines begin;
    Reg2 Nlines end;

    begin = la_lseek (curwksp->brnglas, 0, 1) + 1;
/*  end = la_lseek (curwksp->ernglas, 0, 1) + 1; */
    end = la_lseek (curwksp->ernglas, 0, 1);

    /*
     *  It's possible to have a zero range
     */
    if (begin == end + 1)
	sprintf (msgbuf, "%d lines at %d", 0, begin);
    else
	sprintf (msgbuf, "%d through %d = %d lines", begin, end,
		end - begin + 1);
    mesg (TELALL + 2, (curwksp->wkflags & RANGESET)
		      ? " Current range is "
		      : " Dormant range is ",
		      msgbuf);
    loopflags.hold = YES;
    return;
}

