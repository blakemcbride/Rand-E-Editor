#ifdef COMMENT
--------
file e.u.c
    use and editfile routines.
#endif

#ifdef COMMENT
	Copyright abandoned, 1983, The Rand Corporation
#endif

#include "e.h"
#include "e.m.h"
#include "e.cm.h"
#include "e.tt.h"
#include <sys/stat.h>
#include <pwd.h>
#include <unistd.h>

static void doedit ();
extern char *getmypath ();
       char *ExpandName ();
extern Flag reset_impl_tick ();
extern Cmdret remove_file (Fn fn);

static Fn getnxfn ();

/* variables used to list the edited file */
static int term_width, term_height; /* current display size */
static Flag files_status_flg;   /* display the full status flag */
static int first_tt_line = 0;   /* 1st video line index to display edited files */
static int fi_list_offset = 0;  /* fi_list offset of the 1st displayed file */
static int fi_highlight = 0;    /* file highlighted */
static int fi_list_nb;          /* max number of element in fi_list */
static int fi_list_idx;         /* index of highlighted file */
static int fi_list [MAXFILES];

static const char infstrg [] =
"  Flags: 'N' = edited, 'A' = alternate, 'M' = modified, '*' active window";
static char fi_title1 [128];    /* screen title (1st line) */
static char fi_title2 [128];    /* screen title (2nd line) */

/*  dochangedirectory : Change Directory Editor command */
/* ---------------------------------------------------- */

Cmdret dochangedirectory () {

    extern char default_workingdirectory [];
    int cc;
    char msg [128], *strg;
    char *cdname, *sp;
    char pwdname[PATH_MAX];

    if ( opstr [0] == '~' ) cdname = append (mypath, &opstr[1]);
    else cdname = ( opstr[0] ) ? opstr : default_workingdirectory;
    strg = "Non existant dir";
    cc = access (cdname, F_OK);
    if ( !cc ) {
	strg = "No Read Access to dir";
	cc = access (cdname, R_OK);
	if ( !cc ) {
	    strg = "\"chdir\" error on dir";
	    cc = chdir (cdname);
	    if ( !cc ) {
		sp = getcwd (pwdname, sizeof (pwdname));
		if ( sp ) cdname = sp;
		strg = "CWD";
	    }
	}
    }
    sprintf (msg, "%s : %s", strg, cdname);
    if ( opstr [0] == '~' ) sfree (cdname);
    mesg (TELALL + 1, msg);
    loopflags.hold = YES;
    return CROK;
}


/* display the list of edited files */
/* -------------------------------- */

static void display_line (char *fname, char *str, int size, Flag video_flg)
{
    static const char dot [] = "...";
    int nb;
    char *fn_pt;

    if ( strlen (str) < size ) {
	nb = strlen (str) + strlen(fname) - size;
	if ( nb <= 0 ) fn_pt = fname;
	else {
	    /* too long file name to be displayed */
	    fn_pt = strchr (fname + nb + sizeof (dot) -1, '/');
	    if ( ! fn_pt ) fn_pt = fname + nb;
	    else {
		strcat (str, dot);
	    }
	}
	strcat (str, fn_pt);
    }
    str [size] = '\0';
    if ( video_flg ) (*term.tt_video) (YES);
    (void) fputs (str, stdout);
    if ( video_flg ) (*term.tt_video) (NO);
    (void) fflush (stdout);
}

static int one_edited_file (int fi, int iv_fi, Flag clr_flg)
{
    /* iv_fi : define for which file nb fi, the display must be in inverted
     * video mode, if iv_fi < 0 : inverted video for the Normal edited file
     * If iv_fi > MAXFILES the video will be normale
     */
    extern char * fileslistString ();
    extern char * filestatusString ();
    extern S_term term;

    Flag video_flg;
    int term_sz, sz, nb, vlidx, idx;
    int wi, wn, i0;
    char ch, ch1, *wstg, *fname, *strg;
    char str[256];
    char *pt1, *pt2;
    short flflags;

    i0 = FIRSTFILE + NTMPFILES;
    if ( (fi >= MAXFILES) || (fi < i0) ) return -1;
    flflags = fileflags[fi];
    if ( !(flflags & INUSE) || (flflags & DELETED) ) return -1;

    term_sz = term_width;
    if ( term_sz > (sizeof (str) -1) ) term_sz = sizeof (str) -1;
    strg = fileslistString (fi, &fname);
    if ( ! strg ) return -1;
    if ( ! files_status_flg ) {
	pt1 = strchr (strg, ',');
	pt2 = strchr (strg, ')');
	if ( pt1 && pt2 ) {
	    sz = strlen (++pt2);
	    memmove (++pt1, pt2, sz);
	    pt1 [sz] = '\0';
	}
    }
    if ( nwinlist <= 1 ) {
	ch1 = '*';
	if ( fi == curwin->wksp->wfile ) ch = 'N';
	else if ( fi == curwin->altwksp->wfile ) ch = 'A';
	else ch = ' ';
	memset (str, 0, sizeof (str));
	sprintf (str, "%-2d %c %s", fi - i0 +1, ch, strg);
    } else {
	/* multiple window case */
	ch = ch1 = ' ';     /* '*' for the active window */
	wstg = "";
	wn = 0;
	for ( wi = nwinlist -1 ; wi >= 0 ; wi-- ) {
	    if ( fi == winlist[wi]->wksp->wfile ) ch = 'N';
	    else if ( fi == winlist[wi]->altwksp->wfile ) ch = 'A';
	    else continue;
	    wn = wi +1;
	    if ( winlist[wi] == curwin ) {
		ch1 = '*';
		break;
	    }
	}
	memset (str, 0, sizeof (str));
	if ( wn ) sprintf (str, "%-2d %c%d%c %s", fi - i0 +1, ch1, wn, ch,
			  strg);
	else sprintf (str, "%-2d     %s", fi - i0 +1, strg);
    }

    video_flg = ( iv_fi < 0 ) ? ((ch == 'N') && (ch1 == '*')) : (fi == iv_fi);
    if ( video_flg ) fi_highlight = fi;

    /* found the fi in fi_list, if not, assume to be put at the end of list */
    for ( idx = 0 ; idx < fi_list_nb ; idx++ ) {
	if ( fi_list [idx] == fi ) break;
    }
    vlidx = first_tt_line + (idx - fi_list_offset);
    if ( (vlidx >= first_tt_line) && (vlidx <= (term_height -2)) ) {
	/* inside the current display window */
	(*term.tt_addr) (vlidx, 0);
	if (clr_flg ) (*term.tt_clreol) ();
	display_line (fname, str, term_sz, video_flg);
    }
    return (fi);
}

static void fi_list_redraw (Flag resize_flg)
{
    int i, lnb, offset;

    if ( (fi_list_idx < 0) || (fi_list_idx > fi_list_nb) )
	return;

    lnb = term_height - first_tt_line -1;
    if ( ! resize_flg ) offset = fi_list_idx - (lnb / 2);
    else {
	offset = fi_list_offset;
	if ( lnb < (fi_list_idx - offset +1) ) offset  = fi_list_idx - (lnb -1);
	if ( lnb > (fi_list_nb  - offset)    ) offset -= lnb - fi_list_nb;
    }
    if ( offset > (fi_list_nb - lnb) ) offset = fi_list_nb - lnb;
    if ( offset < 0 ) offset = 0;
    fi_list_offset = offset;

    (*term.tt_addr) (first_tt_line, 0);
    for ( i = 0 ; i <= lnb ; i++ )
	(void) one_edited_file (fi_list[i + fi_list_offset], fi_highlight, YES);
    (*term.tt_addr) (term_height -1, 0);
}

static void display_fi_title ()
{
    char format [16];

    (*term.tt_home) ();
    sprintf (format, "%s%ds", "%.", term_width);
    printf (format, fi_title1);
    (*term.tt_addr) (1, 0);
    printf (format, fi_title2);
}

static void fi_list_resize (int width, int height)
{
    int min_hgt;

    term_width = width;
    term_height = height;

    min_hgt = first_tt_line +2;
    (*term.tt_clear) ();
    if ( term_height >= min_hgt ) {
	display_fi_title ();
	fi_list_redraw (YES);
    } else
	printf ("Too small terminal window : minimum height %d\a\n", min_hgt);

    (*term.tt_addr) (term_height -1, 0);
}

static void fi_list_refresh ()
{
    int vlidx;

    if ( (fi_list_idx < 0) || (fi_list_idx > fi_list_nb) )
	return;
    vlidx = first_tt_line + fi_list_idx - fi_list_offset;
    if ( (vlidx >= first_tt_line) && ((vlidx +1) < term_height) )
	return;

    fi_list_redraw (NO);
}

static Flag my_list_kbhandler (int gk, int *val)
{
    int idx, fi;

    if ( (gk != CCMOVEDOWN) && (gk != CCMOVEUP) )
	return NO;

    idx = fi_list_idx + ((gk == CCMOVEUP) ? -1 : 1);
    if ( (idx >= 0) && (idx < fi_list_nb) ) {
	(void) one_edited_file (fi_list[fi_list_idx], MAXFILES+1, NO);
	fi_list_idx = idx;
	fi = fi_list [idx];
	fi_highlight = one_edited_file (fi, fi, NO);
	(*term.tt_addr) (term_height -1, 0);
	if ( fi_highlight > 0 ) *val = fi - (FIRSTFILE + NTMPFILES) +1;
	fi_list_refresh ();
    } else putchar ('\a');
    return YES;
}

static Flag (*edited_files ())()
{
    static const char flgstrg [] =
    "  Flags: 'N' = edited, 'A' = alternate, 'M' = modified";
    static const char flgstrg2 [] = ", '*' active window";

    int i0, fi, fi_nb, nb;
    int lnb;

    i0 = FIRSTFILE + NTMPFILES;
    nb = MAXFILES - i0;

    strcpy (fi_title2, flgstrg);
    if ( nwinlist <= 1 ) {
	sprintf (fi_title1, "Edited file list (max nb of files %d)\n", nb);
    } else {
	sprintf (fi_title1, "Edited file list (max nb of files %d) in %d windows\n",
		 nb, nwinlist);
	strcat (fi_title2, flgstrg2);
    }
    display_fi_title ();

    first_tt_line = 3;
    fi_highlight = fi_list_nb = fi_list_offset = 0;
    memset (fi_list, 0, sizeof (fi_list));
    for ( fi = i0 ; fi < MAXFILES ; fi++ ) {
	fi_nb = one_edited_file (fi, -1, NO);
	if ( fi_nb > 0 ) {
	    if ( fi_nb == fi_highlight ) fi_list_idx = fi_list_nb;
	    fi_list[fi_list_nb++] = fi_nb;
	}
    }
    fi_list_refresh ();
    return my_list_kbhandler;
}

/* Display the list of edited files and switch files if requested */

static void my_size (int *width_pt, int *height_pt)
{
    if ( width_pt ) *width_pt = term_width;
    if ( height_pt ) *height_pt = term_height;
}

Cmdret displayfiles (Flag status_flg)
{
    extern void show_info (Flag (*my_info ()) (), int *, char *,
			   void (*my_service) (), void (*my_size) (int *, int*) );

    static char listmsg [] =
    "---- up, down or <file nb> & <RETURN> to switch file, <Ctrl C> just to return -----";
    int val, fi;

    term_width = term.tt_width;
    term_height = term.tt_height;
    files_status_flg = status_flg;
    show_info (edited_files, &val, listmsg, fi_list_resize, my_size);
    if ( val > 0 ) {
	fi = val -1 + FIRSTFILE + NTMPFILES;
	if ( (fi < MAXFILES) && (fileflags[fi] & INUSE) ) {
	    if ( fi != curwin->wksp->wfile ) {
		(void) editfile (names [fi], (Ncols) -1, (Nlines) -1, 1, YES);
	    }
	} else {
	    putchar ('\a');
	}
    }
    return CROK;
}

Cmdret displayfileslist ()
{
    return (displayfiles (YES));    /* full files status */
}

#ifdef COMMENT
Cmdret
edit ()
.
    Do the "edit" command.
#endif
Cmdret
edit ()
{
    char *eol, ch;
    int sz, val;
    Cmdret cc;

    if (opstr[0] == '\0') {
	switchfile ();
	return CROK;
    }
    sz = strlen (opstr);
    if ( (sz == 1) && (*opstr == '?') ) {
	cc = displayfiles (NO);
	return cc;
    }
    if ( (sz == 1) && (*opstr == '-') ) {
	cc = remove_file (curfile);
	return cc;
    }

    if (*nxtop) {
	/* for the case of DOS file name which can include space char ! */
	/*      in this case, the file name must be quoted (with " or ') */
	ch = cmdopstr [0];
	if ( (ch != '\"') && (ch != '\'') )
	    return CRTOOMANYARGS;
	sz = strlen (cmdopstr);
	if ( (sz <= 2) || (cmdopstr [sz-1] != ch) )
	    return CRTOOMANYARGS;
	sfree (opstr);
	opstr = append (&cmdopstr[1], "");
	sz = strlen (opstr);
	opstr [sz -1] = '\0';
    }
    eol = &opstr[strlen (opstr)];
    *eol = '\n';
    opstr[dechars (opstr) - 1] = '\0';
    (void) editfile (opstr, (Ncols) -1, (Nlines) -1, 1, YES);
    return CROK;
}

#ifdef COMMENT
Small
editfile (file, col, line, mkopt, puflg)
    char   *file;
    Ncols   col;
    Nlines  line;
    Small   mkopt;
    Flag    puflg;
.
    Installs 'file' as edited file in current window, with window upper-left
      at ['line', 'col'].  Use lastlook if -1.
    If 'file' is not there
	if 'mkopt' == 2 || replaying || recovering, create the file.
	else if 'mkopt' == 1, give the user the option of making it
	else return
    Returns
      -1 if user does not want to make one,
       0 if error condition,
       1 if successfull.
    Writes screen (calls putupwin ()) if puflg is YES.
#endif
Small
editfile (file, col, line, mkopt, puflg)
char   *file;
Ncols   col;
Nlines  line;
Small   mkopt;
Flag    puflg;
{
    extern void savemark (struct markenv *);
    extern void infosetmark ();
    extern char *cwdfiledir [];

    int retval;
    static Flag toomany = NO;
    Short   j;
    Flag    dwriteable,         /* directory is writeable */
	    fwriteable,         /* file is writeable */
	    isdirectory;        /* is a directory - don's set CANMODIFY */
    Flag    hasmultlinks = NO;
    La_stream  tlas;
    La_stream *nlas;
    char *dir = NULL;
    Fn newfn;
    int nlink;
    Flag    nameexpanded = NO;  /* "~/file"  or "~user/file" */
#ifdef SYMLINKS
    Flag    wassymlink = NO;
    char    *origfile;
#endif
    char *cwd, cwdbuf [PATH_MAX];

    if (file == NULL || *file == 0)     /* If null filename */
	Goret (-1);                     /* then can't do too much */

    /*  That the next statement is here is sort of a kludge, so that
     *  when editfile() is called before mainloop, only the last file
     *  edited will be able to set the hold.
     **/
    loopflags.hold = NO;

    /*
     *  Expand "~" to a login directory
     */
    if (*file == '~') {
	file = ExpandName (file);
	nameexpanded = YES;
	if (file == NULL) {
	    retval = 0;
	    goto ret;
	}
    }

#ifdef SYMLINKS
    /*
     *  Deal with symbolic links
     */
    Block {
	char *cp;
	static char *ReadSymLink (char *);

	cp = ReadSymLink (file);
	if (cp == NULL) {
	    retval = 0;
	    goto ret;
	}
	else if (cp != file) {
	    origfile = file;
	    file = cp;
	    wassymlink = YES;
	}
    }
#endif /* SYMLINKS */

    if ((newfn = hvname (file)) != -1) {
	/* we have it active */
	if (newfn == curfile)
	    puflg = NO;
    }
    else if (   (newfn = hvoldname (file)) != -1
	     || (newfn = hvdelname (file)) != -1
	    ) {
	/* has been renamed or deleted */
	/* its directory must be writebale */
	dwriteable = YES;
	goto asknew;
    }
    else {
	/* don't have it as an old name*/
	if (toomany || nopens >= MAXOPENS) {
	    mesg (ERRALL + 1, "Too many files -- Editor limit!");
	    Goret (0);
	}
	/* find the directory */
	if ((j = dircheck (file, &dir, (char **) 0, NO, YES)) == -1)
	    Goret (0);

	/* 3/7/84 changed to handle null directory */
	if (dir == NULL || *dir == 0)
	    dwriteable = access (".", 2) >= 0; 
	else
	    dwriteable = access (dir, 2) >= 0;

	if ((j = filetype (file)) != -1) {
	    /* file already exists */
	    Fn tfn;
	    fwriteable = access (file, 2) >= 0;
	    if (j != S_IFREG &&  j != S_IFDIR ) {
		mesg (ERRALL + 1, "Can only edit files");
		Goret (0);
	    }
	    if (access (file, 4) < 0) {
		mesg (ERRALL + 1, "File read protected.");
		Goret (0);
	    }
	    isdirectory = j == S_IFDIR;
	    if ((newfn = getnxfn ()) >= MAXFILES - 1)
		toomany = YES;      /* we'll get you next time */
	    intok = YES;
	    if ((nlas = la_open (file, "", &tlas, (La_bytepos) 0))
		== NULL) {
		intok = NO;
		if ( la_errno == LA_INT )
		    mesg (ERRALL + 1, "Interrupted");
		else if ( la_errno == LA_NOMEM )
		    mesg (ERRALL + 1, "No more memory available, please get out");
		else if ( la_errno == LA_WRTERR )
		    mesg (ERRALL + 1, "Disc write error (no more space available ?)");
		else if ( la_errno == LA_ERRMAXL )
		    mesg (ERRALL + 1, "Too large file, can't edit this file");
		else
		    mesg (ERRALL + 1, "Can't open the file");
		Goret (0);
	    }
	    intok = NO;
	    if (nlas->la_file == la_chglas->la_file) {
		mesg (ERRALL + 1, "Can't edit the 'changes' file");
		(void) la_close (nlas);
		Goret (0);
	    }
	    Block {
		La_file   *tlaf;
		tlaf = nlas->la_file;
		/* is this another name for one we already have open? */
		for (tfn = FIRSTFILE + NTMPFILES; tfn < MAXFILES; tfn++)
		    if (   (fileflags[tfn] & (INUSE | NEW)) == INUSE
			&& tlaf == fnlas[tfn].la_file
		       ) {
			/* if it is DELETED, then we go and make a NEW one */
			if (fileflags[tfn] & DELETED)
			    break;
			/* deallocate newfn */
			fileflags[newfn] = 0;
			newfn = tfn;
			(void) la_close (nlas);
			toomany = NO;
			mesg (TELALL + 3, "EDIT: ", names[tfn], " (linked)");
			d_put (0);
			loopflags.hold = YES;
			goto editit;
		    }
	    }
	    nopens++;
	    mesg (TELSTRT + 2, "EDIT: ", file);
	    d_put (0);
	    nlink = fmultlinks (la_chan (nlas));
	    if (isdirectory) {
		mesg (1, "  (is a directory)");
		loopflags.hold = YES;
	    }
	    else if (1 < nlink) Block {
		    char numstr[10];
		hasmultlinks = YES;
		sprintf (numstr, "%d", nlink);
		mesg (3, "  (has ", numstr, " links)");
		loopflags.hold = YES;
	    }
#ifdef SYMLINKS
	    if ( wassymlink ) {
		mesg (3, " [symbolic link from ", origfile, "]" );
		loopflags.hold = YES;
		if (nameexpanded)
		    sfree (origfile);
		fileflags[newfn] |= SYMLINK;
	    }
#endif
	    mesg (TELDONE);
#if 0       /* do not remove message !!! */
	    d_put (0);
#endif
	    if (dwriteable)
		fileflags[newfn] |= DWRITEABLE;
	    if (fwriteable)
		fileflags[newfn] |= FWRITEABLE;
/*          if (dwriteable && !isdirectory && (!inplace || fwriteable)) {       */
	    if (dwriteable && !isdirectory && fwriteable)  {    /* CERN 860313 */
		fileflags[newfn] |= CANMODIFY;
		if (inplace && hasmultlinks)
		    fileflags[newfn] |= INPLACE;
	    }
	    fileflags[newfn] |= UPDATE;
	}
	else
 asknew:
	    if (mkopt == 2 || replaying || recovering)
		goto createit;
	else if (mkopt == 1) {
	    mesg (TELSTRT|TELCLR + 3, "Do you want to create ", file, "? ");
	    keyused = YES;
	    getkey (WAIT_KEY);
	    keyused = YES;
	    mesg (TELDONE);
	    if (key != 'y' && key != 'Y')
		Goret (-1);
 createit:
	    if (!dwriteable) {
		/* tell 'em */
		dirncheck (dir, YES, YES);
		Goret (0);
	    }
	    /* ok to create file, so do it */
	    if ((newfn = getnxfn ()) >= MAXFILES - 1)
		toomany = YES;  /* we'll get you next time */
	    fileflags[newfn] |= UPDATE | DWRITEABLE | FWRITEABLE
			     | CANMODIFY | NEW;
	    /* a NEW file can NOT be INPLACE */
	    nlas = la_open (file, "n", &tlas);
	}
	else
	    Goret (-1);
	names[newfn] = append (file, "");
	if ( cwdfiledir [newfn] ) sfree (cwdfiledir [newfn]);
	cwdfiledir [newfn] = NULL;
	cwd = getcwd (cwdbuf, sizeof (cwdbuf));
	if ( cwd ) cwdfiledir [newfn] = append (cwdbuf, "");

	if (   !la_clone (nlas, &fnlas[newfn])
	    || !la_clone (nlas, &lastlook[newfn].las)
	   ) {
	    fatal (FATALBUG, "La_clone failed in editfile()");
	    /* NOTREACHED */
	}
	(void) la_close (nlas);
    }

editit:
    doedit (newfn, line, col);
#ifndef LMCTRAK         /* track shouldn't be fooled with here anyway. */
    curwin->winflgs &= ~TRACKSET;
    infotrack (NO);
#endif
    if (puflg) {
	if (newfn == OLDLFILE)
	    getline (-1);   /* so that the last line modified will show up */
	putupwin ();
    }
    limitcursor ();
    poscursor (curwksp->ccol, curwksp->clin);
    if ( (line != -1) || (col != -1) ) {
	/* set initial value of previous cursor */
	savemark (&curwksp->wkpos);
	infosetmark ();
    }
    retval = 1;

ret:
#ifdef SYMLINKS
    if (wassymlink) {
	sfree (file);
	file = NULL;
    }
#endif
    if (nameexpanded && file != NULL)
	sfree (file);
    if (dir != NULL)
	sfree (dir);
    return retval;
}

#ifdef COMMENT
static void
doedit (fn, line, col)
    Fn      fn;
    Nlines  line;
    Ncols   col;
.
    Now that we have determined that everything is OK, go ahead and
    install the file as element 'fn' in the list of files we are editing.
    If 'line' != -1 make it the line number of the current workspace;
    Similarly for 'col'.
#endif
static void
doedit (fn, line, col)
Fn fn;
Nlines  line;
Ncols   col;
{
    S_wksp *lwksp, *cwksp;

    (void) reset_impl_tick ();
    if (curfile == deffn) {
	/* insure that the new altwksp will be null */
	releasewk (curwin->wksp);
	curwin->wksp->wfile = NULLFILE;
    }
    releasewk (curwin->altwksp);
    exchgwksp (NO);
    lwksp = &lastlook[fn];
    cwksp = curwksp;
    cwksp->wkpos = lwksp->wkpos;    /* restaure the previous position */
    if (line != -1)
	lwksp->wlin = line;     /* line no of upper-left of screen */
    if (col != -1)
	lwksp->wcol = col;      /* col no of column 0 of screen */
    cwksp->wlin = lwksp->wlin;
    cwksp->wcol = lwksp->wcol;
    cwksp->ccol = lwksp->ccol;
    cwksp->clin = lwksp->clin;

    (void) la_clone (&fnlas[fn], curlas = &cwksp->las);
    cwksp->wfile = curfile = fn;
    return;
}

#ifdef COMMENT
static Fn
getnxfn ()
.
    Return the next file number that is not INUSE.
#endif
static Fn getnxfn ()
{
    extern void marktickfile ();
    Fn fn;
    S_wksp *lwksp;

    for ( fn = FIRSTFILE + NTMPFILES ; fileflags[fn] & INUSE ; )
	if ( ++fn >= MAXFILES )
	    fatal (FATALBUG, "Too many files");

    fileflags[fn] = INUSE;

    /* clear cursor positions */
    lwksp = &lastlook[fn];
    marktickfile (fn, NO);
    memset (&lwksp->wkpos, 0, sizeof(lwksp->wkpos));
    lwksp->wlin = lwksp->wcol = lwksp->ccol = lwksp->clin = 0;
    return fn;
}


#ifdef COMMENT
static char *
ReadSymLink (file)
char *file
.
    For symbolic links, get the referenced name of the file to update.
    Treated as if user had edited that file directly.
    If the referenced name is not rooted, construct the appropriate
    relative path.

#endif

#ifdef SYMLINKS


/* read a link and build the file name, or return NO */
static Flag follow_link (char *file, int sz)
{
    int nc, nb;
    char *cp;
    char linkname [PATH_MAX];

    memset (linkname, 0, sizeof (linkname));
    /* documentation says readlink doesn't null terminate */
    if ( (nc = readlink (file, linkname, sizeof(linkname) -1)) == -1 ) {
	mesg (ERRALL + 2, "Can't read symbolic link ", file);
	return NO;
    }
    if (*linkname == '/' || ((cp = rindex (file, '/')) == NULL))
	strncpy (file, linkname, sz);
    else {                  /* have a relative pathname */
	*(cp +1) = '\0';
	nb = sz - strlen (file);
	strncat (file, linkname, nb);
    }
    file [sz-1] = '\0';
    return (YES);
}

static char * ReadSymLink (char *file)
{
    char bld_name [PATH_MAX];
    char *fname;
    int nb;
    struct stat sb;

    if ( !file ) return NULL;

    if ( lstat (file, &sb ) < 0 )
	return file;
    if ( (sb.st_mode & S_IFMT) != S_IFLNK ) /* not a symbolic link */
	return file;

    /* it is a symbolic link, go down to the actual file */
    memset (bld_name, 0, sizeof (bld_name));
    strcpy (bld_name, file);
    for ( nb = 0 ; nb < 100 ; nb++ ) {
	if ( ! follow_link (bld_name, sizeof (bld_name)) )
	    return NULL;
	if ( lstat (bld_name, &sb ) < 0 ) break;
	if ( (sb.st_mode & S_IFMT) != S_IFLNK ) /* not a symbolic link */
	    break;
    }
    fname = append (bld_name, "");
    return (fname);

#if 0
    int nc;
    char *cp;
    char linkname[150];
    struct stat sb;

    if (lstat (file, &sb) < 0)
	return (file);
    if ( (sb.st_mode & S_IFMT) != S_IFLNK ) /* not a symbolic link */
	return (file);

    if ((nc = readlink (file, linkname, sizeof(linkname))) == -1) {
	mesg (ERRALL + 2, "Can't read symbolic link ", file);
	return NULL;
    }
    linkname[nc] = '\0';    /* documentation says readlink doesn't */
			    /*  null terminate */

    if (*linkname == '/' || ((cp = rindex (file, '/')) == NULL))
	file = append (linkname, "");
    else {                  /* have a relative pathname */
	    char tmp[200];
	strcpy (tmp, file);
	tmp[(cp - file) + 1] = '\0';    /* after last '/' */
	file = append (tmp, linkname);
    }

    return (file);
#endif
}
#endif


#ifdef COMMENT
char *
ExpandName (file)
char *file
.
    Convert a leading "~" into the appropriate home directory.
    (Assumes caller verifies the name begins with ~)

#endif

char *
ExpandName (file)
char *file;
{
    Reg1 char *end;
    Reg2 char *beg;
    char tmp[150];
    struct passwd *pw, *getpwnam();

    beg = file+1;         /* skip ~ */

    if (*beg == '/') {           /* "~/file" */
	strcpy (tmp, getmypath());
	strcat (tmp, beg);
    }
    else {                      /* ~user/file */
	for (beg = tmp, end = file+1; *end && *end != '/'; )
	    *beg++ = *end++;
	*beg = '\0';
	if ((pw = getpwnam(tmp)) == NULL) {
	    mesg (ERRALL+2, "No such user: ", tmp);
	    return NULL;
	}
	strcpy (tmp, pw->pw_dir);
	if (*end)
	    strcat (tmp, end);   /* append rest of the name */
    }

    return (append(tmp, ""));
}

