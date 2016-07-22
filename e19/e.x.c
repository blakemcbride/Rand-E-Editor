/*
 * file e.x.c: external definitions
 **/

#ifdef COMMENT
	Copyright abandoned, 1983, The Rand Corporation
#endif

#include "e.h"
#include "e.e.h"
#include "e.inf.h"
#include "e.m.h"
#include "e.sg.h"
#include SIG_INCL

#ifdef VBSTDIO
char *outbuf;           /* stdout buf */
#endif

#ifndef TDIR
#define TDIR P_tmpdir   /* directory for temporaries (see stdio.h) */
#endif

Fd nopens;

La_stream     fnlas[MAXFILES];  /* first Lastream open for the file   */
char      *tmpnames[NTMPFILES] = {       /* similarly as for tmplas */
    "#p", "#o"
};

/* NB : names is set by :
	"name" routine in e.nm.c
	"editfile" routine in e.u.c
	"makescrfile" routine in e.c
*/

char      *cwdfiledir[MAXFILES];    /* current directory for the files names[i] */
char      *names[MAXFILES];
char      *oldnames[MAXFILES];
S_wksp     lastlook[MAXFILES];
short      fileflags[MAXFILES];
Flag       fileticksflags[MAXFILES]; /* tick mark in use for the file */
struct markenv fileticks[MAXFILES];  /* tick mark in the file */


S_wksp  *curwksp;

La_stream *curlas;


Fn        curfile;

S_window       *winlist[MAXWINLIST],
	       *curwin,         /* current editing window               */
		wholescreen,	/* whole screen 			*/
		infowin,        /* window for info                      */
		enterwin;       /* window for CMD                       */
Small   nwinlist;

S_svbuf qbuf[NQBUFS];

Flag optstick = NO;

/* see e.e.h */
AFn qtmpfn[NQBUFS] = {
    OLDLFILE,   /* QADJUST  */
    PICKFILE,   /* QPICK    */
    OLDLFILE,   /* QCLOSE   */
    OLDLFILE,   /* QERASE   */
    OLDLFILE,   /* QRUN     */
};


Scols   cursorcol;              /* physical screen position of cursor   */
Slines  cursorline;             /*  from (0,0)=ulhc of text in window   */

Small   chgborders = 1;

Small   numtyp;                 /* number of text chars since last      */
				/* keyfile flush			*/

/* table of motions for control chars (see defs in e.h)
	UP  1	up
	DN  2	down		 : 4 or less change cursorline
	RN  3	carriage return
	HO  4	home
	RT  5   right
	LT  6   left
	TB  7   tab
	BT  8   backtab
  */
ASmall  cntlmotions[MAXMOTIONS] = {
    0, 0, 0, 0, 0, 0, 0, 0,
    LT, TB, DN, HO, 0, RN, UP, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, BT, 0, RT,
};

char   *myname,
       *mypath,
       *progname;
Flag	inplace;		/* do in-place file updates?		*/

Flag    smoothscroll = NO;      /* do scroll putups one line at a time */
Flag    singlescroll = YES;     /* do scrolling one line at a time */

ANcols *tabs;                   /* array of tabstops */
Short   stabs = NTABS;          /* number of tabs we have alloced room for */
Short   ntabs = NTABS / 2;      /* number of set tabs */

char    blanks [MAXWIDTH+1];    /* a blanks screen line */

Char    key;                /* last char read from tty */
Flag    keyused = YES;      /* last char read from tty has been used. */

/* parameters for line motion buttons */
Nlines  defplline = 10,         /* default plus a line         */
	defmiline = 10,         /* default minus a line        */
	defplpage = 1,          /* default plus a page         */
	defmipage = 1;          /* default minus a page        */
Ncols   deflwin  = 16,          /* default window left         */
	defrwin  = 16;          /* default window right        */
/* ++XXXXXXXXXXXXXXXX deffile in now in e.c */
/* char    deffile[] = XDIR_ERR; */  /* default filename            */
/* --XXXXXXXXXXXXXXXX */
Fn      deffn     = -1;         /* file descriptor of default file */

#ifdef LMCAUTO
Short   linewidth    = 74;      /* used by just, fill, center */
Flag    autofill = NO;          /* YES indicates autofill mode */
Ncols   autolmarg = 0;          /* left margin on autofill */
#else
Short   linewidth    = 75;      /* used by just, fill, center */
#endif
Short   tmplinewidth;           /* used on return from parsing "width=xx" */

char   *paramv;                 /* * globals for param read routine     */
Small   paramtype;





/* initialize cline to be empty */
char   *cline;			/* array holding current line		*/

Short   lcline,                 /* length of cline buffer               */
	ncline;                 /* number of chars in current line      */
Short   icline = 100;           /* initial increment for expansion */

Flag    fcline,                 /* flag - has line been changed ?       */
	ecline,                 /* line contains DEL escapes            */
	cline8,                 /* line may contain chars with 8th bit set */
	xcline;                 /* cline was beyond end of file         */

Fn      clinefn;                /* Fn of cline                          */
Nlines  clineno;                /* line number in workspace of cline    */
La_stream *clinelas;            /* La_stream of current line */

/* One of the following strings must be non-null.
 * That is to say you can NOT disable the backups feature
 * If you try to do so, things won't work because fsd's wil point to oblivion.
 * It's too involved to go into here.  Has to do with the way the LA package
 * works.
 */
char    prebak[]  = ",",        /* text to precede/follow         */
	postbak[] = "";          /* original name for backup file  */

char *searchkey;

#if 0   /* old definition, version < 19.58 */
| #ifdef UNIXV7
| #if defined(__alpha) || defined(_AIX) || defined(__sun)
| int
| #else
| short
| #endif
| #endif
| #ifdef UNIXV6
| char
| #endif
|     userid,
|     groupid;
#endif

#ifdef UNIXV7
uid_t userid;
pid_t groupid;
#else
int userid,
int groupid;
#endif


int mypid;  /* current process id */

Flag    intok;      /* enable la_int () */
Small   intrupnum;  /* how often to call intrup */

Flag	alarmed;

char    putscbuf[10];

Flag windowsup = NO;

FILE   *dbgfile = NULL;

/* session running conditions */
Flag notracks = NO;
Flag norecover = NO;
Flag replaying = NO;
Flag recovering = NO;
Flag silent = NO;       /* replaying silently */
Flag keysmoved = NO;    /* keys file has been moved to backup */

/*********/
/* e.d.c */

#ifdef LMCVBELL
char *vbell = (char *)0;
Flag VBell = NO;
#endif
Flag NoBell = NO;
int DebugVal = 0;

/************/
/* e.fn.h */
/* pathnames for standard files */
char   default_tmppath [] = TDIR;
char   *tmppath = &default_tmppath [0];
#ifdef UNIXV7
char   *ttynstr;
#endif
#ifdef UNIXV6
char   *ttynstr   = "/dev/tty ";
#endif
char    scratch[] = "scratch";


/* working file names */
const char workfileprefix [] = "./.e";   /* ./ is mandatory before the teal prefix string */
    /* The '1' (session nb) may be replaced with a higher digit '2' ... '9' */
const char tmpnstr [] = "c1";    /* change file name */
const char keystr []  = "k1";    /* key strok file name */
const char bkeystr [] = "k1b";   /* backup key strock file name */
const char rstr []    = "s1";    /* state file name */

const int vrschar = 1;   /* relative position of evrsn char in tmpnstr ... */
char evrsn;   /* session nb character : '1', '2', ... '9' */

char   *la_cfile,   /* change file name */
       *keytmp,     /* key stroke file name */
       *bkeytmp,    /* backup (previous) key stroke file name */
       *rfile,      /* state file name and backup name */
       *inpfname;   /* file name for recover */

FILE   *keyfile;    /* strem of keystroke file */
Fd      inputfile;  /* file descriptor number of command input file */

/************/
/* e.sg.h */
#ifndef SYSIII
struct sgttyb instty,
	      outstty;
#endif

#ifdef  SYSIII
struct termio in_termio,                /* System III ioctl */
	      out_termio;
int fcntlsave;
#endif

#ifdef  CBREAK
#ifdef  TIOCGETC
struct tchars spchars;
Flag cbreakflg = YES;
#else
Flag cbreakflg = NO;    /* can't turn off intr and quit without TIOCGETC */
#endif
#ifdef  TIOCGLTC
struct ltchars lspchars;
#endif
#endif

Flag istyflg,
     ostyflg;

unsigned Short oldttmode;

/************/

Flag cmdmode;

Flag patmode;           /* is 1 when in patternmode Added 10/18/82 MAB */

Flag insmode;           /* is 1 when in insertmode */

Nlines parmlines;       /* lines in numeric arg */

Ncols parmcols;         /* columns in numeric arg e.g. 8 in "45x8"      */

char *shpath;

long strttime;	/* time of start of session */

Flag loginflg;  /* = 1 if this is a login process */

Flag ischild;

/************/
/* e.m.h */
char *cmdname;           /* the full name of the command given */
char *cmdopstr;
char *opstr;
char *nxtop;

/**********/
/* e.p.c  */
char *deletdwd = (char *) 0;

struct loopflags loopflags;

struct markenv *curmark,
	       *prevmark;

char    mklinstr [6],   /* string of display of num of lines marked */
	mkcolstr [6];   /* string of display of num of lines marked */

Small   infoarealen;    /* len of string of marked area display */

/************/
/* e.inf.h */

Scols inf_insert;               /* "INSERT" */
Scols inf_track;                /* "TRACK" */
Scols inf_range;                /* "=RANGE" */
#ifdef LMCAUTO
Scols inf_auto;                 /* "AUTO" */
#endif
Scols inf_mark;                 /* "MARK" */
Scols inf_area;                 /* "30x16" etc. */
Scols inf_pat;                  /* "RE" */
Scols inf_at;                   /* "At"         */
Scols inf_line;                 /* line         */
Scols inf_in;                   /* "in"         */
Scols inf_file;                 /* filename     */

Nlines  infoline;       /* line number displayed */
Fn      infofile;       /* file number of filename displayed */

Nlines  marklines;      /* display of num of lines marked */
Ncols   markcols;       /* display of num of columns marked */

/************/
/* file e.ru.c */

char *filters[] = {
    "fill",
    "just",
    "center",
    "print"
};

/* ++XXXXXXXXXXXXXXXXXXXXXXXX
char *filterpaths[] = {
    XDIR_FILL ,
    XDIR_JUST ,
    XDIR_CENTER ,
    XDIR_PRINT
};
   --XXXXXXXXXXXXXXXXXXXXXXXX */

#ifdef LMCLDC
/* file e.tt.c */

Flag line_draw = NO;  /* we are in line drawing mode */
#endif

#ifdef SUN
char _sobuf[BUFSIZ];
#endif 


/* initial setup of global storage */
/* ------------------------------- */

static struct _glb_storage {
    void * storage;
    int sz;
    } glb_storages [] = {
	names, sizeof (names),
	oldnames, sizeof (oldnames),
	cwdfiledir, sizeof (cwdfiledir),
	fileflags, sizeof (fileflags),
	fileticksflags, sizeof (fileticksflags),
	lastlook, sizeof (lastlook),
	fnlas, sizeof (fnlas),
	winlist, sizeof (winlist),
	0, 0
    };
static Flag clean_glb_storages_flg = NO;


void clean_glb_storages ()
{
    int i;
    void * stor;

    if ( clean_glb_storages_flg ) return;
    for ( i = 0 ; stor = glb_storages [i].storage ; i++ )
	memset (stor, 0, glb_storages [i].sz);
    clean_glb_storages_flg = YES;
}
