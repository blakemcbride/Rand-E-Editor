#ifdef COMMENT
--------
file e.e.c
    The editing functions that do open, close, pick, put, erase, etc.
#endif

#ifdef COMMENT
	Copyright abandoned, 1983, The Rand Corporation
#endif

#include "e.h"
#include "e.m.h"
#include "e.cm.h"
#include "e.e.h"

/* XXXXXXXXXXXXXXXXXXXXX */
static Cmdret lacmderr ();

extern void putbks ();
extern void clean ();

#ifdef COMMENT
Cmdret
split ()
.
    Do the 'split' command.
    If marks,
	Split the line at the upper mark and put the remainder of the line
	at the lower mark, inserting blank lines if necesary.
    else
	Split the line at current col and move the remainder of the line to
	the beginning of the next line.
#endif
Cmdret
split ()
{
    if (!okwrite ())
	return NOWRITERR;
    if (curmark)
	return splitmark ();
    return splitlines (curwksp->wlin + cursorline,
			curwksp->wcol + cursorcol,
			 (Nlines) 1, (Ncols) 0, YES);
}

#ifdef COMMENT
Cmdret
splitmark ()
.
    Split the line at the upper mark and put the remainder of the line
    at the lower mark, inserting blank lines if necesary.
#endif
Cmdret
splitmark ()
{
    Reg1 Cmdret retval;
    Reg2 Flag moved;

    moved = gtumark (NO);
    retval = splitlines (topmark (),
			 curwksp->wcol + cursorcol,
			 marklines - 1,
			 curmark->mrkwincol + curmark->mrkcol,
			 !moved);
    domark (moved);
    return retval;
}

#ifdef COMMENT
Cmdret
splitlines (line, col, nl, newc, puflg)
    Nlines  line;
    Ncols   col;
    Nlines  nl;     /* must be >= 1 */
    Ncols   newc;
    Flag    puflg;
.
    Split the line at col and move the remainder of the line to line+nl
    starting at newc.
    Update the display if puflg != 0.
#endif
Cmdret
splitlines (line, col, nl, newc, puflg)
Reg1 Nlines  line;
Reg2 Ncols   col;
Reg3 Nlines  nl;     /* must be >= 1 */
Ncols   newc;
Flag    puflg;
{
    if (line >= la_lsize (curlas))
	return CROK;

    getline (line);     /* must do this to set ncline */
    if (col >= ncline - 1)
	return CROK;

    /* append or insert nl blank lines after line */
    getline (-1);
    if (line == la_lsize (curlas) - 1) {
	if (!extend (nl)) {
	    mesg (ERRALL + 1, "Can't extend the file");
	    goto ret;
	}
    }
    else {
	(void) la_lseek (curlas, line + 1, 0);
	if (la_blank (curlas, nl) != nl) {
	    mesg (ERRALL + 1, "Can't make file that long");
	    goto ret;
	}
    }
    /* shorten the first line */
    getline (line);         /* yes, we must do it again */
    Block {
	Reg4 Ncols  nsave;
	Reg5 Char   csave;

	csave = cline[col];
	cline[col] = '\n';
	nsave = ncline;
	ncline = col + 1;
	fcline = YES;
	putline (NO);
	/* make the second line */
	cline[col] = csave;
	ncline = nsave - col;
	if (ncline <= 0)
	    ncline = 2;
	move (&cline[col], cline, (Uint) (ncline));
    }
    putbks ((Ncols) 0, newc);
    clinelas = curlas;
    clineno = line + nl;
    redisplay (curfile, line, 1, 0, puflg);
    redisplay (curfile, line + 1, 0, nl, puflg);

 ret:
    return CROK;
}

#ifdef COMMENT
Cmdret
join ()
.
    Do the 'join' command.
    If marks,
	Join two lines, and delete all text in between.  Use the upper and
	lower marks as the join points.  Put deleted text into the join
	buffer.
    else
	The join command looks forward starting at the beginning of the
	next line for the first non-blank character in the file and joins
	that and the rest of its line to the end of the current line.
	If the cursor position is beyond the end of the current line the
	join point is the cursor position, else it is one space beyond the
	end of the current line.  In no case is any printing text deleted.
	The join buffer is unaffected.
#endif
Cmdret
join ()
{
    if (!okwrite ())
	return NOWRITERR;
    if (curmark)
	return joinmark ();
    return joinlines (curwksp->wlin + cursorline,
		      curwksp->wcol + cursorcol,
		      YES);
}

#ifdef COMMENT
Cmdret
joinmark ()
.
    Join two lines, and delete all text in between.  Use the upper and lower
    marks as the join points.
#endif
Cmdret
joinmark ()
{
    Reg1 Cmdret retval;
    Reg2 Flag moved;

    moved = gtumark (NO);
    retval = dojoin (topmark (),
		     curwksp->wcol + cursorcol,
		     marklines - 1,
		     curmark->mrkwincol + curmark->mrkcol,
		     YES,
		     !moved);
    domark (moved);
    return retval;
}

#ifdef COMMENT
Cmdret
joinlines (line, col, puflg)
    Nlines  line;
    Ncols   col;
    Flag    puflg;
.
    See join() above.
#endif
Cmdret
joinlines (line, col, puflg)
Reg3 Nlines  line;
Ncols   col;
Flag    puflg;
{
    Reg2 Nlines ln;
    Ncols newc;

    getline (line);
    if (col <= ncline)
	col = ncline;
    for (ln = line + 1; ;ln++) Block {
	Reg1 char *cp;
	getline (ln);
	if (xcline)
	    return CROK;
	for (cp = cline; *cp == ' ' || *cp == '\t'; cp++)
	    continue;
	if (*cp != '\n') {
	    newc = cp - cline;
	    break;
	}
    }
    return dojoin (line, col, ln - line, newc, NO, puflg);
}

#ifdef COMMENT
Cmdret
dojoin (line, col, nl, newc, saveflg, puflg)
    Nlines  line;
    Ncols   col;
    Nlines  nl;
    Ncols   newc;
    Flag    saveflg;
    Flag    puflg;
.
    Join two lines, and delete all text in between.  Use [line,col] and
    [line+nl,newc] as the join points.
    Save the end of the last line into an array.
    If saveflg != 0
	Delete the end of the last line and set fcline.
	Save the beginning of the first line into an array,
	delete the beginning of the line, and set fcline.
	Pick all the lines into the join buffer, so the text
	from the end of the top line and the beginning of the bottom line
	are saved as whole lines and everything in between is saved in
	between.
	Replace the first line with the concatenation of the two
	strings.
    else
	Tack it onto the end of the first line, and set fcline.
    Close the remaining lines without putting them into any qbuffer.
#endif
Cmdret
dojoin (line, col, nl, newc, saveflg, puflg)
Nlines  line;
Reg3 Ncols   col;
Reg4 Nlines  nl;
Ncols   newc;
Flag    saveflg;
Flag    puflg;
{
    getline (-1);   /* flush so that file will be its full size */
    Block {
	Reg1 Nlines lsize;
	if (line >= (lsize = la_lsize (curlas)))
	    return CROK;
	if (nl > lsize - line)
	    nl = lsize - line;
    }

    Block {
	char *l1, *l2;
	Reg1 Ncols nl1;
	Reg2 Ncols nl2;

	/* Save the end of the last line into an array. */
	getline (line + nl);
	if ((nl2 = ncline - newc) > 1) {
	    /* get the end of the lower line */
	    l2 = salloc ((int) nl2, YES);
	    move (&cline[newc], l2, (Uint) nl2);
	}
	if (saveflg) {
	    /* Delete the end of the last line and set fcline. */
	    cline[newc] = '\n';
	    ncline = newc + 1;
	    fcline = YES;
	    /* Save the beginning of the first line into an array, */
	    getline (line);
	    if ((nl1 = min (col, ncline - 1)) > 0) {
		l1 = salloc ((int) nl1, YES);
		move (cline, l1, (Uint) nl1);
		/* delete the beginning of the line, and set fcline. */
		if (col < ncline - 1)
		    move (&cline[col], cline, (Uint) (ncline -= col));
		else {
		    cline[0] = '\n';
		    ncline = 1;
		}
		fcline = YES;
	    }
	    /*? check ret val from ed */
	    ed (OPPICK, QCLOSE, line, (Ncols) 0, nl + 1, (Ncols) 0, NO);
	}
	getline (line);
	if (col + nl2 > lcline)
	    excline (col + nl2);
	if (saveflg) {
	    if (nl1 > 0) {
		move (l1, cline, (Uint) nl1);
		sfree (l1);
	    }
	}
	else
	    nl1 = min (col, ncline - 1);
	if (col - nl1 > 0)
	    fill (&cline[nl1], (Uint) (col - nl1), ' ');
	if (nl2 > 1) {
	    move (l2, &cline[col], (Uint) nl2);
	    sfree (l2);
	}
	else
	    cline[col] = '\n';
	ncline = col + nl2;
    }
    fcline = YES;
    redisplay (curfile, line, 1, 0, puflg);
    /* close line + 1 thru line + nl into the 'join' buffer */
    /*? check ret val from ed */
    savecurs ();
    ed (OPCLOSE, QNONE, line + 1, (Ncols) 0, nl, (Ncols) 0, puflg);
    restcurs ();
    return CROK;
}

#ifdef LMCCASE
#ifdef COMMENT
Cmdret
chgcase (xabs)
	Flag xabs;
.
    Change the case of the current character (or marked area).
#endif
Cmdret
chgcase (xabs)
    Flag xabs;
{
    Flag moved;
    Ncols nc, curcol;
    Nlines nl, i;
    Char ch;

    numtyp++;
    if (!okwrite())
	return NOWRITERR;
    if (curmark)
	return edmark (xabs ? OPCAPS : OPCCASE, QNONE);
    else {
	getline (curwksp->wlin + cursorline);
	curcol = cursorcol + curwksp->wcol;
	if (curcol < ncline - 1) {
	    ch = cline [curcol];
	    cline [curcol] = caseit (ch, xabs);
	    if ( ch != cline [curcol] ) {
		putch (cline [curcol], YES);
		fcline = YES;
	    }
	} else
	    movecursor (RT, 1);
    }
    return CROK;
}

#ifdef COMMENT
Char
caseit (key, xabs)
	Flag xabs;
	Char key;
.
	Return key in other case if xabs true and it is an alpha, return
	key capitalized if xabs false.
#endif
Char
caseit (key, xabs)
	Flag xabs;
	Char key;
{
	if ( key >= 'a' && key <= 'z')
		key = (key + 'A' - 'a') & 0177;
	else if ((key >= 'A' && key <= 'Z') && !xabs)
		key = (key + 'a' - 'A') & 0177;
	return key;
}
#endif /* LMCCASE */

#ifdef COMMENT
Cmdret
areacmd (which)
    Small which;
.
    Do a pick, close, erase, etc. as chosen by 'which' on the current area.
#endif
Cmdret
areacmd (which)
Reg2 Small which;
{
#ifdef LMCCASE
    static short  opc[] = {OPPICK, OPCLOSE, OPERASE, OPOPEN, OPBOX, OPCAPS, OPCCASE};
#else  /* LMCCASE */
    static short  opc[5] = {OPPICK, OPCLOSE, OPERASE, OPOPEN, OPBOX};
#endif /* LMCCASE */
    static ASmall buf[5] = {QPICK, QCLOSE, QERASE};

    if (opstr[0] == '\0') {
	if (curmark)
	    return edmark (opc[which], buf[which]);
	else
	    return ed (opc[which], buf[which], curwksp->wlin + cursorline,
		       (Ncols) 0, 1, (Ncols) 0, YES);
    }
    if (curmark)
	return NOMARKERR;
    if (*nxtop)
	return CRTOOMANYARGS;

    Block {
	char *str;
	str = opstr;
	switch (getpartype (&str, YES, NO, curwksp->wlin + cursorline)) {
	case 1: Block {
		Reg1 char *cp;
		for (cp = str; *cp && *cp == ' '; cp++)
		    continue;
		if (*cp)
		    goto badarg;
	    }
	    return ed (opc[which], buf[which], curwksp->wlin + cursorline,
		       (Ncols) 0, parmlines, (Ncols) 0, YES);

	case 2:
	    return ed (opc[which], buf[which], curwksp->wlin + cursorline,
		       curwksp->wcol + cursorcol, parmlines, parmcols, YES);

	case 3:
	    if (which != 0) {
		/*? check for "a" "al" or "all" */
		break;
	    }
	badarg:
	default:
	    mesg (ERRSTRT + 1, opstr);
	    return CRUNRECARG;
	}
    }
    return CROK;
}

#ifdef COMMENT
Cmdret
edmark (opc, buf)
    Small opc;
    Small buf;
.
    Edit the marked area.  Opc and buf tell which operation and which qbuffer
    to use.
#endif
Cmdret
edmark (opc, buf)
Small opc;
Small buf;
{
    Cmdret retval;
    Reg1 Flag moved;

    moved = gtumark (YES);
    retval = ed (opc, buf, topmark (), markcols ? leftmark () : 0,
		 marklines, markcols, !moved);
    domark (moved);
    return retval;
}

#ifdef COMMENT
domark (moved)
    Flag moved;
.
    common code from the end of splitmark, etc.
#endif
domark (moved)
Reg1 Flag moved;
{
    if (moved) {
	savecurs ();
	putupwin ();
	restcurs ();
    }
    unmark ();
    return;
}

#ifdef COMMENT
Cmdret
ed (opc, buf, line, col, nlines, ncols, puflg)
    Small opc;
    Small buf;
    Nlines line;
    Ncols col;
    Nlines nlines;
    Ncols ncols;
    Flag puflg;
.
    Edit the specified area.  Opc and buf tell which operation and which
    qbuffer to use.
.
    OPPICK:     buf (to)
    OPCLOSE:    buf (to)
    OPERASE:    buf (to)
    OPCOVER:    buf (from)
    OPOVERLAY:  buf (from)
    OPUNDERLAY: buf (from)
    OPBLOT:     buf (from)
    OP_BLOT:    buf (from)
    OPINSERT:   buf (from)
    OPOPEN:      X
    OPBOX:       X
.
    'buf' can be QNONE for OPCLOSE only.
#endif
Cmdret
ed (opc, buf, line, col, nlines, ncols, puflg)
Small opc;
Small buf;
Nlines line;
Ncols col;
Nlines nlines;
Ncols ncols;
Flag puflg;
{
    Nlines lsize;
    Nlines endgap;

    if (   !(opc & OPPICK)
	&& !okwrite ()
       )
	return NOWRITERR;

    if (   (opc & OPBOX)
	&& ncols == 0
       )
	return NOTRECTERR;
    if (opc & OPQFROM) {
	/* buf can NOT be QNONE */
	if (buf == QBOX) {
	    if (nlines <= 0 || ncols <= 0)
		return NOTRECTERR;
	}
	else {
	    if ((nlines = la_reserved (&qbuf[buf].buflas)) == 0)
		return NOBUFERR;
	    ncols = qbuf[buf].ncols;
	}
    }

    getline (-1);   /* flush so that file will be its full size */
    endgap = line - (lsize = la_lsize (curlas));
    if (opc & (OPQFROM | OPBOX)) {
	if (   endgap > 0
	    && !extend (endgap)
	   ) {
	    mesg (ERRALL + 1, "Can't extend the file");
	    return CROK;
	}
    }
#ifdef LMCCASE
    else if (!(opc & OPINPLC)) { /* opc & (OPCLOSE | OPPICK | OPERASE | OPOPEN) */
#else
    else { /* opc & (OPCLOSE | OPPICK | OPERASE | OPOPEN) */
#endif
	if ((opc & OPQTO) && ncols == 0)
	    nlines = min (nlines, lsize - line);
	if (endgap >= 0) {
	    if ((opc & OPQTO) && buf != QNONE)
		la_setrlines (&qbuf[buf].buflas, (Nlines) 0);
	    return CROK;
	}
    }

    if (ncols > 0)
	return edrect (opc, buf, line, col, nlines, ncols, puflg);
    else
	return edlines (opc, buf, line, nlines, puflg);
}

#ifdef COMMENT
Cmdret
edlines (opc, buf, line, nlines, puflg)
    Small opc;
    Small buf;
    Nlines line;
    Nlines nlines;
    Flag puflg;
.
    Edit the specified lines.  Opc and buf tell which operation and which
    qbuffer to use.
.
    OPPICK:     buf (to)
    OPCLOSE:    buf (to)
    OPERASE:    buf (to)
    OPCOVER:    buf (from)
    OPOVERLAY:  buf (from)
    OPUNDERLAY: buf (from)
    OPBLOT:     buf (from)
    OP_BLOT:    buf (from)
    OPINSERT:   buf (from)
    OPOPEN:      X
    OPBOX:       X
.
    'buf' can be QNONE for OPCLOSE only.
#endif
Cmdret
edlines (opc, buf, line, nlines, puflg)
Small opc;
Small buf;
Nlines line;
Nlines nlines;
Flag puflg;
{
    La_stream *dlas;    /* where to put the deleted lines */
    La_stream *blas;    /* the Q buffer */

    if (buf != QNONE) {
	blas = &qbuf[buf].buflas;
	if (opc & OPQTO) {
	    dlas = &fnlas[qtmpfn[buf]];
	    la_stayset (blas);
	    qbuf[buf].ncols = 0;
	}
    }

    (void) la_lseek (curlas, line, 0);

#ifdef LMCCASE
    if (opc & OPINPLC) {
	    mesg (ERRALL + 1, "not implemented yet for lines - use rectangle");
	    return CROK;
    }
#endif

    if (opc & OPCLOSE) {
	if (   curfile == OLDLFILE
	    || buf == QNONE
	   ) {
	    /* this may alloc some more or free some memory */
	    if (la_ldelete (curlas, nlines, (La_stream *) 0) != nlines) {
 err1:
		la_stayclr (blas);
		return lacmderr ("edlines");
	    }
	}
	else Block {
	    Nlines nn;
	    clean (OLDLFILE);
	    nn = la_lsize (dlas);
	    (void) la_align (dlas, blas);
	    if (la_ldelete (curlas, nlines, dlas) != nlines)
		goto err1;
	    la_setrlines (blas, nlines);
	    redisplay (OLDLFILE, nn, 0, nlines, YES);
	}
	redisplay (curfile, line, 0, -nlines, puflg);
    }

    else if (opc & OPPICK) {
	if (curfile == PICKFILE)
	    (void) la_align (curlas, blas);
	else Block {
	    Nlines nn;
	    clean (PICKFILE);
	    (void) la_align (dlas, blas);
	    nn = la_lsize (dlas);
	    if (la_lcopy (curlas, dlas, nlines) != nlines)
		goto err1;
	    redisplay (PICKFILE, nn, 0, nlines, YES);
	}
	la_setrlines (blas, nlines);
    }

    else if (opc & OPERASE) {
	if (curfile == OLDLFILE) {
	    if (   la_ldelete (curlas, nlines, (La_stream *) 0) != nlines
		|| la_blank (curlas, nlines) != nlines
	       )
		goto err1;
	}
	else Block {
	    Nlines nn;
	    clean (OLDLFILE);
	    nn = la_lsize (dlas);
	    (void) la_align (dlas, blas);
	    if (la_ldelete (curlas, nlines, dlas) != nlines)
		goto err1;
	    if (la_blank (curlas, nlines) != nlines) {
		(void) la_ldelete (dlas, nlines, curlas);
		goto err1;
	    }
	    redisplay (OLDLFILE, nn, 0, nlines, YES);
	    la_setrlines (blas, nlines);
	}
	redisplay (curfile, line, nlines, 0, puflg);
    }

    else if (opc & OPOPEN) {
	if (la_blank (curlas, nlines) != nlines)
 err2:      return lacmderr ("edlines");
	redisplay (curfile, line, 0, nlines, puflg);
    }

    else if (opc & OPQFROM) {
	if (opc & OPINSERT) {
	    if (la_lcopy (blas, curlas, nlines) <= 0)
		goto err2;
	    redisplay (curfile, line, 0, nlines, puflg);
	}
	else {
	    /* This may not be easy */
	    /* Probably the thing to do is find the longest line in the
	     * buffer, pretend that it is a rectangular buffer
	     * wide enough for the longest line, and call edrect
	     **/
	    mesg (ERRALL + 1, "not implemented yet");
	}
    }

    if ((opc & OPQTO) && buf != QNONE)
	la_stayclr (blas);
    return CROK;
}

#ifdef COMMENT
Cmdret
edrect (opc, buf, line, col, nlines, ncols, puflg)
    Small opc;
    Small buf;
    Nlines line;
    Ncols col;
    Nlines nlines;
    Ncols ncols;
    Flag puflg;
.
    Edit the specified rectangle.  Opc and buf tell which operation and which
    qbuffer to use.  See 'ed()'.
.
    OPPICK:     buf (to)
    OPCLOSE:    buf (to)
    OPERASE:    buf (to)
    OPCOVER:    buf (from)
    OPOVERLAY:  buf (from)
    OPUNDERLAY: buf (from)
    OPBLOT:     buf (from)
    OP_BLOT:    buf (from)
    OPINSERT:   buf (from)
    OPOPEN:      X
    OPBOX:       X
#endif
Cmdret
edrect (opc, buf, line, col, nlines, ncols, puflg)
Small opc;
Small buf;
Nlines line;
Ncols col;
Nlines nlines;
Ncols ncols;
Flag puflg;
{
    Reg5 char  *linebuf;
    La_stream *dlas;
    La_stream *blas;
    La_stream *olas;
    La_stream qlas;
#ifdef LMCCASE
    La_stream *fnptr;
#endif
    Nlines bufline;
    Fn buffn;
    Cmdret errval;

#ifdef LMCCASE
    if (   (opc & (OPCLOSE | OPPICK | OPERASE | OPOPEN | OPINPLC))
#else
    if (   (opc & (OPCLOSE | OPPICK | OPERASE | OPOPEN))
#endif
	&& (ncols <= 0 || nlines <= 0)
       )
	return CROK;

    if (!(opc & OPPICK))
	clean (OLDLFILE);

    blas = &qbuf[buf].buflas;
    if (opc & OPQTO) {
	if ((buffn = qtmpfn[buf]) == PICKFILE)
	    clean (PICKFILE);
	dlas = &fnlas[buffn];
	(void) la_align (dlas, blas);
	la_stayset (blas);
    }

    if ((opc & OPQFROM) && buf != QBOX) {
	(void) la_clone (blas, &qlas);
	bufline = la_lseek (&qlas, 0, 1);
	nlines = la_reserved (blas);
	ncols = qbuf[buf].ncols;
	olas = curlas;
    }

    errval = CROK;
    if (!(opc & (OPCLOSE | OPBOX))) {
	if (!(linebuf = salloc ((int) (ncols + 1), NO))) {
	    errval = NOMEMERR;
	    goto err3;
	}
    }

    /* first write out lines to qbuf */
    if (opc & OPQTO) Block {
	static Nlines zerolines = 0;
	Reg3 Nlines itmp;
	Reg4 Nlines nn;
	int collect;
	collect = YES;
	for (itmp = 0; itmp < nlines; itmp++) Block {
	    Reg1 char *cp;
	    Reg2 Ncols j;
	    getline (line + itmp);
	    if ((j = (ncline - col)) > 0) {
		getline (-1);    /* because we are about to corrupt cline */
		j = min (j, ncols);
		(cp = &cline[col])[j] = '\n';
		nn = la_lcollect (collect, cp, (int) dechars (cp));
	    }
	    else
		nn = la_lcollect (collect, "\n", 1);
	    if (nn < 0) {
		nlines = 0;
 err1:          errval = lacmderr ("edrect");
		goto err2;
	    }
	    collect = NO;
	}
	nn = la_lsize (&fnlas[buffn]);
	if (la_lrcollect (dlas, &zerolines) != nlines) {
	    nlines = 0;
	    goto err1;
	}
	else
	    redisplay (buffn, nn, 0, nlines, YES);
	la_stayclr (blas);
	la_setrlines (blas, nlines);
	qbuf[buf].ncols = ncols;
    }

    /* now to modify the lines in curfile */
    if (!(opc & OPPICK)) Block {
	Reg4 Nlines itmp;
	Nlines nn;
	Ncols endcol;
	int collect;
	Nlines ndel;
	endcol = col + ncols;
	collect = YES;
	for (itmp = 0; itmp < nlines; itmp++) {
	    if (opc & OPQFROM) {
		if (buf != QBOX) Block {
		    Reg1 Ncols j;
		    curlas = &qlas;
		    getline (bufline + itmp);
		    curlas = olas;
		    if (min (ncols, ncline) > 0)
			move (cline, linebuf, (Uint) min (ncols, ncline));
		    if (   (j = ncline - 1) < ncols
			&& ncols - j > 0
		       )
			fill (&linebuf[j], (Uint) (ncols - j), ' ');
		}
		else {
		    if (   ncols > 1
			&& (itmp == 0 || itmp == nlines - 1)
		       ) Block {
			Reg1 char *cp;
			cp = linebuf;
			if (nlines > 1) {
			    *cp++ = '+';
			    linebuf[ncols - 1] = '+';
			}
			fill (cp, (Uint)
				  (nlines > 1 ? (ncols - 2) : ncols), '-');
		    }
		    else {
			linebuf[0] = '|';
			linebuf[ncols - 1] = '|';
			if (ncols > 2)
			    fill (&linebuf[1], (Uint) ncols - 2, ' ');
		    }
		}
	    }

	    getline (line + itmp);
	    getline (-1);  /* because we are about to do a dechars on cline */

	    if (   (opc & (OPQFROM | OPBOX))
		|| col < ncline
	       ) {
		if (opc & (OPLENGTHEN)) {
		    putbks (col, ncols);/* this makes cline long enough too */
		    fcline = NO;
		}
#ifdef LMCCASE
		else if (!(opc & OPINPLC)) {
#else
		else {
#endif
		    if (endcol >= ncline) {
			if (opc & (OPCLOSE | OPERASE))
			    /* simply terminate line */
			    ncline = col + 1;
			else {
			    putbks ((Ncols) (ncline - 1),
				    endcol + 1 - ncline);
			    fcline = NO;
			    /* this makes cline long enough too */
			}
		    }
		    else {
			/* new end will be before current end of line */
			if (opc & OPCLOSE) {
			    /* copy down rest of line */
			    move (&cline[endcol], &cline[col],
				  (Uint) (ncline - endcol));
			    ncline -= ncols;
			}
			else if (opc & OPERASE)
			    fill (&cline[col], (Uint) ncols, ' ');
		    }
		}
#ifdef LMCCASE
		if (opc & (OPQFROM | OPINPLC)) {
		    if (opc & (OPOVERLAY | OPUNDERLAY | OPBLOT | OP_BLOT |
			       OPINPLC)) Block {
#else
		if (opc & OPQFROM) {
		    if (opc & (OPOVERLAY | OPUNDERLAY | OPBLOT | OP_BLOT
			       )) Block {
#endif
			Reg1 Ncols j;
			Reg2 char *cpfrom;
			Reg3 char *cpto;
			cpfrom = linebuf;
			cpto = &cline[col];
			for (j = ncols; --j >= 0; cpfrom++, cpto++) {
			    switch (opc) {
			    case OPBLOT:
				if (*cpfrom != ' ')
				    *cpto = ' ';
				continue;
			    case OP_BLOT:
				if (*cpfrom == ' ')
				    *cpto = ' ';
				continue;
			    case OPOVERLAY:
				if (*cpfrom == ' ')
				    continue;
				break;
			    case OPUNDERLAY:
				if (*cpto != ' ')
				    continue;
				break;
#ifdef LMCCASE
			    case OPCCASE:
			    case OPCAPS:
				*cpto = caseit (*cpto,
				    opc & OPCCASE ? NO: YES);
				continue;
#endif
			    }
			    if (buf == QBOX)
				boxseg (cpto, *cpfrom, (Ncols) 1);
			    else
				*cpto = *cpfrom;
			}
		    }
		    else
			move (linebuf, &cline[col], (Uint) ncols);
		}
		else if (opc & OPBOX) Block {
		    Reg1 char *cpto;
		    cpto = &cline[col];
		    if (   ncols > 1
			&& (itmp == 0 || itmp == nlines - 1)
		       ) {
			if (nlines != 1) {
			    *cpto++ = '+';
			    cpto[ncols - 2] = '+';
			}
			boxseg (cpto, '-', nlines == 1 ? ncols : ncols - 2);
		    }
		    else {
			boxseg (cpto, '|', (Ncols) 1);
			boxseg (&cpto[ncols - 1], '|', (Ncols) 1);
		    }
		}
		cline[ncline - 1] = '\n';
	    }
	    if (la_lcollect (collect, cline, (int) dechars (cline)) < 0)
		goto err1;
	    collect = NO;
	}
	nn = la_lsize (&fnlas[OLDLFILE]);
	(void) la_lseek (curlas, line, 0);
	ndel = nlines;
#ifdef LMCCASE
	fnptr = (opc & OPINPLC) ? (La_stream *) 0 : &fnlas [OLDLFILE];
	if ((itmp = la_lrcollect (curlas, &ndel, fnptr)) != nlines) {
#else
	if ((itmp = la_lrcollect (curlas, &ndel, &fnlas[OLDLFILE])) != nlines) {
#endif
	    nlines = 0;
	    goto err2;
	}
	else {
	    /*? redisplay should take start and end col arguments */
	    redisplay (curfile, line, nlines, 0, puflg);
	    if (opc & OPQTO)
		redisplay (OLDLFILE, nn, 0, nlines, YES);
	}
    }

 err2:
    if (!(opc & (OPCLOSE | OPBOX)))
	sfree (linebuf);
 err3:
    if ((opc & OPQFROM) && buf != QBOX)
	(void) la_close (&qlas);
    return errval;
}

#ifdef COMMENT
void
putbks (col, n)
    Ncols col;
    Ncols n;
.
    Inserts n blanks starting at col of cline.
    Lengthens cline and fills distance from ncline to col as necessary.
#endif
void
putbks (col, n)
Reg1 Ncols col;
Reg2 Ncols n;
{
    fcline = YES;
    if (n <= 0)
	return;
    if (col >= ncline) { /* remember that there's a '\n' at cline[ncline-1] */
	n += col - (ncline - 1);
	col = ncline - 1;
    }
    if (lcline <= (ncline += n))
	excline (n);

    if (ncline - col - n > 0)
	move (&cline[col], &cline[col + n],
	      (Uint) (ncline - col - n));
    if (n > 0)
	fill (&cline[col], (Uint) n, ' ');
    return;
}

#ifdef COMMENT
boxseg (cpto, chr, num)
    char *cpto;
    char chr;
    Ncols num;
.
    /*
    Write num chr's at cpto.
    Chr will be a box character, i.e. '|', '-', or '+'.
    Look to see if a box character going perpendicular to chr is at each
    position, and if so, put in a '+' for a crossing.
    */
#endif
boxseg (cpto, chr, num)
Reg1 char *cpto;
Reg2 char chr;
Reg3 Ncols num;
{
    for (; --num >= 0; cpto++) {
	if (*cpto != '+') {
	    if (chr == '|') {
		if (*cpto == '-') {
		    *cpto = '+';
		    continue;
		}
	    }
	    else if (chr == '-') {
		if (*cpto == '|') {
		    *cpto = '+';
		    continue;
		}
	    }
	    *cpto = chr;
	}
    }
    return;
}

#ifdef COMMENT
void
clean (tmpfile)
    Small tmpfile;
.
    Clean up tmpfile, i.e. trim it down to free up some memory.
#endif
void
clean (tmpfile)
Small tmpfile;
{
#ifdef lint
    if (tmpfile)
	{}
#endif
}

#ifdef COMMENT
static
Cmdret
lacmderr (str)
char *str;
.
    Do the right thing depending on which LA err happened.
#endif
static
Cmdret
lacmderr (str)
char *str;
{
    switch (la_error ()) {
    case LA_INT:
	mesg (ERRALL + 1, "Operation interrupted");
	return CROK;
    case LA_NOMEM:
	return NOMEMERR;
    case LA_WRTERR:
	return NOSPCERR;
    case LA_ERRMAXL:
	return TOOLNGERR;
    }
    fatal (FATALBUG, str);
    /* NOTREACHED */
}
