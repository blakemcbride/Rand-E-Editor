/*
 * file e.h - "include" file used by all
 **/

#ifdef COMMENT
	Copyright abandoned, 1983, The Rand Corporation
#endif

/* various options.  Value can be modified by c.env.h or localenv.h */
#undef  CHAR7BITS       /* 7 bits only ASCII character set support */
#define TRACK           /* track command */
#define RANGE           /* range command */
#define NAME            /* name command */
#define DELETE          /* delete command */
#define KBFILE          /* -kbfile option */
#define SVINPLACE       /* in-place saves */
#define DUPCMD          /* duplicated commands on command line */
#define TABFILE         /* tabfile command */
#define ELOGIN          /* try to run as a login program */
#define DOCALL          /* call command -- should be defunct */
#define DOSHELL         /* shell command -- should be changed */
#define DOCOMMAND       /* command command */
#define BULLETS         /* border bullets */
#define NOTRACKS        /* -notracks option */
#define GETPATH         /* getpath routines, used by e.ru.c and tabfile */
#define LMCOPT          /* -dk for keyb=standard */
#define LMCHELP         /* help function */
#define LMCSTATE        /* -state option */
#undef  LMCVBELL        /* visible bell support */
#define LMCDWORD        /* delete word command */
#define LMCAUTO         /* power typing mode */
#define LMCCMDS         /* enable function key extensions */
#define LMCGO           /* special case go command <CMD>###<RET> */
#define LMCCASE         /* CCASE and CCAPS commands */
#define LMCTRAK         /* make TRACK and RANGE commands toggle */
#define LMCMARG         /* tab and margin tics (cute) */
#undef  LMCLDC          /* Tcap.c uses line drawing defs for windows */
#define LMCSRMFIX       /* Replace "stuck at rt margin" with reasonable sub */
#define V19             /* force version 19 --- Must be defined --- */

#ifdef  V19
#define LMCV19          /* Version 19 state file */
#else
#ifdef  LMCAUTO
#define LMCV19          /* forced by V19 or LMCAUTO */
#endif
#endif

#include <c_env.h>
#include <localenv.h>

#ifdef UNIXV7
#include <unistd.h>
#include <sys/types.h>
#endif

#include <la.h>     /* ff.h is included by la.h */

/* IMPORTANT: NOFILE is defined in ff.h -- BE SURE it is the true number of
 * file descriptors you are allowed to have open! */


#define Block

/* phototypesetter v7 c compiler has a bug with items declared as chars
 * If the variable is used as an index i.e. inside [], then it is likely
 * to get movb'ed to a register, which is then used as the index,
 * including the undefined upper byte.  Also true of things declared
 * "register char"
 * By assigning "Z" to the register, a sign extension will occur
 **/

/* Char and Short are used to point out the minimum storage required for
 *   the item.  It may be that for a given compiler, int will take up no
 *   more storage and actually execute faster for one or both of these,
 *   so they should be defined to be ints for that case.
 *   Especially, note that defining these to be ints gets around bugs in
 *   both the Ritchie PDP-11 compiler and the Johnson VAX compiler regarding
 *   declaring types smaller than int in registers.
 **/
#define Char  int
#define UChar unsigned int
#define Short int

#define Uint unsigned int

#define Z 0  /* define to "zero" for the VAX compiler bug if Char and Short */
	     /* above aren't both defined to int */
#include "e.t.h"

/* For each type there is Type and Atype.
 * use AType for array and structure member declarations,
 * and use Type everywhere else
 **/
typedef Char  Flag;             /* YES or NO */
typedef char  AFlag;            /* YES or NO */
typedef Char  Small;            /* small integer that will fit into char */
typedef char  ASmall;           /* small integer that will fit into char */
#ifdef LA_LONGFILES
typedef La_linepos Nlines;      /* number of lines in a file */
typedef La_linepos ANlines;     /* number of lines in a file */
#else
typedef La_linepos Nlines;      /* number of lines in a file */
typedef La_linepos ANlines;     /* number of lines in a file */
#endif
#ifdef LA_LONGLINES
typedef La_linesize Ncols;      /* number of columns in a line */
typedef La_linesize ANcols;     /* number of columns in a line */
#else
typedef int         Ncols;      /* number of columns in a line */
typedef int         ANcols;     /* number of columns in a line */
#endif
#if 0   /* old fation, version < 19.58 */
| typedef Char  Fd;               /* unix file descriptor */
| typedef char  AFd;              /* unix file descriptor */
| typedef Char  Fn;               /* index into files we are editing */
| typedef char  AFn;              /* index into files we are editing */
#endif
typedef int   Fd;               /* unix file descriptor */
typedef int   AFd;              /* unix file descriptor */
typedef int   Fn;               /* index into files we are editing */
typedef int   AFn;              /* index into files we are editing */
typedef Small Cmdret;           /* comand completion status */

#ifdef NOSIGNEDCHAR
#define Uchar char
#ifndef UNSCHAR
#define UNSCHAR
#endif
#else
#ifdef UNSCHAR
#define Uchar unsigned char
#else
#define Uchar char
#endif
#endif

#ifdef UNIXV6
#define wait        waita       /* wait(a) waita () */
#define sgttyb      sgtty
#define sg_ispeed   sg_ispd
#define sg_ospeed   sg_ospd
#define sg_flags    sg_flag
#define ALLDELAY    (NLDELAY | TBDELAY | CRDELAY | VTDELAY)
#define B4800       12
#define st_mode     st_flags
#define S_IREAD     IREAD
#define S_IWRITE    IWRITE
#define S_IEXEC     IEXEC
#define S_IFMT      IFMT
#define S_IFDIR     IFDIR
#define S_IFCHR     IFCHR
#define S_IFBLK     IFBLK
#define S_IFREG     0
#define SIGKILL     SIGKIL
#define SIGALRM     SIGCLK
#define SIGQUIT     SIGQIT
#endif

#ifdef  NDIR
#define FNSTRLEN 255            /* max allowable filename string length */
#else
#define FNSTRLEN 14             /* max allowable filename string length */
#endif

#ifndef abs
#define abs(a) ((a)<0?(-(a)):(a))
#endif

#define min(a,b) (((a) < (b))? (a): (b))
#define max(a,b) (((a) > (b))? (a): (b))
#define Goret(a) {retval= a;goto ret;}
#ifndef YES
#define YES 1
#define NO  0
#endif
/* XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
#define feoferr(p)         (((p)->_flag&(_IOERR|_IOEOF))!=0)
   XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX */
#define feoferr(p)         (feof(p) | ferror(p))

# ifdef UNIXV6
# define ENAME "/bin/e" /* this will get exec'ed after forked shell */
# endif

/*  define RUNSAFE               * see e.ru.c */
#define MESG_NO                 /* if messages to be disallowed during edit */
/* #define SIGARG                * if signal# passed as arg to signal(SIG) */
#define LOGINNAME "E"
/* #define CLEARONEXIT           * if defined, clears screen on exit    */
/* #define DEBUGDEF              * misc debugging stuff                 */
# define sfree(c) free (c)

#define CATCHSIGS               /* ignore signals */
#define DUMPONFATAL             /* dump always on call to fatal (..)    */
#define FATALEXDUMP -1          /* "exit dump" command given */
#define FATALMEM 0              /* out of memory */
#define FATALIO  1              /* fatal I/O err */
#define FATALSIG 2              /* signal caught */
#define FATALBUG 3              /* bug */
#define LAFATALBUG 4            /* bug in LA Package */
#define DEBUGFILE   "e.dbg"
#define PGMOTION (key==CCPLPAGE||key==CCPLLINE||key==CCMIPAGE||key==CCMILINE)

#ifdef CHAR7BITS
#define ISCTRLCHAR(ch)   ((ch)  < 040 || 0177 <= (ch))
#else
#define ISCTRLCHAR(ch)   ( (((ch)  & 0177) < 040) || ((ch)  == 0177) )
#endif
#define CTRLCHAR         (ISCTRLCHAR(key))

#define NHORIZBORDERS 2         /* number of horiz borders per window   */
#ifndef VBSTDIO
#define SMOOTHOUT               /* if defined, try to smooth output     */
				/* to terminal by doing flushing at     */
				/* line boundaries near end of buffer   */
#endif
#define TABCOL 8                /* num of cols per tab character        */

#define NENTERLINES 1           /* number of ENTER lines on screen      */
#define NINFOLINES 1            /* number of ENTER lines on screen      */
#define NPARAMLINES (NENTERLINES + NINFOLINES)

#define FILEMODE 0644           /* mode of editor-created files         */
#define MAXTYP 25               /* # of modifying chars typed before    */
				/* keyfile buffer is flushed            */
#define DPLYPOL 4               /* how often to poll dintrup            */
#define SRCHPOL 50              /* how often to poll sintrup            */
#define EXECTIM 5               /* alarm time limit for Exec function   */

#define NTABS 80

/* switches for mesg() note: rightmost 3 bits are reserved for arg count */
#define TELSTRT 0010    /* Start a new message                      */
#define TELSTOP 0020    /* End of this message                      */
#define TELCLR  0040    /* Clear rest of Command Line after message */
#define TELDONE 0060    /* (TELSTOP | TELDONE)                      */
#define TELALL  0070    /* (TELSTRT | TELDONE)                      */
#define TELERR  0100    /* This is an error message.                */
#define ERRSTRT 0110
#define ERRSTOP 0120    /* no more to write     */
#define ERRCLR  0140    /* clear rest of line   */
#define ERRDONE 0160
#define ERRALL  0170

#ifdef LMCSRMFIX
extern Flag optstick;
#endif


/* the struct markenv was initialy define in e.m.h */
struct markenv
{
    Nlines  mrkwinlin;
    ANcols  mrkwincol;
    ASlines mrklin;
    Scols   mrkcol;
};

/* workspace - two per window: wksp and altwksp
 **/
typedef struct workspace {
    La_stream las;              /* lastream opened for this workspace   */
    AFn     wfile;              /* File number of file - 0 if none      */
    ASlines clin;               /* cursorline when inactive             */
    AScols  ccol;               /* curorcol when inactive               */
    ANlines wlin;               /* line no of ulhc of screen            */
    ANcols  wcol;               /* col no of column 0 of screen         */
    ANlines rngline;            /* start line of search range */
    La_stream *brnglas;         /* beginning of range, if set */
    La_stream *ernglas;         /* end of range, if set */
    ASmall wkflags;
    struct markenv wkpos;       /* last goto position */
} S_wksp;
/* S_wksp flags: */
# define RANGESET 1             /* RANGE is set */

extern
S_wksp  *curwksp;


extern
La_stream *curlas;


/* The routine "read2" in e.t.c that gets characters for the editor
 * reads NREAD characters from the keyboard into a char array and then
 * takes them out one at a time.  If a macro is being executed, then
 * read2 is pointed at a char array that contains the keystrokes of the
 * macro.  When the macro buffer is exhausted, then read2 goes back to
 * reading the previous buffer; thus macros can be nested.
 **/
#define NREAD 32

typedef struct inp
{   int         icnt;   /* no. of chars left in the buffer */
    char       *iptr;   /* pointer into the current position in the buffer */
    char       *ibase;  /* where the base of the buffer is */
    struct inp *iprevinp;   /* pointer to previous structure for nesting */
} S_inp;

extern char  keybuf[NREAD];

#define STDIN   0
#define STDOUT  1
#define STDERR  2
#define MAXSTREAMS NOFILE
/* We need some channels for overhead stuff:
 *  0 - keyboard input
 *  1 - screen output
 *      change file
 *      (not yet: fsd file - future feature of la package)
 *      keystroke file
 *      replay input file
 *      pipe[0] --                     -- origfd
 *      pipe[1]  | for run    for save |  tempfd
 *      pipe[2] --                     --
 *      (pipe[2] not needed if RUNSAFE)
 **/
#ifdef RUNSAFE
#define WORKFILES 7
#else
#define WORKFILES 8
#endif
#define MAXOPENS (MAXSTREAMS - WORKFILES)
#define MAXFILES (MAXOPENS + 10)

#ifdef VBSTDIO
char *outbuf;           /* stdout buf */
#endif

extern Fd nopens;

/* There is an entry in each of these arrays for every file we have open
 * for editing.  The correct type for an array index is an Fn.
 **/
#define NULLFILE  0     /* This is handy since workspaces are calloc-ed */
#define CHGFILE   1
#define PICKFILE  2     /* file where picks go. Gets special */
			/* consideration: can't be renamed, etc.        */

#define OLDLFILE  3     /* file where closes go. Gets special */
			/* consideration: can't be renamed, etc.        */
#define NTMPFILES 2

#define FIRSTFILE PICKFILE

extern La_stream     fnlas[];       /* first La_stream open for the file    */
extern char         *tmpnames[];    /* names for tmp files */
extern char         *names[];       /* current name of the file            */
extern char         *oldnames[];    /* if != 0, orig name before renamed   */
extern S_wksp        lastlook[];
extern short         fileflags[];
#define INUSE        1              /* this array element is in use        */
#define DWRITEABLE   2              /* directory is writeable              */
#define FWRITEABLE   4              /* file is writeable                   */
#define CANMODIFY  010              /* ok to modify the file               */
#define INPLACE    020              /* to be saved in place                */
/*                 040                 not used                            */
#define SAVED     0100              /* was saved during the session        */
			/* A file can have no more than one of the
			 * following three bits set.
			 * The same name can appear in more than one fn,
			 *  but only in the following combinations:
			 *  names[i] (DELETED)    == names[j] (NEW)
			 *  names[i] (DELETED)    == names[j] (RENAMED)
			 *  oldnames[j] (RENAMED) == names[j] (NEW)
			 * If (NEW | DELETED | RENAMED) == 0
			 *   file exists and we are using it               */
#define NEW       0200  /* doesn't exist yet, we want to create it         */
#define DELETED   0400  /* exists, and we'll delete it on exit             */
#define RENAMED  01000  /* exists and we will rename it on exit            */
#define UPDATE   02000  /* update this file on exit */
#ifdef SYMLINKS
#define SYMLINK  04000  /* file pointed to by a symbolic link */
#endif

extern
Fn      curfile;

/* This array is indexed by unix file descriptor */

/*
 window - describes a viewing window with file
   all marg values, and ltext and ttext, are limits relative
	to (0,0) = ulhc of screen.  the other six limits are
	relative to ltext and ttext.
  */
typedef struct window
{
    S_wksp *wksp;               /* workspace window is showing          */
    S_wksp *altwksp;            /* alternate workspace                  */
    ASmall  prevwin;            /* number of the ancester win           */
				/* boundaries of text within window     */
				/*  may be same as or one inside margins*/
    ASlines ttext;              /* rel to top of full screen            */
    AScols   ltext;             /* rel to left of full screen           */
    AScols   rtext;             /*  = width - 1                         */
    ASlines btext;              /*  = height - 1                        */
    ASlines tmarg;              /*  rel to upper left of full screen    */
    AScols   lmarg;             /* margins                              */
    AScols   rmarg;
    ASlines bmarg;
    ASlines tedit;
    AScols   ledit;             /* edit window limits on screen         */
    AScols   redit;
    ASlines bedit;
    Slines  size;               /* arrays size of : firstcol, lastcol, lmchars, rmchars */
    AScols  *firstcol;          /* first col containing nonblank        */
    AScols  *lastcol;           /* last col containing nonblank         */
    char   *lmchars;            /* left margin characters               */
    char   *rmchars;            /* right margin characters              */
    AFlag   winflgs;            /* flags */
    Nlines  plline;             /* default plus a line         */
    Nlines  miline;             /* default minus a line        */
    Nlines  plpage;             /* default plus a page         */
    Nlines  mipage;             /* default minus a page        */
    Ncols   lwin;               /* default window left         */
    Ncols   rwin;               /* default window right        */
    ASlines clin;               /* cursorline on switchwindow (for resize) */
    AScols  ccol;               /* cursorline on switchwindow (for resize) */
} S_window;
/* S_window flags: */
#define TRACKSET 1              /* track wksp and altwksp */

#define SWINDOW (sizeof (S_window)) /* size in bytes of window */

#define MAXWINLIST 40   /* should be a linked list - not an array */
extern
S_window       *winlist[MAXWINLIST],
	       *curwin,         /* current editing win                  */
		wholescreen,    /* whole screen                         */
		infowin,        /* window for info                      */
		enterwin;       /* window for CMD and ARG               */
extern
Small   nwinlist;

#define COLMOVED    8
#define LINMOVED    4
#define WCOLMOVED   2
#define WLINMOVED   1
#define WINMOVED    (WCOLMOVED | WLINMOVED)
#define CURSMOVED   (COLMOVED | LINMOVED)

/*      savebuf - structure that describes a pick or delete buffer      */

typedef struct savebuf {
    La_stream buflas;
    ANcols  ncols;
} S_svbuf;


/* input control character assignments */
/*  Must be in range 000 ... 037  0177...0237 for 8 bits characters (ASCII) */

#define CCCMD           000 /* ^@ enter parameter       */
#define CCLWINDOW       001 /* ^A window left           */
#define CCSETFILE       002 /* ^B set file              */
#define CCINT           003 /* ^C interrupt         *** was chgwin */
#define CCOPEN          004 /* ^D insert                */
#define CCMISRCH        005 /* ^E minus search          */
#define CCCLOSE         006 /* ^F delete                */
#define CCMARK          007 /* ^G mark a spot       *** was PUT */
#define CCMOVELEFT      010 /* ^H backspace             */
#define CCTAB           011 /* ^I tab                   */
#define CCMOVEDOWN      012 /* ^J move down 1 line      */
#define CCHOME          013 /* ^K home cursor           */
#define CCPICK          014 /* ^L pick                  */
#define CCRETURN        015 /* ^M return                */
#define CCMOVEUP        016 /* ^N move up 1 lin         */
#define CCINSMODE       017 /* ^O insert mode           */
#define CCREPLACE       020 /* ^P replace           *** was GOTO */
#define CCMIPAGE        021 /* ^Q minus a page          */
#define CCPLSRCH        022 /* ^R plus search           */
#define CCRWINDOW       023 /* ^S window right          */
#define CCPLLINE        024 /* ^T minus a line          */
#define CCDELCH         025 /* ^U character delete      */
#define CCRWORD         026 /* ^V move right one word   */
#define CCMILINE        027 /* ^W plus a line           */
#define CCLWORD         030 /* ^X move left one word    */
#define CCPLPAGE        031 /* ^Y plus a page           */
#define CCCHWINDOW      032 /* ^Z change window     *** was WINDOW */
#define CCTABS          033 /* ^[ set tabs              */
#define CCCTRLQUOTE     034 /* ^\ knockdown next char   */
#define CCBACKTAB       035 /* ^] tab left              */
#define CCBACKSPACE     036 /* ^^ backspace and erase   */
#define CCMOVERIGHT     037 /* ^_ forward move          */
#define CCDEL          0177 /* <del>    -- not assigned --  *** was EXIT */
#define CCSTOP         0200 /* *@ stop replay           */
#define CCERASE        0201 /* *A erase                 */
#define CCUNAS1        0202 /* *B -- not assigned --    */
#define CCSPLIT        0203 /* *C split                 */
#define CCJOIN         0204 /* *D join                  */

#ifdef LMCCMDS
#define CCEXIT         0205
#define CCABORT        0206
#define CCREDRAW       0207
#define CCCLRTABS      0210
#define CCCENTER       0211
#define CCFILL         0212
#define CCJUSTIFY      0213
#define CCCLEAR        0214
#define CCTRACK        0215
#define CCBOX          0216
#define CCSTOPX        0217
#define CCQUIT         0220
#define CCCOVER        0221
#define CCOVERLAY      0222
#define CCBLOT         0223

#ifdef LMCHELP
#define CCHELP         0224
#else
#define CCHELP         CCUNAS1

#endif
#ifdef LMCCASE
#define CCCCASE        0225
#define CCCAPS         0226
#else
#define CCCCASE        CCUNAS1
#define CCCAPS         CCUNAS1
#endif

#ifdef LMCAUTO
#define CCAUTOFILL     0227
#else
#define CCAUTOFILL     CCUNAS1
#endif

#define CCRANGE        0230
#define CCNULL         0231

#ifdef LMCDWORD
#define CCDWORD        0232
#else
#define CCDWORD        CCUNAS1
#endif

#define CCTICK         0233

		    /* 0234 is currently free */

#define CCFNAVIG       0235
#define CCFLIST        0236

/* Resize entry code : to be used for replay only
 *   do not use for keyboard assignement
 * CCRESIZE is followed by <(line +32), (column +32), CCRETURN>
 */
#define CCRESIZE       0237

#else /* -LMCCMDS */
#define CCEXIT         CCUNAS1
#define CCABORT        CCUNAS1
#define CCREDRAW       CCUNAS1
#define CCCLRTABS      CCUNAS1
#define CCCENTER       CCUNAS1
#define CCFILL         CCUNAS1
#define CCJUSTIFY      CCUNAS1
#define CCCLEAR        CCUNAS1
#define CCTRACK        CCUNAS1
#define CCBOX          CCUNAS1
#define CCSTOPX        CCUNAS1
#define CCQUIT         CCUNAS1
#define CCCOVER        CCUNAS1
#define CCOVERLAY      CCUNAS1
#define CCBLOT         CCUNAS1
#define CCHELP         CCUNAS1
#define CCCCASE        CCUNAS1
#define CCCAPS         CCUNAS1
#define CCAUTOFILL     CCUNAS1
#define CCRANGE        CCUNAS1
#define CCNULL         CCUNAS1
#define CCDWORD        CCUNAS1
#endif  /* -LMCCMDS */

extern
Scols   cursorcol;              /* physical screen position of cursor   */
extern
Slines  cursorline;             /*  from (0,0)=ulhc of text in window   */

/* chgborders is set outside putup (), then looked at by putup ()
 *                              then putup resets it to 1 */
extern Small chgborders;        /* 0: don't change the borders          */
				/* 1: update them */
				/* 2: set them to inactive (dots) */

extern
Small   numtyp;                 /* number of text chars since last      */
				/* keyfile flush                        */

#define MAXMOTIONS 32
extern
ASmall cntlmotions[MAXMOTIONS];
#define UP 1    /* Up           */
#define DN 2    /* Down         */
#define RN 3    /* Return       */
#define HO 4    /* Home         */
#define RT 5    /* Right        */
#define LT 6    /* Left         */
#define TB 7    /* Tab          */
#define BT 8    /* Backtab      */

extern
char   *myname,
       *mypath,
       *progname;
extern
Flag    inplace;                /* do in-place file updates?            */

extern
Flag    smoothscroll;           /* do scroll putups one line at a time */
extern
Flag    singlescroll;           /* do scrolling one line at a time */

extern ANcols *tabs;            /* array of tabstops */
extern Short   stabs;           /* number of tabs we have alloced room for */
extern Short   ntabs;           /* number of tabs set */

extern char blanks [];          /* blank screen line */

/* Argument to getkey is one of these: */
#define WAIT_KEY      0 /* wait for a char, ignore interrupted read calls. */
#define PEEK_KEY      1 /* peek for a char */
#define WAIT_PEEK_KEY 2 /* wait for a char, then peek at it;      */
			/* if read is interrupted, return NOCHAR. */

#define NOCHAR 0400
extern Char key;             /* last char read from tty */
extern Flag keyused;         /* last char read from tty has been used */

/* default parameters */
extern Nlines defplline,        /* default plus a line          */
	      defplpage,        /* default minus a line         */
	      defmiline,        /* default plus a page          */
	      defmipage;        /* default minus a page         */
extern Ncols  deflwin,          /* default window left          */
	      defrwin;          /* default window right         */
extern char  deffile[];         /* default filename             */
extern Fn    deffn;             /* file descriptor of default file      */

extern
Short   linewidth,              /* used by just, fill, center           */
	tmplinewidth;           /* used on return from parsing "width=xx" */
#ifdef LMCAUTO
extern Flag autofill;           /* YES if in autofill mode */
extern Ncols autolmarg;         /* left margin on autofill */
#endif

extern
char   *paramv;                 /* * globals for param read routine     */
extern
Small   paramtype;

/* array to hold current line being edited */
/* tabs are expanded on input. */
extern
char   *cline;                  /* array holding current line           */
extern
Short   lcline,                 /* length of cline buffer               */
	ncline,                 /* number of chars in current line      */
	icline;                 /* increment for next expansion         */
extern
Flag    fcline,                 /* flag - has line been changed ?       */
	cline8,                 /* line may contain chars with 8th bit set */
	ecline,                 /* line may contain ESCCHAR(s)          */
	xcline;                 /* cline was beyond end of file         */
extern
Fn      clinefn;                /* Fn of cline                          */
extern
Nlines  clineno;                /* line number in workspace of cline    */
extern
La_stream *clinelas;            /* La_stream of current line */

extern char
	prebak[],               /* text to precede/follow               */
	postbak[];              /* original name for backup file        */

extern
char *searchkey;

#if 0   /* old definition, version < 19.58 */
| extern
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
|    groupid;
#endif

#ifdef UNIXV7
extern uid_t userid;
extern pid_t groupid;
#else
extern int userid,
extern int groupid;
#endif

extern
FILE   *keyfile;                /* channel number of keystroke file     */

extern
Fd      inputfile;              /* channel number of command input file */
extern
Flag    intok;                  /* enable la_int ().  Normally NO, */
				/* set to YES for duration desired */
extern
Small   intrupnum;              /* how often to call intrup             */
extern
Flag    alarmed;

#define NUMFFBUFS       10

extern char putscbuf[];

extern Flag windowsup;   /* screen is in use for windows */

#define d_put(c) (putscbuf[0]=(c),d_write(putscbuf,1))


extern short revision;  /* revision number of this e */
extern short subrev;    /* sub-revision number of this e */

#if 0
extern
Char evrsn;   /* the character used in the chng, strt, & keys filename   */
		/* '0', '1', ...        */
#endif

extern Flag notracks;   /* don't use or leave any strt file */
extern Flag norecover,
	    replaying,
	    recovering,
	    silent;     /* replaying silently */
extern Flag keysmoved;  /* keys file has been moved to backup */

/*********/
/* e.d.c */

#ifdef LMCVBELL
extern char *vbell;
extern Flag VBell;
#endif
extern Flag NoBell;

/* e.c file */
extern FILE *dbgfile;
extern void debug_info (char *func, char *file, int line);
extern Flag helpflg, verbose_helpflg, dbgflg;
extern int DebugVal;

/* these used to be in e.c only */

extern
Flag cmdmode;
extern
Flag patmode;           /* is YES when in patternmode Added 10/18/82 MAB */
extern
Flag insmode;           /* is YES when in insertmode                      */
extern
Nlines parmlines;       /* lines in numeric arg                         */
extern
Ncols parmcols;         /* columns in numeric arg e.g. 8 in "45x8"      */

extern
char *shpath;

/* Look up structures */
/* ------------------ */
typedef struct lookuptbl {
    char *str;
    short val;
} S_looktbl;

typedef struct lookupstruct {
    S_looktbl *table;               /* user defined, in alpahbetic order */
    int table_sz;                   /* number of defined items */
    S_looktbl *(*storage)[];        /* storage for sorted and alias tables */
    const int sizeof_storage;       /* sizeof (byte) of storage */
    S_looktbl *(*sorted_table)[];   /* sorted table */
    int sorted_table_sz;            /* number of defined items in sorted_table */
    S_looktbl *(*alias_table)[];    /* aliases table, NULL if not in use */
    int alias_table_sz;             /* number of defined items in alias_table */
    int (* one_help) (S_looktbl *, char *); /* help process routine for one item */
    const Flag as_alias;            /* with or without aliases table */
    const int type;                 /* 0 unknown, 1 command, 2 key function */
    S_looktbl *major_name_table;    /* if as_alias, for complex aliases */
    int major_name_table_sz;        /* nb of items */
} S_lookstruct;


extern
long strttime;  /* time of start of session */

extern
Flag loginflg;  /* = 1 if this is a login process */

extern Flag ischild;    /* YES if this is a child process */

extern int zero;

#ifdef LMCLDC

/* file e.tt.c */

extern
Flag line_draw;       /* we are in line drawing mode */

#endif
/* Functions */

/* ../lib/getshort.c */
extern short getshort ();

/* ../lib/getlong.c */
extern long getlong ();

/* e.cm.c */
extern Cmdret command ();
extern Cmdret gotocmd ();
extern Cmdret doupdate ();
/* e.dif.c */
extern Cmdret diff ();
/* e.e.c */
extern Cmdret areacmd ();
extern Cmdret splitmark ();
extern Cmdret splitlines ();
extern Cmdret joinmark ();
/* e.f.c */
extern Flag multlinks ();
extern Flag fmultlinks ();
extern Fn hvname ();
extern Fn hvoldname ();
extern Fn hvdelname ();
/* e.h.c */
#ifdef LMCHELP
extern Cmdret morehelp();
#endif
/* e.la.c */
extern Flag putline ();
extern Ncols dechars ();
extern Flag extend ();
extern Nlines lincnt ();
/* e.mk.c */
extern Nlines topmark ();
extern Ncols leftmark ();
extern Flag gtumark ();
extern Small exchmark ();
/* e.nm.c */
extern Cmdret name ();
extern Cmdret delete ();
extern Flag dotdot ();
/* e.p.c */
extern Small printchar ();
/* e.pa.c */
extern Small getpartype ();
extern char *getword ();
extern Cmdret scanopts ();
extern Small getopteq ();
extern Small doeq ();
/* e.put.c */
extern Cmdret insert ();
extern Cmdret insbuf ();
/* e.q.c */
extern Cmdret eexit ();
extern Flag saveall ();
extern Flag savestate ();
/* e.ru.c */
extern Cmdret print ();
extern Cmdret filter ();
extern Cmdret filtmark ();
extern Cmdret filtlines ();
extern Cmdret run ();
/* XXXXXXXXXXXXXXXXXXXXXX
extern Cmdret runlines ();
extern Flag dowait ();
   XXXXXXXXXXXXXXXXXXXXXX */
extern Flag receive ();
/* e.sb.c */
extern char *getmypath ();
extern char *gsalloc ();
extern char *salloc ();
extern char *okalloc ();
extern char *append ();
extern char *copy ();
extern char *s2i ();
extern char *itoa();
extern Flag mv ();
extern Flag okwrite ();
extern Small filecopy ();
/* XXXXXXXXXXXXXXXXXXXXXXXX */
extern void sig ();
extern void srprintf ();
/* e.se.c */
extern void dosearch ();
extern Cmdret replace ();
extern Small strsearch ();
extern Ncols skeylen ();
/* e.sv.c */
extern Cmdret save ();
extern Flag savefile ();
extern Flag svrename ();
/* e.t.c */
extern Small vertmvwin ();
extern Small horzmvwin ();
extern Small movewin ();
extern unsigned Short getkey ();
extern Flag dintrup ();
extern Flag la_int();
extern Flag sintrup ();
/* e.tb.c */
extern Cmdret dotab ();
extern Cmdret dotabs ();
extern Small getptabs ();
extern Cmdret tabfile ();
extern Flag gettabs ();
/* e.u.c */
extern Cmdret use ();
extern Small editfile ();
/* extern Fn getnxfn (); */
/* e.wi.c */
extern S_window *setupwindow ();
/* e.wk.c */
extern Flag swfile ();

extern void chgwindow ();
extern void clean ();
extern void cleanup ();
extern void clrbul ();
extern void credisplay ();
extern void d_write ();
extern void dbgpr ();
extern void dobullets ();
extern void drawborders ();
extern void eddeffile ();
extern void edscrfile ();
extern void exchgwksp ();
extern void excline ();
extern void fatal ();
extern void fatalpr ();
extern void fixtty ();
extern void flushkeys ();
extern void fresh ();
extern void getarg ();
extern void getline ();
extern void getpath ();
extern void gotomvwin ();
extern void igsig ();
extern void rand_info ();
extern void infoprange ();
extern void inforange ();
extern void infotrack ();
extern void limitcursor ();
extern void mainloop ();
extern void mark ();
extern void mesg ();
extern void movecursor ();
extern void param ();
extern void poscursor ();
extern void putbks ();
extern void putch ();
extern void putup ();
extern void putupwin ();
extern void redisplay ();
extern void releasewk ();
extern void removewindow ();
extern void replkey ();
extern void restcurs ();
extern void savecurs ();
extern void savewksp ();
extern void screenexit ();
extern void sctab ();
extern void setbul ();
extern void setitty ();
extern void setotty ();
extern void switchfile ();
extern void switchwindow ();
extern void tabevery ();
extern void tglpatmode ();      /* added MAB */
extern void tglinsmode ();
extern Flag unmark ();
extern void writekeys ();
