 #ifdef COMMENT
--------
file e.la.c
    Stuff related to the LA Package.

    F. Perriollat : version for Linux / Dec 1998

	In the version for Linux, Window 95 / Window NT, a facility to define the
        file style (MicroSoft style or Unix style) has been added.
        The current file style is sample on the first read line, by
        cheking if the line is ended by cr/lf (MicroSoft) or just lf (Unix).
	By default a file is created as Unix style (Linux)
	and Microsoft style (Window).
        The style in which the file will be saved can be redefined by the
        user. This is done with the "file" command.
        The curent file status can be displayed with the "?file" or
        "file ?" command.
        Most of the handling for this facility is provided by the e_sv.c file

	For MSDOS file system the end of line is CR LF.
	On Window system :
	    On getline the CR is removed, and inserted in the file by putline
	    The file are assumed to be opened in BINARY mode when it is read
		( no CR-LF <=> LF conversion on file I/O ).
	On Linux (Unix) system :
	    For Microsoft file style (cr/lf),
	    on getline the CR is removed, and inserted in the file by
	    la_lflush routine (lalflush.c file).

    F. Perriollat : version for Window 95 / NT / Dec 1997
		    version for Linux / Dec 1998
#endif

#ifdef COMMENT
	Copyright abandoned, 1983, The Rand Corporation
#endif

#define CHAR8BITS

#include "e.h"

#include "e.m.h"

#include SIG_INCL

void getline ();
void chkcline ();
void excline ();

#ifdef COMMENT
void
getline (ln)
    Nlines  ln;
.
    Gets line ln of the current workspace into cline.
    If (ln < 0) then flush cline if modified
      and set clinelas to 0 so that the next call to getline is
      guaranteed to get a fresh copy of the line desired from the file.
#endif
void
getline (ln)
Nlines  ln;
{
    Reg5 char *clp;     /* cline pointer */
    Reg6 char *endcl;
    La_stream *rlas;

    int cclsk;
    Flag creturn;       /* CR at the end of the data block */
    char file_style;    /* CR/LF (MS style) or LF (UNIX style) end of line mark */
    Ff_file *ffile_pt;
#if 1
    int sz;
    Nlines ls;

    sz = sizeof (ls);
#endif

    if (ln < 0) {
	if (fcline)
	    putline (NO);
	clinelas = (La_stream *) 0;
	return;
    }

#if 1
    ls = la_lsize (curlas);
#endif

    if (ln >= la_lsize (curlas)) {
	if (fcline)
	    putline (NO);
	xcline = YES;         /* past end of file */
	/* make up a blank line */
	clinelas = curlas;
	clineno = ln;
	ecline = NO;
	cline8 = NO;
	fcline = NO;
	ncline = 1;
	cline[0] = '\n';
	return;
    }

    if ( clinelas &&  clinelas->la_file == curlas->la_file
	&& ln == clineno
       ) {
	return;  /* already have the line */
    }

    if (fcline)
	putline (NO);

    clinefn = curfile;
    clinelas = rlas = curlas;

    cclsk = (int) la_lseek (rlas, (La_linepos) ln, 0);

    ecline = NO;
    cline8 = NO;
    fcline = NO;
    ncline = 0;
    xcline = NO;

    clineno = ln;

    clp = cline;
    endcl = &cline[lcline - (TABCOL + 1)];
    for (;;) Block {
	Reg4 Uchar *cp;
	Reg3 Small nleft;
	Uchar *bufp;

	if ((nleft = la_lpnt (rlas, &bufp)) <= 0) {
	    fatal (FATALIO, "getline read error for line %d # cclsk %d, %d %d",
		   ln, cclsk, la_error (), nleft);
	}
        ffile_pt = rlas->la_file->la_ffs->f_file;
        file_style = 0; 
	creturn = NO;
	cp = bufp;
	while (--nleft >= 0) Block {
	    Reg2 Char ch;
	    if (clp > endcl) Block {    /* check if room in cline */
		Reg1 Ncols i1;
		ncline = (i1 = clp - cline) + 1;
		excline ((Ncols) 0);          /* 8 is max needed (for tab)        */
		clp = &cline[i1];
		endcl = &cline[lcline - 9];
	    }
#ifdef CHAR7BITS
	    ch = creturn ? '\r' : *cp++ & 0177;
#else
	    ch = creturn ? '\r' : *cp++;
#endif

	    if ( ch == '\r' ) {
		if ( nleft <= 0 ) {
		    creturn = YES;
		    break;
		    }
		if ( (*cp & 0177) == '\n' ) {
#ifdef CHAR7BITS
		    ch =*cp++ & 0177;
#else
		    ch =*cp++ & 0377;
#endif
		    nleft--;
                    file_style = MS_FILE;
		    }
		}
	    creturn = NO;

	    if ( ! ISCTRLCHAR(ch) )
		*clp++ = ch;
	    else if (ch == '\t') Block {
		Reg1 Ncols ri;
		for (ri = 8 - (07 & (clp - cline)); ri--; )
		    *clp++ = ' ';
	    }
	    else if ( ch == '\n' ) {
		while (--clp >= cline && (*clp == ' ' || *clp == '\t'))
		    continue;
		*++clp = '\n';
		ncline = clp - cline + 1;
                if ( ! file_style ) file_style = UNIX_FILE;
                /* the current file style is comming from the first read line */
                if ( file_style && !ffile_pt->style ) 
                    ffile_pt->style = file_style;
		return;
	    }
	    else {
		{
		    ecline = YES;   /* line has control char        */
		    *clp++ = ESCCHAR;
		    if (ch != ESCCHAR)
			ch |= 0100;
		}
		*clp++ = ch;
	    }
	}
    }
    /* returns from inside while loop */
    /* NOTREACHED */
}

#ifdef COMMENT
Flag
putline (allflg)
    Flag allflg;
.
    If fcline != 0, inserts the line in cline in place of the current one.
    Returns YES is successful, NO if not.
    In future, several clines may be cached, and if allflg == YES, putline
    will flush them all out.
#endif
Flag
putline (allflg)
Flag allflg;
{
    La_stream tlas;
    int nr;

#ifdef lint
    if (allflg)
	allflg = allflg;
#endif

    if (!fcline)
	return YES;

    if (!la_clone (clinelas, &tlas))
	return NO;

    (void) la_lseek (&tlas, clineno, 0);
    cline[ncline - 1] = '\n';
    chkcline ();
    nr = dechars (cline);
    fcline = NO;
    clinelas = (La_stream *) 0; /* force a getline next time */
    Block {
	La_linepos ln;
	La_linepos nins;
	ln = 1;
	nins = la_lreplace (&tlas, cline, nr, &ln,
			    la_lwsize (&tlas) > 1
			    ? &fnlas[OLDLFILE]
			    : (La_stream *) 0);
	(void) la_close (&tlas);
	if (la_lsize (&tlas) < clineno)
	    fatal (FATALBUG, "lsize (%d) < clineno (%d)",
		   la_lsize (&tlas), clineno);
	if (nins < 1)
	    return NO;
	return YES;
    }
}

#ifdef COMMENT
void
chkcline ()
.
    Check cline for a ^J before the end of the line.  If there is one,
    truncate the line and redisplay it.
#endif
void
chkcline ()
{
    Reg2 char  *fm;

    if (!ecline)
	return;
    fm = cline;
    for (;;) Block {
	Reg1 Char cc;   /* current character */
	if ((cc = *fm++) == ESCCHAR && *fm != '\n') {
	    if ((cc = (*fm == ESCCHAR) ? *fm++ : (*fm++ & 037)) == '\n') {
		fm -=2;
		*fm++ = '\n';
		break;
	    }
	}
	if (cc == '\n')
	    break;
    }
    if (fm - cline < ncline) {
	ncline = fm - cline;
	redisplay (clinefn, clineno, 1, 0, YES);
    }
    return;
}

#ifdef COMMENT
Ncols
dechars (line)
    char   *line;
.
    Performs in-place character conversion from internal
    to external format of the characters starting at line up to a newline or
    a ^J.  May alter contents of line.  Line MUST have a newline in it
    note: replaces initial spaces with tabs; deletes trailing spaces
    returns number of characters in the converted, external representation
#endif
Ncols
dechars (line)
char   *line;
{
    Ncols   cn;          /* col number                 */
    char  *fm;
    char  *to;           /* pointers for move          */
    Ncols  lnb;          /* 1 + last non-blank col     */

    fm = to = line;
    cn = 0;
    lnb = 0;
    for (;;) Block {
	Char    cc;    /* current character          */
	if ((cc = *fm++) == ESCCHAR && *fm != '\n')
	    cc = (*fm == ESCCHAR) ? *fm++ : (*fm++ & 037);
/*
	if ( cc == ESCCHAR8 ) {
	    cc = *fm++ | 0200;
	}
*/
	if ( (cc == '\r') && (*fm == '\n') )
	    break;
	if (cc == '\n')
	    break;
	if (cc != ' ') {
	    if (lnb == 0)
		while (8 + (lnb & ~7) <= cn) {
		    *to++ = (lnb & 7) == 7 ? ' ' : '\t';
		    lnb &= ~7;
		    lnb += 8;
		}
	    while (++lnb <= cn)
		*to++ = ' ';
	    *to++ = cc;
	}
	++cn;
    }
    while ( (to > line) && (*(to-1) == '\r') ) to--;
    *to++ = '\n';
    return to - line;
}

#ifdef COMMENT
void
shortencline ()
.
    Shorten cline if necessary so that there are no trailing blanks or tabs,
    and make sure cline[ncline -1] = '\n';
#endif
void
shortencline ()
{
    Reg1 char *cp;
    Reg2 char *rcline = cline;

    if (ncline <= 1)
	return;
    for (cp = &rcline[ncline - 2]
	; cp >= rcline && (*cp == ' ' || *cp == '\t')
	; cp--
	)
	continue;
    *++cp = '\n';
    ncline = cp - rcline + 1;
    return;
}

#ifdef COMMENT
void
excline (length)
    Ncols length;
.
    Expand cline to max (length, lcline + icline)
#endif
void
excline (length)
Ncols length;
{
    Reg1 Ncols  j;
    Reg2 char  *tmp;

    if ((j = lcline + icline) < length)
	j = length;

    tmp = salloc ((int) (j + 1), YES);
    if (ncline > 0)
	move (cline, tmp, (Uint) ncline);
    icline += icline / 2;
    lcline = j;
    sfree (cline);
    cline = tmp;
    return;
}

#ifdef COMMENT
Flag
extend (ext)
    Nlines ext;
.
    Extend the file with ext blank lines so that
	la_lsize (curlas) = line
#endif
Flag
extend (ext)
Reg1 Nlines ext;
{
    if (ext > 0) {
	(void) la_lseek (curlas, 0, 2);
	if (la_blank (curlas, ext) != ext)
	    return NO;
    }
    return YES;
}

#ifdef COMMENT
Nlines
lincnt (stline, num, type)
    Nlines  stline;
    Nlines  num;
    Flag    type;
.
    Type: -2 and 2 - paragraphs backward and forward
    Type: -1 and 1 - lines backward and forward
    Return the actual number of lines.  End of file can cause
    the number to be less than num.
#endif
Nlines
lincnt (stline, num, type)
Nlines  stline;
Nlines  num;
Flag    type;
{
    putline (YES);
    return la_lcount (curlas, stline, num, type);
}
