#ifdef COMMENT
	Copyright abandoned, 1983, The Rand Corporation
#endif

#define MY_CTRL(x) ((x) & 31)

#ifdef COMMENT
--------
file term/standard.c
    Standard stuff in support of
    terminal-dependent code and data declarations
#endif

unsigned char lexstd[32] = {
    CCCMD       , /* <BREAK > */
    CCCMD       , /* <cntr A> */    /* must be CCCMD */
    CCLWORD     , /* <cntr B> */
    CCBACKSPACE , /* <cntr C> */
    CCMILINE    , /* <cntr D> */
    CCMIPAGE    , /* <cntr E> */
    CCPLLINE    , /* <cntr F> */
    CCHOME      , /* <cntr G> */
    CCMOVELEFT  , /* <cntr H> */
    CCTAB       , /* <cntr I> */
    CCMOVEDOWN  , /* <cntr J> */
    CCMOVEUP    , /* <cntr K> */
    CCMOVERIGHT , /* <cntr L> */
    CCRETURN    , /* <cntr M> */
    CCRWORD     , /* <cntr N> */
    CCOPEN      , /* <cntr O> */
    CCPICK      , /* <cntr P> */
    CCUNAS1     , /* <cntr Q> */
    CCPLPAGE    , /* <cntr R> */
    CCUNAS1     , /* <cntr S> */
    CCMISRCH    , /* <cntr T> */
    CCMARK      , /* <cntr U> */
    CCCLOSE     , /* <cntr V> */
    CCDELCH     , /* <cntr W> */
    CCUNAS1     , /* <cntr X> */
    CCPLSRCH    , /* <cntr Y> */
    CCINSMODE   , /* <cntr Z> */
    CCINSMODE   , /* <escape> */
    CCINT       , /* <cntr \> */
    CCREPLACE   , /* <cntr ]> */
    CCERASE     , /* <cntr ^> */
    CCSETFILE   , /* <cntr _> */
};

/* use this for column or row cursor addressing if the terminal doesn't
 * support separate row and column cursor addressing
 **/
bad ()
{
    fatal (FATALBUG, "Impossible cursor address");
}

nop () {}

/* Special characters */

/* look up table for kb file : window edges characters set user definition */
S_looktbl bordersyms [] = {
    "blcmch",  BLCMCH  ,    /* bottom left corner border */
    "bmch",    BMCH    ,    /* bottom border */
    "brcmch",  BRCMCH  ,    /* bottom right corner border */
    "btmch",   BTMCH   ,    /* bottom tab border */
    "elmch",   ELMCH   ,    /* empty left border */
    "flipbkarrow",LASTSPCL+5,/* flip (1) DEL ("bksp") and DELCH ("dchar") key function for the ascii <Del> ('\177') key */
    "graphchar",LASTSPCL +1,/* use (1) graphic char set and user defined window edges */
    "graphenter",LASTSPCL+2,/* enter graphic char set or unicode mode string */
    "graphleave",LASTSPCL+3,/* leave graphic char set or unicode mode string */
    "inmch",   INMCH   ,    /* inactive border */
    "lmch",    LMCH    ,    /* left border */
    "mlmch",   MLMCH   ,    /* more left border */
    "mrmch",   MRMCH   ,    /* more right border */
    "rmch",    RMCH    ,    /* right border */
    "tlcmch",  TLCMCH  ,    /* top left corner border */
    "tmch",    TMCH    ,    /* top border */
    "trcmch",  TRCMCH  ,    /* top right corner border */
    "ttmch",   TTMCH   ,    /* top tab border */
    "utf8",    LASTSPCL +4, /* use (1) utf-8 unicode mode for graphical char set */
    0,0        /* end of window edges key functions */
    };

/* NB : "utf8" must be defined before "graphenter" or "graphleave". The string
	updated depend upon the current value of utf8_flg. */

static struct _bsymbs_com {
    short val;
    char *str;
} bsymbs_com [] = {     /* comments for the border char definition */
    LASTSPCL +4,"<1> = use utf-8 unicode mode for graphical char set",
    LASTSPCL +2,"enter graphic (utf8=0) or unicode (utf8=1) mode string",
    LASTSPCL +3,"leave graphic (utf8=0) or unicode (utf8=1) mode string",
    LASTSPCL +1,"<1> = use graphic char set and user defined window edges",
    BLCMCH  ,   "bottom left corner border",
    BMCH    ,   "bottom border",
    BRCMCH  ,   "bottom right corner border",
    BTMCH   ,   "bottom tab border",
    ELMCH   ,   "empty left border",
    INMCH   ,   "inactive border",
    LMCH    ,   "left border",
    MLMCH   ,   "more left border",
    MRMCH   ,   "more right border",
    RMCH    ,   "right border",
    TLCMCH  ,   "top left corner border",
    TMCH    ,   "top border",
    TRCMCH  ,   "top right corner border",
    TTMCH   ,   "top tab border",
    0,0
    };

static struct _bsymbs_com miscsymbs_com [] = {  /* miscellaneous kbfile keywords */
    LASTSPCL +5,"<1> = flip \"bksp\" and \"dchar\" key function for the ascii <Del> (<0177>) key",
    0,0
    };


static unsigned char defxlate [NSPCHR] = {  /* default setting */
    0177,    /* ESCCHAR escape character */
	/*
	 * For terminals that don't support any BLOT character,
	 * their kbd_init() routine should reset BULCHAR to something
	 * else like '*'.
	 */
    0177,   /* BULCHAR bullet character */
    '|',    /* LMCH    left border */
    '|',    /* RMCH    right border */
    '<',    /* MLMCH   more left border */
    '>',    /* MRMCH   more right border */
    '-',    /* TMCH    top border */
    '-',    /* BMCH    bottom border */
    '+',    /* TLCMCH  top left corner border */
    '+',    /* TRCMCH  top right corner border */
    '+',    /* BLCMCH  bottom left corner border */
    '+',    /* BRCMCH  bottom right corner border */
    '+',    /* BTMCH   bottom tab border */
    '+',    /* TTMCH   top tab border */
    ';',    /* ELMCH   empty left border */
    '.'     /* INMCH   inactive border */
};

static unsigned char ibmpcxlate [NSPCHR] = {
    0177,   /* ESCCHAR escape character */
	/*
	 * For terminals that don't support any BLOT character,
	 * their kbd_init() routine should reset BULCHAR to something
	 * else like '*'.
	 */
    0177,   /* 0x7F BULCHAR bullet character */
    186,    /* 0xBA LMCH    left border */
    186,    /* 0xBA RMCH    right border */
    185,    /* 0xCC MLMCH   more left border */
    204,    /* 0xB9 MRMCH   more right border */
    205,    /* 0xCD TMCH    top border */
    205,    /* 0xCD BMCH    bottom border */
    201,    /* 0xC9 TLCMCH  top left corner border */
    187,    /* 0xBB TRCMCH  top right corner border */
    200,    /* 0xC8 BLCMCH  bottom left corner border */
    188,    /* 0xBC BRCMCH  bottom right corner border */
    209,    /* 0xD1 BTMCH   top tab border */
    207,    /* 0xCF TTMCH   bottom tab border */
    179,    /* 0xB3 ELMCH   empty left border */
    '.'     /* 0x2E INMCH   inactive border */
    };

static unsigned char gnome_utf8_xlate [NSPCHR] = {
    0177,   /* ESCCHAR escape character */
	/*
	 * For terminals that don't support any BLOT character,
	 * their kbd_init() routine should reset BULCHAR to something
	 * else like '*'.
	 */
    0177,   /* 0x7F BULCHAR bullet character */
    0x19,   /*      LMCH    left border */
    0x19,   /*      RMCH    right border */
    0x16,   /*      MLMCH   more left border */
    0x15,   /*      MRMCH   more right border */
    0x12,   /*      TMCH    top border */
    0x12,   /*      BMCH    bottom border */
    0x0D,   /*      TLCMCH  top left corner border */
    0x0C,   /*      TRCMCH  top right corner border */
    0x0E,   /*      BLCMCH  bottom left corner border */
    0x0B,   /*      BRCMCH  bottom right corner border */
    0x18,   /*      BTMCH   top tab border */
    0x17,   /*      TTMCH   bottom tab border */
    0x1F,   /*      ELMCH   empty left border */
    '.'     /* 0x2E INMCH   inactive border */
    };

static unsigned char vt100_linedrawing_xlate [NSPCHR] = {
    0177,   /* ESCCHAR escape character */
	/*
	 * For terminals that don't support any BLOT character,
	 * their kbd_init() routine should reset BULCHAR to something
	 * else like '*'.
	 */
    0177,   /* 0x7F BULCHAR bullet character */
    0x78,   /*      LMCH    left border */
    0x78,   /*      RMCH    right border */
    0x75,   /*      MLMCH   more left border */
    0x74,   /*      MRMCH   more right border */
    0x71,   /*      TMCH    top border */
    0x71,   /*      BMCH    bottom border */
    0x6C,   /*      TLCMCH  top left corner border */
    0x6B,   /*      TRCMCH  top right corner border */
    0x6D,   /*      BLCMCH  bottom left corner border */
    0x6A,   /*      BRCMCH  bottom right corner border */
    0x77,   /*      BTMCH   top tab border */
    0x76,   /*      TTMCH   bottom tab border */
    0x7E,   /*      ELMCH   empty left border */
    '.'     /* 0x2E INMCH   inactive border */
    };

static unsigned char alt_vt100xlate [NSPCHR] = {
    0177,   /* ESCCHAR escape character */
	/*
	 * For terminals that don't support any BLOT character,
	 * their kbd_init() routine should reset BULCHAR to something
	 * else like '*'.
	 */
    0177,   /* 0x7F BULCHAR bullet character */
    186,    /* 0xBA LMCH    left border */
    186,    /* 0xBA RMCH    right border */
    185,    /* 0xCC MLMCH   more left border */
    204,    /* 0xB9 MRMCH   more right border */
    205,    /* 0xCD TMCH    top border */
    205,    /* 0xCD BMCH    bottom border */
    201,    /* 0xC9 TLCMCH  top left corner border */
    187,    /* 0xBB TRCMCH  top right corner border */
    200,    /* 0xC8 BLCMCH  bottom left corner border */
    188,    /* 0xBC BRCMCH  bottom right corner border */
    209,    /* 0xD1 BTMCH   top tab border */
    207,    /* 0xCF TTMCH   bottom tab border */
    179,    /* 0xB3 ELMCH   empty left border */
    '.'     /* 0x2E INMCH   inactive border */
    };

/* On linux system execute 'showcfont' to see the current set of chararcters */

static Flag xlate_pt_update (char *);

/* xlate tables : convertion for window edges characters */
/* ----------------------------------------------------- */
/*  can be customized by kbfile at startup */

static Flag xlate_init_done = NO;
static unsigned char utf8xlate [NSPCHR];    /* convertion table for unicode case */
static unsigned char  altxlate [NSPCHR];    /* convertion table for alternate char set */
static unsigned char *graphxlate = defxlate;        /* current convertion table to be use with graphic edges */

static Flag utf8_flg = NO;      /* use utf_8 encoding for edges characaters */
static Flag set_utf8_flg = NO;  /* utf_8 set reset is allowed */
static Flag graph_flg = NO;     /* use graphic convertion table */

/* widow edges convertion table to be used */
unsigned char *stdxlate = defxlate; /* current border character set pointer */


/* strings to set and reset the char set table to be used during editing session */
/* ref"console_codes" man page for Linux console */
static unsigned char *   set_charset_strg = NULL;
static unsigned char * reset_charset_strg = NULL;

static unsigned char IBMPC_tc [] ="\033(U";    /* G0 -> translation table c */
static unsigned char IBMPC_ta [] ="\033(B";    /* G0 -> translation table a */

/* character set to be used for the window edges */
static unsigned char * enter_graph_charset = NULL;
static unsigned char * leave_graph_charset = NULL;

static unsigned char vt100_graph_charset [] = "\016\033)0";   /* ^N Shift Out : G1 100 DEC special and line drawing char */
static unsigned char vt100_alpha_charset [] = "\017\033(B";   /* ^O Shift In  : G0 USASCII char set */

static unsigned char vt100_G0_special [] = "\033(0";   /* vt100 G0 DEC special */
static unsigned char vt100_G0_USACII  [] = "\033(B";   /* vt100 G0 USASCII */

/* default escape sequence to enter, leave utf-8 encoding mode */
static unsigned char enter_utf8 [] = "\033%G";
static unsigned char leave_utf8 [] = "\033%@";

static unsigned char * enter_unicode_mode = enter_utf8;
static unsigned char * leave_unicode_mode = leave_utf8;

/* my_ch_to_utf8 : look into console-tools-0.3.3/lib/generic/unicode.c
    and console-tools-0.3.3/screenfonttools/showcfont.c
    for the magic way to display any graphic char with utf8 encoding !!
    To be check if it is working on other OS than Linux */

static char * my_ch_to_utf8 (unsigned char ch)
{
    static char utf [4];
    int c;

    /* Ref to UCS2 special value in range 0xF000..0xF1FF
	and in 'PRIVATE AREA' of the unicode man page on linux */
    c = ch + 0xF000;
    utf[0] = 0xe0 | (c >> 12);        /*  1110**** 10****** 10******  */
    utf[1] = 0x80 | ((c >> 6) & 0x3f);
    utf[2] = 0x80 | (c & 0x3f);
    utf[3] = 0;
    return utf;
}

char * graph_char_txt (Flag *utf_flg_pt)
{
    extern char * escseqstrg (char *, Flag);
    static char str [256];
    char str1[64];

    if ( utf_flg_pt ) *utf_flg_pt = utf8_flg;
    if ( utf8_flg ) {
	strcpy (str1, escseqstrg (enter_unicode_mode, NO));
	sprintf (str, "\
    Unicode 'PRIVATE AREA' (0xF000..0xF1FF) using UTF-8 encoded UCS characters\n\
    Enter utf-8 mode : \"%s\"  Leave utf-8 mode \"%s\"\n",
		 str1, escseqstrg (leave_unicode_mode, NO));
    } else {
	if ( enter_graph_charset ) {
	    strcpy (str1, escseqstrg (enter_graph_charset, NO));
	    sprintf (str, "\
    Alternate character set\n\
    Enter alt char set : \"%s\"  Leave alt char set \"%s\"\n",
		 str1, escseqstrg (leave_graph_charset, NO));
	} else {
	    return ("Normal character set");
	}
    }
    if ( stdxlate != defxlate )
	strcat (str, "    Currently using graphic window edges\n");
    else
	strcat (str, "    Currently using default SET (alphabetic window edges)\n");
    return &str[0];
}

void put_graph_char (int ch, Flag gr_flg)
{
    int c;

    c = ch & 0377;
    if ( !gr_flg || !utf8_flg )
	if ( c < ' ' ) c = ' ';
    if ( gr_flg && utf8_flg ) {
	/* to be check if it is working on other OS than Linux */
	fputs (enter_unicode_mode, stdout);
	fputs (my_ch_to_utf8 (c), stdout);
	fputs (leave_unicode_mode, stdout);
	return;
    }
    if ( gr_flg && enter_graph_charset ) {
	(void) fputs ((char *) enter_graph_charset, stdout);
	putchar (c);
	(void) fputs ((char *) leave_graph_charset, stdout);
	return;
    }
    putchar (c);
}

void display_miscsymbs (FILE *kb_file)
{
    int i, j, idx;
    unsigned char cch;
    char *symb, *cmt;
    unsigned char * str1;
    char tstrg [128];
    Flag kbfile_flg;
    FILE *out;

    out = (kb_file) ? kb_file : stdout;
    cch = (out == stdout) ? '#' : '!';
    kbfile_flg = (out != stdout);

    fprintf (out,   "\n%c Miscellaneous key assignment commands\n", cch);
    if ( out != stdout )
	fprintf (out, "%c -------------------------------------\n", cch);
    for ( j = 0 ; idx = miscsymbs_com[j].val ; j++ ) {
	cmt = miscsymbs_com[j].str;
	symb = "???";
	for ( i = 0 ; bordersyms[i].val ; i++ ) {
	    if ( idx == bordersyms[i].val ) {
		symb = bordersyms[i].str;
		break;
	    }
	}
	memset (tstrg, 0, sizeof (tstrg));
	switch (idx) {
	    case LASTSPCL +5 :
		/* flip DEL and DELCH key functions for ascii '\177'key */
		    sprintf (tstrg, "<%s>:<0>", symb);
		    break;
	}
	if ( tstrg[0] ) fprintf (out, "%-20s # %s\n", tstrg, cmt);
    }
}

void display_xlate (char *term_name, FILE *kb_file)
{
    extern Flag verbose_helpflg;
    extern char * escseqstrg (char *, Flag);
    int i, j, idx;
    unsigned char c, cch;
    char *symb, *cmt;
    unsigned char * str1;
    char tstrg [128];
    Flag misc_flg, kbfile_flg;
    FILE *out;

    out = (kb_file) ? kb_file : stdout;
    cch = (out == stdout) ? '#' : '!';
    kbfile_flg = (out != stdout);
    (void) xlate_pt_update (NULL);
    misc_flg = NO;
    fprintf (out,   "\n%c Character set used for window edges\n", cch);
    if ( out != stdout )
	fprintf (out, "%c -----------------------------------\n", cch);
    for ( j = 0 ; idx = bsymbs_com[j].val ; j++ ) {
	if ( idx < FIRSTSPCL ) continue;
	cmt = bsymbs_com[j].str;
	symb = "???";
	for ( i = 0 ; bordersyms[i].val ; i++ ) {
	    if ( idx == bordersyms[i].val ) {
		symb = bordersyms[i].str;
		break;
	    }
	}
	memset (tstrg, 0, sizeof (tstrg));
	if ( idx <= LASTSPCL ) {
	    /* edges character */
	    if ( ! misc_flg ) {
		fprintf (out, "\n%c Window edges character set for terminal type \'%s\'\n", cch, term_name);
		if ( graph_flg ) {
		    if ( utf8_flg )
			fprintf (out, "%c    using unicode char set and utf-8 encoding\n", cch);
		    else if ( enter_graph_charset )
			fprintf (out, "%c    using graphical char set\n", cch);
		} else
		    fprintf (out, "%c    using text (normal) char set\n", cch);
		misc_flg = YES;
	    }
	    c = stdxlate [idx - FIRSTSPCL];
	    sprintf (tstrg, "<%s>:<0x%02X>", symb, c);
	    if ( !verbose_helpflg && (out == stdout) ) {
		fprintf (out, "%-20s # \'", tstrg);
		put_graph_char (c, graph_flg);
		fprintf (out, "\' = %s\n", cmt);
	    } else {
		fprintf (out, "%-20s #", tstrg);
		fprintf (out, " %s\n", cmt);
	    }
	} else {
	    /* various setting */
	    switch (idx) {
		case LASTSPCL +1 :
		    /* use graphic or text char for window edges */
		    sprintf (tstrg, "<%s>:<%d>", symb, (graph_flg) ? 1 : 0);
		    break;

		case LASTSPCL +2 :
		    str1 = utf8_flg ? enter_unicode_mode : enter_graph_charset;
		    sprintf (tstrg, "<%s>:", symb);
		    strcat (tstrg, escseqstrg (str1, kbfile_flg));
		    break;

		case LASTSPCL +3 :
		    str1 = utf8_flg ? leave_unicode_mode : leave_graph_charset;
		    sprintf (tstrg, "<%s>:", symb);
		    strcat (tstrg, escseqstrg (str1, kbfile_flg));
		    break;

#ifdef __linux__
		case LASTSPCL +4 :
		    /* use utf-8 encoding for graphic window edges */
		    sprintf (tstrg, "<%s>:<%d>", symb, (utf8_flg) ? 1 : 0);
		    break;
#endif
	    }
	    if ( tstrg[0] ) fprintf (out, "%-20s # %s\n", tstrg, cmt);
	}
    }
    putc ('\n', out);
}

/* terminal character set and window edges routines */
/* ------------------------------------------------ */

/* character set used to display text */

static void reset_charset ()
{
    if ( reset_charset_strg )
	(void) fputs ((char *) reset_charset_strg, stdout);
}

void (*set_charset (char *term_name)) ()
{
    unsigned char *charset_strg;

    (void) xlate_pt_update (term_name);
    charset_strg = ( term_name ) ? set_charset_strg : reset_charset_strg;
    if ( charset_strg ) (void) fputs ((char *) charset_strg, stdout);
    return (reset_charset);
}

/* graphic character set (used for window edges) */

static void xlate_init (char *term_name)
    /* initialize xlate convertion table (if needed),
       must be called only within xlate_pt_update */
{
    extern Flag Xterm_global_flg;
    unsigned char *tmputf8xlate, *tmpaltxlate;

    if ( xlate_init_done )
	return;

    xlate_init_done = YES;
    if ( term_name == NULL ) term_name = tname;

    /* init to default the graph xlate array */
    memcpy (utf8xlate, defxlate, sizeof (utf8xlate));
    memcpy ( altxlate, defxlate, sizeof ( altxlate));
    utf8_flg = set_utf8_flg = graph_flg = NO;
    enter_unicode_mode = enter_utf8;
    leave_unicode_mode = leave_utf8;
    enter_graph_charset = vt100_graph_charset;
    leave_graph_charset = vt100_alpha_charset;

    /* no alternate text char set implemented yet */
    reset_charset_strg = NULL;
    set_charset_strg   = NULL;

    tmputf8xlate = tmpaltxlate = NULL;
    if ( term_name ) {
	if ( Xterm_global_flg || (strncasecmp (term_name, "xterm", 5) == 0) ) {
	    tmpaltxlate = vt100_linedrawing_xlate;
	    enter_graph_charset = vt100_graph_charset;
	    leave_graph_charset = vt100_alpha_charset;
#ifdef __linux__
	    /* to be extended if more xterm will support the unicode utf-8 mode */
	    tmputf8xlate = gnome_utf8_xlate;
#endif
	}
#ifdef __linux__
#if defined ( __i386 ) + defined ( __i486 ) + defined ( __i586 ) + defined ( __i686 )
	else if ( strncasecmp (term_name,"linux", 5) == 0 ) {
	    graph_flg = utf8_flg = set_utf8_flg = YES;
	    tmpaltxlate  = vt100_linedrawing_xlate;
	    tmputf8xlate = ibmpcxlate;
	    enter_graph_charset = vt100_graph_charset;
	    leave_graph_charset = vt100_alpha_charset;
	}
#endif /* __i386 ... */
#endif /* __linux__ */
	else {
	    /* default : assume something like a vt100 compatible */
	    tmpaltxlate = vt100_linedrawing_xlate;
	    enter_graph_charset = vt100_graph_charset;
	    leave_graph_charset = vt100_alpha_charset;
	}
    }
    if ( tmputf8xlate ) memcpy (utf8xlate, tmputf8xlate, sizeof (utf8xlate));
    if ( tmpaltxlate )  memcpy (altxlate,  tmpaltxlate,  sizeof (altxlate));
}

static Flag xlate_pt_update (char *term_name)
    /* init if needed xlate tables, and update xlate pointers */
{
    unsigned char *old_stdxlate;

    if ( ! xlate_init_done ) xlate_init (term_name);
#ifndef __linux__
    utf8_flg = NO;
#endif
    old_stdxlate   = stdxlate;
    graphxlate = ( utf8_flg )   ? utf8xlate  : altxlate;
    stdxlate   = ( graph_flg )  ? graphxlate : defxlate;
    return ( old_stdxlate != stdxlate );
}

Flag get_graph_flg ()
{
    return graph_flg;
}

Flag set_reset_graph (Flag flg)
{
    Flag ret_flg, ret1_flg;

    ret_flg = (flg != graph_flg);
    graph_flg = flg;
    ret1_flg = xlate_pt_update (NULL);
    return (ret_flg || ret1_flg);
}

Flag set_reset_utf8 (Flag flg)
/* setting of utf8 can be done only for predefined set of
   terminal type : linux console, and Gnome terminal
   or if <utf8> was set to 1 by kbfile
*/
{
    extern char *emul_class;
    Flag ret_flg;

    if ( ! set_utf8_flg ) {
	if ( emul_class && (strncasecmp (emul_class, "GnomeTerminal", 13) == 0) )
	    set_utf8_flg = YES;
	else if ( flg ) {
	    mesg (ERRALL + 1, "Not allowed, <utf8> must be defined in kbfile");
	}
    }
    ret_flg = ((utf8_flg != flg) && set_utf8_flg);
    utf8_flg = flg && set_utf8_flg;
    if ( utf8_flg ) graph_flg = YES;
    ret_flg = ret_flg || xlate_pt_update (NULL);
    return ret_flg;
}

Flag get_utf8_flg (Flag *can_set_flg_pt)
{
    if ( can_set_flg_pt ) *can_set_flg_pt = set_utf8_flg;
    return utf8_flg;
}

/* customize_xlate : set the border character from kbfile specification */
/*  kb file must provide in info in this syntax :
	<border symbol>:<display character code>
	eg : <lmch>:<0xBA>   IBM PC charter set
	  or <lmch>:"|"      default
*/
void customize_xlate (unsigned char idx, unsigned char * val)
{
    extern int  itswapdeldchar (char *, int, int *);
    extern char del_strg [];

    static char *enter_strg = NULL;
    static char *leave_strg = NULL;
    int i;
    Flag flip_flg;

    if ( (idx > FIRSTMCH) && (idx <= LASTSPCL) ) {
	/* customize on of the window edges */
	(void) xlate_pt_update (NULL);
	if ( graphxlate != defxlate )
	    graphxlate [idx - FIRSTSPCL] = val[0];
	return;
    }

    if ( idx >= LASTSPCL ) {
	switch (idx) {
	    case LASTSPCL +1 :
		/* use graphic or not for window edges */
		graph_flg = ! ( (val[0] == 0) || (val[0] == '0') );
		(void) xlate_pt_update (NULL);
		break;

	    case LASTSPCL +2 :
		/* define enter graphic or unicode mode string */
		if ( enter_strg ) free (enter_strg);
		enter_strg = (char *) malloc (strlen (val) +1);
		if ( ! enter_strg ) break;
		strcpy (enter_strg, val);
		if ( utf8_flg ) enter_unicode_mode = enter_strg;
		else enter_graph_charset = enter_strg;
		break;

	    case LASTSPCL +3 :
		/* define leave graphic or unicode mode string */
		if ( leave_strg ) free (leave_strg);
		leave_strg = (char *) malloc (strlen (val) +1);
		if ( ! leave_strg ) break;
		strcpy (leave_strg, val);
		if ( utf8_flg ) leave_unicode_mode = leave_strg;
		else leave_graph_charset = leave_strg;
		break;

	    case LASTSPCL +4 :
		/* use utf-8 or graphic alternate char set for window edges */
		set_utf8_flg = YES;
		utf8_flg = ! ( (val[0] == 0) || (val[0] == '0') );
		(void) xlate_pt_update (NULL);
		break;

	    case LASTSPCL +5 :
		/* flip functions DEL and DELCH of the ascii <Del> key */
		flip_flg = ! ( (val[0] == 0) || (val[0] == '0') );
		if ( flip_flg ) (void) itswapdeldchar (del_strg, 0, NULL);
		break;
	}
    }
}

/* standard character translate, using stdxlate[] array */
xlate0 (chr)
#ifdef UNSCHAR
Uchar chr;
#else
int chr;
#endif
{
    unsigned char c;

#ifndef UNSCHAR
    chr &= 0377;
#endif

    if ( (chr >= FIRSTSPCL) && (chr <= LASTSPCL) ) {
	c = (stdxlate [chr - FIRSTSPCL]);
	put_graph_char (c, graph_flg);
    } else if (chr) putchar (chr);
}

/* standard character translate, using stdxlate[] array */
/* except using '^' for ESCCHAR */
xlate1 (chr)
#ifdef UNSCHAR
Uchar chr;
#else
int chr;
#endif
{
#ifndef UNSCHAR
    chr &= 0377;
#endif
    if ( chr == ESCCHAR ) putchar ('^');
    else xlate0 (chr);
/*
    else if (chr >= FIRSTSPCL)
	putchar (stdxlate[chr - FIRSTSPCL]);
    else
	if (chr) putchar (chr);
*/
}

kini_nocbreak()
{
#ifdef  CBREAK
    cbreakflg = NO;
#endif
}


Flag in_std_p ();

in_std (lexp, count)
char *lexp;
int *count;
{
    int nr;
    Uint chr;
    char *icp;
    char *ocp;

    icp = ocp = lexp;
    nr = *count;
    for (; nr > 0; nr--) {
		if (in_std_p (&icp, &ocp, &nr)) break;
    }
    Block {
	Reg1 int conv;
	*count = nr;     /* number left over - still raw */
	conv = ocp - lexp;
	while (nr-- > 0)
	    *ocp++ = *icp++;
	return conv;
    }
}

Flag in_std_p (ic, oc, n)
	char **ic, **oc;
	int (*n);
{
	char chr;

	/* RAW MODE on V7 inputs all 8 bits so we and with 0177 */
	if ((chr = *(*ic)++ & 0177) >= 32) {
	    if (chr == 0177)            /* DEL KEY KLUDGE FOR NOW */
		*(*oc)++ = CCBACKSPACE;
	    else
		*(*oc)++ = chr;
	}
	else if (chr == MY_CTRL ('X')) {
	    if (*n < 2) {
		(*ic)--;
		return (YES);
	    }
	    (*n)--;
	    chr = *(*ic)++ & 0177;
	    switch (chr) {
	    case MY_CTRL ('a'):
		*(*oc)++ = CCSETFILE;
		break;
	    case MY_CTRL ('b'):
		*(*oc)++ = CCSPLIT;
		break;
	    case MY_CTRL ('c'):
		*(*oc)++ = CCCTRLQUOTE;
		break;
	    case MY_CTRL ('e'):
		*(*oc)++ = CCERASE;
		break;
	    case MY_CTRL ('h'):
		*(*oc)++ = CCLWINDOW;
		break;
	    case MY_CTRL ('j'):
		*(*oc)++ = CCJOIN;
		break;
	    case MY_CTRL ('l'):
		*(*oc)++ = CCRWINDOW;
		break;
	    case MY_CTRL ('n'):
		*(*oc)++ = CCDWORD;
		break;
	    case MY_CTRL ('r'):
		*(*oc)++ = CCREPLACE;
		break;
	    case MY_CTRL ('t'):
		*(*oc)++ = CCTABS;
		break;
	    case MY_CTRL ('u'):
		*(*oc)++ = CCBACKTAB;
		break;
	    case MY_CTRL ('w'):
		*(*oc)++ = CCCHWINDOW;
		break;
	    default:
		*(*oc)++ = CCUNAS1;
		break;
	    }
	}
	else
	    *(*oc)++ = lexstd[chr];
	return (NO);
}

S_kbd kb_std = {
/* kb_inlex */  in_std,
/* kb_init  */  nop,
/* kb_end   */  nop,
};

