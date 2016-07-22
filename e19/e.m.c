#ifdef COMMENT
--------
file e.m.c
    mainloop
#endif

#ifdef COMMENT
	Copyright abandoned, 1983, The Rand Corporation
#endif

#include "e.h"
#include "e.cm.h"
#include "e.inf.h"
#include "e.m.h"
#include "e.tt.h"
#include "e.e.h"

#ifdef SYSSELECT
#include <sys/time.h>
#else
#include SIG_INCL
#endif

char ambiguous_err_str [] =  "Ambiguous or too short abreviation of Editor command.";

extern Flag impl_tick_flg;
extern void set_impl_tick ();
extern Flag reset_impl_tick ();
extern void test_at_impl_tick ();
extern void reset_searchkey_ref ();

/* check_pattern : check and compiled regular expression if needed */
/* --------------------------------------------------------------- */
/*      return YES if every thing ok */

Flag check_pattern (char *patv)
{
    extern char *re_comp();
    char *s2, *tmpb, b[BUFSIZ];

    if ( ! patmode ) return (YES);

    for (s2 = patv, tmpb = b ; *s2 ; tmpb++, s2++ ) {
	if (*s2 == ESCCHAR && (s2[1]|0140) == 'j'){
	    mesg (ERRALL + 1, "use ^ or $ rather than ^J");
	    return (NO);
	}
	else *tmpb = *s2;
    }
    if (tmpb[-1] == '\\')
	tmpb[-1] = '$';
    *tmpb = '\0';
    if (b[0] == '^' && b[1] == '\0'){
	b[1] = '.';
	b[2] = '\0';
    }
    else if (b[0] == '$' && b[1] == '\0'){
	b[0] = '.';
	b[1] = '$';
	b[2] = '\0';
    }
    if ((s2 = re_comp(b)) != (char *) 0){
	mesg (ERRALL + 3, b, ": ", s2);
	return (NO);
    }
    return (YES);
}

/* my_getkey : local to call getkey */
/* -------------------------------- */

static void bare_getkey ()
{
    extern void testandset_resize ();
    static struct timeval nulsec = { 0, 0};

#ifdef LMCLDC
    (*term.tt_xlate) (0);   /* reset the graphics if needed */
#endif

    testandset_resize (YES, NULL, NULL);
    getkey (WAIT_KEY, nulsec);
    testandset_resize (NO, NULL, NULL);
}


static void my_getkey (Flag ccnull_flg)
{
    bare_getkey ();
#ifdef LMCCMDS
    if ( ccnull_flg && (key == CCNULL) ) return;
#endif
    if (loopflags.hold) {
	/* erase previous message */
	extern char *command_msg;
	loopflags.hold = NO;
	mesg (TELALL);
	command_msg = NULL;
    }
}

static Cmdret call_help_ambiguous (Flag ambig_flg, char *str)
{
    extern char * help_ambiguous_str ();
    char my_para [128];
    Cmdret donetype;

    sprintf (my_para, "%s %s", help_ambiguous_str (ambig_flg), str ? str : "");
    donetype = command (CMDHELP, my_para);
    return donetype;
}


/* Wait for a key, erase error message, check for ambiguous help
 *   and continue in command line processing.
 */
static void getkey_help (Flag uselast_flg)
{
    extern int check_registered_error ();
    extern Flag check_ambiguous_param ();
    extern char * help_cmd_str ();

    if ( ! uselast_flg ) {
	/* nothing to re process in command line */
	my_getkey (NO);
	return;
    }
    if ( loopflags.hold ) {
	/* wait with cursor on command line */
	mesg (TELSTRT);
	bare_getkey ();
	loopflags.hold = NO;
	mesg (TELDONE);
    }
    if ( (key == CCHELP) && (check_registered_error () < 0) ) {
	(void) call_help_ambiguous (check_ambiguous_param (), NULL);
    }
}

/* Set_info_level & update_info_line :
    Routines to set and write the 'At' or 'ch' info at bottom of the window

    at_info_level values :
       -1       : cursor position (only line nb), old style
	0       : cursor position : <line>x<column>
	1, 2, 3 : char at cursor position code in octal, decimal, hexa
*/

static int at_info_level = 0;

void update_info_line ()
{
    Nlines nline;
    Ncols ncol;
    unsigned char ch, ich[32];

    if ( ! curwksp ) {
	ich[0] = '\0';
	infoline = -1;
    } else {
	test_at_impl_tick ();
	nline = curwksp->wlin + cursorline +1;
	ncol  = curwksp->wcol + cursorcol  +1;
	if ( at_info_level <= 0 ) {
	    /* display cursor position */
	    if ( at_info_level < 0 ) {
		if ( infoline == nline ) return;
		/* old style : just line nb, update only if line was changed */
		sprintf (ich, "%5d", nline);
	    }
	    else /* new style, display cursor position : 'line'x'column' */
		sprintf (ich, "%5dx%d", nline, ncol);
	} else {
	    /* display char code */
	    if ( clineno != (nline-1) ) getline (nline-1);
	    if ( ncol <= ncline ) {
		ch = (unsigned char) cline[ncol-1];
		switch (at_info_level) {
		    case 1 :    /* octal */
			sprintf (ich, "%04o", ch);
			if ( ich[1] == '0' ) ich[0] = ' ';
			break;

		    case 2 :    /* decimal */
			sprintf (ich, "%3d", ch);
			break;

		    case 3 :    /* hexadecimal */
			sprintf (ich, "0X%02x", ch);
			break;

		    default :
			return;
		}
	    } else ich[0] = '\0';
	}
	infoline = nline;
    }
    rand_info (inf_line, 9, ich);
}

void set_info_level (int level)
{
    /* 0 is the default value : no vale provided for 'set info' command */
    if ( (level > 3) || (level < -1) ) return;   /* value out of range */
    at_info_level = level;
    rand_info (inf_at, 2, (at_info_level > 0) ? "ch" : "At");
    infoline = -1;  /* force refresh in any case */
    update_info_line ();
}

int get_info_level ()
{
    return at_info_level;
}

#ifdef COMMENT
void
mainloop ()
.
    Get keys from the input and process them.
#endif
void
mainloop ()
{
    extern int command_class ();
    extern void testandset_resize ();
    extern void marktick (Flag set);
    extern void marktick_msg (Flag set, Flag msg);
    extern void goto_tick_or_prev ();
    extern char * help_cmd_str ();
    extern void clear_ambiguous_param ();
    extern Flag test_ctrlc ();

    static char *nix = "\0";

#ifdef LMCCMDS
    static char *abortstg = "ab\0";
    static char *quitstg = "q\0";
    static char *pickstg = "pick\0";
    static char *boardstg = "status\0";
    static char *editstg = "?\0";
    /*
    char *boardstg = "l\0";
    */
#endif
    static struct timeval nulsec = { 0, 0};

    Flag uselast;       /* for re_param call */
    Flag gotcmd_flg;    /* go to gotcmd directly */
    int sz;

    uselast = gotcmd_flg = NO;

    /* Main editor loop */
    /* ---------------- */
    for (;;) {
	if ( gotcmd_flg ) {
	    /* continue with command line input after an error msg */
	    getkey_help (uselast);
	    keyused = YES;
	    gotcmd_flg = NO;
	    if ( !test_ctrlc () ) goto gotcmd;
	    /* return with Ctrl C : do not reuse the last command line */
	    uselast = NO;
	}

funcdone:
	loopflags.clear = YES;

newnumber:
	if (loopflags.clear && !loopflags.hold) {
	    mesg (TELALL + 1, loopflags.beep ? "\007" : "");
	    loopflags.beep = NO;
	}

	if (curfile != infofile && (fileflags[curfile] & INUSE)) {
	    if ( names [infofile] ) sz = strlen (names [infofile]);
	    else {
		sz = 255;
	    }
	    sz = (names [infofile]) ? strlen (names [infofile]) : 255;
	    rand_info (inf_file, sz, names[curfile]);
	    infofile = curfile;
	}

	if (loopflags.flash) {
	    setbul (YES);
	    loopflags.flash = NO;
	}

contin:
	update_info_line ();
	keyused = YES;
	if (curwksp->wkflags & RANGESET) {
	    /* Update the RANGE postion display */
	    infoprange (curwksp->wlin + cursorline);
	}

	if (curmark) Block {
	    /* Update the MARK postion display */
	    Reg2 Nlines nlines;
	    Block {
		Reg1 Nlines curline;
		curline = curwksp->wlin + cursorline;
		if ((nlines = curline - (curmark->mrkwinlin + curmark->mrklin)) < 0)
		    nlines = -nlines;
	    }
	    ++nlines;
	    Block {
		Reg1 Ncols ncols;
		if ((ncols = curwksp->wcol + cursorcol
		    - (curmark->mrkwincol + curmark->mrkcol)) < 0)
		    ncols = -ncols;
		if (marklines != nlines) {
#ifdef LA_LONGFILES
		    sprintf (mklinstr, "%d", nlines);
#else
		    sprintf (mklinstr, "%d", nlines);
#endif
		    marklines = nlines;
		}
		if (markcols != ncols) {
		    if (ncols)
#ifdef LA_LONGLINES
			sprintf (mkcolstr, "x%D", ncols);
#else
			sprintf (mkcolstr, "x%d", ncols);
#endif
		    else
			mkcolstr[0] = '\0';
		    markcols = ncols;
		}
	    }
	    Block {
		Reg1 char *cp;
		Reg3 int len;
		cp = append (mklinstr, mkcolstr);
		len = strlen (cp);
		rand_info (inf_area, max (len, infoarealen), cp);
		infoarealen = len;
		sfree (cp);
	    }
	}

	if (borderbullets) {
	    dobullets (loopflags.bullet, loopflags.clear);
	    loopflags.bullet = NO;
	}
	loopflags.clear = NO;

	if (cmdmode) {
	    goto gotcmd;
	}

	/* ++ new version : >= 19.58 */
	my_getkey (YES);    /* CCNULL does not clear command window msg */

#ifdef LMCCMDS
	if ( key == CCNULL ) {
	    /* hold message generated during getkey processing */
	    goto newnumber;
	}
#endif
	/* -- new version : >= 19.58 */


#if 0   /* ------ old version : < 19.58 */

#ifdef LMCLDC
	(*term.tt_xlate) (0);   /* reset the graphics if needed */
#endif

	testandset_resize (YES, NULL, NULL);
	getkey (WAIT_KEY, nulsec);
	testandset_resize (NO, NULL, NULL);

#ifdef LMCCMDS
	if ( key == CCNULL ) {
	    /* hold message generated during getkey processing */
	    goto newnumber;
	}
#endif

	if (loopflags.hold) {
	    extern char *command_msg;

	    loopflags.hold = NO;
	    mesg (TELALL);
	    command_msg = NULL;
	}
#endif /* ------ old version */

	Block {
	    Reg1 Small donetype;
	    if (   !CTRLCHAR
		|| key == CCCTRLQUOTE
		|| key == CCBACKSPACE
		|| key == CCDELCH
	       ) {
		donetype = printchar ();
		/* write something in the file window, reset impl_tick mark */
		if ( impl_tick_flg ) reset_impl_tick ();
		goto doneswitch;
	    }
	    Block {
		Reg2 Small cm;
		if (key < MAXMOTIONS && (cm = cntlmotions[key])) {
		    /* in the next several lines, a redisplay is always done,
		     *  EXCEPT
		     * +TAB, -TAB, and <- and->arrows within the screen
		     **/
		    if (   (cm == RT && cursorcol + 1 <= curwin->redit)
			|| (cm == LT && cursorcol - 1 >= curwin->ledit)
			|| cm == TB
			|| cm == BT
		       )
			{}
		    else
			credisplay (NO);
		    if (key == CCRETURN)
			if (autofill) Block {
			    Ncols nc, df;
			    Nlines nl;
			    nl = (cursorline == curwin->btext) ? defplline : 0;
			    nc = autolmarg - curwksp->wcol;
			    for (df = 0; nc < 0; nc += deflwin, df += deflwin);
			    cursorline -= nl;
			    cursorcol = nc;
			    if (nl || df)
			      movewin(curwksp->wlin + nl, curwksp->wcol - df,
				cursorline, cursorcol, YES);
			}
			else if (cursorline == curwin->btext)
			    /* Return on bottom line => +lines    */
			    vertmvwin (defplline);
		    /*
		     *  e18 mod:  An RT (right arrow) at the right window
		     *  boundary no longer shifts the window left 1.
		     *  Also, an LT (left arrow) no longer shifts the
		     *  window right 1.
		     */
#ifdef LMCSRMFIX
		    if (optstick) {
#endif
			if (cm == RT) {
			    if (cursorcol + 1 <= curwin->redit)
				movecursor (cm, 1);
			}
			else if (cm == LT) {
			    if (cursorcol - 1 >= curwin->ledit)
				movecursor (cm, 1);
			}
			else
			   movecursor (cm, 1);
#ifdef LMCSRMFIX
		    } else {
			if (cm == RT && cursorcol >= curwin->redit) {
			    movewin (curwksp->wlin, curwksp->wcol + defrwin,
				     cursorline, cursorcol + 1 - defrwin, YES);
			} else if (cm == LT && cursorcol <= curwin->ledit) {
			    Ncols df;
			    df = min (deflwin, curwksp->wcol);
			    movewin (curwksp->wlin, curwksp->wcol - df,
				     cursorline, cursorcol - 1 + df, YES);
			} else
			    movecursor (cm, 1);
		    }
#endif

		    if (infoline != curwksp->wlin + cursorline + 1)
			goto newnumber;
		    goto contin;
		}
	    }
	    credisplay (NO);
	    flushkeys ();
	    donetype = CROK;

	    /* margin-stick feature */
	    if (cursorcol > curwin->rtext)
		poscursor (curwin->rtext, cursorline);

	    switch (key) {
		case CCCMD:
		    keyused = YES;
		    goto gotcmd;

		case CCFNAVIG:
		    keyused = YES;
		    goto gotcmd;

		case CCSETFILE:
		    if (curmark)
			goto nomarkerr;
		    (void) reset_impl_tick ();
		    switchfile ();
		    goto funcdone;

		case CCCHWINDOW:
		    if (curmark)
			goto nomarkerr;
		    if (nwinlist > 1) {
			(void) reset_impl_tick ();
			chgwindow (-1);
			loopflags.bullet = YES;
		    }
		    goto funcdone;

		case CCOPEN:
		case CCCLOSE:
		case CCPICK:
		case CCERASE:
		    donetype = edkey (key, NO);
		    goto doneswitch;

		case CCMISRCH:
		case CCPLSRCH:
		    dosearch (key == CCPLSRCH ? 1 : -1);
		    goto funcdone;

		case CCINSMODE:
		    tglinsmode ();
		    goto funcdone;

		case CCLWINDOW:
		    horzmvwin (-deflwin);
		    goto funcdone;

		case CCRWINDOW:
		    horzmvwin (defrwin);
		    goto funcdone;

		case CCPLLINE:
		    vertmvwin (defplline);
		    goto funcdone;

		case CCMILINE:
		    vertmvwin (-defmiline);
		    goto funcdone;

		case CCPLPAGE:
		    set_impl_tick (NO);
		    vertmvwin (defplpage * (1 + curwin->btext));
		    goto funcdone;

		case CCMIPAGE:
		    set_impl_tick (NO);
		    vertmvwin (-defmipage * (1 + curwin->btext));
		    goto funcdone;

		case CCINT:
		    {
			Flag flg;
			flg = unmark ();
			if ( reset_impl_tick () || flg ) goto funcdone;
			goto nointerr;
		    }

		case CCTABS:
		    sctab (curwksp->wcol + cursorcol, YES);
		    goto funcdone;

		case CCMARK:
		    mark ();
		    goto funcdone;

		case CCTICK:
		    goto_tick_or_prev ();
		    goto funcdone;

		case CCREPLACE:
		    replkey ();
		    goto funcdone;

		case CCSPLIT:
		    donetype = split ();
		    goto funcdone;

		case CCJOIN:
		    donetype = join ();
		    goto funcdone;

#ifdef LMCCMDS
		case CCCENTER:
		    command (CMDCENTER, nix);
		    goto funcdone;

		case CCREDRAW:
		    command (CMDREDRAW, nix);
		    goto funcdone;

		case CCFILL:
		    command (CMDFILL, nix);
		    goto funcdone;

		case CCJUSTIFY:
		    command (CMDJUST, nix);
		    goto funcdone;

		case CCSTOPX:
		    command (CMDSTOP, nix);
		    goto funcdone;

		case CCBOX:
		    donetype = command (CMDBOX, nix);
		    goto doneswitch;

		case CCTRACK:
		    donetype = command (CMDTRACK, nix);
		    goto doneswitch;

		case CCRANGE:
		    donetype = command (CMDRANGE, nix);
		    goto doneswitch;

		case CCCLEAR:
		    command (CMDCLEAR, nix);
		    goto funcdone;

		case CCEXIT:
		    command (CMDEXIT, nix);
		    goto funcdone;

		case CCABORT:
		    command (CMDEXIT, abortstg);
		    goto funcdone;

		case CCQUIT:
		    command (CMDEXIT, quitstg);
		    goto funcdone;

		case CCCLRTABS:
		    command (CMD_TABS, nix);
		    goto funcdone;
#ifdef LMCHELP
		case CCHELP:
		    command (CMDHELP, nix);
		    goto funcdone;
#endif
		case CCCOVER:
		    donetype = command (CMDCOVER, pickstg);
		    goto doneswitch;

		case CCOVERLAY:
		    donetype = command (CMDOVERLAY, pickstg);
		    goto doneswitch;

		case CCBLOT:
		    donetype = command (CMDBLOT, pickstg);
		    goto doneswitch;
#ifdef LMCCASE
		case CCCCASE:
		    donetype = chgcase (NO);
		    goto doneswitch;

		case CCCAPS:
		    donetype = chgcase (YES);
		    goto doneswitch;
#endif
#ifdef LMCAUTO
		case CCAUTOFILL:
		    infoauto(NO);
		    goto funcdone;
#endif
#endif
		case CCLWORD:
		    donetype = mword(-1, 1);
		    goto funcdone;

		case CCRWORD:
		    donetype = mword(1, 1);
		    goto funcdone;

#ifdef LMCDWORD
		case CCDWORD:
		    donetype = dodword (YES);
		    goto funcdone;
#endif

		case CCUNAS1:
		case CCDEL:
		    goto notimperr;

		case CCDELCH:
		case CCMOVELEFT:
		case CCTAB:
		case CCMOVEDOWN:
		case CCHOME:
		case CCRETURN:
		case CCMOVEUP:
		default:
		    goto badkeyerr;

		case CCFLIST:
		    (void) command (CMDEDIT, editstg);
		    goto doneswitch;

#ifdef LMCCMDS
/* old processing of CCNULL
		case CCNULL:
		    goto funcdone;
*/
#endif
	    }
	    uselast = NO;
gotcmd:
	    clear_ambiguous_param ();
	    gotcmd_flg = NO;
	    re_param (uselast);
	    uselast = NO;

	    if ( cmdmode && (key != CCRETURN) && (key != CCHELP) )
		goto notcmderr;

	    switch (key) {
		case CCCMD:
		    goto funcdone;

		case CCFNAVIG:
		    goto nomorefile;

		case CCLWINDOW:
		case CCRWINDOW:
		    switch (paramtype) {
		    case 0:
			horzmvwin (key == CCRWINDOW
				   ? (Ncols) cursorcol : - curwksp->wcol);
			break;

		    case 1:
			horzmvwin (key == CCRWINDOW
				   ? (Ncols) parmlines : (Ncols) -parmlines);
			break;

		    case 2:
			goto norecterr;

		    default:
			goto notinterr;
		    }
		    goto funcdone;

		case CCSETFILE:
		    /* enter in edited file list and select (19.57) */
		    (void) command (CMDEDIT, editstg);
		    goto funcdone;
		    /* before 19.57
		    goto notimperr;
		    */

		case CCINT:
		    (void) unmark ();
		    (void) reset_impl_tick ();
		    goto funcdone;

		case CCMISRCH:
		case CCPLSRCH:
		    if (paramtype == 0)
			getarg ();
		    if (*paramv == 0)
			goto noargerr;

		    if (patmode){
			if ( ! check_pattern (paramv) ) goto funcdone;
		    }

		    if (searchkey)
			sfree (searchkey);
		    reset_searchkey_ref ();
		    searchkey = append (paramv, "");
		    dosearch (key == CCPLSRCH ? 1 : -1);
		    goto funcdone;

		case CCBACKSPACE:
		    /* donetype can only be 0 */
		    Block {
			Ncols  k;

			if ( cursorcol > 0 ) {
			    k = curwksp->wcol;
			    if (insmode) {
				donetype = ed (OPCLOSE, QCLOSE,
					       curwksp->wlin + cursorline, k,
					       (Nlines) 1, (Ncols) cursorcol, YES);
				poscursor (0, cursorline);
			    } else {
				savecurs ();
				donetype = ed (OPERASE, QERASE,
					       curwksp->wlin + cursorline, k,
					       (Nlines) 1, (Ncols) cursorcol, YES);
				restcurs ();
			    }
			}
		    }
		    goto doneswitch;

		case CCDELCH:
		    /* donetype can only be 0 */
		    Block {
			Reg2 Ncols k;
			getline (curwksp->wlin + cursorline);
			if ((k = ncline - 1 - curwksp->wcol - cursorcol) > 0) {
			    donetype = ed (OPERASE, QERASE,
					   curwksp->wlin + cursorline,
					   curwksp->wcol + cursorcol,
					   (Nlines) 1, k, YES);
			    goto doneswitch;
			}
		    }
		    goto funcdone;

		case CCOPEN:
		case CCCLOSE:
		case CCPICK:
		case CCERASE:
		    donetype = edkey (key, YES);
		    goto doneswitch;

		case CCMARK:
		    if (paramtype != 0)
			goto notimperr;
		    (void) unmark ();
		    goto funcdone;

		case CCTICK:
		    /* reset implicite tick if any, else set tick */
		    if ( ! reset_impl_tick () ) {
			marktick_msg (YES, (paramtype == 0));
			putchar ('\a');
		    }
		    if (paramtype != 0) {
			/* continue command entry */
			keyused = uselast = YES;
			goto gotcmd;
		    }
		    goto funcdone;

		case CCMOVELEFT:
		case CCMOVEDOWN:
		case CCMOVEUP:
		case CCMOVERIGHT:

		case CCTAB:
		case CCHOME:
		case CCBACKTAB:
		    switch (paramtype) {
			int lns;
		    case 0:
			/* empty command line, just <cmd> <key function> */
			switch (key) {
			case CCHOME:
			    movecursor (cntlmotions[CCHOME], 1);
			    key = CCMOVEDOWN;
			    lns = curwin->btext - cursorline;
			    break;

			case CCMOVEDOWN:
			    if ((lns = la_lsize (curlas) -
				       (curwksp->wlin + cursorline)) > 0)
				lns = min (lns, curwin->btext - cursorline);
			    else
				lns = curwin->btext - cursorline;
			    break;

			case CCMOVEUP:
			    lns = cursorline;
			    break;

			case CCBACKTAB:
			case CCTAB:
			    goto notimperr;

			case CCMOVELEFT:
			    lns = cursorcol;
			    break;

			case CCMOVERIGHT:
			    getline (curwksp->wlin + cursorline);
			    if ((lns = ncline - 1 - (curwksp->wcol + cursorcol))
				  > 0)
				lns = min (lns, curwin->rtext - cursorcol);
			    else
				lns = curwin->rtext - cursorcol;
			    break;
			}
			if (lns <= 0)
			    goto funcdone;
			movecursor (cntlmotions[key], lns);
			break;

		    case 1:
			/* a number */
			if (parmlines <= 0)
			    goto notposerr;
			lns = parmlines;
			movecursor (cntlmotions[key], lns);
			break;

		    case 2:
			/* a rectangle specification */
			goto norecterr;

		    case 3 :
			/* single word string : just a command ? */
			break;

		    default:
			/* multi word string */
			{
			    int cc;
			    if ( key != CCTAB ) {
				/* ignore the key strock */
				putchar ('\a'); /* ring bell */
				keyused = uselast = YES;
				goto gotcmd;
			    }
			    cc = command_class (NULL, NULL, NULL);
			    if ( cc ==  HELP_CMD_CLASS ) goto helperr;
			    if ( cc ==  FILE_CMD_CLASS ) goto notfilerr;
			    if ( cc ==  DIR_CMD_CLASS  ) goto notdirerr;
			    if ( cc ==  0 ) goto nowordcomplerr;
			    if ( cc == -2 ) goto ambiguouserr;
			    goto notinterr;
			}
		    }
		    if ( key == CCTAB ) {
			/* somehing been expanded ? */
			keyused = uselast = YES;
			goto gotcmd;
		    }
		    goto funcdone;

		case CCRETURN:
		    donetype = command (0, nix);
		    goto doneswitch;
#ifdef LMCHELP
		case CCHELP:
		    /* help key function in command line */
		    {
			extern int check_registered_error ();
			extern Flag check_ambiguous_param ();
			extern Cmdret help_ambiguous (Flag *ctrlc_flg, Flag ambig_flg);
			Flag ctrlc_flg;

			if ( check_registered_error () < 0 ) {
			    donetype = help_ambiguous (&ctrlc_flg, check_ambiguous_param ());
			    if ( donetype == CROK ) {
				/* re-display the ambiguous command */
				keyused = YES;
				uselast = !ctrlc_flg;
				if ( uselast ) goto gotcmd;
				goto doneswitch;
			    }
			}
		    }

		    if ( (paramtype == 3) || (paramtype == 4) ) {
			/* 1 or more words, get help on the command (1st word) */
			extern S_looktbl cmdtable[];
			char my_para [128], para1 [128];
			char *str1, *str2;
			int cc;

			memset (my_para, 0, sizeof (my_para));
			memset (para1, 0, sizeof (para1));
			/* get the command 1st word */
			str1 = paramv;
			str2 = getword (&str1);
			if ( str2 [0] != '\0' ) {
			    strncpy (para1, str2, sizeof (para1) -1);
			    sfree (str2);
			    cc = lookup (para1, cmdtable);
			    if ( cc == -2 ) {
				donetype = call_help_ambiguous (YES, para1);
				keyused = uselast = YES;
				goto gotcmd;
			    } else if ( cc < 0 ) {
				if ( cc == -1 ) {
				    extern Flag test_ctrlc ();
				    donetype = call_help_ambiguous (NO, para1);
				    uselast = !test_ctrlc ();
				    keyused = YES;
				    if ( uselast ) goto gotcmd;
				    goto funcdone;
				}
				keyused = uselast = gotcmd_flg = YES;
				goto noargerr;
			    }
			    sprintf (my_para, "%s %s", help_cmd_str (), para1);
			}
			donetype = command (CMDHELP, my_para);
			keyused = uselast = YES;
			goto gotcmd;
		    }
		    else if ( paramtype == 0 ) {
			/* get the default help info */
			char *str;
			str = help_cmd_str ();
			donetype = command (CMDHELP, str);
			keyused = YES;
			uselast = NO;
			goto gotcmd;
		    }

		    keyused = uselast = gotcmd_flg = YES;
		    goto noargerr;
#endif
#ifdef LMCAUTO
		case CCAUTOFILL:
		    infoauto(YES);
		    goto funcdone;
#endif
		case CCMIPAGE:
		case CCPLPAGE:
		    switch (paramtype) {
			extern void savemark (struct markenv *);
			extern void copymark (struct markenv *, struct markenv *);
			struct markenv tmpmk;

		    case 0:
			set_impl_tick (NO);
			gotomvwin (key == CCPLPAGE ? la_lsize (curlas) : 0);
			break;

		    case 1:
			set_impl_tick (NO);
			vertmvwin ((key == CCPLPAGE ? parmlines : -parmlines )
				    * (1 + curwin->btext));
			break;

		    case 2:
			goto norecterr;

		    default:
			goto notinterr;
		    }
		    goto funcdone;

		case CCMILINE:
		case CCPLLINE:
		    switch (paramtype) {
		    case 0:
			vertmvwin (cursorline - (key == CCPLLINE
				   ? 0 : curwin->btext));
			break;

		    case 1:
			vertmvwin (key == CCPLLINE ? parmlines : -parmlines);
			break;

		    case 2:
			goto norecterr;

		    default:
			goto notinterr;
		    }
		    goto funcdone;

		case CCCHWINDOW:
		    if (curmark)
			goto nomarkerr;
		    switch (paramtype) {
		    case 0:
			goto notimperr; /* should be backwards move */

		    case 1:
			if (parmlines <= 0)
			    goto notposerr;
			(void) reset_impl_tick ();
			chgwindow (parmlines - 1);
			loopflags.bullet = YES;
			break;

		    case 2:
			goto norecterr;

		    default:
			goto notinterr;
		    }
		    goto funcdone;

		case CCTABS:
		    if (paramtype == 0)
			sctab (curwksp->wcol + cursorcol, NO);
		    else
			goto notimperr;
		    goto funcdone;

		case CCINSMODE:
		    goto badkeyerr;

		case CCLWORD:
		case CCRWORD:
		    switch (paramtype) {
		    case 0:
			goto notimperr;

		    case 1:
			if (parmlines <= 0)
			    goto notposerr;
			mword((key == CCLWORD) ? -1 : 1, parmlines);
			break;

		    case 2:
			goto norecterr;

		    default:
			goto notinterr;
		    }
		    goto funcdone;

#ifdef LMCDWORD
		case CCDWORD:
		    donetype = dodword (NO);
		    goto funcdone;
#endif
#ifdef LMCCMDS
		case CCCOVER:
		    donetype = command (CMDINSERT, pickstg);
		    goto doneswitch;

		case CCOVERLAY:
		    donetype = command (CMDUNDERLAY, pickstg);
		    goto doneswitch;

		case CCBLOT:
		    donetype = command (CMD_BLOT, pickstg);
		    goto doneswitch;

		case CCFILL:
		    linewidth = cursorcol;
		    command (CMDFILL, nix);
		    goto funcdone;

		case CCJUSTIFY:
		    linewidth = cursorcol;
		    command (CMDJUST, nix);
		    goto funcdone;

		case CCCENTER:
		    linewidth = cursorcol;
		    command (CMDCENTER, nix);
		    goto funcdone;

		case CCEXIT:
		    command (CMDEXIT, nix);
		    goto funcdone;

		case CCRANGE:
		    donetype = command (CMDQRANGE, nix);
		    goto doneswitch;

		case CCREDRAW:
		    donetype = command (CMDREDRAW, nix);
		    keyused = uselast = YES;
		    goto gotcmd;

		case CCBOX:
		case CCABORT:
		case CCCLRTABS:
		case CCCLEAR:
		case CCSTOPX:
		case CCQUIT:
#endif
		case CCSPLIT:
		case CCJOIN:
		case CCUNAS1:
		case CCREPLACE:
		case CCDEL:
		default:
		    goto notimperr;
	    }

doneswitch:
	    switch (donetype) {
	    case RE_CMD:
		/* re display the last command (to correct and replay) */
		keyused = uselast = gotcmd_flg = YES;
		break;

	    case NOMEMERR:
		mesg (ERRALL + 1, "You have run out of memory.  Exit now!");
		break;

	    case NOSPCERR:
		mesg (ERRALL + 1, "No disk space.  Get help!");
		break;

	    case TOOLNGERR:
		mesg (ERRALL + 1, "Can't make file that long");
		break;

	    case NOTSTRERR:
		mesg (ERRALL + 1, "Argument must be a string.");
		keyused = uselast = gotcmd_flg = YES;
		break;

	    case NOWRITERR:
		mesg (ERRALL + 1, "You cannot modify this file!");
		break;

	    noargerr:
	    case NOARGERR:
		mesg (ERRALL + 2, paramv, " : Invalid argument.");
		keyused = uselast = gotcmd_flg = YES;
		break;

	    nomarkerr:
	    case NDMARKERR:
		mesg (ERRALL + 1, "Area must be marked");
		break;

	    case NOMARKERR:
		mesg (ERRALL + 1, "Can't do that with marks set");
		break;

	    norecterr:
	    case NORECTERR:
		mesg (ERRALL + 1, "Can't do that to a rectangle");
		keyused = uselast = gotcmd_flg = YES;
		break;

	    case NOTRECTERR:
		mesg (ERRALL + 1, "Can only do that to a rectangle");
		break;

	    case NORANGERR:
		mesg (ERRALL + 1, "Range not set");
		break;

	    case NOBUFERR:
		mesg (ERRALL + 1, "Nothing in that buffer.");
		break;

	    case CONTIN:
		goto contin;

	    case MARGERR:
		mesg (ERRALL + 1, "Cursor stuck on right margin.");
		break;

	    notinterr:
	    case NOTINTERR:
		mesg (ERRALL + 3, "\"", paramv, "\" : Argument must be numeric.");
		keyused = uselast = gotcmd_flg = YES;
		break;

	    notposerr:
	    case NOTPOSERR:
		mesg (ERRALL + 3, "\"", paramv, "\" : Argument must be positive.");
		keyused = uselast = gotcmd_flg = YES;
		break;

	    notimperr:
	    case NOTIMPERR:
		mesg (ERRALL + 1, "That key sequence is not implemented.");
		break;
	    }
	    /* any other case (included CROK) */

	    /* end of switch donetype : continue in main loop */
	    continue;
	    /* ---------------------------------------------- */

	    /* local error cases */

	    notcmderr:
	    mesg (ERRALL + 1, "Can't do that in Command Mode");
	    continue;

	    nointerr:
	    mesg (ERRALL + 1, "No operation to interrupt");
	    continue;

	    badkeyerr:
	    mesg (ERRALL + 1, "Bad key error - editor error");
	    continue;

	    notfilerr:
	    mesg (ERRALL + 3, "\"", paramv, "\" : Argument must be a file name.");
	    keyused = uselast = gotcmd_flg = YES;
	    continue;

	    notdirerr:
	    mesg (ERRALL + 3, "\"", paramv, "\" : Argument must be a directory name.");
	    keyused = uselast = gotcmd_flg = YES;
	    continue;

	    nomorefile:
	    mesg (ERRALL + 1, "No more file are currently edited.");
	    continue;

	    ambiguouserr:
	    mesg (ERRALL + 4, "\"", paramv, "\" : ", ambiguous_err_str);
	    continue;

	    toomanyargerr:
	    mesg (ERRALL + 3, "\"", paramv, "\" : Too many arguments.");
	    keyused = uselast = gotcmd_flg = YES;
	    continue;

	    helperr:
	    mesg (ERRALL + 3, "\"", paramv, "\" : Argument must be editor command or key function");
	    keyused = uselast = gotcmd_flg = YES;
	    continue;

	    nowordcomplerr:
	    mesg (ERRALL + 3, "\"", paramv, "\" : Word completion is not available for this command");
	    keyused = uselast = gotcmd_flg = YES;
	    continue;

	}
    }
    /* never returns */
    /* NOTREACHED */
}


#ifdef COMMENT
Cmdret
edkey (key, cmdflg)
    Char key;
    Flag cmdflg;
.
    Common code for OPEN, CLOSE, and PICK keys.
    If cmdflg != 0, then do CMD OPEN, CMD CLOSE, CMD PICK.
#endif
Cmdret
edkey (key, cmdflg)
Char key;
Flag cmdflg;
{
    Small opc;
    Small buf;

    switch (key) {
    case CCOPEN:
	opc = OPOPEN;
	buf = 0;
	break;

    case CCPICK:
	opc = OPPICK;
	buf = QPICK;
	break;

    case CCCLOSE:
	opc = OPCLOSE;
	buf = QCLOSE;
	break;

    case CCERASE:
	opc = OPERASE;
	buf = QERASE;
	break;
    }

    if (curmark) {
	if (!cmdflg)
	    return edmark (opc, buf);
	else
	    return NOMARKERR;
    }
    else {
	if (!cmdflg)
	    return ed (opc, buf,
		       curwksp->wlin + cursorline, (Ncols) 0,
		       (Nlines) 1, (Ncols) 0, YES);
	else {
	    switch (paramtype) {
	    case 0:
		if (key == CCOPEN)
		    return NOTIMPERR;
		return ed (OPINSERT, buf,
			   curwksp->wlin + cursorline,
			   curwksp->wcol + cursorcol,
			   (Nlines) 0, (Ncols) 0, YES);

	    case 1:
		if (parmlines <= 0)
		    return NOTPOSERR;
		return ed (opc, buf,
			   curwksp->wlin + cursorline, (Ncols) 0,
			   parmlines, (Ncols) 0, YES);

	    case 2:
		if (parmlines <= 0 || parmcols <= 0)
		    return NOTPOSERR;
		return ed (opc, buf,
			   curwksp->wlin + cursorline,
			   curwksp->wcol + cursorcol,
			   parmlines, (Ncols) parmcols, YES);
	    }
	    return NOTINTERR;
	}
    }
}
