#ifdef COMMENT
--------
file e.pa.c
    command line parsing routines
#endif

#ifdef COMMENT
	Copyright abandoned, 1983, The Rand Corporation
#endif

#ifdef UNIXV7
#include <ctype.h>
#endif
#include "e.h"
#include "e.m.h"
#include "e.cm.h"

/* Default mini nb of alphabetic chars in abreviation for look up processing */
#define DEFAULT_ABV 1

/* Comments for various lookup tables */

extern S_looktbl cmdtable [];
extern S_lookstruct cmdstruct;
extern S_looktbl itsyms [];
extern S_lookstruct keyfstruct;
extern S_looktbl exittable [];
extern S_looktbl wordopttable [];
extern S_looktbl setopttable [];
extern S_looktbl helptable [];
extern S_looktbl qbuftable [];
extern S_looktbl gototable [];
extern S_looktbl filltable [];
extern S_looktbl filetable [];
extern S_looktbl updatetable [];

static struct _lookuptbl_comment {
    S_looktbl *table;
    S_lookstruct *tblstruct;
    char *comt;
    } lookuptbl_comment [] = {
	{ cmdtable, &cmdstruct,  "Editor Command" },
	{ itsyms,   &keyfstruct, "Keybord key function" },
	{ helptable,    NULL,    "\"Help\" command argument" },
	{ exittable,    NULL,    "\"Exit\" command argument" },
	{ setopttable,  NULL,    "\"Set\" command argument" },
	{ wordopttable, NULL,    "\"Set worddelimiter\" command argument" },
	{ qbuftable,    NULL,    "\"Blot\", \"Cover\", \"Insert\", \"Overlay\", \"Underlay\" command argument" },
	{ gototable,    NULL,    "\"Goto\" command argument" },
	{ filltable,    NULL,    "\"Wp\" command argument" },
	{ filetable,    NULL,    "\"File\" Class command argument" },
	{ updatetable,  NULL,    "\"Update\" Class command argument" },
	{ NULL,         NULL,    NULL }
    };
#define lookuptbl_comment_sz ( sizeof (lookuptbl_comment) / sizeof (lookuptbl_comment[0]) )

static S_lookstruct tblstr [lookuptbl_comment_sz];


#ifdef COMMENT
Small
getpartype (str, parflg, defparflg, parline)
    char  **str;
    Flag    parflg;
    Flag    defparflg;
    Nlines  parline;
.
    Parse for things like "3p", "3l", "45", "45x8" etc.
    When a number of paragraphs is specified, the "parline" argument
    is used as the first line of the paragraph parsing.
    If parflg != 0, then allow paragraph specs.
    If defparflg != 0, then default is paragraphs.
    str is the address of a pointer to the first character
    Returns:
      0  *str is empty (== "")
      1  one number found, and its value is stored in parmlines
	    parmlines will be < 0 if it means num of paragraphs.
      2  two numbers found - stored in paramlines and paramcols
      3  not recognizable as 0, 1, or 2
    str is set to next word if this word returns 1 or 2
#endif
Small
getpartype (str, parflg, defparflg, parline)
char **str;
Flag    parflg;
Flag    defparflg;
Nlines  parline;
{
    register char *cp,
		  *cp1;
    int tmp;    /* must be int for s2i */
    Nlines plines;

    cp = *str;
    if (*cp == 0)
	return 0;   /* no argument at all */
    for (; *cp && *cp == ' '; cp++)
	continue;
    cp1 = cp;
    cp = s2i (cp, &tmp);
    tmp = la_ltrunc (tmp);
    if (cp == cp1)
	return 3;   /* string */
    if (parflg && (*cp == 'p' || *cp == 'P')) {
	cp++;
	plines = lincnt (parline, tmp, 2);
    }
    else if (*cp == 'l' || *cp == 'L') {
	cp++;
	plines = tmp;
    }
    else if (defparflg)
	plines = lincnt (parline, tmp, 2);
    else
	plines = tmp;
    if (*cp == ' ' || *cp == 0) {
	for (; *cp && *cp == ' '; cp++)
	    continue;
	*str = cp;
	parmlines = plines;
	return 1;
    }
    if (*cp != 'x' && *cp != 'X')
	return 3;
    cp++;
    cp1 = cp;
    cp = s2i (cp, &tmp);
    if (cp == cp1)
	return 3;   /* string */
    if (*cp == ' ' || *cp == 0) {
	for (; *cp && *cp == ' '; cp++)
	    continue;
	*str = cp;
	parmlines = plines;
	parmcols = tmp;
	return 2;
    }
    return 3;
}

#define MAXLUPN 63      /* longer than longest possible name */

/* last lookup call table which generate a -2 (ambiguous) error */
/* ------------------------------------------------------------ */

static Flag non_existing_cmd_flg = NO;
static Flag too_short_abrev_flg = NO;
static Flag ambiguous_file_flg = NO;
static S_looktbl *last_ambiguous_table = NULL;
static char last_ambiguous_str [MAXLUPN+1];
static char last_extension_str [MAXLUPN+1];

static int get_larger (char *lname, S_looktbl *table, int  idx)
{
    int i, j, sz, sz0;
    char *str0, *str;

    sz0 = strlen (lname);
    str0 = table[idx].str;
    sz = strlen (str0);
    for ( i = idx +1 ; table[i].str ; i++ ) {
	str = table[i].str;
	if ( strncmp (str, lname, sz0) != 0 ) break;
	for ( j = sz0 ; j < sz ; j++ ) if ( str[j] != str0[j] ) break;
	sz = j;
	if ( sz == sz0 ) break;
    }
    return (sz - sz0);
}

void set_ambiguous_param (S_looktbl *table, char *str, Flag abv)
{
    if ( !table ) return;

    last_ambiguous_table = table;
    too_short_abrev_flg = abv;
    memset (last_ambiguous_str, 0, sizeof (last_ambiguous_str));
    memset (last_extension_str, 0, sizeof (last_extension_str));
    if ( str ) strncpy (last_ambiguous_str, str, sizeof (last_ambiguous_str) -1);
}

static void save_ambiguous_param (S_looktbl *table, char *str, Flag abv, int i)
{
    int sz, sz_ext;

    if ( !table ) return;
    set_ambiguous_param (table, str, abv);
    if ( abv || !str ) return;

    sz_ext = get_larger (str, table, i);
    if ( sz_ext <= 0 ) return;
    sz = strlen (str);
    memcpy (last_extension_str, table[i].str +sz, sz_ext);
}

static S_lookstruct * bld_sorted_tbl (int idx)
{
    extern int cmp_pt_looktb (S_looktbl **, S_looktbl **);
    S_lookstruct *tbls_pt;
    S_looktbl *table;
    int i, nb, *sz_pt;

    table = lookuptbl_comment [idx].table;
    if ( ! table ) return NULL;
    if ( lookuptbl_comment [idx].tblstruct ) return;

    tbls_pt = &(tblstr[idx]);
    lookuptbl_comment [idx].tblstruct = tbls_pt;
    for ( nb = 0 ; table [nb].str ; nb++ ) ;
    if ( ! nb ) return NULL;
    tbls_pt->table = table;
    tbls_pt->table_sz = nb;
    tbls_pt->sorted_table = (S_looktbl *(*)[]) calloc (nb +1, sizeof (S_looktbl *));
    if ( ! tbls_pt->sorted_table ) return;
    tbls_pt->sorted_table_sz = nb;
    for ( i = 0 ; i < nb ; i++ )
	(*tbls_pt->sorted_table) [i] = table + i;

    /* sort the sorted table now */
    qsort (tbls_pt->sorted_table, tbls_pt->sorted_table_sz,
	   sizeof (S_looktbl *),
	   (int (*)(const void *,const void *)) cmp_pt_looktb);

    return tbls_pt;
}

/* Get the sorted table return the size (or 0) and the comment */

int get_sorted_table (S_looktbl *table, S_looktbl *(**sorted_table_pt)[],
		      S_lookstruct **tblstruct_ptpt, char **comment)
{
    S_lookstruct *tblstruct_pt;
    int idx;

    if ( !table ) return 0;
    for ( idx = 0 ; lookuptbl_comment [idx].table != NULL ; idx++ ) {
	if ( lookuptbl_comment [idx].table == table )
	    break;
    }
    if ( tblstruct_ptpt ) *tblstruct_ptpt = NULL;
    if ( sorted_table_pt ) *sorted_table_pt = NULL;
    if ( !lookuptbl_comment [idx].table ) {
	/* not found */
	if ( comment ) *comment = "";
	return 0;
    }

    if ( comment ) *comment = lookuptbl_comment [idx].comt;

    /* try to return the sorted table */
    tblstruct_pt = lookuptbl_comment [idx].tblstruct;
    if ( !tblstruct_pt )
	tblstruct_pt = bld_sorted_tbl (idx);
    if ( tblstruct_ptpt )
	*tblstruct_ptpt = tblstruct_pt;
    if ( tblstruct_pt && sorted_table_pt )
	*sorted_table_pt = tblstruct_pt->sorted_table;

    if ( tblstruct_pt ) return tblstruct_pt->sorted_table_sz;
    else return 0;
}


int get_ambiguous_param (char **amb_kwd, S_looktbl **table_pt,
			 S_looktbl *(**sorted_table_pt)[],
			 int *sorted_table_sz, Flag *abv, char **comment)
/*  Return : ambiguous type
 *           0 : nothing
 *           1 : ambiguous command parameter : return all other value
 *           2 : ambiguous files expanded
 */
{
    S_lookstruct *tblstruct_pt;
    int idx;

    if ( amb_kwd ) *amb_kwd = NULL;
    if ( !last_ambiguous_table )
	return ( ambiguous_file_flg ? 2 : 0 );

    if ( !last_ambiguous_table || !table_pt )
	return 0;

    if ( amb_kwd ) *amb_kwd = last_ambiguous_str;
    for ( idx = 0 ; lookuptbl_comment [idx].table ; idx++ ) {
	if ( lookuptbl_comment [idx].table == last_ambiguous_table )
	    break;
    }
    *table_pt = last_ambiguous_table;
    if ( sorted_table_pt ) *sorted_table_pt = NULL;
    if ( sorted_table_sz ) *sorted_table_sz = 0;
    if ( abv ) *abv = too_short_abrev_flg;
    if ( !lookuptbl_comment [idx].table ) {
	/* not found */
	if ( comment ) *comment = NULL;
	return 1;
    }

    if ( comment ) *comment = lookuptbl_comment [idx].comt;
    if ( !*last_ambiguous_str && sorted_table_pt ) {
	/* all keyword : try to return the sorted table */
	tblstruct_pt = lookuptbl_comment [idx].tblstruct;
	if ( !tblstruct_pt )
	    tblstruct_pt = bld_sorted_tbl (idx);
	if ( tblstruct_pt ) {
	    *sorted_table_pt = tblstruct_pt->sorted_table;
	    if ( sorted_table_sz )
		*sorted_table_sz = tblstruct_pt->sorted_table_sz ;
	}
    }
    return 1;
}

void clear_ambiguous_param ()
{
    last_ambiguous_table = NULL;
    ambiguous_file_flg = non_existing_cmd_flg = too_short_abrev_flg = NO;
}

void set_ambiguous_file ()
{
    ambiguous_file_flg = YES;
}

void set_non_existing_cmd ()
{
    non_existing_cmd_flg = YES;
}

Flag check_ambiguous_param ()
{
    return ( (last_ambiguous_table != NULL) || ambiguous_file_flg );
}

int check_registered_error ()
/* Return value : compatible with return from lookup () call */
{
    if ( non_existing_cmd_flg ) return -1;
    if ( ambiguous_file_flg ) return -4;
    if ( last_ambiguous_table != NULL ) return -2;
    if ( too_short_abrev_flg ) return -2;
    return 0;
}

/* Check a keyword against the entry of the given lookup table */
/* ----------------------------------------------------------- */

int check_keyword (char *keyword, int pos,
		   S_looktbl *table, int mini_abv,
		   int *insert_pos, char **name_expantion_pt, int *wlen_pt,
		   int *key_idx_pt, Flag max_flg)
/*
 * Check a keyword against the entry of the given lookup table
 * If table is NULL, use the saved table which generate the "ambiguous" error
 * If keyword is NULL, check the begining of the command line string "ccmdp" array
 * if mini_abv = 0 : use the default minimum size of abreviation
 *   for editor commands.
 * Return -1 : not found
 *        -2 : ambiguous
 *        -3 : empty keyword or some parameter out of boundary
 *        -4 : reserved for ambiguous file name (for file name parameter)
 *         0 : exact match
 *         1 : abreviation
 *      *insert_pos : index (in keyword or ccmdp char array) of insertion point
 *      *name_expantion_pt : what to insert
 *      *wlen_pt : lengh of insertion
 *      *key_idx_pt : lookup return value (index or error)
 */
{
    int lookup_abv (char *, S_looktbl *, int);
    char *getparam (char *str, int *beg_pos, int *end_pos);

    int idx, idx1, sz;
    int pos1, pos2;
    char *name, *cmd, *kwd;
    char strg [256]; /* must be large enough : see MAXLUPN in e.pa.c */

    if ( insert_pos ) *insert_pos = 0;
    if ( name_expantion_pt ) *name_expantion_pt = NULL;
    if ( wlen_pt ) *wlen_pt = 0;
    if ( key_idx_pt ) *key_idx_pt = -1;

    if ( !keyword ) {
	/* check for keyword in command line */
	extern int getccmd (char *strg, int strg_sz, int *ccmdpos);

	(void) getccmd (strg, sizeof(strg), NULL);
	if ( !table ) table = cmdtable;
    } else {
	/* a given keyword */
	memset (strg, 0, sizeof (strg));
	strncpy (strg, keyword, sizeof (strg) -1);
    }

    if ( (pos < 0) || (pos >= strlen (strg)) )
	return -3;
    pos1 = pos;
    kwd = getparam (strg, &pos1, &pos2);
    if ( insert_pos ) *insert_pos = pos2;
    if ( ! kwd ) return -3;

    strg [pos2] = '\0';
    if ( ! *kwd ) return -3;  /* empty */

    if ( table == NULL ) table = last_ambiguous_table;
    if ( table == NULL ) return -1;
    last_ambiguous_table = NULL;

    if ( mini_abv ) idx = lookup_abv (kwd, table, mini_abv);
    else idx = lookup (kwd, table);
    if ( key_idx_pt ) *key_idx_pt = idx;
    if ( idx == -1 ) return -1;   /* not found */
    else if ( idx == -2 ) {       /* ambiguous */
	*name_expantion_pt = last_extension_str;
	*wlen_pt = strlen (*name_expantion_pt);
	putchar ('\a');
	return -2;
    }
    else if ( idx < 0 )
	return -3;

    sz = strlen (kwd);

    if ( max_flg ) {
	/* found the longest keyword string */
	static int longest_cmd (char *, S_looktbl *, int);
	idx1 = longest_cmd (kwd, table, table[idx].val);
	if ( idx1 >= 0 ) idx = idx1;
    }

    name = table[idx].str;
    if ( sz >= strlen (name) )
	return 0;   /* exact match ( cannot be > ) */

    /* not an ambiguous abreviation */
    *name_expantion_pt = &name[sz];
    *wlen_pt = strlen (*name_expantion_pt);
    return 1;
}

/* Look up commands table processing */
/* --------------------------------- */
/*
 *  Lookup name in table.  Will take nonambiguous abbreviations.  If you
 *    want to insist that a certain table entry must be spelled out, enter it
 *    twice in the table.  Table entries must be sorted by name, and a name
 *    which is a substring of a longer name comes earlier in the table.
 *    Accepts upper or lower case if table entry is lower case.
 *  Returns:
 *   >= 0 table entry index
 *     -1 not found
 *     -2 ambiguous
 *
 *  Table entries must be sorted by name in strict ascii char code order
 *
 *  lookup : minimum abreviation (alphabetic characters)
 *           (the default lenth is defined in the routine.
 *  lookp_abv : minimum abreviation (>0) is defined by the caller
 *
 */


static int lookup_exec (char *name, S_looktbl *table, int abv)
{
    char *cmd;
    int cc, i, idx, sz, mini_abv, sz_ext;
    char lname [MAXLUPN+1];

    if ( !name || !*name ) return -1;
    if ( strlen (name) >= MAXLUPN ) return -1;

    mini_abv = ( abv > 0 ) ? abv : DEFAULT_ABV;
    sz = strlen (name);
    memset (lname, 0, sizeof (lname));
    for ( i = 0 ; i < sz ; i++ ) lname[i] = tolower (name [i]);

    idx = -1;   /* default value : not found */
    for ( i = 0 ; (cmd = table[i].str) != NULL ; i++ ) {
	cc = strncmp (cmd, lname, sz);
	if ( cc < 0 ) continue;
	if ( cc > 0 ) break;    /* nothing more */

	if ( sz == strlen (cmd) )
	    return i; /* exact match */
	if ( (idx >= 0) && (table[idx].val != table[i].val) ) {
	    save_ambiguous_param (table, lname, NO, idx);
	    return -2;  /* realy ambiguous */
	}
	idx = i;    /* remind the longest string */
    }
    if ( idx >= 0 ) {
	if ( !isalpha (name[0]) ) mini_abv++;
	if ( sz < mini_abv ) {  /* less than mini abreviation : ambiguous */
	    save_ambiguous_param (table, lname, YES, idx);
	    return -2;
	}
    }
    return idx;
}

#if 0
static int lookup_proc (char *name, S_looktbl *table, int abv)
{
    int cc;
    cc = (lookup_exec (name, table, abv));
    if ( cc == -1 ) last_ambiguous_table = NULL;
    return cc;
}
#endif

/* look up with caller defined mini abreviation size */
int lookup_abv (char *name, S_looktbl *table, int abv)
{
    return (lookup_exec (name, table, abv));
}

/* look up with default abreviation size */
int lookup (char *name, S_looktbl *table)
{
    return (lookup_exec (name, table, 0));
}

/* find the larger matching command of the given value */
static int longest_cmd (char *name, S_looktbl *table, int val)
{
    int nb, i, idx, sz;
    char *cmd;

    nb = strlen (name);
    idx = -1;   /* default value : not found */
    for ( i = 0 ; (cmd = table[i].str) != NULL ; i++ ) {
	if ( table[i].val != val ) continue;
	sz = strlen (cmd);
	if ( sz <= nb ) continue;
	if ( strncmp (cmd, name, nb) != 0 ) continue;
	nb = sz;
	idx = i;
    }
    return idx;
}

#if 0       /* old fashion !! */
int
lookup (name, table)     /* table entries must be sorted by name */
char *name;
S_looktbl *table;

    char *namptr, *tblptr;
    int ind;
    int value = 0;
    Small length;
    Small longest = 0;
    Flag ambig = NO;
    char lname [MAXLUPN];

    namptr = name;
    tblptr = lname;
    for (;;) {
	if ((*tblptr++ = isupper (*namptr)? tolower (*namptr++): *namptr++)
	    == '\0')
	    break;
	if (tblptr >= &lname[MAXLUPN])
	    return -1;
    }

    for (ind = 0; (tblptr = table->str) != 0; table++, ind++) {
	namptr = lname;
	for (; *tblptr == *namptr; tblptr++, namptr++) {
	    if (*tblptr == '\0')
		break;
	}
	if (*namptr == '\0') {  /* end of name or exact match */
	    length = namptr - lname;
	    if (longest < length) {
		longest = length;
		ambig = NO;
		value = ind;
		if (*tblptr == '\0')
		    break;          /* exact match */
	    }
	    else /* longest == length */
		ambig = YES;
	}
	else if ( *namptr < *tblptr )
	    break;
    }
    if (ambig)
	return -2;
    if (longest)
	return value;
    return -1;
}
#endif

/* get the next parameter in string (no allocated space) */
/*
 * Look for the next word (according to the getword processing) in str
 * On entry if beg_pos != NULL the value ( >=0 ) define where to start
 *    the scan for the next word.
 * Return : the pointer to the begining of next word or NULL
 *    *beg_pos : index of the 1st char of the next word
 *    *end_pos : index of the char (or '\0') after the next word
 */

char *getparam (char *str, int *beg_pos, int *end_pos)
{
    char *cp1, *cp2;
    int pos;

    pos = ( beg_pos && *beg_pos > 0 ) ? *beg_pos : 0;
    if ( end_pos ) *end_pos = pos;
    if ( !str || !*str ) return NULL;

    cp1 = str;
    if ( beg_pos && *beg_pos > 0 ) cp1 += *beg_pos;
    for ( ; *cp1 == ' '; cp1++) ;
    for (cp2 = cp1; *cp2 && *cp2 != ' '; cp2++) ;
    if ( beg_pos ) *beg_pos = cp1 -str;
    if ( end_pos ) *end_pos = cp2 -str;
    return ( *cp1 ) ? cp1 : NULL;
}

#ifdef COMMENT
char *
getword (str)
    char **str;
.
    Finds the first non-blank in *str then finds the first blank or '\0'
    after that, then allocs enough space for the 'word' thus delimited,
    copies it into the alloced space, and null-terminates the new string.
    If the returned string is a null string (not a null pointer) then it
    was NOT alloced.
    Returns a pointer to the new string and makes *str to point to the
    next 'word'.
#endif
char *
getword (str)
char **str;
{
    static char nullstr [] = "";

    char *cp1, /* first non-blank in str */
	 *cp2, /* first blank or '\0' after cp1 */
	 *cp3; /* first non-blank or '\0' after cp2 */
    char *newstr;
    int sz, end_pos;

    if ( !str || !*str )
	return nullstr;

     cp1 = getparam (*str, NULL, &end_pos);
     if ( ! cp1 )
	return nullstr;

    cp2 = *str + end_pos;
    sz = cp2 - cp1;
    if ( sz <= 0 )
	return nullstr;

    for (cp3 = cp2; *cp3 == ' '; cp3++) ;
    *str = cp3;
    newstr = salloc (sz + 1, YES);
    memcpy (newstr, cp1, sz);
    newstr [sz]= '\0';
    return newstr;
}

#ifdef COMMENT
Cmdret
scanopts (cpp, pardefault, table, tblfunc)
    char **cpp;
    Flag pardefault;
    S_looktbl *table;
    int (*tblfunc) ();
.
    Scans the command line for options from a table of options,
    and will take an areaspec along the way.
    You might not be at end of string if this routine returns >= 0
    returns:
	 3 marked area        \
	 2 rectangle           \
	 1 number of lines      > may have stopped at an unknown option
	 0 no area spec        /
	-2 ambiguous option     ) see parmlines, parmcols for type of area
     <= -3 other error
#endif
Cmdret
scanopts (cpp, pardefault, table, tblfunc)
char **cpp;
Flag pardefault;
S_looktbl *table;
int (*tblfunc) ();
{
    int areatype;
    int tmp;

    if (curmark)
	areatype = 3;
    else
	areatype = 0;
    parmlines = 0;
    parmcols  = 0;
    for (;;) {
	tmp = getpartype (cpp, YES, pardefault, curwksp->wlin + cursorline);
	switch (tmp) {
	case 0: /* end of string */
	    return (Cmdret) areatype;

	case 1:
	case 2:
	    if (areatype) {
		if (areatype == 3)
		    return CRMARKCNFL;
		else
		    return CRMULTARG;
	    }
	    areatype = tmp;
	    break;

	default:
	    tmp = getopteq (cpp, table, tblfunc);
	    if (tmp <= -2)
		return (Cmdret) tmp;
	    if (tmp != 1 )
		return (Cmdret) areatype;
	}
    }
    /* only return is from inside for loop */
    /* NOTREACHED */
}

#ifdef COMMENT
Small
getopteq (str, table, tblfunc)
    char **str;
    S_looktbl *table;
    int (*tblfunc) ();
.
    Look for things like width=XXX
    Returns:
       1  valid option dealt with by calling tblfunc
       0  end of string
      -1  option not found
      -2  ambiguous option
     < 0  other error
#endif
Small
getopteq (str, table, tblfunc)
char **str;
S_looktbl *table;
int (*tblfunc) ();
{
    Small tmp;
    char *cp;
    char *cp1;
    Char svchr;
    Flag equals;
    int cc;

    /* skip over blanks */
    for (cp = *str; *cp && *cp == ' '; cp++)
	continue;
    cp1 = cp;
    equals = NO;
    /* delimit a word */
    for (; *cp && *cp != ' '; cp++)
	if (*cp == '=') {
	    equals = YES;
	    break;
	}
    if (cp == cp1) {
	if (equals)
	    return -1;
	return 0;
    }

    svchr = *cp;
    *cp = '\0';
    tmp = lookup_abv (cp1, table, 1);
    *cp = svchr;

    if ( tmp == -1 ) {
	extern Cmdret help_cmd_arguments (char *str, S_looktbl *table);
	/* check for "xxx ?" command help style*/
	cc = help_cmd_arguments (cp1, table);
    }

    if (tmp < 0)
	return tmp;
    if (tblfunc)
	return (*tblfunc) (cp, str, tmp, equals);
    return CRBADARG;
}

#ifdef COMMENT
Small
doeq (cpp, value)
    char **cpp;
    int *value;
.
    Parse stuff after '=' -- called only by fillopts().
    Returns:
       0 good value, and it was stuffed in *value
     < 0 error type
#endif
Small
doeq (cpp, value)
char **cpp;
int *value;
{
    char *cp;
    char *cp1;
    int tmp;

    cp = *cpp;
    if (*cp++ != '=')
	return CRNOVALUE;
    cp1 = cp;
    cp = s2i (cp1, &tmp);
    if (cp == cp1)
	return CRNOVALUE;
    if (*cp == ' ' || *cp == '\0') {
	for (; *cp && *cp == ' '; cp++)
	    continue;
	*value = tmp;
	*cpp = cp;
	return 0;
    }
    return CRBADARG;
}
