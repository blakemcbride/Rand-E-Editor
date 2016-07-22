#ifdef COMMENT
--------
file e.cm.c
    command dispatching routine and some actual command-executing routines.
#endif

#ifdef COMMENT
	Copyright abandoned, 1983, The Rand Corporation
#endif

#ifdef __linux__
#define _FILE_OFFSET_BITS 64
#endif

#include <time.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include "e.h"
#include "e.e.h"
#include "e.m.h"
#include "e.ru.h"
#include "e.cm.h"
#include "e.wi.h"
#ifdef LMCHELP
#include "e.tt.h"
#endif

extern Cmdret fileStatus ();
extern void resize_screen ();
extern char * itsyms_by_val (short val);
extern char * get_debug_name ();
extern Flag get_xterm_name (char **class_pt, char ** name_pt);
extern int  itswapdeldchar (char *, int, int *);
extern char * get_myhost_name ();

extern Flag inputcharis7bits;
extern Flag fill_hyphenate;

extern int get_sorted_table (S_looktbl *table, S_looktbl *(**sorted_table_pt)[],
			     S_lookstruct **tblstruct_ptpt, char **comment);
extern int check_keyword (char *, int, S_looktbl *, int,
			  int *, char **, int *, int *, Flag);
extern void refresh_info ();
extern char * get_pref_file_name (Flag create_dir, Flag *exist);
extern Cmdret do_fullscreen (Cmdret (*process) (), void *p1, void *p2, void *p3);

#include SIG_INCL

extern S_looktbl cmdtable [];
extern S_looktbl itsyms [];
extern S_looktbl exittable [];
extern S_looktbl wordopttable [];
       S_looktbl setopttable [];
extern S_looktbl helptable [];
extern S_looktbl qbuftable [];
       S_looktbl gototable [];
extern S_looktbl filltable [];
extern S_looktbl filetable [];
       S_looktbl updatetable [];

       S_lookstruct cmdstruct;
extern S_lookstruct keyfstruct;

void get_preferences (Flag, Flag);

/* flag to signal when in get preferences processing */
static char * preferences_file_name = NULL;
static Flag use_preferences_flg = NO;
static Flag get_preferences_flg = NO;

/* flag to signal various set option done */
static Flag defplline_flg = NO;
static Flag defmiline_flg = NO;
static Flag defplpage_flg = NO;
static Flag defmipage_flg = NO;
static Flag deflwin_flg   = NO;
static Flag defrwin_flg   = NO;
static Flag linewidth_flg = NO;
static Flag autolmarg_flg = NO;

/* structure to init all the lookup table of the system,
 *  it must include all look up table of the Rand.
 * Each lookup table must be ended by a null record (str = NULL).
 */
static struct _lookup_table {
	S_looktbl *table;
	S_lookstruct *lstruct;  /* NULL if not provided */
	int table_sz;           /* number of entry in the lookup table */
	char * comment;         /* use in case of error */
    } all_lookup_tables [] = {
	cmdtable     , &cmdstruct,  0, "cmdtable",
	itsyms       , &keyfstruct, 0, "itsyms (key functions)",
	exittable    , NULL,        0, "exittable",
	wordopttable , NULL,        0, "wordopttable",
	setopttable  , NULL,        0, "setopttable",
	helptable    , NULL,        0, "helptable",
	qbuftable    , NULL,        0, "qbuftable",
	gototable    , NULL,        0, "gototable",
	filltable    , NULL,        0, "filltable",
	filetable    , NULL,        0, "filetable",
	updatetable  , NULL,        0, "updatetable"
    };

/* command_msg : if not NULL, display the message on return from a command processing */
char *command_msg = NULL;

/* string for <Del> character */
char del_strg [] ="\177";

/* Command table :
 *  For alias name definition, the last name in modified alphabetic order
 *      is taken as the major name for an alias suite.
 *  This table do not have to be ordered in alphabetic order, this
 *      is done by the "build_lookup_struct" routine.
 */

S_looktbl major_cmdtable [] = {
    "exit"    , CMDEXIT     ,
    0, 0
};

#define MAJOR_CMDTB_SZ (sizeof (major_cmdtable) / sizeof (major_cmdtable [0]))


S_looktbl cmdtable [] = {
#ifdef CMDVERALLOC
    "#veralloc", CMDVERALLOC,
#endif
    "-blot"   , CMD_BLOT    ,
    "-close"  , CMD_CLOSE   ,
    "-command", CMD_COMMAND ,
#ifdef NOTYET
    "-diff"   , CMD_DIFF    ,
#endif
#ifdef LMCDWORD
    "-dword"  , CMD_DWORD   ,
#endif
    "-erase"  , CMD_ERASE   ,
    "-join"   , CMDSPLIT    ,
#ifdef FUTURCMD
    "-macro"  , CMD_MACRO   ,
#endif
    "-mark"   , CMD_MARK    ,
    "-pick"   , CMD_PICK    ,
    "-range"  , CMD_RANGE   ,
    "-re"     , CMD_PATTERN ,   /* Added Purdue CS 10/18/82 MAB */
    "-regexp" , CMD_PATTERN ,   /* Added Purdue CS 10/18/82 MAB */
    "-replace", CMD_REPLACE ,
    "-search" , CMD_SEARCH  ,
    "-split"  , CMDJOIN     ,
    "-tab"    , CMD_TAB     ,
    "-tabfile", CMD_TABFILE ,
    "-tabs"   , CMD_TABS    ,
    "-tick"   , CMD_TICK    ,
    "-track"  , CMD_TRACK   ,
    "-update" , CMD_UPDATE  ,
    "-w"      , CMD_WINDOW  ,
    "-window" , CMD_WINDOW  ,
#ifdef LMCAUTO
    "-wp"     , CMD_AUTO    ,
#endif
    "?file"   , CMDQFILE    ,
    "?range"  , CMDQRANGE   ,
    "?set"    , CMDQSET     ,
    "?tick"   , CMDQTICK    ,
    "b"       , CMDEXIT     ,
    "blot"    , CMDBLOT     ,
    "box"     , CMDBOX      ,
    "build-kbfile", CMDBKBFILE ,
    "build-kbmap",  CMDBKBMAP  ,
    "bye"     , CMDEXIT     ,
    "call"    , CMDCALL     ,
#ifdef LMCCASE
    "caps"    , CMDCAPS     ,
    "ccase"   , CMDCCASE    ,
#endif
    "cd"      , CMDCD       , 
    "center"  , CMDCENTER   ,
    "checkscr", CMDCHECKSCR ,
    "clear"   , CMDCLEAR    ,
    "close"   , CMDCLOSE    ,
    "command" , CMDCOMMAND  ,
    "cover"   , CMDCOVER    ,
    "delete"  , CMDDELETE   ,
#ifdef NOTYET
    "diff"    , CMDDIFF     ,
#endif
    "dword"   , CMDDWORD    ,
    "e"       , CMDEDIT     ,
    "edit"    , CMDEDIT     ,
#ifdef FUTURCMD
    "endm"    , CMDENDM     ,
#endif
    "erase"   , CMDERASE    ,
    "exit"    , CMDEXIT     ,
    "feed"    , CMDFEED     ,
    "file"    , CMDFILE     ,
    "files"   , CMDSHFILES  ,
    "fill"    , CMDFILL     ,
    "flipbkar", CMDFLIPBKAR ,
    "flipbkarrow", CMDFLIPBKAR ,
    "goto"    , CMDGOTO     ,
    "help"    , CMDHELP     ,
    "insert"  , CMDINSERT   ,
    "join"    , CMDJOIN     ,
    "justify" , CMDJUST     ,
    "logoff"  , CMDLOGOUT   ,
    "logout"  , CMDLOGOUT   ,
#ifdef FUTURCMD
    "macro"   , CMDMACRO    ,
#endif
    "mark"    , CMDMARK     ,
    "name"    , CMDNAME     ,
    "open"    , CMDOPEN     ,
    "overlay" , CMDOVERLAY  ,
    "pick"    , CMDPICK     ,
#ifdef FUTURCMD
    "print"   , CMDPRINT    ,
#endif
    "pwd"     , CMDSTATS    ,
    "q"       , CMDEXIT     ,
    "quit"    , CMDEXIT     ,
    "range"   , CMDRANGE    ,
    "re"      , CMDPATTERN  ,
    "redraw"  , CMDREDRAW   ,
    "regexp"  , CMDPATTERN  ,
    "replace" , CMDREPLACE  ,
/*  "resize"  , CMDRESIZE   ,   used purely internaly */
    "run"     , CMDRUN      ,
    "save"    , CMDSAVE     ,
    "search"  , CMDSEARCH   ,
    "set"     , CMDSET      ,
    "shell"   , CMDSHELL    ,
    "split"   , CMDSPLIT    ,
    "status"  , CMDSTATUS   ,
#ifdef SIGTSTP  /* 4bsd vmunix */
    "stop"    , CMDSTOP     ,
#endif
    "tab"     , CMDTAB      ,
    "tabfile" , CMDTABFILE  ,
    "tabs"    , CMDTABS     ,
    "tick"    , CMDTICK     ,
    "track"   , CMDTRACK    ,
    "underlay", CMDUNDERLAY ,
    "undo"    , CMDUNDO     ,
    "update"  , CMDUPDATE   ,
    "version" , CMDVERSION  ,
    "w"       , CMDWINDOW   ,
    "window"  , CMDWINDOW   ,
#ifdef LMCAUTO
    "wp"     ,  CMDAUTO     ,
#endif
    0, 0
};

#define CMDTB_SZ (sizeof (cmdtable) / sizeof (cmdtable [0]))

/* storage for sorted and aliases (pointers to S_lookup records */
static S_looktbl *cmd_storage [CMDTB_SZ +1] = { NULL };

extern int one_help_cmd (S_looktbl *, char *);

/* Editor commands lookup table structure */
S_lookstruct cmdstruct = {
    cmdtable, CMDTB_SZ -1,  /* reference lookup table */
    &cmd_storage, sizeof (cmd_storage), /* storage for tables */
    NULL, 0,                /* sorted table */
    NULL, 0,                /* aliases table */
    one_help_cmd,           /* help routine */
    YES,                    /* as aliases */
    1,                      /* type = editor commands */
    major_cmdtable, MAJOR_CMDTB_SZ -1  /* major name table */
};



/* Expand file name static storage */
/* ------------------------------- */

static struct dirent **namelist = NULL;
static int nb_namelist = 0;
static int dir_cmd_flg;
static int namelist_idx;
static char *dir_name = NULL;
static char *dir_name_fname = NULL;
static int dir_name_fname_sz = 0;
static int fname_para_sz = 0;
static char name_exp [PATH_MAX+2];


/* Set command arguments look up table */
/* ----------------------------------- */

#define SET_PLLINE      1
#define SET_PLPAGE      2
#define SET_MILINE      3
#define SET_MIPAGE      4
#define SET_SHOW        5       /* very tacky usage below! */
#define SET_BELL        6
#define SET_WINLEFT     7
#define SET_LINE        8
#define SET_NOBELL      9
#define SET_PAGE        10
#define SET_WINRIGHT    11
#define SET_WIDTH       12
#define SET_WIN         13
#define SET_WORDDELIM   14
#define SET_DEBUG       15              /* internal debugging */
#ifdef LMCAUTO
#define SET_LMARG       16
#endif
#ifdef LMCVBELL
#define SET_VBELL       17
#endif
#ifdef LMCSRMFIX
#define SET_RMSTICK     18
#define SET_RMNOSTICK   19
#endif
#define SET_HY          20
#define SET_NOHY        21
#define SET_GRAPHSET    22
#define SET_CHAR7B      23
#define SET_CHAR8B      24
#define SET_UTF8        25
#define SET_INFOLEVEL   26
#define SET_PREF        27

S_looktbl setopttable [] = {
    "?",            SET_SHOW,      /* show options */
    "+line",        SET_PLLINE,    /* defplline */
    "+page",        SET_PLPAGE,    /* defplpage */
    "-line",        SET_MILINE,    /* defmiline */
    "-page",        SET_MIPAGE,    /* defmipage */
    "bell",         SET_BELL,      /* echo \07 */
#ifdef __linux__
    "charset",      SET_GRAPHSET,  /* set char used for window edges */
#endif
#ifndef CHAR7BITS
    "cs7",          SET_CHAR7B,    /* set input 7 bit characters */
    "cs8",          SET_CHAR8B,    /* set input 8 bit characters */
#endif
    "debuglevel",   SET_DEBUG,     /* set the debug level, -debug must be active */
    "graphset",     SET_GRAPHSET,  /* set char used for window edges */
    "hy",           SET_HY,        /* fill: split hyphenated words */
    "info",         SET_INFOLEVEL, /* what to display for "AT" */
    "left",         SET_WINLEFT,   /* deflwin */
    "line",         SET_LINE,      /* defplline and defmiline */
#ifdef LMCAUTO
    "lmargin",      SET_LMARG,     /* left margin */
#endif
    "nobell",       SET_NOBELL,    /* do not echo \07 */
    "nohy",         SET_NOHY,      /* fill: don't split hy-words */
#ifdef LMCSRMFIX
    "nostick",      SET_RMNOSTICK, /* auto scroll past rt edge */
#endif
    "page",         SET_PAGE,      /* defmipage and defplpage */
    "preferences",  SET_PREF,      /* save the preference setting */
    "right",        SET_WINRIGHT,  /* defrwin */
#ifdef LMCAUTO
    "rmargin",      SET_WIDTH,     /* same as width */
#endif
#ifdef LMCSRMFIX
    "stick",        SET_RMSTICK,   /* no auto scroll past rt edge */
#endif
#ifdef __linux__
    "utf8",         SET_UTF8,      /* set reset the utf8 encoding */
#endif
#ifdef LMCVBELL
    "vbell",        SET_VBELL,     /* visible bell */
#endif
    "width",        SET_WIDTH,     /* linewidth */
    "window",       SET_WIN,       /* deflwin and defrwin */
    "worddelimiter",SET_WORDDELIM, /* set word delimiter */
    0        ,  0
};


/* Commands Arguments description and definition structure */
/* ------------------------------------------------------- */

/* The look up table must be also described in lookuptbl_comment []
 *   in file e.pa.c
 */

#define NOTHING_ARG     0       /* nothing */
#define SLOOKUP_ARG     1       /* argument from a lookup table */
#define STRING_ARG      2       /* argument is a string */
#define NUMB_ARG        3       /* argument is a number */
#define NOTHING_OR_ARG  4       /* nothing or argument */
#define COMPLEX_ARG     5       /* complex arguments */

struct argument_desc {
    Flag as_default;
    int arg_type;
    char * comment;     /* specific argument description */
    union {
	struct slook {      /* arg type SLOOKUP_ARG */
	    S_looktbl * table;
	    int default_val;
	} type;
	char * default_str; /* arg type STRING_ARG */
	int default_numb;   /* arg type NUMB_ARG */
    } u;
    int upper_val;          /* upper level arg value, 0 = any thing */
};


#define MAX_NUMB_ARGS 4     /* maxi number of param in descriptor */

static struct _class_cmd {
	int cmd_val;
	int class;
	int arg_nb;
	struct argument_desc arguments [MAX_NUMB_ARGS];
    } class_cmds [] = {
	{ CMDEDIT     , FILE_CMD_CLASS, 1, {
			    {NO,  NOTHING_OR_ARG, " or '?'"},
			    }},
	{ CMDNAME     , FILE_CMD_CLASS, 1 },
	{ CMDTABFILE  , FILE_CMD_CLASS, 1 },
	{ CMD_TABFILE , FILE_CMD_CLASS, 1 },
	{ CMDWINDOW   , FILE_CMD_CLASS, 1, {
			    {NO,  NOTHING_OR_ARG},
			    }},
	{ CMDSAVE     , FILE_CMD_CLASS, 1 },
	{ CMDCD       , DIR_CMD_CLASS , 1, {
			    {NO,  NOTHING_OR_ARG},
			    }},
	{ CMDBKBFILE  , FILE_CMD_CLASS, 1, {
			    {NO,  NOTHING_OR_ARG},
			    }},
	{ CMDHELP     , HELP_CMD_CLASS, 1, {    /* special class */
			    {NO,  SLOOKUP_ARG, "nothing or", {helptable,  0}},
			    }},
	{ CMDEXIT     , ARG_CMD_CLASS,  1, {
			    {YES, SLOOKUP_ARG, NULL, {exittable, 2}}
			    }},
	{ CMDSET      , ARG_CMD_CLASS,  2, {
			    {NO,  SLOOKUP_ARG, NULL, {setopttable,  0}, 0},
			    {YES, SLOOKUP_ARG, NULL, {wordopttable, 1}, SET_WORDDELIM}
			    }},
	{ CMDGOTO     , ARG_CMD_CLASS,  1, {
			    {YES, SLOOKUP_ARG, NULL, {gototable, 0}}
			    }},
	{ CMDFILL     , ARG_CMD_CLASS,  1, {
			    {NO,  SLOOKUP_ARG, NULL, {filltable, 0}}
			    }},
	{ CMDINSERT   , ARG_CMD_CLASS,  1, {
			    {NO,  SLOOKUP_ARG, NULL, {qbuftable, 0}}
			    }},
	{ CMDCOVER    , ARG_CMD_CLASS,  1, {
			    {NO,  SLOOKUP_ARG, NULL, {qbuftable, 0}}
			    }},
	{ CMDUNDERLAY , ARG_CMD_CLASS,  1, {
			    {NO,  SLOOKUP_ARG, NULL, {qbuftable, 0}}
			    }},
	{ CMDOVERLAY  , ARG_CMD_CLASS,  1, {
			    {NO,  SLOOKUP_ARG, NULL, {qbuftable, 0}}
			    }},
	{ CMD_UPDATE  , ARG_CMD_CLASS,  1, {
			    {NO,  SLOOKUP_ARG, "nothing or", {updatetable, 0}}
			    }},
	{ CMDUPDATE   , ARG_CMD_CLASS,  1, {
			    {NO,  SLOOKUP_ARG, "nothing or", {updatetable, 0}}
			    }},
	{ CMDFILE     , ARG_CMD_CLASS,  1, {
			    {NO,  SLOOKUP_ARG, "nothing or", {filetable, 0}}
			    }},
	{ CMDSHELL    , COMPLEX_ARG, 1 },
	{ CMD_SEARCH  , COMPLEX_ARG, 1 },
	{ CMDSEARCH   , COMPLEX_ARG, 1 },
	{ CMD_REPLACE , COMPLEX_ARG, 1 },
	{ CMDREPLACE  , COMPLEX_ARG, 1 },
	{ CMDRANGE    , COMPLEX_ARG, 1 },
	{ CMDOPEN     , COMPLEX_ARG, 1 },
	{ CMDJUST     , COMPLEX_ARG, 1 },
	{ CMDERASE    , COMPLEX_ARG, 1 },
	{ CMDCLOSE    , COMPLEX_ARG, 1 },
	{ CMDCCASE    , COMPLEX_ARG, 1 },
	{ CMDCAPS     , COMPLEX_ARG, 1 },
	{ CMDCALL     , COMPLEX_ARG, 1 },
	{ CMDCENTER   , COMPLEX_ARG, 1 }
    };

#define CLASS_CMDS_NB (sizeof (class_cmds) / sizeof (class_cmds[0]))


void init_all_lookup_tables ()
/* does not return in case of error */
{
    static int comp_alpha_looktb (S_looktbl *obj1, S_looktbl *obj2);
    static Flag done = NO;
    int i, nb, cc;
    struct _lookup_table *ltbl;
    S_looktbl *tbl;

    if ( done ) return;

    for ( i = 0 ; i < (sizeof (all_lookup_tables) / sizeof (all_lookup_tables[0])) ; i++ ) {
	ltbl = &all_lookup_tables [i];
	tbl = ltbl->table;
	nb = 0;
	if ( tbl ) for ( nb = 0 ; tbl[nb].str ; nb++ ) ;
	ltbl->table_sz = nb;
	if ( ltbl->lstruct ) {
	    if ( ltbl->table_sz != ltbl->lstruct->table_sz )
		getout (YES, "Fatal inconsistancy in lookup descriptor for ",
			ltbl->comment);
	    cc = build_lookup_struct (ltbl->lstruct);
	    if ( cc ) getout (YES, "Fatal Error in size of array \"cmd_storage\" in ",
			      ltbl->comment);
	} else if ( tbl && (nb > 0) ) {
	    qsort (tbl, nb, sizeof (S_looktbl[1]),
		   (int (*)(const void *,const void *)) comp_alpha_looktb);
	}
    }
    done = YES;
}

static int expand_cmd_class_para (struct _class_cmd *clscmd, char *cmd_str,
				  char **next_arg_pt)
{
    extern char *getparam (char *str, int *beg_pos, int *end_pos);
    extern int expand_max_keyword_name (int pos, S_looktbl *table, int *val_pt);
    int nb, i, cc, pos1, pos2, val;
    char *next_arg;

    cc = 0;     /* default nothing done */
    if ( ! *next_arg_pt ) return cc;
    nb = clscmd->arg_nb;
    if ( nb <= 0 ) return cc;

    val = 0;    /* match any thing */
    pos2 = *next_arg_pt - cmd_str;
    for ( i = 0 ; i < nb ; i++ ) {
	/* get the next parameter if any */
	pos1 = pos2;
	next_arg = getparam (cmd_str, &pos1, &pos2);
	if ( !next_arg ) return 0;  /* nothing more */

	if ( (val != 0) && (val != clscmd->arguments[i].upper_val) )
	    return 0;
	if ( clscmd->arguments[i].arg_type == SLOOKUP_ARG ) {
	    cc = expand_max_keyword_name (pos1, clscmd->arguments[i].u.type.table, &val);
	    if ( cc != 0 ) return cc;
	}
    }
    return cc;
}

/* build the sorted command table */
/* ------------------------------ */

static int comp_alpha_looktb (S_looktbl *obj1, S_looktbl *obj2)
/* Compare object string value in strick alphabetical order */
{
    char *p1, *p2;
    int cmp;

    p1 = obj1->str;
    p2 = obj2->str;
    cmp = strcmp (p1, p2);
    return (cmp);
}

int cmp_looktb (S_looktbl *obj1, S_looktbl *obj2)
/* Compare object string value for better result for prefixed commands.
 * In used prefixes are '+' '-' '?'
 */
{
    char *p1, *p2;
    int cmp;

    p1 = ( isalpha (obj1->str[0]) ) ? obj1->str : &obj1->str[1];
    p2 = ( isalpha (obj2->str[0]) ) ? obj2->str : &obj2->str[1];
    cmp = strcmp (p1, p2);
    if ( cmp == 0 ) cmp = (obj2->str[0] - obj1->str[0]);
    return (cmp);
}

/* Compares table which are pointer to S_looktbl records.
 */
int cmp_pt_looktb (S_looktbl **obj1, S_looktbl **obj2)
{
    return (cmp_looktb (*obj1, *obj2));
}

int rev_cmp_pt_looktb (S_looktbl **obj1, S_looktbl **obj2)
{
    return (cmp_looktb (*obj2, *obj1));
}

/* build sorted and aliases lookup tables, and init lookup structure description */
/* ----------------------------------------------------------------------------- */
    /* Reference table is sorted in struct alphabetic order,
     * The dimension of the storage must be one more than the
     *   reference table dimension,
     *   need one extra NULL pointer at the end of the aliases.
     * The storage is used for aliases (1st part) and sorted table (2nd).
     * Return 0 on succes, 1 on error in sorted table dimension
     */

int build_lookup_struct (S_lookstruct *lk_struct)
{
    int i, ii, j, k1, k2, val, last, sz;
    S_looktbl *ref_pt, *major_pt;

    if ( ! lk_struct->storage ) return 1;
    sz = (lk_struct->table_sz +2) * sizeof (S_looktbl *);
    if ( lk_struct->sizeof_storage < sz )
	return 1;

    /* check the reference table size */
    for ( i = 0 ;  ; i++ ) if ( ! lk_struct->table[i].str ) break;
    if ( i != lk_struct->table_sz ) lk_struct->table_sz = i;

    /* sort alphabeticaly the reference table */
    qsort (lk_struct->table, lk_struct->table_sz, sizeof (S_looktbl[1]),
	   (int (*)(const void *,const void *)) comp_alpha_looktb);

    /* build the sorted table */
    memset (lk_struct->storage, 0, lk_struct->sizeof_storage);
    k1 = k2 = 0;
    last = (lk_struct->sizeof_storage) / sizeof (S_looktbl *) -2;
    for ( i = lk_struct->table_sz -1 ; i >= 0 ; i-- ) {
	/* the revers order takes the last entry in case of aliases */
	if ( lk_struct->as_alias ) {
	    /* remove aliases */
	    val = lk_struct->table [i].val;
	    for ( j = 0 ; j < k1 ; j++ ) {
		if ( val == (*lk_struct->storage)[last-j]->val ) break;
	    }
	    if ( j == k1 )  /* not yet in sorted table */
		(*lk_struct->storage)[last - k1++] = &lk_struct->table[i];
	    else
		(*lk_struct->storage)[k2++] = &lk_struct->table[i];
	} else {
	    /* do not remove aliases */
	    (*lk_struct->storage)[last - k1++] = &lk_struct->table [i];
	}
    }

    /* sorted table address and size */
    lk_struct->sorted_table = (S_looktbl *(*)[]) &((*lk_struct->storage)[last - k1 +1]);
    lk_struct->sorted_table_sz = k1;

    /* aliases table address and size */
    lk_struct->alias_table = ( lk_struct->as_alias ) ? (S_looktbl *(*)[]) &((*lk_struct->storage)[0]) : NULL;
    lk_struct->alias_table_sz = ( lk_struct->as_alias ) ? k2-1 : 0;

    /* if as_alias, get major names */
    if ( lk_struct->as_alias && lk_struct->major_name_table ) {
	for ( i = lk_struct->major_name_table_sz -1 ; i >= 0 ; i-- ) {
	    ref_pt = &(lk_struct->major_name_table[i]);
	    major_pt = (S_looktbl *) bsearch (ref_pt,
			      lk_struct->table, lk_struct->table_sz, sizeof (S_looktbl[1]),
			      (int (*)(const void *,const void *)) comp_alpha_looktb);
	    if ( major_pt == NULL ) continue; /* not in cmd table */
	    val = lk_struct->major_name_table[i].val;
	    for ( j = lk_struct->alias_table_sz -1 ; j >= 0 ; j-- ) {
		if ( major_pt == (*lk_struct->alias_table)[j] ) break;
	    }
	    if ( j < 0 ) continue;
	    /* found in alias table, swap with the entry in sorted table */
	    for ( ii = lk_struct->sorted_table_sz -1 ; ii >= 0 ; ii-- ) {
		if ( val == (*lk_struct->sorted_table)[ii]->val ) break;
	    }
	    if ( ii < 0 ) continue;
	    ref_pt = (*lk_struct->sorted_table)[ii];
	    (*lk_struct->sorted_table)[ii] = major_pt;
	    (*lk_struct->alias_table)[j]   = ref_pt;
	}
    }

    /* sort the sorted and aliases tables now */
    qsort (lk_struct->sorted_table, lk_struct->sorted_table_sz,
	   sizeof (S_looktbl *),
	   (int (*)(const void *,const void *)) cmp_pt_looktb);
    qsort (lk_struct->alias_table, lk_struct->alias_table_sz,
	   sizeof (S_looktbl *),
	   (int (*)(const void *,const void *)) rev_cmp_pt_looktb);

    return 0;
}

/* get_keyword_strg : get the non abreviated name string */
/* ----------------------------------------------------- */
char * get_keyword_strg (S_looktbl *table, int val)
{
    S_looktbl *(*sorted)[];
    S_lookstruct *tblstruct_pt;
    int i, sz;

    if ( ! table ) return NULL;

    sz = get_sorted_table (table, &sorted, &tblstruct_pt, NULL);
    if ( sz <= 0 ) {
	/* no sorted table */
	for ( i = 0 ; table[i].str ; i++ ) {
	    if ( table[i].val == val ) return table[i].str;
	}
    } else {
	/* have sorted name list */
	for ( i = 0 ; i < sz ; i++ ) {
	    if ( (*sorted)[i]->val == val ) return (*sorted)[i]->str;
	}
    }
    return "";
}

/* command_class : check for command class : type of argument */
/* ---------------------------------------------------------- */
/*
 *  Get the class command and return the next parameter
 *  Return
 *    *next_arg_pt : point to the 2nd parameter (which can be a file name),
 *                   point into param (or paramv) string.
 *    return value =
 *         -1 : param does not start with an editor command
 *              or it is an ambiguous command
 *         -2 : param start with abreviation of a command
 *          0 : not a command with expantion
 *         >0 : see class_cmds
 *  On return, *file_param_pt point to the parameter after the
 *    editor command (if return > 0).
 */

static int ext_command_class (char *param, char **next_arg_pt,
			      int *cmdval_pt, struct _class_cmd **clscmd_pt)
{
    extern char *getparam (char *str, int *beg_pos, int *end_pos);

    int i, idx, cc, pos1, pos2, wlen;
    char ch, *name_expantion, *cmdstr, *fname;
    Short cmdval;

    if ( clscmd_pt ) *clscmd_pt = NULL;
    dir_cmd_flg = 0;
    if ( next_arg_pt ) *next_arg_pt = NULL;

    cmdstr = ( param ) ? param : paramv;

    cc = check_keyword (cmdstr, 0, cmdtable, 0, &pos2,
			&name_expantion, &wlen, &idx, NO);
    if ( cc == -2 ) {
	/* ambiguous command */
	if ( wlen > 0 ) (void) cmd_insert (wlen, name_expantion);
	return -2;
    }
    if ( cc < 0 ) return -1;    /* nothing to do */
    if ( idx == -2 ) return -2; /* ambiguous command */
    if ( idx < 0 ) return -1;   /* command not found */

    /* get the next parameter */
    pos1 = pos2;
    fname = getparam (cmdstr, &pos1, &pos2);
    if ( !fname ) {
	if ( cc == 1 )
	    return -2;   /* abreviation of a command */
    }

    /* get the command class */
    cmdval = cmdtable [idx].val;
    if ( cmdval_pt ) *cmdval_pt = cmdval;
    for ( i = 0 ; class_cmds [i].cmd_val != 0 ; i++ ) {
	if ( class_cmds [i].cmd_val != cmdval ) continue;
	if ( clscmd_pt ) *clscmd_pt = &class_cmds [i];
	if ( next_arg_pt ) *next_arg_pt = fname;
	dir_cmd_flg = (class_cmds [i].class == DIR_CMD_CLASS);
	return class_cmds [i].class;
    }
    return NOT_CMD_CLASS;
}

int command_class (char *param, char **next_arg_pt, int *cmdval_pt)
{
    return (ext_command_class (param, next_arg_pt, cmdval_pt, NULL));
}

/* Process a request for help on argument / parameter value */
/* -------------------------------------------------------- */
/* Display the paramter keyword, or default value
 *  Return : YES if something was displayed
 */

static void simple_arg (char * cmdname, struct _class_cmd * class_cmd_pt,
			char *para_txt)
{
    int nb;
    char txt [256];
    struct argument_desc * arg_desc_pt;

    memset (txt, 0, sizeof (txt));
    sprintf (txt, "Command \"%s\" ", cmdname);
    nb = class_cmd_pt->arg_nb;
    if ( nb > 0 ) {
	arg_desc_pt = &(class_cmd_pt->arguments[0]);
	if ( nb > 1 ) sprintf (&txt[strlen (txt)], "%d parameters : ", nb);
	else strcat (txt, "parameter : ");
	if ( arg_desc_pt->arg_type == NOTHING_OR_ARG ) {
	    strcat (txt, "nothing");
	    strcat (txt, arg_desc_pt->comment ? ", " : " or ");
	}
	if ( para_txt ) strcat (txt, para_txt);
	if ( arg_desc_pt->comment ) strcat (txt, arg_desc_pt->comment);
    } else {
	strcat (txt, "no parameter");
    }
    mesg (ERRALL + 1, txt);
}

static void display_slook_para (int idx, struct argument_desc * arg_desc_pt)
{
    extern void print_sort_table (S_lookstruct *);
    S_looktbl *table, *(*sorted)[];
    S_lookstruct *tblstruct_pt;
    int i, cc, sz, pdval;
    char *comment, *str;

    table = arg_desc_pt->u.type.table;
    comment = NULL;
    sz = get_sorted_table (table, &sorted, &tblstruct_pt, &comment);
    if ( sz <= 0 ) return;

    if ( idx > 0 ) printf ("\n--%d-- %s\n\n", idx +1, comment);
    if ( !tblstruct_pt || !sorted ) return;

    print_sort_table (tblstruct_pt);
    if ( ! arg_desc_pt->as_default ) return;

    pdval = arg_desc_pt->u.type.default_val;
    for ( i = tblstruct_pt->sorted_table_sz -1; i >= 0 ; i-- ) {
	if ( (*sorted)[i]->val == pdval ) break;
    }
    if ( i >= 0 ) {
	str = (*sorted)[i]->str;
	if ( str && *str ) {
	    printf ("\n      >>> Default value for parameter %d : \"%s\" <<<\n",
		    idx +1, (*sorted)[i]->str);
	    return;
	}
    }
    /* default argument is empty */
    printf ("\n      >>> Default value for parameter %d : nothing <<<\n",
	    idx +1);
}

static int help_argu_para ( struct _class_cmd * class_cmd_pt, char *txt)
{
    extern Flag test_ctrlc ();
    extern int call_waitkeyboard (char *, int *);
    extern void show_help_msg ();
    struct argument_desc * arg_desc_pt;
    S_looktbl *table, *(*sorted)[];
    S_lookstruct *tblstruct_pt;
    int i, sz, cc, imax, idx, gk;
    char * comment;

    if ( ! class_cmd_pt ) return CROK;

    imax = class_cmd_pt->arg_nb;
    if ( imax > MAX_NUMB_ARGS ) imax = MAX_NUMB_ARGS;

    if ( txt ) {
	if ( imax > 0 ) {
	    if ( imax > 1 ) strcat (txt, "s");
	    if ( class_cmd_pt->arguments[0].comment ) {
		strcat (txt, " : ");
		strcat (txt, class_cmd_pt->arguments[0].comment);
	    } else if ( class_cmd_pt->arguments[0].as_default ) {
		strcat (txt, " : nothing or");
	    }
	}
	puts (txt); putchar ('\n');
    }
    if ( imax <= 0 ) puts ("\n  No argument for this command\n");
    else {
	if ( imax > 1 ) puts ("--1--\n");
	for ( i = 0 ; i < imax ; i++ ) {
	    arg_desc_pt = &class_cmd_pt->arguments[i];
	    switch ( arg_desc_pt->arg_type ) {
		case SLOOKUP_ARG :  /* argument from a lookup table */
		    display_slook_para (i, arg_desc_pt);
		    break;

		case STRING_ARG :   /* argument is a string */
		    printf ("\n--%d-- argument is a string\n\n", i +1);
		    break;

		case NUMB_ARG :     /* argument is a number */
		    printf ("\n--%d-- argument is a number\n\n", i +1);
		    break;
	    }
	}
    }
    switch (class_cmd_pt->class) {
	case HELP_CMD_CLASS :
	    /* special case for help command */
	    show_help_msg ();
	    break;

	case ARG_CMD_CLASS :
	    /* can display the command descriptor */
	    cc = call_waitkeyboard ("-- Press SPACE for description, ENTER or <Ctrl C> return to the Edit session --",
				    &gk);
	    if ( cc == CROK ) {
		/* Ctrl C : interrupt */
		extern void reset_ctrlc ();
		reset_ctrlc ();
		return CROK;
	    }
	    if ( gk != ' ' ) return cc; /* do not display description */

	    /* space key : display the command description now */
	    for ( idx = cmdstruct.table_sz -1 ; idx >= 0 ; idx-- ) {
		if ( cmdstruct.table[idx].val == class_cmd_pt->cmd_val ) break;
	    }
	    if ( idx < 0 ) return RE_CMD;
	    (*term.tt_clear) ();
	    cc = (* cmdstruct.one_help) (&(cmdstruct.table [idx]), NULL);
	    if ( test_ctrlc () ) return CROK;
	    break;
    }
    cc = call_waitkeyboard (NULL, NULL);
    return cc;
}

static Flag parameter_help (int cmdval, char * cmdname, Cmdret *retval_pt)
{
    extern void reset_ctrlc ();
    extern char * help_cmd_str ();
    static Cmdret call_help (char *);
    int i, cc;
    struct _class_cmd * class_cmd_pt;
    char txt[256];

    reset_ctrlc ();
    sprintf (txt, "\"%s\" Editor command argument", cmdname);
    txt[1] = toupper (txt[1]);

    if ( retval_pt ) *retval_pt = CROK;

    /* get the command descriptor */
    for ( i = CLASS_CMDS_NB ; i >= 0 ; i-- )
	if ( class_cmds [i].cmd_val == cmdval ) break;
    if ( i < 0 ) {
	/* no command descriptor availaible */
	mesg (ERRALL + 2, cmdname, " : No argument, use 'help' key to display the command descriptor");
	if ( retval_pt ) *retval_pt = RE_CMD;
	return YES;
    }

    class_cmd_pt = & class_cmds [i];

    switch ( class_cmd_pt->class ) {
	case ARG_CMD_CLASS :
	    cc = do_fullscreen (help_argu_para, (void *) class_cmd_pt,
				(void *) txt, NULL);
	    if ( retval_pt ) *retval_pt = cc;
	    return YES;

	case FILE_CMD_CLASS :
	    simple_arg (cmdname, class_cmd_pt, "a file name");
	    if ( retval_pt ) *retval_pt = RE_CMD;
	    return YES;

	case DIR_CMD_CLASS :
	    simple_arg (cmdname, class_cmd_pt, "a directory path name");
	    if ( retval_pt ) *retval_pt = RE_CMD;
	    return YES;

	case HELP_CMD_CLASS :
	    cc = do_fullscreen (help_argu_para, (void *) class_cmd_pt,
				(void *) txt, NULL);
	    if ( retval_pt ) *retval_pt = cc;
	    return YES;

	case COMPLEX_ARG :
	    /* display the command descriptor */
	    sprintf (txt, "%s %s", help_cmd_str (), cmdname);
	    cc = command (CMDHELP, txt);
	    if ( retval_pt ) *retval_pt = RE_CMD;
	    return YES;
    }
    return NO;
}

/* Processing of "tab" expension key within a cmd capture */
/* ------------------------------------------------------ */
/* process_tab_in_cmd :
 *  Assume that the command name was already expanded
 *      with "expand_cmd_line_para ()" call.
 *  Return the command class (or error)
 */
int process_tab_in_cmd (Flag cmdflg, int *old_wlen_pt, Flag *done_flg_pt,
			Flag *used_file_flg_pt, Flag *cmd_file_flg_pt,
			Flag *ambiguous_flg_pt)
{
    extern int cmd_insert (int wlen, char *name_expantion);
    extern int cmd_fname_expantion (int wlen, char *name_expantion);
    extern int getccmd (char *strg, int strg_sz, int *pos_pt);

    struct _class_cmd *clscmd;
    int nb, class, cmd_val, nbentr, wlen, dir_flg, cmd_pos;
    char ch;
    char *next_arg, *file_name_expantion;
    char cmd_str [1024];    /* must be large enough */

    /* get a fresh copy of command line */
    (void) getccmd (cmd_str, sizeof(cmd_str), &cmd_pos);

    /* default : not a file or directory command */
    *cmd_file_flg_pt = NO;

    *ambiguous_flg_pt = NO;
    next_arg = NULL;
    class = ext_command_class (cmd_str, &next_arg, &cmd_val, &clscmd);

    if ( class == -1 ) {
	/* not an editor command */
	putchar ('\a'); /* ring bell */
	return -1;
    }
    if ( class == -2 ) {
	/* abreviation of a command */
	/* already expanded by expand_cmd_line_para */
	return -2;
    }

    switch (class) {

	case NOT_CMD_CLASS :
	    /* not a command with word completion, ignore tab */
	    *done_flg_pt = NO;
	    return class;

	case FILE_CMD_CLASS :
	case DIR_CMD_CLASS :
	    /* the command could use a file name para */
	    *used_file_flg_pt = YES;
	    if ( next_arg && (cmd_pos >= (next_arg - cmd_str)) ) {
		ch = cmd_str [cmd_pos];
		/* at a directory separator ? */
		/*   yes, expand the previous token */
		if (ch == '/') cmd_str [cmd_pos] = '\0';
	    }
	    if ( next_arg ) {
		nbentr = expand_file_para (next_arg, &file_name_expantion, &wlen, &dir_flg);
		if ( nbentr > 0 ) {
		    nb = cmd_fname_expantion (wlen, file_name_expantion);
		    fname_para_sz += nb;    /* what was displayed */
		    if ( dir_flg && (nb == 0) ) {
			/* nothing more displayed, display the 1st entry */
			wlen = get_next_file_name (1, &file_name_expantion);
			if ( wlen > 0 ) *old_wlen_pt = cmd_fname_expantion (wlen, file_name_expantion);
		    }
		}
		if ( nbentr > 1 ) {
		    extern void set_ambiguous_file ();
		    putchar ('\a');
		    *ambiguous_flg_pt = YES;
		    set_ambiguous_file ();
		}
		*cmd_file_flg_pt = YES;
	    }
	    *done_flg_pt = NO;
	    return class;

	case HELP_CMD_CLASS :
	    {
		/* can use a editor cmd of key function */
		int cc, pos1, pos2;
		char * kwd;
		char * cmd_str_expantion;

		if ( cmd_val == CMDHELP ) {
		    /* help command, 1st expand the help argument */
		    extern int expand_help (char *strg, int strg_sz, int *pos1);

		    if ( next_arg ) {
			/* more arg */
			pos1 = next_arg - cmd_str;
			cc = expand_help (cmd_str, sizeof(cmd_str), &pos1);
		    } else cc = -3; /* no more argument */
		    if ( cc == -2 ) {
			putchar ('\a'); /* ambiguous */
			*ambiguous_flg_pt = YES;
		    }
		    if ( cc != -1 ) {
			/* nothing more to be done */
			*done_flg_pt = NO;
			next_arg = NULL;
			break;
		    }
		}

		/* try editor command expantion */
		nbentr = check_keyword (next_arg, 0, cmdtable, 1, NULL,
					&cmd_str_expantion, &wlen, NULL, NO);
		if ( nbentr > 0 ) {
		    (void) cmd_insert (wlen, cmd_str_expantion);
		    *ambiguous_flg_pt = NO;
		} else if ( nbentr == -1 ) {
		    /* try key function name expantion */
		    nbentr = check_keyword (next_arg, 0, itsyms, 1, NULL,
					    &cmd_str_expantion, &wlen, NULL, NO);
		    if ( nbentr > 0 ) {
			(void) cmd_insert (wlen, cmd_str_expantion);
			*ambiguous_flg_pt = NO;
		    }
		}
		if ( nbentr == -2 ) {
		    /* ambiguous */
		    if ( wlen > 0 ) (void) cmd_insert (wlen, cmd_str_expantion);
		    *ambiguous_flg_pt = YES;
		}
		*done_flg_pt = NO;
		return class;
	    }

	case ARG_CMD_CLASS :
	    {
		int cc;
		cc = expand_cmd_class_para (clscmd, cmd_str, &next_arg);
		if ( cc == -2 ) *ambiguous_flg_pt = YES;
		else if ( cc < 0 ) putchar ('\a'); /* not found */
		*done_flg_pt = NO;
		return class;
	    }

	default :
	    /* not a command with para expantion or file name */
	    if ( ! cmdflg ) {
		return class;
	    }
	}
    *done_flg_pt = NO;
    return class;
}


/* Expand file name routines */
/* ------------------------- */

void show_namelist () {
    int i, j, k, sz, max_sz;
    int col, max_col, h, height;
    char flname [PATH_MAX+2];

    if ( !namelist || (nb_namelist <= 0) ) {
	puts ("Empty file name list");
	return;
    }

    strcpy (flname, name_exp);
    flname [fname_para_sz] = '\0';
    printf ("%d File names matching \"%s%s\"\n\n", nb_namelist,
	    dir_name ? dir_name : "", flname);
    max_sz = 0;
    for ( i = nb_namelist -1 ; i >= 0 ; i-- ) {
	if ( !namelist[i] ) continue;
	sz = strlen (namelist[i]->d_name);
	if ( sz > max_sz ) max_sz = sz;
    }
    if ( max_sz <= 0 ) max_sz = 1;
    max_sz += 1;    /* room for separator */
    height = term.tt_height -4;
    max_col = ((term.tt_width -2) / max_sz);
    if ( height <= 0 ) height = 1;
    h = (nb_namelist -1) / max_col +1;   /* nb of lines for max columns */
    if ( h < 5 ) h = 5;
    if ( height > h ) height = h;
    col = (nb_namelist -1) / height +1;
    if ( col > max_col ) col = max_col;
    if ( col == 1 ) height = nb_namelist;
    for ( i = 0 ; i < height ; i++ ) {
	for ( j = 0 ; j < col ; j++ ) {
	    k = j * height + i;
	    if ( k >= nb_namelist ) continue;
	    printf ("%*s", -max_sz, namelist ? namelist[k]->d_name : "");
	}
	putchar ('\n');
    }
}

void reset_namelist_idx () {
    namelist_idx = -1;
}

void clear_namelist () {
    int i;

    if ( namelist ) {
	if ( nb_namelist > 0 ) {
	    for ( i = nb_namelist-1 ; i >= 0 ; i-- )
		if ( namelist [i] ) free (namelist [i]);
	}
	free (namelist);
    }
    if ( dir_name ) free (dir_name);
    namelist = (struct dirent **) NULL;
    dir_name = dir_name_fname = NULL;
    nb_namelist = fname_para_sz = dir_name_fname_sz = 0;
    namelist_idx = -2;
}

/* dir_entry_mode : return the entry mode (type) */
/* --------------------------------------------- */

static mode_t dir_entry_mode (char *fname, struct dirent *dent)
{
    int cc;
    mode_t dirmode;
    struct stat stat_buf;


    if ( !(dir_name && dir_name_fname) ) return 0;

#ifdef _DIRENT_HAVE_D_TYPE
    if ( dent && dent->d_type )
	return (DTTOIF (dent->d_type));
#endif

    strncpy (dir_name_fname, fname, dir_name_fname_sz);
    cc = stat (dir_name, &stat_buf);
    memset (dir_name_fname, 0, dir_name_fname_sz);
    dirmode = (cc == 0) ? stat_buf.st_mode : 0;
#ifdef _DIRENT_HAVE_D_TYPE
    if ( dent ) dent->d_type = IFTODT (dirmode);
#endif
    return dirmode;
}

/* dir_entry test for directory name entry */
/* --------------------------------------- */
/* name_exp must have enough free space to be expanded with a '/'
    return 1 if the entry is a directory
*/

static int dir_entry (char *name_ex, struct dirent *dent)
{
    mode_t mode;

    mode = dir_entry_mode (name_ex, dent);
    if ( ! S_ISDIR (mode) ) return 0;
    strcat (name_ex, "/");
    return 1;
}

/* get_next_file_name : get the file name expantion from the selected list */
/* ----------------------------------------------------------------------- */
/*  Return : >= 0 size of the expantion string
	       -1 at the top of the list
	       -2 at the bottom of the list
	       -3 only one name in the list
	       -4 no name in the list
	       -5 nothing selected
*/

int get_next_file_name (int delta, char ** name_expantion_pt)
{
    int idx, sz, flg;
    struct dirent *dent;

    if ( namelist == NULL ) return -5;  /* nothing selected */
    if ( nb_namelist < 2 ) return ((nb_namelist == 0) ? -4 : -3);

    idx = namelist_idx + delta;
    if ( idx < 0 ) return -1;               /* top of the list */
    if ( idx >= nb_namelist ) return -2;    /* bottom of the list */

    namelist_idx = idx;
    dent = namelist[namelist_idx];
    memset (name_exp, 0, sizeof(name_exp));
    strncpy (name_exp, dent->d_name, sizeof(name_exp)-2);
    flg = dir_entry (name_exp, dent);
    *name_expantion_pt = &(name_exp[fname_para_sz]);
    sz = strlen (*name_expantion_pt);
    return sz;
}

/* expand_file_para : extend the file name */
/* --------------------------------------- */

/* static data for direntry_cmp routine */
static char *fname_para;    /* could point to volatile (stack) data ! */

int direntry_cmp (struct dirent *dent)
{
    char *str, chr;
    mode_t mode;

    str = dent->d_name;
    if ( *str == '.' ) {
	chr = str[1];
	if ( (chr == '\0') || (chr == '.' && str[2] == '\0') )
	    return 0;   /* allways remove the . and .. directory entry */
    }

    if ( fname_para_sz > 0 ) {
	if ( strncmp (str, fname_para, fname_para_sz) != 0 )
	return 0;
    }

#ifdef _DIRENT_HAVE_D_TYPE
    dent->d_type = 0;
#endif
    mode = dir_entry_mode (str, dent);

    if ( dir_cmd_flg ) return S_ISDIR (mode);
    return S_ISDIR (mode) || S_ISREG (mode) || S_ISLNK (mode);
}


int expand_file_para (char *file_para, char **name_expantion_pt,
		      int *wlen_pt, int *dir_flg_pt)
{
    extern int alphasort ();
    extern int scandir ();

    int i, j, sz, flg;
    char fp_strg[512];
    char *dir;
    struct dirent *dent;
    char path[PATH_MAX+2];
    struct stat stat_buf;
    int (*dir_select)();

    *name_expantion_pt = NULL;
    *wlen_pt = *dir_flg_pt = flg = 0;
    if ( !file_para ) return 0;

    clear_namelist ();

    memset (fp_strg, '\0', sizeof(fp_strg));
    strncpy (fp_strg, file_para, sizeof(fp_strg)-1);
    dir = fname_para = fp_strg;
    fname_para = strrchr (fp_strg, '/');
    if ( fname_para ) {
	*fname_para = '\0';
	fname_para++;
    } else {
	dir = ".";
	fname_para = fp_strg;
    }

    /* save the directory path name */
    sz = strlen(dir)+2;
    dir_name_fname_sz = PATH_MAX;
    /* alloc enough space for a full file name path */
    dir_name = (char *) calloc (sz + dir_name_fname_sz +1, sizeof(char));
    if ( dir_name ) {
	strcpy (dir_name, dir);
	strcat (dir_name, "/");
	/* save the position of file name */
	dir_name_fname = &(dir_name[strlen (dir_name)]);
    } else dir_name_fname_sz = 0;

    if ( dir[0] == '\0' ) dir = "/";

    fname_para_sz = strlen (fname_para);
    /*
    dir_select = ( dir_cmd_flg || (fname_para_sz != 0) ) ? direntry_cmp : (int(*)()) NULL;
    */
    dir_select = direntry_cmp;
    nb_namelist = scandir (dir, &namelist, dir_select, alphasort);
    namelist_idx = -1;
    /* to debug
    {
	char info_str [16];
	sprintf (info_str, "nb %d", nb_namelist);
	rand_info (7, 5, info_str);
	printf (" '%s / %s' : dflg %d %d %d", dir, fname_para, dir_cmd_flg, fname_para_sz, nb_namelist); fflush(stdout); sleep(5);
    }
    */
    if ( nb_namelist > 0 ) {
	memset (name_exp, 0, sizeof(name_exp));
	dent = namelist[0];
	strncpy (name_exp, dent->d_name, sizeof(name_exp)-2);
	if ( nb_namelist == 1 ) {
	    /* a single file name, check for directory */
	    flg = dir_entry (name_exp, dent);
	    sz = 0;
	} else {
	    /* more than one file name, get the larger common substring */
	    sz = strlen (name_exp);
	    for ( i = 1 ; i < nb_namelist ; i++ ) {
		dent = namelist[i];
		for ( j = fname_para_sz ; j < sz ; j++ ) {
		    if ( dent->d_name[j] == name_exp[j] ) continue;
		    name_exp[j] = '\0';
		    sz = strlen (name_exp);
		    break;
		}
		if ( sz == 0 ) break;
	    }
	}
	*name_expantion_pt = &(name_exp[fname_para_sz]);
	*wlen_pt = strlen (*name_expantion_pt);
	*dir_flg_pt = (fname_para_sz == 0); /* evry thing in the directory */
    }
    return nb_namelist;
}

#if 0
void incr_fname_para_sz (int nb)
{
    fname_para_sz += nb;    /* what was displayed */
}
#endif


#ifdef LMCHELP
static Cmdret call_help (char * helpcmd)
{
    Cmdret retval;
    char helparg [256];

    memset (helparg, 0, sizeof(helparg));
    strncpy (helparg, helpcmd, sizeof(helparg) -1);
    if (*term.tt_help == NULL) retval = help_std (helparg);
    else retval = (*term.tt_help) (helparg);
    return retval;
}
#endif


static msg_unknown (char *strg)
{
    extern void set_non_existing_cmd ();
    if ( !strg || !*strg ) return;
    mesg (ERRALL + 3, "\"", strg, "\" Unknown editor command.");
    set_non_existing_cmd ();
}

static msg_ambiguous (char *strg)
{
    extern char ambiguous_err_str [];
    if ( strg ) mesg (ERRALL + 4, "\"", strg, "\" : ", ambiguous_err_str);
}

#ifdef COMMENT
Cmdret
command (forcecmd, forceopt)
    int forcecmd;
    char *forceopt;
.
    Parses a command line and dispatches on the command.
    If a routine that was called to execute a command returns a negative
    value, then the error message is printed out here, and returns CROK.
    Else returns the completion status of the command.

    If forcecmd is != 0 the command value and parameters are taken from
	the command parameters, else theu are extracted from the
	command input line string.
	If LMCCMDS (function key extensions compilation mark) is not
	defined, the parameters are always comming from the command line
	string
#endif
Cmdret
command (forcecmd, forceopt)
    int forcecmd;
    char *forceopt;
{
    extern void cmds_prompt_mesg ();
    extern Cmdret displayfileslist ();
    extern Cmdret buildkbfile (char *);
    extern Cmdret buildkbmap ();
    extern void infotick ();
    extern void marktick (Flag set);
    extern void dostop ();

    Short cmdtblind;
    Cmdret retval, retval_copy;
    Short cmdval;
    char *cmdstr;
    int clrcnt;                 /* Added 10/18/82 MAB */
    Flag done_flg;
    Flag errmsg_flg;
    /* saved no re-entrant variables (e.x.c) */
    char *sv_cmdname;
    char *sv_cmdopstr;
    char *sv_opstr;
    char *sv_nxtop;
    int value;

#ifdef LMCHELP
    extern S_looktbl helptable [];
    int idx;
#endif

#ifndef LMCCMDS      /* function key extensions */
    forcecmd = 0;
#endif

    done_flg = NO;

    /* save previous value (for recursive call) */
    sv_cmdname  = cmdname;
    sv_cmdopstr = cmdopstr;
    sv_opstr    = opstr;
    sv_nxtop    = nxtop;

    if ( forcecmd != 0 ) {
	/* a command value is provided */
	cmdval = forcecmd;
	for ( cmdtblind = CMDTB_SZ -2 ; cmdtblind >= 0 ; cmdtblind-- )
	    if ( cmdval == cmdtable[cmdtblind].val ) break;
	if ( cmdtblind < 0 ) {
	    /* bad call, nothing can be done */
	    /* restaure previous command parameters */
	    cmdname  = sv_cmdname;
	    cmdopstr = sv_cmdopstr;
	    opstr    = sv_opstr;
	    nxtop    = sv_nxtop;
	    return CROK;
	}
	nxtop = forceopt;
    } else {
	/* get the command from the command input line */
	/* must no be called recursively */
	nxtop = paramv;
	cmdstr = getword (&nxtop);
	if (cmdstr[0] == '\0') {
	    /* restaure previous command parameters */
	    cmdname  = sv_cmdname;
	    cmdopstr = sv_cmdopstr;
	    opstr    = sv_opstr;
	    nxtop    = sv_nxtop;
	    return CROK;
	}
	cmdtblind = lookup (cmdstr, cmdtable);
#ifndef LMCCMDS
	if (cmdtblind == -1) {
	    msg_unknown (cmdstr);
	    sfree (cmdstr);
	    /* restaure previous command parameters */
	    cmdname  = sv_cmdname;
	    cmdopstr = sv_cmdopstr;
	    opstr    = sv_opstr;
	    nxtop    = sv_nxtop;
	    return RE_CMD;
	}
#else /* LMCCMDS defined */
	if ( (cmdtblind < 0) && (strcmp (cmdstr, "!") == 0) ) {
	    /* execute "help allcommands" */
	    extern char * help_ambiguous_str (Flag);
	    retval = call_help (help_ambiguous_str (NO));
	    sfree (cmdstr);
	    /* restaure previous command parameters */
	    cmdname  = sv_cmdname;
	    cmdopstr = sv_cmdopstr;
	    opstr    = sv_opstr;
	    nxtop    = sv_nxtop;
	    return retval;
	}

	if (cmdtblind == -1) {
#ifdef LMCGO
	    if (cmdstr[0] >= '1' && cmdstr[0] <= '9') {
		/* implicite goto command */
		nxtop = paramv;
		cmdtblind = lookup ("goto", cmdtable);
	    } else {
#endif /* LMCGO */
#ifdef LMCHELP
		idx = lookup (cmdstr, helptable);
		/* implicite help command */
		if ( idx >= 0 ) {
		    nxtop = paramv;
		    cmdtblind = lookup ("help", cmdtable);
		} else {
#endif /* LMCHELP */
		    msg_unknown (cmdstr);
		    sfree (cmdstr);
		    /* restaure previous command parameters */
		    cmdname  = sv_cmdname;
		    cmdopstr = sv_cmdopstr;
		    opstr    = sv_opstr;
		    nxtop    = sv_nxtop;
		    return RE_CMD;
#ifdef LMCHELP
		}
#endif /* LMCHELP */
#ifdef LMCGO
	    }
#endif /* LMCGO */
	}
#endif /* LMCCMDS */
	else if (cmdtblind == -2) {
	    msg_ambiguous (cmdstr);
	    sfree (cmdstr);
	    /* restaure previous command parameters */
	    cmdname  = sv_cmdname;
	    cmdopstr = sv_cmdopstr;
	    opstr    = sv_opstr;
	    nxtop    = sv_nxtop;
	    return RE_CMD;
	}
	sfree (cmdstr);
    }

    cmdopstr = nxtop;
    cmdname = cmdtable[cmdtblind].str;
    cmdval = cmdtable[cmdtblind].val;
    opstr = getword (&nxtop);

    if ( opstr && *opstr && (strcmp (opstr, "!") == 0) ) {
	/* can be a request for parameter help */
	done_flg = parameter_help (cmdval, cmdname, &retval);
    }

    /* process the command, if it was not a request for param help */
    if ( !done_flg ) switch (cmdval) {

#ifdef CMDVERALLOC
	case CMDVERALLOC:
	    veralloc ();
	    retval = CROK;
	    break;
#endif

	case CMDRANGE:
	case CMD_RANGE:
	case CMDQRANGE:
	    retval = rangecmd (cmdval);
	    break;

	case CMDTRACK:
#ifdef LMCTRAK
	    if ((curwin->winflgs & TRACKSET) == 0)
		curwin->winflgs |= TRACKSET;
	    else
		curwin->winflgs &= ~TRACKSET;
	    infotrack (TRACKSET & curwin->winflgs);
#else
	    curwin->winflgs |= TRACKSET;
	    infotrack (YES);
#endif
	    retval = CROK;
	    break;

	case CMD_TRACK:
	    curwin->winflgs &= ~TRACKSET;
	    infotrack (NO);
	    retval = CROK;
	    break;

#ifdef SIGTSTP  /* 4bsd vmunix */
	case CMDSTOP:
	    dostop ();
	    retval = CROK;
	    break;
#endif

	case CMDUPDATE:
	case CMD_UPDATE:
	    retval = doupdate (cmdval == CMDUPDATE);
	    break;

	case CMDTAB:
	    retval = dotab (YES);
	    break;

	case CMD_TAB:
	    retval = dotab (NO);
	    break;

	case CMDTABS:
	    retval = dotabs (YES);
	    break;

	case CMD_TABS:
	    retval = dotabs (NO);
	    break;

	case CMDTABFILE:
	    retval = tabfile (YES);
	    break;

	case CMD_TABFILE:
	    retval = tabfile (NO);
	    break;

	case CMDREPLACE:
	case CMD_REPLACE:
	    if (!okwrite ()) {
		retval = NOWRITERR;
		break;
	    }
	    retval = replace (cmdval == CMDREPLACE? 1: -1);
	    break;

	case CMDPATTERN:            /* Added at CS/Purdue 10/3/82 MAB */
	case CMD_PATTERN:
	    if (*cmdopstr)
		retval = CRTOOMANYARGS;
	    else if (patmode && cmdval == CMDPATTERN){
		if ( ! get_preferences_flg )
		    mesg (ERRALL+1, "You are in RE mode");
		retval = CROK;
	    }
	    else if (!patmode && cmdval == CMD_PATTERN){
		if ( ! get_preferences_flg )
		    mesg (ERRALL+1, "You are not in RE mode");
		retval = CROK;
	    }
	    else{
		    tglpatmode();
		    clrcnt = 0;
		    if (searchkey) {
			sfree(searchkey);
			searchkey = (char *) 0;
			clrcnt++;
		    }
		    clrcnt += zaprpls();
		    retval = CROK;
	    }
	    break;

	case CMDNAME:
	    retval = name ();
	    break;

	case CMDDELETE:
	    retval = delete ();
	    break;

	case CMDCOMMAND:
	    cmds_prompt_mesg ();
	case CMD_COMMAND:
	    cmdmode = cmdval == CMDCOMMAND ? YES : NO;
	    retval = CROK;
	    break;

	case CMDPICK:
	case CMDCLOSE:
	case CMDERASE:
	case CMDOPEN:
	case CMDBOX:
#ifdef LMCCASE
	case CMDCAPS:
	case CMDCCASE:
#endif
	    retval = areacmd (cmdval - CMDPICK);
	    break;

	case CMDCOVER:
	case CMDOVERLAY:
	case CMDUNDERLAY:
	case CMDBLOT:
	case CMD_BLOT:
	case CMDINSERT:
	    retval = insert (cmdval - CMDCOVER);
	    break;

	case CMD_PICK:
	case CMD_CLOSE:
	case CMD_ERASE:
	    retval = insbuf (cmdval - CMD_PICK + QPICK);
	    break;

	case CMDCALL:
	    retval = call ();
	    /* if the syntax of the shell command was correct and all of the
	       saves went OK, forkshell will never return. */
	    break;

	case CMDSHELL:
	    retval = shell ();
	    /* if the syntax of the shell command was correct and all of the
	       saves went OK, forkshell will never return. */
	    break;

	case CMDLOGOUT:
	    if (!loginflg) {
		mesg (ERRALL + 1, "This is not your login program.  Use \"exit\".");
		break;
	    }
	case CMDEXIT:
	    retval = eexit ();
	    /* if the syntax of the exit command was correct and all of the
	       saves went OK, 'eexit' will never return. */
	    break;

#ifdef LMCHELP
	case CMDSTATUS:
	    retval = call_help ("status");
	    break;

	case CMDHELP:
	    retval = call_help (opstr);
	    break;
#endif

	case CMDCHECKSCR:
	    {
		extern void check_keyboard ();
		extern int set_crlf ();
		extern void reset_crlf ();
		extern void getConsoleSize ();
		int oflag;
		int col, lin, x, y;
		char * strg;

		col = cursorcol; lin = cursorline;
		savecurs ();
		( *term.tt_clear ) ();
		( *term.tt_home ) ();
		poscursor (0, term.tt_height -1);
		oflag = set_crlf ();

		strg = termtype ? "" : " (use termcap or terminfo)";
		printf ("Terminal : %s%s, keyboard : %s (%d)\n",
			tname, strg, kname, kbdtype);
		getConsoleSize (&x, &y);
		printf ("Current screen size %d lines of %d columns\n", y, x);

		check_keyboard (NO);

		reset_crlf (oflag);
		mesg (TELALL+1, " ");
		fflush (stdout);
		fresh ();
		restcurs ();
		retval = CROK;
		break;
	    }

	case CMDREDRAW:
	    fresh ();
	    refresh_info ();
	    retval = CROK;
	    break;

	case CMDSPLIT:
	case CMDJOIN:
	    if (*cmdopstr)
		retval = CRTOOMANYARGS;
	    else
		retval = cmdval == CMDJOIN ? join () : split ();
	    break;

	case CMDRUN:
	case CMDFEED:
	    if (!okwrite ()) {
		retval = NOWRITERR;
		break;
	    }
	    retval = run (cmdopstr, cmdval);
	    break;

	case CMDFILL:
	    retval = filter (FILLNAMEINDEX, YES);
	    break;

	case CMDJUST:
	    retval = filter (JUSTNAMEINDEX, YES);
	    break;

	case CMDCENTER:
	    retval = filter (CENTERNAMEINDEX, YES);
	    break;

	case CMDSAVE:
	    if (curmark) {
		retval = NOMARKERR;
		break;
	    }
	    retval = save ();
	    break;

	case CMDEDIT:
	    if (curmark) {
		retval = NOMARKERR;
		break;
	    }
	    retval = edit ();
	    break;

	case CMDWINDOW:
	    if (curmark) {
		retval = NOMARKERR;
		break;
	    }
	    if (*opstr == '\0')
		makewindow ((char *)0);
	    else {
		if (*nxtop) {
		    retval = CRTOOMANYARGS;
		    break;
		}
		makewindow (opstr);
	    }
	    loopflags.bullet = YES;
	    retval = CROK;
	    break;

	case CMD_WINDOW:
	    if (curmark) {
		retval = NOMARKERR;
		break;
	    }
	    if (*opstr) {
		retval = CRTOOMANYARGS;
		break;
	    }
	    removewindow ();
	    retval = CROK;
	    break;

	case CMDRESIZE:
	    resize_screen ();
	    break;

	case CMDGOTO:
	    retval = gotocmd ();
	    break;

#ifdef LMCAUTO
	case CMDAUTO:
	    retval = parsauto (NO);
	    break;

	case CMD_AUTO:
	    retval = parsauto (YES);
	    break;
#endif

#ifdef LMCDWORD
	case CMDDWORD:
	    retval = dodword (YES);
	    break;

	case CMD_DWORD:
	    retval = dodword (NO);
	    break;
#endif

	case CMDCD:
	    {
		extern Cmdret dochangedirectory ();
		retval = dochangedirectory ();
		break;
	    }

	case CMDVERSION:
	    Block {
		extern char verstr[];
		char strg [128];
		strncpy (strg, verstr, sizeof (strg));
		strg [sizeof (strg) -1] = '\0';
		if ( strg [strlen (strg) -1] == '\n' )
		    strg [strlen (strg) -1] = '\0';
		mesg (TELALL + 1, strg);
		loopflags.hold = YES;
		retval = CROK;
		break;
	    }

	case CMDSTATS:          /* pwd command */
	    Block {
		int sz;
		char pwdname[PATH_MAX +16];
		char *sp;

		strcpy (pwdname, "CWD = ");
		sz = strlen (pwdname);
		sp = getcwd (&pwdname[sz], sizeof (pwdname) - sz);
		if ( ! sp ) sp = "./";
		sz = strlen (pwdname);
		if ( sz >= term.tt_width -2 ) pwdname [term.tt_width -2] = '\0';
		mesg (TELALL + 1, pwdname);
		loopflags.hold = YES;
		retval = CROK;
	    }
	    break;

	case CMDQFILE:      /* current file status query */
	    retval = fileStatus (YES);
	    break;
	case CMDFILE:       /* current file status set / query */
	    retval = fileStatus (NO);
	    break;

	case CMDSHFILES:    /* display currently edited files list */
	    retval = displayfileslist ();
	    break;

	case CMDBKBFILE:    /* build a kb file */
	    retval = buildkbfile (opstr);
	    break;

	case CMDBKBMAP:    /* interactively build a keyboard mapping file */
	    retval = buildkbmap (NO);
	    break;

	case CMDSET:
	    retval = setoption (NO);
	    break;

	case CMDQSET:
	    retval = setoption (YES);
	    break;

#ifdef NOTYET
	case CMD_DIFF:
	    retval = diff (-1);
	    break;

	case CMDDIFF:
	    retval = diff (1);
	    break;
#endif

	case CMDSEARCH:
	case CMD_SEARCH:
	    if ( *cmdopstr ) {
		if ( patmode ){
		    extern Flag check_pattern ();
		    if ( ! check_pattern (cmdopstr) ) {
			retval = CROK;
			break;
		    }
		}
		if (searchkey) sfree (searchkey);
		searchkey = append (cmdopstr, "");
	    }
	    dosearch (cmdval == CMDSEARCH ? 1 : -1);
	    retval = CROK;
	    break;

	case CMDFLIPBKAR:
	    /* flip the function of BackArrow key between "del" and "dchar"
	     *  assumed the BackArrow key generate the <del> (0177) ASCII char
	     */
	    value = (cmdopstr && *cmdopstr) ? atoi (cmdopstr) : 0;
	    (void) itswapdeldchar (del_strg, value, NULL);
	    retval = CROK;
	    break;

	case CMDMARK:
	    mark ();
	    retval = CROK;
	    break;

	case CMD_MARK:
	    unmark ();
	    retval = CROK;
	    break;

	case CMDTICK:
	    if ( opstr[0] == '?' ) infotick ();
	    else marktick (YES);
	    retval = CROK;
	    break;

	case CMD_TICK:
	    marktick (NO);
	    retval = CROK;
	    break;

	case CMDQTICK:
	    infotick ();
	    retval = CROK;
	    break;

	default:
	    mesg (ERRALL + 3, "Command \"", cmdtable[cmdtblind].str,
		    "\" not implemented yet");
	    retval = CROK;
	    break;
    }
    /* end of cmdtblind switch and !done_flg */
    done_flg = NO;

    if ( opstr && *opstr )
	sfree (opstr);

    if ( (retval >= CROK) || (retval == RE_CMD) ) {
	/* succesfull command proocessing */
	if ( command_msg ) {
	    mesg (TELALL +1, command_msg);
	    loopflags.hold = YES;
	}
	/* restaure previous command parameters */
	cmdname  = sv_cmdname;
	cmdopstr = sv_cmdopstr;
	opstr    = sv_opstr;
	nxtop    = sv_nxtop;
	return retval;
    }

    /* error return from command processing */
    errmsg_flg = YES;
    retval_copy = retval;
    retval = RE_CMD;  /* default */

    switch (retval_copy) {
    case CRUNRECARG:
	mesg (1, " unrecognized argument to ");
	if ( cmdopstr && *cmdopstr ) {
	    /* test for a '!' : parameter descriptor request */
	    char * str;
	    str = &cmdopstr[strlen(cmdopstr) -1];
	    for ( ; *str == ' ' ; str-- ) ;
	    if ( (*str == '!') && (*(str -1) == ' ') ) {
		mesg (ERRDONE + 3, "\"", cmdname, "\"");
		(void) parameter_help (cmdval, cmdname, &retval);
		errmsg_flg = NO;
	    }
	}
	break;

    case CRAMBIGARG:
	/* can be "ambiguous" return from lookup */
	mesg (1, " ambiguous argument to ");
	break;

    case CRTOOMANYARGS:
	mesg (ERRSTRT + 1, "Too many of arguments to ");
	break;

    case CRNEEDARG:
	mesg (ERRSTRT + 1, "Need an argument to ");
	break;

    case CRNOVALUE:
	mesg (ERRSTRT + 1, "No value for option to ");
	break;

    case CRMULTARG:
	mesg (ERRSTRT + 1, "Duplicate arguments to ");
	break;

    case CRMARKCNFL:
	errmsg_flg = NO;
	retval = NOMARKERR;
	break;

    case CRBADARG:

    default:
	mesg (ERRSTRT + 1, "Bad argument(s) to ");
    }

    if ( errmsg_flg ) mesg (ERRDONE + 3, "\"", cmdname, "\"");
    /* restaure previous command parameters */
    cmdname  = sv_cmdname;
    cmdopstr = sv_cmdopstr;
    opstr    = sv_opstr;
    nxtop    = sv_nxtop;
    return retval;
}

/* Goto command processing */
/* ----------------------- */

S_looktbl gototable [] = {
    "b",            0,  /* guranteed abbreviation */
    "beginning",    0,
    "e",            1,  /* guranteed abbreviation */
    "end",          1,
    "p",            5,
    "prev",         5,
    "rb",           2,  /* guranteed abbreviation */
    "rbeginning",   2,
    "re",           3,  /* guranteed abbreviation */
    "rend",         3,
    "t",            4,
    "tick",         4,
     0,             0,
};



#ifdef COMMENT
Cmdret
gotocmd ()
.
    Do the "goto" command.
#endif
Cmdret
gotocmd ()
{
    extern void gototick ();
    extern void savemark (struct markenv *);
    extern Small move2mark (struct markenv *, Flag);
    extern void copymark (struct markenv *, struct markenv *);

    struct markenv tmpmk;
    Nlines lnb, dlnb;

    savemark (&tmpmk);

    if (opstr[0] == '\0') {
	/* default : goto top of file */
	gotomvwin ((Nlines) 0);
	copymark (&tmpmk, &curwksp->wkpos);
	return CROK;
    }
    if (*nxtop)
	return CRTOOMANYARGS;

    Block {
	Short tmp;
	char ch;
	char *cp, *cp1;

	for (cp1 = opstr; *cp1 && *cp1 == ' '; cp1++) continue;
	ch = (cp1) ? *cp1 : '\0';
	if ( (ch == '+') || (ch == '-') ) cp1++;
	tmp = getpartype (&cp1, 0, 0, 0);
	if (tmp == 1) {
	    for (cp = cp1; *cp && *cp == ' '; cp++)
		continue;
	    if (*cp == 0) {
		lnb = parmlines - 1;
		if ( ch == '-' ) lnb = curwksp->wlin + cursorline - parmlines;
		if ( ch == '+' ) lnb = curwksp->wlin + cursorline + parmlines;
		dlnb = lnb - curwksp->wlin;
		if ( (dlnb < 0) || (dlnb > curwin->btext) ) gotomvwin (lnb);
		else poscursor (cursorcol, dlnb);
		copymark (&tmpmk, &curwksp->wkpos);
		return CROK;
	    }
	}
	else if (tmp == 2)
	    return CRBADARG;
    }
    Block {
	Small ind;
	Small val;

	if ((ind = lookup (opstr, gototable)) < 0) {
	    if ( ind == -1 ) {
		/* check for "goto ?" */
		extern Cmdret help_cmd_arguments (char *str, S_looktbl *table);
		(void) help_cmd_arguments (opstr, gototable);
	    }
	    mesg (ERRSTRT + 1, opstr);
	    return (ind == -2) ? CRAMBIGARG : CRUNRECARG;
	}

	switch (val = gototable[ind].val) {
	case 0:
	    /* beginning */
	    gotomvwin ((Nlines) 0);
	    break;

	case 1:
	    /* end */
	    gotomvwin (la_lsize (curlas));
	    break;

	case 2:
	    /* rbeginning */
	case 3:
	    /* rend */
	    if (curwksp->brnglas)
		gotomvwin (la_lseek (val == 2
				     ? curwksp->brnglas : curwksp->ernglas,
				     0, 1));
	    else
		return NORANGERR;
	    break;

	case 4:
	    /* tick */
	    gototick ();
	    break;

	case 5:
	    /* previous */
	    (void) move2mark (&curwksp->wkpos, YES);
	    break;
	}
	copymark (&tmpmk, &curwksp->wkpos);
    }
    return CROK;
}

/* 'update' processing */

S_looktbl updatetable [] = {
    "-inplace", 0,
    "inplace",  1,
    0        ,  0
};

#ifdef COMMENT
Cmdret
doupdate (on)
    Flag on;
.
    Do the "update" command.
    The 'on' argument is non-0 for "update" and 0 for "-update"
#endif
Cmdret
doupdate (on)
Flag on;
{
    int idx, ind;

    if (*nxtop || !on && opstr[0] != '\0')
	return CRTOOMANYARGS;

    if (on && !(fileflags[curfile] & DWRITEABLE)) {
	mesg (ERRALL + 1, "Don't have directory write permission");
	return CROK;
    }
    if (opstr[0] != '\0') {
	idx = lookup (opstr, updatetable);
	if (idx == -1  || idx == -2) {
	    mesg (ERRSTRT + 1, opstr);
	    return idx;
	}
	ind = updatetable [idx].val;
	/* at this point, ind can be 0 = -inplace or 1 = inplace */
	if (ind) { /* inplace */
	    if (fileflags[curfile] & NEW) {
		mesg (ERRALL + 1, "\"inplace\" is useless;  file is new");
		return CROK;
	    }
	    if (!(fileflags[curfile] & FWRITEABLE)) {
		mesg (ERRALL + 1, "Don't have file write permission");
		return CROK;
	    }
	    fileflags[curfile] |= INPLACE | CANMODIFY;
	}
	else /* -inplace */
	    fileflags[curfile] &= ~INPLACE;
    }
    if (on)
	fileflags[curfile] |= UPDATE;
    else
	fileflags[curfile] &= ~UPDATE;
    return CROK;
}

/* setoption_msg : build current options state message */
/* --------------------------------------------------- */
void setoption_msg (char *buf)
	/* buf must be >= 128 char */
{
    char * buf_pt;

#ifndef LMCVBELL
    sprintf (buf, "+li %d, -li %d, +pg %d, -pg %d, wr %d, wl %d, wid %d, bell %s, hy %s, %s",
	     defplline, defmiline, defplpage, defmipage, defrwin, deflwin,
	     linewidth, NoBell ? "off" : "on", fill_hyphenate ? "on" : "off",
	     inputcharis7bits ? "cs7" : "cs8");
#else
    sprintf (buf, "+li %d, -li %d, +pg %d, -pg %d, wr %d, wl %d, wid %d, bell %s, vb %s, hy %s",
	     defplline, defmiline, defplpage, defmipage, defrwin, deflwin,
	     linewidth, NoBell ? "off" : "on", VBell ? "on" : "off",
	     fill_hyphenate ? "on" : "off",
	     inputcharis7bits ? "cs7" : "cs8");
#endif
    buf_pt = & buf [strlen (buf)];
    if ( DebugVal == 0 ) strcpy (buf_pt, ", debug off");
    else sprintf (buf_pt, ", debug %d", DebugVal);
}

#ifdef COMMENT
Cmdret
setoption( showflag )
.
	Set/show various options
#endif

Cmdret
setoption ( showflag )
  int showflag;
{
    extern void set_info_level (int level);
    extern void fresh ();
    extern Flag set_reset_utf8 (Flag);
    extern Flag set_reset_graph (Flag);
    extern char * get_debug_name ();
    static void save_preferences ();
    extern int open_dbgfile (Flag append_flg);
    extern int get_debug_default_level ();

    int cc;
    char *ternm, *dbgfname;
    char *arg;
    Small ind;
    Small value;
    Cmdret retval;

    arg = (char *)NULL;
    if (showflag) {
	for ( ind = 0 ; setopttable [ind].str ; ind++ ) {
	    if ( setopttable [ind].val == SET_SHOW ) break;
	}
    }
    else {
	if ( !opstr[0] ) {
	    void set_ambiguous_param (S_looktbl *, char *, Flag);
	    set_ambiguous_param (setopttable, NULL, NO);
	    return CRNEEDARG;
	}

	ind = lookup (opstr, setopttable);
	if ( ind == -1 ) {
	    /* check for "set ?" */
	    extern Cmdret help_cmd_arguments (char *str, S_looktbl *table);
	    retval = help_cmd_arguments (opstr, setopttable);
	    if ( retval != CRUNRECARG )
		return retval;
	}

	if (ind == -1 || ind == -2) {
	    mesg (ERRSTRT + 1, opstr);
	    return ind;
	}

	arg = getword (&nxtop);
	if ( !arg ) {   /* ???? in which case ???? */
	    cmdname = cmdopstr;
	    return CRNEEDARG;
	}
    }

    retval = CROK;
    switch( setopttable[ind].val ) {

	case SET_SHOW:
	    {   char buf[128];
		setoption_msg (buf);
		mesg (TELALL + 1, buf);
	    }
	    loopflags.hold = YES;
	    retval = CROK;
	    break;

	case SET_PLLINE:
	    if ((value = abs( atoi (arg))) == 0)
		goto BadVal;
	    defplline = value;
	    defplline_flg = YES;
	    curwin->plline = defplline;
	    curwin->winflgs |= PLINESET;
	    retval = CROK;
	    break;

	case SET_MILINE:
	    if ((value = abs( atoi (arg))) == 0)
		goto BadVal;
	    defmiline = value;
	    defmiline_flg = YES;
	    curwin->miline = defmiline;
	    curwin->winflgs |= MLINESET;
	    retval = CROK;
	    break;

	case SET_LINE:
	    if ((value = abs( atoi (arg))) == 0)
		goto BadVal;
	    defplline = defmiline = value;
	    curwin->plline = curwin->miline = defplline;
	    defplline_flg = defmiline_flg = YES;
	    curwin->winflgs |= PLINESET;
	    curwin->winflgs |= MLINESET;
	    retval = CROK;
	    break;

	case SET_MIPAGE:
	    if ((value = abs( atoi (arg))) == 0)
		goto BadVal;
	    defmipage = value;
	    defmipage_flg = YES;
	    curwin->mipage = defmipage;
	    retval = CROK;
	    break;

	case SET_PLPAGE:
	    if ((value = abs( atoi (arg))) == 0)
		goto BadVal;
	    defplpage = value;
	    defplpage_flg = YES;
	    curwin->plpage = defplpage;
	    retval = CROK;
	    break;

	case SET_PAGE:
	    if ((value = abs( atoi (arg))) == 0)
		goto BadVal;
	    defplpage = defmipage = value;
	    defplpage_flg = defmipage_flg = YES;
	    curwin->plpage = curwin->mipage = defplpage;
	    retval = CROK;
	    break;

	case SET_WINLEFT:
	    if ((value = abs( atoi (arg))) == 0)
		goto BadVal;
	    deflwin = value;
	    deflwin_flg = YES;
	    curwin->lwin = deflwin;
	    retval = CROK;
	    break;

	case SET_WIN:
	    if ((value = abs( atoi (arg))) == 0)
		goto BadVal;
	    deflwin = defrwin = value;
	    deflwin_flg = defrwin_flg = YES;
	    curwin->lwin = curwin->rwin = defrwin;
	    retval = CROK;
	    break;

	case SET_WINRIGHT:
	    if ((value = abs( atoi (arg))) == 0)
		goto BadVal;
	    defrwin = value;
	    defrwin_flg = YES;
	    curwin->rwin = defrwin;
	    retval = CROK;
	    break;

	case SET_WIDTH:
	    if ((value = abs( atoi (arg))) == 0)
		goto BadVal;
#ifdef LMCAUTO
	    if (value <= autolmarg)
		mesg (ERRALL+1, "rmar must be greater than lmar.");
	    else
#endif
	    {
		setmarg (&linewidth, value);
		linewidth_flg = YES;
	    }
	    retval = CROK;
	    break;
#ifdef LMCAUTO
	case SET_LMARG:
	    if ((value = abs( atoi (arg))) < 0)
		goto BadVal;
	    if (value >= linewidth)
		mesg (ERRALL+1, "lmar must be less than rmar.");
	    else {
		setmarg (&autolmarg, value);
		autolmarg_flg = YES;
	    }
	    retval = CROK;
	    break;
#endif
	case SET_BELL:
		NoBell = NO;
#ifdef LMCVBELL
	    VBell = NO;
#endif
	    retval = CROK;
	    break;

	case SET_NOBELL:
	    NoBell = YES;
#ifdef LMCVBELL
	    VBell = NO;
#endif
	    retval = CROK;
	    break;

#ifdef LMCVBELL
	case SET_VBELL:
	    if (*term.tt_vbell == NULL)
		    mesg (ERRALL+1, "No vis-bell on this terminal.");
	    else {
		    NoBell = NO;
		    VBell = YES;
	    }
	    retval = CROK;
	    break;
#endif

	case SET_WORDDELIM:
	    retval = setwordmode (arg);
	    break;

	case SET_DEBUG:
	    dbgfname = get_debug_name ();
	    if ( dbgfname ) {
		DebugVal = (arg && *arg) ? atoi (arg) : get_debug_default_level ();
		if ( DebugVal == 0 ) {
		    /* suspend debug */
		    dbgflg = NO;
		    if ( dbgfile ) {
			fputs ("\n======= Suspend Rand debugging session =======\n", dbgfile);
			fflush (dbgfile);
			if ( dbgfile != stdout ) fclose (dbgfile);
		    }
		    dbgfile = NULL;
		} else {
		    /* continue debug */
		    dbgflg = YES;
		    if ( !dbgfile ) {
			cc = open_dbgfile (YES);
			if ( cc < 0 )
			    mesg (ERRALL + 1, "Cannot open the debug output file");
		    }
		}
	    } else {
		mesg (ERRALL + 1, "\"set debuglevel\" invalid outside \"-debug=\" option");
	    }
	    retval = CROK;
	    break;

#ifdef LMCSRMFIX
	case SET_RMSTICK:
	    optstick = YES;
	    retval = CROK;
	    break;

	case SET_RMNOSTICK:
	    optstick = NO;
	    retval = CROK;
	    break;
#endif

	case SET_HY:
	    fill_hyphenate = YES;
	    retval = CROK;
	    break;

	case SET_NOHY:
	    fill_hyphenate = NO;
	    retval = CROK;
	    break;

	case SET_GRAPHSET:
	    value = abs( atoi (arg));
	    if ( set_reset_graph ( value == 1 ) )
		fresh ();
	    retval = CROK;
	    break;

	case SET_UTF8:
	    if ( set_reset_utf8 ( (value = abs( atoi (arg))) == 1 ) )
		fresh ();
	    retval = CROK;
	    break;

	case SET_CHAR7B:
	    inputcharis7bits = YES;
	    retval = CROK;
	    break;

	case SET_CHAR8B:
	    inputcharis7bits = NO;
	    retval = CROK;
	    break;

	case SET_INFOLEVEL:
	    /* define what to display about cursor position */
	    set_info_level (atoi (arg));
	    break;

	case SET_PREF:
	    if ( arg && (*arg == '?') && (arg[1] == '\0') )
		get_preferences (YES, YES);
	    else
		save_preferences ();
	    retval = CROK;
	    break;

	default:
BadVal:     retval = CRBADARG;
	    break;
    }

    if ( arg && *arg )
	sfree (arg);

    return (retval);
}


void save_set_val (FILE * pref_file)
{
    extern int get_info_level ();
    extern Flag get_graph_flg ();
    extern Flag get_utf8_flg (Flag *);
    extern char * getwordmode ();

    int ind, val;
    char *opt;
    char *str, *setstr, strg[32];
    Flag can_set_flg;

    setstr = get_keyword_strg (cmdtable, CMDSET);
    memset (strg, 0, sizeof (strg));
    str = NULL;
    for ( ind = 0 ; setopttable [ind].str ; ind++ ) ;
    for ( ind-- ; ind >= 0 ; ind-- ) {
	opt = setopttable [ind].str;
	strg[0] = '\0';
	str = &strg[0];
	switch( setopttable[ind].val ) {
	    case SET_SHOW:
	    case SET_LINE:
	    case SET_PAGE:
	    case SET_WIN:
		continue;

	    case SET_WIDTH:
		if ( !linewidth_flg ) continue;
		val = linewidth;
		str = NULL;
		break;

#ifdef LMCAUTO
	    case SET_LMARG:
		if ( !autolmarg_flg ) continue;
		val = autolmarg;
		str = NULL;
		break;
#endif

	    case SET_MILINE:
		if ( !defmiline_flg ) continue;
		val = defmiline;
		str = NULL;
		break;

	    case SET_PLLINE:
		if ( !defplline_flg ) continue;
		val = defplline;
		str = NULL;
		break;

	    case SET_MIPAGE:
		if ( !defmipage_flg ) continue;
		val = defmipage;
		str = NULL;
		break;

	    case SET_PLPAGE:
		if ( !defplpage_flg ) continue;
		val = defplpage;
		str = NULL;
		break;

	    case SET_WINLEFT:
		if ( !deflwin_flg ) continue;
		val = deflwin;
		str = NULL;
		break;

	    case SET_WINRIGHT:
		if ( !defrwin_flg ) continue;
		val = defrwin;
		str = NULL;
		break;

	    case SET_WORDDELIM:
		str = getwordmode ();
		if ( ! str ) continue;
		break;

	    case SET_BELL:
		if ( NoBell ) continue;
		break;

	    case SET_NOBELL:
		if ( ! NoBell ) continue;
		break;

	    case SET_DEBUG:
		if ( ! dbgflg ) continue;
		str = NULL;
		val = DebugVal;
		break;

	    case SET_HY:
		if ( ! fill_hyphenate ) continue;
		break;

	    case SET_NOHY:
		if ( fill_hyphenate ) continue;
		break;

	    case SET_UTF8:
		str = NULL;
		val = (int) get_utf8_flg (&can_set_flg);
		if ( ! can_set_flg ) continue;
		break;

	    case SET_GRAPHSET:
		if ( strcmp (opt, "charset") == 0 ) continue;
		str = NULL;
		val = (int) get_graph_flg ();
		break;

	    case SET_CHAR7B:
		if ( ! inputcharis7bits ) continue;
		break;

	    case SET_CHAR8B:
		if ( inputcharis7bits ) continue;
		break;

	    case SET_INFOLEVEL:
		str = NULL;
		val = get_info_level ();
		break;

	    default:
		continue;
	}
	fprintf (pref_file, "cmd %s %s ", setstr, opt);
	if ( str ) fputs (str, pref_file);
	else fprintf (pref_file, "%d", val);
	putc ('\n', pref_file);
    }
}

/* User preferences file processing */

static void write_preferences (FILE *pref_file)
{
    extern char * get_debug_file_name (Flag create_dir, Flag *exist);
    extern void save_option_pref (FILE *pref_file);

    int i, kfval, init_val;
    char *str, *str1;

    /* save general values */
    str = itsyms_by_val (CCINSMODE);    /* insert mode */
    if ( str ) fprintf (pref_file, "key %s %d\n", str, insmode);

    kfval = itswapdeldchar (del_strg, -1, &init_val);
    if ( kfval != 0 ) {
	/* define the <del> key function value */
	str = get_keyword_strg (cmdtable, CMDFLIPBKAR);
	str1 = itsyms_by_val (kfval);
	if ( str ) fprintf (pref_file, "cmd %s %d # %s\n", str, kfval, str1);
    }

    if ( ntabs > 0 ) {
	/* tabstops */
	str = get_keyword_strg (cmdtable, CMD_TABS);
	if ( str ) fprintf (pref_file, "cmd %s\n", str);
	str = get_keyword_strg (cmdtable, CMDTAB);
	for ( i = 0 ; i < ntabs ; i++ ) {
	    if ( (i % 10) == 0 ) {
		if ( i > 0 ) fputc ('\n', pref_file);
		fprintf (pref_file, "cmd %s", str);
	    }
	    fprintf (pref_file, " %3d", tabs [i]+1);
	}
	fputc ('\n', pref_file);
    }

    str = get_keyword_strg (cmdtable, patmode ? CMDPATTERN : CMD_PATTERN);
    if ( str && *str ) fprintf (pref_file, "cmd %s\n", str);

#ifdef LMCAUTO
    if ( autofill ) {
	/* wp mode */
	str = get_keyword_strg (cmdtable, CMDAUTO);
	if ( str && *str ) fprintf (pref_file, "cmd %s\n", str);
    }
#endif

    /* save some of the set values (must be done after the general values) */
    save_set_val (pref_file);

    /* save calling options */
    save_option_pref (pref_file);
}

static file_error_msg (char *name)
{
    static char cmd_msg [128];

    sprintf (cmd_msg, "File \"%s\" error : %s", name, strerror (errno));
    command_msg = cmd_msg;
    return;
}

static char * open_preferences (Flag read_flg, FILE ** pref_file_pt,
				Flag* exist_flg_pt, FILE ** pref_old_file_pt,
				char *pref_old_name, int pref_old_name_sz)
{
    char * pref_name, * str;
    int cc;

    if ( ! pref_file_pt ) return NULL;
    *pref_file_pt = NULL;
    if ( pref_old_file_pt ) *pref_old_file_pt = NULL;
    pref_name = get_pref_file_name (YES, exist_flg_pt);
    if ( !*exist_flg_pt && read_flg ) {
	if ( pref_old_name ) {
	    memset (pref_old_name, 0, pref_old_name_sz);
	    strncpy (pref_old_name, pref_name, pref_old_name_sz -1);
	}
	return NULL;
    }

    if ( read_flg ) {
	/* open the preferences file to read it */
	*pref_file_pt = fopen (pref_name, "r");
	if ( ! *pref_file_pt ) {
	    file_error_msg (pref_name);
	    return NULL;
	}
	return pref_name;
    }

    /* open the preferences file to write */
    memset (pref_old_name, 0, pref_old_name_sz);
    if ( !*exist_flg_pt ) {
	/* does not exist yet, create */
	*pref_file_pt = fopen (pref_name, "w");
	if ( ! *pref_file_pt ) {
	    file_error_msg (pref_name);
	    return NULL;
	}
	return pref_name;
    }

    /* file exist, rename it and open */
    str = strrchr (pref_name, '/');
    if ( str ) memcpy (pref_old_name, pref_name, (str - pref_name) +1);
    strcat (pref_old_name, ",");
    strcat (pref_old_name, str +1);
    cc = rename (pref_name, pref_old_name);
    if ( cc < 0 ) {
	file_error_msg (pref_name);
	return;
    }
    if ( pref_old_file_pt ) {
	*pref_old_file_pt = fopen (pref_old_name, "r");
	if ( ! *pref_old_file_pt ) {
	    file_error_msg (pref_old_name);
	    return NULL;
	}
    }
    *pref_file_pt = fopen (pref_name, "w");
    if ( ! *pref_file_pt ) {
	file_error_msg (pref_name);
	return NULL;
    }
    return pref_name;
}

static char * get_pref_line (FILE **pref_old_file_pt, char *buf, int buf_sz)
/* on error or EOF, **pref_old_file_pt = NULL, return NULL */
{
    int cc;
    char ch, *line;

    line = fgets (buf, buf_sz, *pref_old_file_pt);
    if ( ! line ) {     /* error or EOF */
	fclose (*pref_old_file_pt);
	*pref_old_file_pt = NULL;
	return NULL;
    }
    for ( ; ch = *line ; line++ )   /* remove spaces */
	if ( (ch != ' ') && (ch != '\t') ) break;
    return line;
}

static char * get_term_name ()
/* get the terminal session name :
 * if xterminal : name or class name
 * else value of $TERM (tname)
 */
{
    static char *session_tname = NULL;
    char *xtncl, *xtnnm;

    if ( session_tname ) return session_tname;

    session_tname = tname ? tname : "";
    if ( get_xterm_name (&xtncl, &xtnnm) ) {
	if ( xtnnm && *xtnnm && (strcmp (xtnnm, "xterm") != 0) ) session_tname = xtnnm;
	else if ( xtncl && *xtncl ) session_tname = xtncl;
    }
    return session_tname;
}

static void save_preferences ()
{
    static const char updstrg [] = "updated";
    char pref_old_name [PATH_MAX];
    char ch, ch1, buf [256], marker [64], comt [128];
    char * pref_name;
    FILE *pref_file, *pref_old_file;
    time_t strttime;
    char *asct, *str, *comt1, *setstr, *line;
    Flag exist_flg, copy_flg, upd_flg;
    int i, marker_sz;

#if 0
    /* to check get_preferences only !!! */
    get_preferences (YES, NO);
    return;
#endif

    pref_name = open_preferences (NO, &pref_file, &exist_flg,
				  &pref_old_file, pref_old_name,
				  sizeof (pref_old_name));
    if ( ! pref_name )
	return;  /* file error */

    sprintf (marker, "[%s]", get_term_name ());
    marker_sz = strlen (marker);

    /* write the file header */
    (void) time (&strttime);
    asct = asctime (localtime (&strttime));
    asct [strlen(asct) -1] = '\0';
    str = get_keyword_strg (setopttable, SET_PREF);
    setstr = get_keyword_strg (cmdtable, CMDSET);

    memset (buf, 0, sizeof (buf));
    line = NULL;

    if ( ! pref_old_file ) {
	/* a new file, dump header */
	fprintf (pref_file, "# %s for user \"%s\"\n", pref_name, myname);
	fprintf (pref_file, "# Rand editor user preference file created on %s, section %s\n", asct, marker);
	fprintf (pref_file, "# -- Automaticaly generated file by \"%s %s\" command,\n", setstr, str);
	fputs (             "#      Do change the line order (it is significant).\n\n", pref_file);
    } else {
	/* copy the old header (comments) and update it */
	sprintf (comt, "#    File %s on %s, section %s\n\n", updstrg, asct, marker);
	upd_flg = NO;
	ch1 = '\n';
	for ( ; ; ) {   /* skip empty lines and comments */
	    line = get_pref_line (&pref_old_file, buf, sizeof (buf));
	    if ( ! line ) break;    /* EOF or error */
	    ch = *line;
	    if ( ch == '\n' ) {
		if ( ch1 == '\n' ) continue;  /* skip */
		else if ( (ch1 == '!') || (ch1 == '#') ) {
		    fputs (comt, pref_file);
		    ch1 = '\n';
		    upd_flg = YES;
		}
	    } else if ( (ch == '!') || (ch == '#') ) {
		/* copy, excepted old "... updated ..." line */
		if ( strstr (line, marker) && strstr (line, updstrg) ) continue;
		fputs (buf, pref_file);
		ch1 = *line;
	    } else {
		break;
	    }
	}
	if ( ! upd_flg ) {
	    fputs (comt, pref_file);
	    ch1 = '\n';
	    upd_flg = YES;
	}
	if ( ch1 != '\n' ) fputc ('\n', pref_file);
    }
    ch1 = '\n';

    if ( pref_old_file && line ) {
	/* copy up to the current terminal section */
	for ( copy_flg = YES ; ; ) {
	    if ( copy_flg ) {
		copy_flg = (strncmp (line, marker, marker_sz) != 0);
		if ( copy_flg ) {
		    if ( (ch1 != '\n') || (*line != '\n') ) {
			fputs (buf, pref_file);
			ch1 = *line;
		    }
		}
	    } else if ( *line == '[' ) break;
	    line = get_pref_line (&pref_old_file, buf, sizeof (buf));
	    if ( ! line ) break;    /* EOF or error */
	}
    }

    /* write the preferences */
    if ( ch1 != '\n' ) fputc ('\n', pref_file);
    fprintf (pref_file, "%s  # from \"%s\"\n", marker, get_myhost_name ());
    write_preferences (pref_file);
    ch1 = '\0';

    if ( pref_old_file && line ) {
	/* copy remaining part of the old file */
	if ( *line == '[' ) {
	    if ( ch1 != '\n' ) fputc ('\n', pref_file);
	    fputs (buf, pref_file);
	    ch1 = *line;
	}
	for ( ; ; ) {
	    line = get_pref_line (&pref_old_file, buf, sizeof (buf));
	    if ( ! line ) break;    /* EOF or error */
	    if ( (ch1 == '\n') && (*line == '\n') ) continue;
	    if ( (*line == '[') && (ch1 != '\n') ) fputc ('\n', pref_file);
	    fputs (buf, pref_file);
	    ch1 = *line;
	}
    }

    if ( pref_old_file ) fclose (pref_old_file);
    if ( pref_old_name [0] ) (void) unlink (pref_old_name);

    fclose (pref_file);
    mesg (TELALL + 4, "Preferences saved in \"", pref_name, "\" section ", marker);
    loopflags.hold = YES;
}

/* process_preference : process one line of preferences
 *  if show_flg just test if the preference will be set in any case
 */
static Flag process_preference (char *line, int lnb, Flag all_flg, Flag show_flg)
{
    extern void set_option_pref (char *cmd, char *par);
    Cmdret cc;
    int idx, idx1, sz, value;
    char *kwd, *str, *str1, *cmd, *par;
    char strn [256], strn1 [64];
    Flag flg;

    if ( show_flg ) all_flg = NO;
    if ( ! line ) return;
    sz = strlen (line);
    if ( (sz <= 0) || (*line == '\n') ) return;
    if ( line[sz -1] == '\n' ) line[sz -1] = '\0';

    /* parse the line */
    strcpy (strn, line);
    kwd = strtok (strn, " ");
    cmd = strtok (NULL, " ");
    str = strtok (NULL, "#");   /* delimiter is the comment mark */
    par =  str ? str : "";
    if ( !kwd || !cmd ) return NO;

    cc = 0;
    if ( strcmp (kwd, "cmd") == 0 ) {
	/* a command preference */
	idx = lookup (cmd, cmdtable);
	if ( idx < 0 ) return;
	if ( ! all_flg ) {
	    /* check what must be set when replaying an old status file */
	    switch (cmdtable[idx].val) {
		case CMDFLIPBKAR :
		    break;

		case CMDSET :
		    memset (strn1, 0, sizeof (strn1));
		    strncpy (strn1, par, sizeof (strn1) -1);
		    str1 = strchr (strn1, ' ');
		    if ( !str1 ) return NO;
		    *str1 = '\0';
		    idx1 = lookup (strn1, setopttable);
		    if ( idx1 < 0 ) return NO;
		    switch (setopttable[idx1].val) {
			/* to be set in all case */
			case SET_MILINE:
			case SET_PLLINE:
			case SET_MIPAGE:
			case SET_PLPAGE:
			case SET_WINLEFT:
			case SET_WINRIGHT:
			case SET_BELL:
			case SET_NOBELL:
			    break;
			default :
			    return NO;
		    }
		    break;

		default :
		    return NO;
	    }
	}
	if ( ! show_flg ) {
	    cc = command (cmdtable[idx].val, par);
	    if ( cc != CROK ) {
		sprintf (strn, "Command error %d at line %d: \"%s\"", cc, lnb, line);
		mesg (ERRALL+1, strn);
		return NO;
	    }
	}
    }
    else if ( strcmp (kwd, "key") == 0 ) {
	/* a key function preference */
	if ( ! all_flg ) return NO;
	idx = lookup (cmd, itsyms);
	if ( idx < 0 ) return;
	switch ( itsyms[idx].val ) {
	    case CCINSMODE :
		flg = (abs( atoi (par)) != 0);
		if ( (insmode && !flg) || (!insmode && flg) )
		    tglinsmode ();
		break;
	}
    }
    else if ( strcmp (kwd, "option") == 0 ) {
	/* an option preference */
	if ( ! show_flg ) set_option_pref (cmd, par);
    }
    return YES;
}

Cmdret read_preferences (FILE **pref_file_pt, Flag *all_flg_pt, Flag *show_flg_pt)
{
    int lnb, stname_sz, cc;
    char ch, buf [256];
    char *line;
    char *stname;
    Flag all_flg, show_flg, any_flg;

    show_flg = *show_flg_pt;
    all_flg = show_flg ? NO : *all_flg_pt;

    memset (buf, 0, sizeof (buf));
    stname = get_term_name ();
    stname_sz = strlen (stname);
    if ( show_flg ) {
	printf ("Preferences from \"%s\", section [%s]\n\n",
		preferences_file_name, stname);
    }
    for ( lnb = 0 ; ; ) {
	/* skip up to the current terminal section */
	lnb++;
	line = get_pref_line (pref_file_pt, buf, sizeof (buf));
	if ( ! line ) break;    /* EOF or error */
	if ( line[0] != '[' ) continue;
	if (   (strncmp (line+1, stname, stname_sz) == 0)
	    && (line [stname_sz+1] == ']') ) break;
    }
    if ( ! *pref_file_pt ) return 0;
    if ( feof (*pref_file_pt) ) {
	/* section for this terminal name not found */
	if ( show_flg ) {
	    fputs ("No preferences for this terminal, use \"set preferences\" command\n", stdout);
	}
	fclose (*pref_file_pt);
	*pref_file_pt = NULL;
	return 0;
    }
    get_preferences_flg = YES;
    if ( ! show_flg ) use_preferences_flg = YES;
    for ( ; ; ) {
	/* get the terminal preferences */
	lnb++;
	line = get_pref_line (pref_file_pt, buf, sizeof (buf));
	if ( ! line ) break;    /* EOF or error */
	ch = *line;
	if ( (ch == '\n') || (ch == '!') || (ch == '#') ) continue;
	if ( ch == '[' ) break; /* new section */
	any_flg = process_preference (line, lnb, all_flg, show_flg);
	if ( show_flg ) printf ("%c %s\n", any_flg ? '*' : ' ', buf);
    }
    get_preferences_flg = NO;
    if ( show_flg ) {
	puts ("\n\"*\" : this preference is set even if a previous session state file is reused");
	cc = call_waitkeyboard (NULL, NULL);
    }
    return 0;
}

void get_preferences (Flag all_flg, Flag show_flg)
{
    char *pref_name, *msg;
    FILE *pref_file;
    Flag exist_flg;
    char pref_old_name [PATH_MAX];

    preferences_file_name = "";
    pref_old_name [0] = '\0';
    get_preferences_flg = use_preferences_flg = NO;
    pref_name = open_preferences (YES, &pref_file, &exist_flg, NULL,
				  pref_old_name, sizeof (pref_old_name));
    if ( !pref_name || !exist_flg ) {
	if ( show_flg ) {
	    msg = exist_flg ? " : preferences file error"
			    : " : preferences file does not exist";
	    mesg (ERRALL +2, pref_old_name, msg);
	}
	return;  /* file error */
    }
    if ( ! pref_file ) return;

    preferences_file_name = pref_name;
    if ( ! show_flg )
	(void) read_preferences (&pref_file, &all_flg, &show_flg);
    else
	(void) do_fullscreen (read_preferences, (void *) &pref_file,
			      (void *) &all_flg, (void *) &show_flg);

    if ( pref_file ) fclose (pref_file);
}

char * get_session_pref (char **stname_pt, Flag *use_flg_pt)
{
    char *pref_name;
    Flag pref_exist;

    if ( use_flg_pt ) *use_flg_pt = use_preferences_flg;
    if ( stname_pt ) *stname_pt = get_term_name ();
    pref_name = get_pref_file_name (NO, &pref_exist);
    return (pref_exist && *pref_name) ? pref_name : NULL;
}
