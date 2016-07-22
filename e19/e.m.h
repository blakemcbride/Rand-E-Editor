/*
 * file e.m.h - some specialized include stuff
 **/

#ifdef COMMENT
	Copyright abandoned, 1983, The Rand Corporation
#endif

extern char *cmdname;   /* the full name of the command given */
extern char *cmdopstr;  /* command operand string - points to the rest of
			   paramv after the command */
extern char *opstr;     /* first word in cmdopstr from a call to getword () */
extern char *nxtop;     /* next word after opstr - use for further getwords */

/* commands: <arg> command <ret> */
    /* the value 0 must not be used,
       see command (forcecmd, forceopt) function in e.cm.c
    */
#define CMDCHWINDOW     1
#define CMDEXIT         2
#define CMDLOGOUT       3
#define CMD_COMMAND     4
#define CMDCENTER       5
#define CMDMARK         6
#define CMDWINDOW       7
#define CMD_WINDOW      8
#define CMDRUN          9
#define CMDSAVE         10
#define CMDSTOP         11
#define CMDFILL         12
#define CMDJUST         13
#define CMDEDIT         14
#define CMDREDRAW       15
#define CMDGOTO         16
#define CMDSPLIT        17
#define CMD_JOIN        17
#define CMDJOIN         18
#define CMD_SPLIT       18
#define CMDPRINT        19
#define CMDCOMMAND      20
#define CMD_MARK        21
#define CMD_PICK        26 /************************************/
#define CMD_CLOSE       27 /*  these three must be consecutive */
#define CMD_ERASE       28 /************************************/
#define CMDTAB          29
#define CMD_TAB         30
#define CMDTABS         31
#define CMD_TABS        32
#define CMDTABFILE      33
#define CMD_TABFILE     34
#define CMDFEED         35
#define CMDHELP         36
#define CMDDELETE       37
#define CMDCLEAR        38
#define CMDNAME         39
#define CMDSHELL        40
#define CMDCALL         41
#define CMDMACRO        42
#define CMD_MACRO       43
#define CMDENDM         44
#define CMDSEARCH       45
#define CMD_SEARCH      46
#define CMDREPLACE      47
#define CMD_REPLACE     48
#define CMDUPDATE       49
#define CMD_UPDATE      50
#define CMDCOVER        51 /***********************************/
#define CMDOVERLAY      52 /*  these six must be consecutive  */
#define CMDUNDERLAY     53 /*                                 */
#define CMDBLOT         54 /*                                 */
#define CMD_BLOT        55 /*                                 */
#define CMDINSERT       56 /***********************************/
#define CMDUNDO         57
#ifdef DEBUGALLOC
#define CMDVERALLOC     58
#endif
#define CMDTRACK        59
#define CMD_TRACK       60
#define CMDRANGE        61
#define CMD_RANGE       62
#define CMDQRANGE       63
#define CMDSET          64
#define CMDQSET         65
#define CMDPATTERN      66
#define CMD_PATTERN     67
#ifdef LMCAUTO
#define CMDAUTO         68
#define CMD_AUTO        69
#endif
#ifdef LMCDWORD
#define CMDDWORD        70
#define CMD_DWORD       71
#endif
#define CMD_DIFF        72
#define CMDDIFF         73
#define CMDSTATS        74
#define CMDRESIZE       75
#define CMDPICK         100  /************************************/
#define CMDCLOSE        101  /*  these seven must be consecutive */
#define CMDERASE        102  /*                                  */
#define CMDOPEN         103  /*                                  */
#define CMDBOX          104  /*                                  */
#ifdef LMCCASE
#define CMDCAPS         105  /*                                  */
#define CMDCCASE        106  /************************************/
#endif

/* new command available on Unix, and LynxOS */
#define CMDCHECKSCR     110  /* check the current screen and keyboard */
#define CMDFILE         111  /* set / query file status */
#define CMDQFILE        112  /* query file status */
#define CMDVERSION      113  /* query program version */
#define CMDCD           114  /* change current directory */
#define CMDSTATUS       115  /* help status equivalent */
#define CMDSHFILES      116  /* display the curent files list */
#define CMDBKBFILE      117  /* interactively build a kb file (keyboard functions assignement) */
#define CMDFLIPBKAR     118  /* flip BackArrow key between 'del' and 'dchar' */
#define CMDTICK         119  /* set a tick at current position */
#define CMD_TICK        120  /* unset tick in current file */
#define CMDQTICK        121  /* display tick state */
#define CMDBKBMAP       122  /* interactively build a keyboard map file */



/* these came from the top of e.m.c */
extern
struct loopflags {
    AFlag clear;        /* clear enterwin  */
    AFlag hold;         /* hold cmd line until next key input */
    AFlag beep;         /* beep on clearing cmd line */
    AFlag flash;        /* bullet at cursor position is to be temporarily */
    AFlag bullet;       /* redo all border bullets */
 } loopflags;

/* XXXXXXXXXXXXXXXXXXX
extern
struct loopflags loopflags;
   XXXXXXXXXXXXXXXXXXX */

extern char   *copy ();

/**/

#if 0
/* now struct markenv is defined in e.h */
struct markenv
{
    Nlines  mrkwinlin;
    ANcols  mrkwincol;
    ASlines mrklin;
    Scols   mrkcol;
};
#endif

extern
struct markenv *curmark,
	       *prevmark;

extern
Nlines  marklines;      /* display of num of lines marked */
extern
Ncols   markcols;       /* display of num of columns marked */

extern
char    mklinstr [],   /* string of display of num of lines marked */
	mkcolstr [];   /* string of display of num of lines marked */
extern
Small   infoarealen;    /* len of string of marked area display */

