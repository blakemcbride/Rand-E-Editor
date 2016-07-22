#ifdef COMMENT
--------
file e.iit.c
    initialize input (keyboard) table parsing and lookup
#endif

#ifdef COMMENT
	Copyright abandoned, 1983, The Rand Corporation
#endif


#include <string.h>
#include "e.h"
#ifdef  KBFILE
#include "e.it.h"

extern Flag helpflg, verbose_helpflg, dbgflg;
extern char *salloc();
static void itadd ();

int kbfile_wline = 0;   /* line number of a duplicated string */

/* directives which can be use in kb files */
/* A directive line must start with '#!' string
 *   and only upper case char must be used to define the directive strings
 *   in kb file upper or lower case char can be used
 *   see check_directive routine
 */

/* kb file valid directives list */
/*  must be in upper case (see check_noX11 routine).
    A directive is a line staing with '#!' followed by one of the defined string
    In the kb file upper and lower case char can be used.
*/
static char directive_prefix [] = "#!";
/*      do not change the order in the kbf_directives array, used in "term/linux_pc.c" */
static char *kbf_directives [] = {
    "NOX11",      /* flag for noX11 usage for keyboard mapping */
    "COMMENT",    /* begining of a comments section */
    "-COMMENT",   /* end of the comments section */
     NULL };
#define directives_nb (sizeof (kbf_directives) / sizeof (kbf_directives[0]) -1)

static comment_section_flg = NO;    /* in comment section */

struct itable *ithead = NULLIT;

/* itsyms table must be in alphabetic order */

S_looktbl itsyms [] = {
    "+line",   CCPLLINE,
    "+page",   CCPLPAGE,
    "+sch",    CCPLSRCH,
    "+tab",    CCTAB,
    "+word",   CCRWORD,
    "-line",   CCMILINE,
    "-page",   CCMIPAGE,
    "-sch",    CCMISRCH,
    "-tab",    CCBACKTAB,
    "-word",   CCLWORD,
#ifdef LMCCMDS
    "abort",   CCABORT,
#endif
    "alt",     CCSETFILE,
    "bksp",    CCBACKSPACE,
#ifdef LMCCMDS
    "blot",    CCBLOT,
    "box",     CCBOX,
#ifdef LMCCASE
    "caps",    CCCAPS,
    "ccase",   CCCCASE,
#endif
#endif
    "cchar",   CCCTRLQUOTE,
#ifdef LMCCMDS
    "center",  CCCENTER,
#endif
    "chwin",   CCCHWINDOW,
#ifdef LMCCMDS
    "clear",   CCCLEAR,
#endif
    "close",   CCCLOSE,
#ifdef LMCCMDS
    "cltabs",  CCCLRTABS,
#endif
    "cmd",     CCCMD,
#ifdef LMCCMDS
    "cover",   CCCOVER,
#endif
    "dchar",   CCDELCH,
    "del",     CCBACKSPACE,
/* -------------------------- strange marginal effect
    "del",     0177,
   -------------------------- */
    "down",    CCMOVEDOWN,
#ifdef LMCDWORD
    "dword",   CCDWORD,
#endif
    "edit",    CCSETFILE,
    "erase",   CCERASE,
#ifdef LMCCMDS
    "exit",    CCEXIT,
    "fill",    CCFILL,
    "flist",   CCFLIST,
    "fnavigate", CCFNAVIG,
#endif
#ifdef LMCCMDS
#ifdef LMCHELP
    "help",    CCHELP,
#endif
#endif
    "home",    CCHOME,
    "insmd",   CCINSMODE,
    "int",     CCINT,
    "join",    CCJOIN,
#ifdef LMCCMDS
    "justify", CCJUSTIFY,
#endif
    "kbend",   KBEND,
    "kbinit",  KBINIT,
    "left",    CCMOVELEFT,
    "mark",    CCMARK,
#ifdef LMCCMDS
    "null",    CCNULL,
#endif
    "open",    CCOPEN,
#ifdef LMCCMDS
    "overlay", CCOVERLAY,
#endif
    "pick",    CCPICK,
#ifdef LMCCMDS
    "quit",    CCQUIT,
#endif
#ifdef LMCCMDS
    "range",   CCRANGE,
    "redraw",  CCREDRAW,
#endif
    "replace", CCREPLACE,
    "ret",     CCRETURN,
    "right",   CCMOVERIGHT,
    "split",   CCSPLIT,
    "srtab",   CCTABS,
#ifdef LMCCMDS
    "stopx",   CCSTOPX,
#endif
    "tab",     CCTAB,
    "tick",    CCTICK,
#ifdef LMCCMDS
    "track",   CCTRACK,
#endif
    "undef",   CCUNAS1,
    "up",      CCMOVEUP,
    "wleft",   CCLWINDOW,
#ifdef LMCAUTO
    "wp",      CCAUTOFILL,
#endif
    "wright",  CCRWINDOW,
    0,0,            /* end of key functions */
 };

S_looktbl ctrl_chr [] = {
    "esc",     033,
    "del",     0177,
    "space",   040,
    "quote",   042,
    0,0
 };

#define ITSYMS_SZ (sizeof (itsyms) / sizeof (itsyms [0]))
const int itsyms_sz = ITSYMS_SZ -1;

/* storage for sorted itsyms (pointers to S_lookup records */
static S_looktbl *keyf_storage [ITSYMS_SZ +1] = { NULL };

#if 0
/* space for sorted cmd table and aliases table */
S_looktbl sorted_itsyms [ITSYMS_SZ +1] =
    { 0,0 };
int sorted_itsyms_sz = 0;   /* number of items in sorted table */
#endif

extern int one_help_keyf (S_looktbl *, char *);

/* Editor key functions lookup table structure */
S_lookstruct keyfstruct = {
    itsyms, ITSYMS_SZ -1,   /* reference lookup table */
    &keyf_storage, sizeof (keyf_storage), /* storage for tables */
    NULL, 0,                /* sorted table */
    NULL, 0,                /* aliases table */
    one_help_keyf,          /* help routine */
    NO,                     /* no alias */
    2,                      /* type = key functions */
    NULL, 0                 /* no alias, no major name table */
};



/* terminal initialization and exit escape sequences */
char *kbinistr = NULL;
char *kbendstr = NULL;
int  kbinilen = 0;
int  kbendlen = 0;

static struct itable *it_leave_ctrlc = NULL;
static struct itable  it_leave_ctrlc_ref;
static int ctrlc_is_ret;
static char ccreturn_val [2] = { CCRETURN, 0 };

#if 0
int build_sorted_itsyms ()
{
    extern int build_sorted_looktbl (S_looktbl *, int, S_looktbl *, int *,
				     int, S_looktbl **, int *);
    int cc;

    cc = build_sorted_looktbl (itsyms, itsyms_sz, sorted_itsyms,
			       &sorted_itsyms_sz, sizeof (sorted_itsyms),
			       NULL, NULL);
    return (cc);
}
#endif

/* Get the <Ctrl C> value */
int get_ctrlc_fkey ()
{
    if ( ctrlc_is_ret || !it_leave_ctrlc ) return CCRETURN;
    return (int) *(it_leave_ctrlc->it_val);
}

/* switch <Ctrl C> to <ret> and back (for help output) */
void switch_ctrlc (ret)
Flag ret;
{
    if ( ctrlc_is_ret || !it_leave_ctrlc ) return;
    it_leave_ctrlc->it_len = ( ret ) ? 1 : it_leave_ctrlc_ref.it_len;
    it_leave_ctrlc->it_val = ( ret ) ? ccreturn_val : it_leave_ctrlc_ref.it_val;
}

/* include_cccmd : force <Ctrl M> to be <ret>,
 * if <Ctrl C> is not allocated, set to <ret> (used in help output),
 * and try to add a Control char for <cmd> function
 */
static void include_cccmd ()
{
    extern int itget ();
    extern int itgetleave ();
    extern struct itable *ithead;

    char ch, ch0, cmd[256], *ch_pt;
    int cc, nb;

    ch = ch0 = ('M' & '\037');
    ch_pt = &ch; nb = 1;
    cc = itget (&ch_pt, &nb, ithead, cmd);
    if ( (cc != 1) || (cmd[0] != CCRETURN) ) {
	/* force <Ctrl M> to be CCRETURN */
	cmd[0] = CCRETURN ; cmd[1] = '\0';
	itadd (&ch0, 1, &ithead, cmd, 1, "-- Force setting of 'Ctrl M' to <ret> --", -1);
    }
    ch = ch0 = ('C' & '\037');
    ch_pt = &ch; nb = 1;
    cc = itget (&ch_pt, &nb, ithead, cmd);
    if ( cc == IT_NOPE ) {
	/* <Ctrl C> not allocated, force to CCRETURN */
	cmd[0] = CCRETURN ; cmd[1] = '\0';
	itadd (&ch0, 1, &ithead, cmd, 1, "-- Default setting of 'Ctrl C' to <ret> --", -1);
    }
    /* setup data to be use for temporary overwrite <Crtl C> with <ret> */
    cmd[0] = ('C' & '\037'); cmd[1] = '\0';
    cc = itgetleave (cmd, &it_leave_ctrlc, ithead);
    ctrlc_is_ret = (   (it_leave_ctrlc->it_len == 1)
		    && (*(it_leave_ctrlc->it_val) == CCRETURN) );
    it_leave_ctrlc_ref = *it_leave_ctrlc;

    /* found an empty slot in Ctrl A ... Ctrl Z */
    ch0 = 0;
    for ( ch = ('Z' & '\037') ; ch >= ('A' & '\037') ; ch-- ) {
	ch_pt = &ch; nb = 1;
	cc = itget (&ch_pt, &nb, ithead, cmd);
	if ( (cc == 1) && (cmd[0] == CCCMD) ) return;
	if ( cc == IT_NOPE ) ch0 = ch;     /* empty slot */
    }
    if ( ch0 >= ('A' & '\037') ) {
	cmd[0] = CCCMD; cmd[1] = '\0';
	itadd (&ch0, 1, &ithead, cmd, 1, "-- Default internal setting of <cmd> --", -1);
    }
}

char * get_directive (unsigned int idx)
{
    static char str [80];
    char *cmt;

    memset (str, 0, sizeof (str));
    strcpy (str, directive_prefix);
    if ( idx >= directives_nb ) return str;

    cmt = &str [strlen (str)];
    strcpy (cmt, kbf_directives [idx]);
    if ( idx == 0 ) {
	cmt++; *cmt = tolower (*cmt);
    } else {
	if ( ! isalpha (*cmt) ) cmt++;
	for ( cmt++ ; *cmt ; cmt++ ) *cmt = tolower (*cmt);
    }
    return str;
}

static Flag check_directive (char *line)
{
    extern Flag noX11flg;
    extern void set_noX_msg (char *);
    char chr, str[32], *strg, *direc;
    int i;

    if ( strncmp (line, directive_prefix, sizeof (directive_prefix) -1) != 0 )
	return NO;

    memset (str, 0, sizeof (str));
    strncpy (str, line +2, sizeof (str));
    str[sizeof (str) -1] = '\0';
    for ( strg = str ; chr = *strg ; strg++) *strg = toupper(chr);
    for ( i = 0 ; ; i++ ) {
	direc = kbf_directives [i];
	if ( ! direc )
	    return YES; /* not a defined directive */
	if ( strncmp (str, direc, strlen (direc)) == 0 )
	    break;
    }

    /* directive found */
    switch (i) {
	case 1 :    /* beginnig of comment section */
	    comment_section_flg = YES;
	    break;

	case 2 :    /* end of comment section */
	    comment_section_flg = NO;
	    break;
	}

    if ( comment_section_flg ) return NO;

    switch (i) {
	case 0 :    /* NoX11 flag */
	    if ( comment_section_flg ) break;
	    if ( ! noX11flg ) {
		noX11flg = YES;
		set_noX_msg ("\'#!noX11\' directive in kb file");
	    }
	    break;

	default :
	    /* directive defined, but no processing */
	    return YES;
    }

    if ( verbose_helpflg ) printf ("--- \'%s\' directive found ---\n", direc);
    return YES;
}

static char * check_include (line, path)
char *line, *path;
{
    static const char incl[] = "include ";
    char *fname, *str;
    char c;
    int sz;

    fname = line;
    sz = strlen (incl);
    if ( strncmp (fname, incl, sz) != 0 ) return NULL;
    fname += sz;
    for ( ; c = *fname++ ; ) if ( c != ' ') break;
    if ( ! c ) return NULL;
    if ( (c != '<') && (c != '"') ) return NULL;
    if ( c == '<' ) c = '>';
    str = strchr (fname, c);
    if ( ! str ) return NULL;

    *str = '\0';    /* ignore the rest of the line */
    if ( *fname != '/' ) {
	/* this include file path name is relative to the including file */
	str = strrchr (path, '/');
	if ( str ) {
	    str++;
	    sz = str - path;
	    /* make room for path */
	    memmove (fname + sz, fname, strlen (fname) +1);
	    memcpy (fname, path, sz);
	}
    }
    return fname;
}

static void val_kbini_kbend (int fcmd, char *str, int len, char **kb_str, int* kb_len)
{
    char * itsyms_by_val (short);
    char *sp;

    if ( *kb_str ) {
	if ( verbose_helpflg ) {
	    sp = itsyms_by_val ((short) fcmd);
	    printf ("WARNING : kbfile overwrite previously defined value : <%s>\n",
		    sp ? sp : "???");
	}
	free (*kb_str);
	*kb_len = 0;
    }
    *kb_len = len;
    *kb_str = salloc (len +1, YES);
    strcpy (*kb_str, str);
}


static Flag process_kbfile (filename, level)
char *filename;
int level;
{
    extern void customize_xlate ();
    static Flag itparse ();

    char line[TMPSTRLEN], string[TMPSTRLEN], value[TMPSTRLEN];
    FILE *f;
    int lnb, str_len, val_len;
    char *incl_fname;
    char c;
    Flag cc, alt_flg;

    if ( verbose_helpflg ) printf ("Process kbfile : \"%s\"\n", filename);
    if ( level > 32 ) {
	if ( verbose_helpflg )
	    printf ("  Too large included depth %d : this is probably a loop of include\n", level);
	return NO;
    }
    if ((f = fopen (filename, "r")) == NULL) {
	/* ---- old version
	getout (YES, "Can't open keyboard file \"%s\"", filename);
	---- */
	if ( verbose_helpflg ) printf ("ERROR fopen error %d : %s\n", errno, strerror (errno));
	else if ( helpflg || dbgflg )
	    printf ("\n\"%s\"ERROR  fopen error %d :\n\n  %s\n", filename, errno, strerror (errno));
	return NO;
    }

    for ( lnb = 1 ; ; lnb++ ) {
	memset (line, 0, sizeof (line));
	memset (string, 0, sizeof (string));
	memset (value, 0, sizeof (value));
	if ( fgets (line, sizeof line, f) == NULL ) break;
	line[strlen (line)-1] = '\0';   /* remove newline */
	if ( verbose_helpflg ) printf ("%3d  %s\n", lnb, line);
	if ( line[0] == '#' ) {         /* FP 14 Jan 2000 */
	    if ( check_directive (line) ) continue; /* FP 10 Jan 2001 */
	    if ( comment_section_flg ) continue;
	    incl_fname = line+1;
	    for ( ; c = *incl_fname ; incl_fname++ ) if ( c != ' ') break;
	    incl_fname = check_include (incl_fname, filename);
	    if ( incl_fname ) {
		cc = process_kbfile (incl_fname, level +1);
		if ( verbose_helpflg ) printf ("---Back in kbfile : \"%s\"\n", filename);
	    }
	    continue;
	}
	if ( comment_section_flg ) continue;
	if ( line[0] == '!'             /* add FP 14 Jan 2000 */
	     || line[0] == '\0' )       /* gww 21 Feb 82 */
	    continue;                   /* gww 21 Feb 82 */
	alt_flg = itparse (line, string, &str_len, value, &val_len, filename);
	if ( alt_flg ) {
	    customize_xlate (string [0], value);
	}
	else {
	    switch (string[0]) {
	    case KBINIT:
		val_kbini_kbend (KBINIT, value, val_len, &kbinistr, &kbinilen);
/*
		kbinilen = val_len;
		kbinistr = salloc (kbinilen +1, YES);
		move (value, kbinistr, kbinilen +1);
*/
		break;
	    case KBEND:
		val_kbini_kbend (KBEND, value, val_len, &kbendstr, &kbendlen);
/*
		kbendlen = val_len;
		kbendstr = salloc (kbinilen +1, YES);
		move (value, kbendstr, kbendlen +1);
*/
		break;
	    default:
		itadd (string, str_len, &ithead, value, val_len, line, lnb);
	    }
	}
    }
    fclose (f);
    return (YES);
}

Flag
getkbfile (filename)
char *filename;
{
    /* use the Rand options "-verbose -help" to debug the kbfile */
    extern void overwrite_PF1PF4 ();
    Flag cc;

    cc = process_kbfile (filename, 1);
    if ( !kbinistr ) kbinistr = "";
    if ( !kbendstr ) kbinistr = "";
    include_cccmd ();
    overwrite_PF1PF4 ();
#ifdef  DEBUG_KBFILE
    itprint (ithead, 0);
#endif
    if ( verbose_helpflg ) printf ("\nEnd of Processing kbfile(s)\n");
    return (cc);
}

static void itadd (str, str_len, headp, val, val_len, line, line_nb, filename)
char *str;              /* Character string */
struct itable **headp;  /* Pointer to head (where to start) */
char *val;              /* Value */
int str_len, val_len;
char *line;             /* For debugging */
int line_nb;
char * filename;
{
    char * itsyms_by_val (short);
    struct itable *it;         /* Current input table entry */
    struct itable *pt;         /* Previous table entry */
    char *sp;

    if (str_len == 0)
	getout (YES, "kbfile invalid prefix in %s\n", line);
    for (it = *headp; it != NULLIT; pt = it, it = it->it_next) {
	if (it->it_c == *str) {         /* Character match? */
	    if (it->it_leaf) {          /* Can't add this */
		if ( verbose_helpflg ) {
		    sp = itsyms_by_val ((short) *it->it_val);
		    printf ("WARNING : kbfile overwrite previously defined value : <%s>\n",
			    sp ? sp : "???");
		    kbfile_wline = line_nb;
		}
		/* overwrite the previously defined value */
		if ( val_len != it->it_len ) {
		    sfree (it->it_val);
		    it->it_val = salloc (val_len, YES);
		    it->it_len = val_len;
		}
		move (val, it->it_val, val_len);
	    } else      /* Go down the tree */
		itadd (str+1, str_len-1, &it->it_link, val, val_len, line, line_nb);
	    return;
	}
    }
    it = (struct itable *) salloc (sizeof *it, YES);           /* Get new node */
    if (*headp == 0)                    /* Change head if tree was empty */
	*headp = it;
    else
	pt->it_next = it;               /* Otherwise update prev node */
    it->it_c = *str++;                  /* Save current character */
    it->it_next = 0;
    if (--str_len > 0) {                /* Is this a leaf? */
	it->it_leaf = 0;                /* No */
	it->it_link = 0;
	itadd (str, str_len, &it->it_link, val, val_len, line, line_nb);
    } else {
	it->it_leaf = 1;
	it->it_val = salloc (val_len, YES);
	it->it_len = val_len;
	move (val, it->it_val, val_len);
    }
    return;
}

/* itparse : parse a line of kb file */
/*  strg is the 1st token : before ':', value is the 2nd token : after ':' */

static Flag itparse (inp, strp, str_lenp, valp, val_lenp, filename)
char *inp;
char *strp, *valp;      /* Pointers to string to match and value to return */
int *str_lenp, *val_lenp; /* Where to put the respective lengths */
char *filename;
{
    extern S_looktbl bordersyms [];
    char c;
    unsigned int n;
    long int ln;
    int i;
    int gotval = 0;
    char *outp;
    char tmpstr[50], *tp;
    char *line = inp;            /* Save for error messages */
    Flag alt_flg;

    outp = strp;
    alt_flg = NO;   /* default is normal key code symbol */
    while ((c = *inp++) != '\0') {
	switch (c) {
	    case ' ':
		break;      /* ignore space */

	    case '#':       /* comment mark */
	    case '!':
		break;

	    case '"':       /* String "foo bar" (with no quotes) */
		while ((c = *inp++) != '"')
		    *outp++ = c;
		break;
	    case '<':
#if 0
		/* old version : does not allow decimal or hexadecimal number */
		if ((c = *inp) >= '0' && c <= '7') {
		    for (n = 0; (c = *inp++) != '>'; ) {
			if (c < '0' || c > '7')
			    getout (YES, "kbfile \"%s"\ bad digit in %s\n", filename, line);
			n = n*8 + (c-'0');
		    }
		    if (n > 0377)
			getout (YES, "Number %d too big in kbfile \"%s\"\n", n, filename);
		    *outp++ = (char) n;
#else
		if ((c = *inp) >= '0' && c <= '9') {
		    /* support decimal, octal (0...) and hexadecimal (0x..) number */
		    ln = strtol (inp, &tp, 0);
		    if ( *tp != '>' )
			getout (YES, "kbfile \"%s\" bad digit '%c' in line %s\n", filename, c, line);
		    if ( (ln > 0377) || (ln < 0) )
			getout (YES, "Number %d out of range (0...255) in kbfile \"%s\" in line %s\n", ln, filename, line);
		    inp = tp +1;
		    *outp++ = (char) ln;
#endif
		} else {
		    for (tp = tmpstr; (c = *inp++) != '>'; ) {
			if (c == '\0')
			    getout (YES, "kbfile \"%s\" mismatched < in line %s\n", filename, line);
			*tp++ = c;
		    }
		    *tp = '\0';
		    i = lookup (tmpstr, itsyms);
		    if ( i >= 0 ) *outp++ = (char) itsyms[i].val;
		    else {
			/* try alternate symbols : border char set */
			alt_flg = YES;
			i = lookup (tmpstr, bordersyms);
			if ( i >= 0 ) *outp++ = (char) bordersyms[i].val;
			else
			    getout (YES, "Bad symbol <%s> in kbfile \"%s\" in line %s\n", tmpstr, filename, line);
		    }
		}
		break;
	    case '^':
		c = *inp++;
		if (c != '?' && (c < '@' || c > '_'))
		    getout (YES, "kbfile \"%s\" control char ^%c out of range (^?, ^@...^_)in line %s\n", filename, c, line);
		*outp++ = c^0100;
		break;
	    case ':':
		*str_lenp = outp - strp;
		if (gotval++)
		    getout (YES, "kbfile \"%s\" too many colons in line %s\n", filename, line);
		if (*str_lenp > TMPSTRLEN)
		    getout (YES, "kbfile \"%s\" line too long : %s\n", filename, line);
		outp = valp;
		break;
	    default:
		getout (YES, "kbfile \"%s\" bad char \'%c\' (0%o) in line %s\n", filename, c, c, line);
	}
	if ( (c == '#') || (c == '!') ) break;  /* begining of comment */
    }
    *val_lenp = outp - valp;
    if (*str_lenp > TMPSTRLEN)
	getout (YES, "kbfile \"%s\" line too long : %s\n", filename, line);
    return alt_flg;
}

/* escstrg : return a printable string from a string with control char */

char * escstrg (char *escst)
{
    static char st[128];
    char *sp, ch;
    int sz;

    memset (st, 0, sizeof (st));
    for ( sp = escst; (ch = *sp) ; sp++ ) {
	if ( ch == '\033' ) strcat (st, "<Esc>");
	else if ( ch == '\177' ) strcat (st, "<Del>");
	else if ( ch == '\t' ) strcat (st, "<Tab>");
	else if ( ch < ' ' ) {
	    if ( !st[0] )
		sprintf (st, "<Ctrl %c>", ch + '@');
	    else {
		sz = strlen (st);
		st[sz] = '^';
		st[sz+1] = ch + '@';
	    }
	}
	else st[strlen(st)] = ch;
    }
    return (st);
}

char * itsyms_by_val (short val)
/* return NULL if not found */
{
    int i;
    char *sp;

    for ( i = 0 ; (sp = itsyms [i].str) ; i++ ) {
	if ( itsyms [i].val == val ) break;
    }
    return (sp);
}

/* -------------------------------------- */

/* it_walk : get entry in the it tree */
/* ---------------------------------- */
/*
    On the first call head must point to the head of the it list
	and n must be 0 (current level call in the recursion).
	strg must be large enough to receive the longer it string
*/


static struct itable *it_walk (head, n, strg)   /* n should be 0 the first time */
struct itable *head;
int n;
char *strg;
{
    struct itable *it, *it1;
    int i;
    char c, c1;
    unsigned char *cp, *sp;

    for (it = head; it != NULLIT; it = it->it_next) {
	c = it->it_c;
	c1 = strg [n];
	if ( c1 && (c != c1) )
	    continue;    /* continue to found the last processed leaf */
	strg [n] = c;
	if ( c1 == '\0' ) strg [n+1] = '\0';
	if (it->it_leaf) {
	    if ( ! c1 )
		return (it);    /* a new leaf is found : process it */
	    strg [n] = '\0';    /* this leaf is already processed */
	}
	else {
	    /* go to child leaf */
	    it1 = it_walk (it->it_link, n+1, strg);
	    if ( it1 )
		return (it1);   /* a new leaf must be processed */
	}
    }
    if (n > 0 ) strg [n -1] = '\0';  /* continue on the previous level */
    return (NULL);    /* no more leaves on this level */
}

/* it_listall : display all the elements of the it tree */
/* ---------------------------------------------------- */

static int it_listall ()
{
    int i, nb, nb_leaves;
    struct itable *it;
    unsigned char *cp, *sp;
    char strg [80];

    memset (strg, 0, sizeof (strg));
    nb_leaves = 0;
    for ( nb = 0 ; nb < 10000 ; nb ++ ) {
	it = it_walk (ithead, 0, strg);
	if ( ! it ) break;  /* no more leaves */

	nb_leaves++;
	printf ("%3d : %-10s = ", nb_leaves, escstrg (strg));
	for (cp = (unsigned char *) it->it_val, i = it->it_len ; i-- > 0 ; ) {
	    sp = (unsigned char *) itsyms_by_val ((short) *cp);
	    printf (" (%#4o)", *cp++);
	    if ( sp ) printf ("<%s>", sp);
	}
	putchar ('\n');
    }
    if ( it ) {
	printf ("======= error in it structure ==========\n");
	return -1;
    }
    return (nb_leaves);
}

/* itwalk : get a string for a given value */
/* --------------------------------------- */
/*
    On the first call head must point to the head of the it list
	and n must be 0 (index of the beegining of strg)
*/

static void itwalk (int val, struct itable **it_pt, int *n, char *strg)
{
    struct itable *it, *it_child;
    int n_child;
    int i;
    char c;
    unsigned char *cp;

    for (it = *it_pt; it != NULLIT; it = it->it_next) {
	*it_pt = it;
	c = it->it_c;
	strg [*n] = c;
	strg [*n+1] = '\0';
	if (it->it_leaf) {
	    for (cp = (unsigned char *) it->it_val, i = it->it_len; i-- > 0; ) {
		if ( *cp = val ) return;
	    }
	}
	else {
	    /* go to child */
	    it_child = it->it_link;
	    n_child = *n +1;
	    itwalk (val, &it_child, &n_child, strg);
	}
    }
    return;
}




#ifdef DEBUG_KBFILE
#include <ctype.h>

itprint (head, n)        /* n should be 0 the first time */
struct itable *head;
int n;
{
    register struct itable *it;
    int i;
    char c;
    char *cp;

    for (it = head; it != NULLIT; it = it->it_next) {
	for (i = 0; i < n; i++)
	    putchar (' ');
	c = it->it_c;
	if (isalnum (c))
	    printf ("%c  ", c);
	else
	    printf ("<%3o>", c);
	if (it->it_leaf) {
	    printf ("=");
	    for (cp = it->it_val, i = it->it_len; i-- > 0; )
		printf ("<%o>", *cp++);
	    printf (" (len %d)\n", it->it_len);
	}
	else {
	    printf ("\n");
	    itprint (it->it_link, n+2);
	}
    }
    return;
}
#endif

/* print all the elements of the it table tree (escape seq to key function) */
/* ------------------------------------------------------------------------ */

int print_it_escp ()
{
    int nb;

    printf ("\n---- Escape Sequence to key function -----\n", nb);
    nb = it_listall ();
    printf ("---- defined entries in Escape Seq table : %d -----\n", nb);
    return nb;
}

/* it_value_to_string : return all the string for a given it value */
/* --------------------------------------------------------------- */
/*  if keyflg : return the keys which are assigned to the value */

int it_value_to_string (int value,
		    Flag key_flg,   /* to get the keyboard assignement */
		    char *buff,     /* buffer to receive the result */
		    int buff_sz)    /* buffer size */
{
    int i, sz, nb_leaves;
    struct itable *it;
    unsigned char *cp, *sp;
    char strg [80];

    nb_leaves = 0;
    memset (strg, 0, sizeof (strg));
    for ( ; ; ) {
	it = it_walk (ithead, 0, strg);
	if ( ! it ) break;  /* no more leaves */
	cp = (unsigned char *) it->it_val;  /* if more than 1 value, it must be <cmd><xxx> */
	i = it->it_len;
	if ( cp[i-1] != value ) continue;

	/* TO BE COMPLETED
	if ( key_flg ) {
	} else {
	}
	*/

	sz = strlen (strg);
	if ( strlen (buff) + sz +2 < buff_sz ) {
	    nb_leaves++;
	    strcat (buff, strg);
	    strcat (buff, " ");
	}
    }
    return (nb_leaves);
}

void print_all_ccmd ()
{
    int i, nb, nb_it, sz, val;
    char *strg, *val_strg;
    char ccmd [256];

    printf ("\n---- Key Function to Escape Sequence assignement -----\n");
    nb = 0;
    for ( i = 0 ; itsyms[i].str ; i++ ) {
	memset (ccmd, 0, sizeof (ccmd));
	val = itsyms[i].val;
	val_strg = itsyms[i].str;
	memset (ccmd, 0, sizeof (ccmd));
	nb_it = it_value_to_string (val, NO, ccmd, sizeof (ccmd));
	if ( ! nb_it ) continue;
	if ( !ccmd[0] ) continue;
	sz = printf ("(%#4o) <%s>", val, val_strg);
	for ( ; sz < 17 ; sz++ ) putchar (' ');
	printf (": %s\n", escstrg (ccmd));
	nb++;
    }
    printf ("---- assigned entries : %d (defined key functions : %d) -----\n", nb, i);
}

#endif
