#ifdef COMMENT
--------
file e.c
    main program and startup code.
    All code in this file is used only prior to calling mainloop.
	(not any more, some service routines can be called during execution)

    This code assume that not initialised global and static variables
	are by default initialized to 0
#endif

#ifdef COMMENT
	Copyright abandoned, 1983, The Rand Corporation
#endif

#define DEF_XDIR

#include "e.h"
#include "e.e.h"
#ifdef  KBFILE
#include "e.it.h"
#endif  /* KBFILE */
#include "e.tt.h"
#include "e.wi.h"
#include "e.fn.h"
#include "e.sg.h"
#include "e.inf.h"
#include <sys/stat.h>
#include <string.h>
#ifdef SYSIII
#include <fcntl.h>
#endif /* SYSIII */
#include <unistd.h>
#include <time.h>
/*
 * #include <assert.h>
 */

extern char *getenv ();
extern char *getmypath ();
extern int mypid;
extern void msg_mark (struct markenv *, char *, char *, Flag, Flag);
extern void clean_glb_storages ();
extern void set_noX_msg (char *);
extern char default_tmppath [];

static Flag build_wkfiles (Flag bld_flg);
char * get_debug_file_name (Flag create_dir, Flag *exist);
static void parse_debug_option (char *opt_str, Flag malloc_flg);

#include SIG_INCL
#ifdef TTYNAME
extern char *ttyname ();
#endif

#define DEFAULT_TABS 4      /* default tabs setting to 4 */
#define DEFAULT_DebugVal 1


/* Flags and various data in use by Rand editing session */
/* ===================================================== */

/* Flag to remember if the state file was used to setup the session */
static Flag state_reused_flg = NO;

Flag optnocmd = NO;          /* YES = -nocmdcmd; <cmd><cmd> functions disabled */

/* Names of the host running Rand,
 *   from which computer, and display on from_host
 *   from_host and from_display are NULL if running on the local computer
 */
char *myHost, *fromHost, *fromDisplay, *fromHost_alias;
char *ssh_ip, *sshHost, *sshHost_alias; /* host of a ssh session */

/* Assumed Rand editor directory tree structure :
 * ==============================================
 * The following structure is assumed in order to found automaticaly
 *      the various pathes to services files (help, keyboard definition ...)
 *
 * The Rand pacakge directory is the directory which include :
 *       - the Rand Editor installed executable file (which cannot be called a.out)
 *       - the service programs (center, fill, run) executable files
 *       - the help files : errmsg, helpkey, recovermsg, Crashdoc
 *       - the kbfiles directory (directory of keyboard definition files)
 *   Special case : building a new version (compiling and linking)
 *       - the new executable file name must be : a.out
 *      - the help files and kbfiles directory must be at the relative
 *          position defined by helpdir constant string : ../help
 */

static char *dirpackg = NULL;   /* Rand editor package directory */
static Flag buildflg = NO;      /* newly build executable (a.out) */

static const char loopback [] = "127.0.0.1";

/* Rand package directory tree structure related names */
/* --------------------------------------------------- */
    /* default default package dir (used to extract the package sub dir) */
#ifdef DEF_XDIR
extern char def_xdir_dir [];    /* build by NewVersion script (e.r.c) */
#else
static char def_xdir_dir [] = XDIR_DIR;     /* defined in ../include/linux_localenv.h */
#endif
    /* system wide configuration directory */
static char syst_cnfg []  = "/etc";     /* computer configuration directory */
static char alt_local_cnfg []  = "/../etc";  /* local relative to pacakage */
static char local_cnfg [] = "/usr/local/etc";   /* cluster configuration dir */
    /* all-purpose keyboard definition file name */
static char universal_kbf_name [] = "universalkb";
    /* kbfiles directory relative to rand package and configurations directory */
static char kbfilesdir [] = "/kbfiles";
    /* help directory relative to the directory including a.out (just build editor) */
static char helpdir [] = "/../help";
static char execdir [] = "/../fill";
static char buildexec [] = "a.out";
    /* postfix added to the keyboard name (or terminal name) to generate the kbfile name */
static char kbfile_postfix [] = "kb";
static char kbmap_postfix [] = ".kbmap";
static char preference [] = "preferences";
static char debug [] = "debug_output.txt";
static char tmpdir [] = "/tmp";

static char *full_prgname = NULL;   /* full Rand program name */
static char *prog_fname;        /* actual Rand editor file name */
static char *pwdname = NULL;    /* current working directory */

static char *optdirpackg = NULL;    /* option dir package */
static Flag optusedflg = NO;        /* some program options in use */

char default_workingdirectory [PATH_MAX+1] = "./";

#ifdef VBSTDIO
extern unsigned char _sobuf[];
#endif

Small numargs;
Small curarg;
char *curargchar;
char **argarray;

static char *dbgname = NULL;    /* argument to -debug=xxx option */
static char *replfname = NULL;  /* argument to -replay=xxx option */

#ifdef LMCSTATE
Flag state=NO;      /* -state option */
char *statefile;    /* argument to -state=xxx option */
#endif

char stdk[] = "standard";   /* standard comipled keyboard name */
Flag stdkbdflg;     /* use the standard compiled keyboard definition */
Flag Xterm_global_flg = NO; /* YES for xterm terminal familly */

Flag helpflg,
     verbose_helpflg,
     dbgflg;

static Flag crashed = NO;       /* prev session crashed (or dumped) */
static Flag chosereplay = NO;   /* user chose replay or recovery of a crashed or aborted session */

char *opttname;
char *optkname;     /* Keyboard name */

char *kbfcase;          /* who define kbfile */
char *kbfile = NULL;    /* name of input (keyboard) table file */
char *optkbfile;        /* Keyboard translation file name option */

Flag optbullets = -1;   /* YES = -bullets, NO = -nobullets */
#ifdef TERMCAP
Flag optdtermcap;       /* YES = force use of termcap */
#endif
static char *tcap_name = NULL; /* name of termcap entry used */
static char tcap_name_default[] = "vt100";

Flag dump_state_file = NO;  /* YES = dump the state file defined with -state=<fname> option */
Flag noX11flg = NO;         /* YES = do not use X11 to get the keyboard mapping */
static Flag noX11opt = NO;  /* YES = noX11flg defined by -noX11 option */


/* Rand option table and description */
/* ================================= */

/*  After the two-byte revision number in the keystroke file
 *  comes a character telling what to use from the state file.
 *  As of this writing, only ALL_WINDOWS and NO_WINDOWS are usable.
 **/
#define NO_WINDOWS  ' ' /* don't use any files from state file */
#define ONE_FILE    '-' /* use only one file from current window */
#define ONE_WINDOW  '+' /* use current and alt file from current window */
#define ALL_WINDOWS '!' /* use all windows and files */

#define OPTREPLAY       0
#define OPTRECOVER      1
#define OPTDEBUG        2
#define OPTSILENT       3
#define OPTHELP         4
#define OPTNOTRACKS     5
#define OPTINPLACE      6
#define OPTNORECOVER    7
#define OPTTERMINAL     8
#define OPTKEYBOARD     9
#ifdef  KBFILE
#define OPTKBFILE      10
#endif
#define OPTBULLETS     11
#define OPTNOBULLETS   12
#ifdef TERMCAP
#define OPTDTERMCAP    13
#endif
#define OPTPATTERN     15
#ifdef LMCSTATE
#define OPTSTATE       16
#endif
#ifdef LMCOPT
#define OPTSTDKBD      17
#endif
#ifdef LMCSRMFIX
#define OPTSTICK       18
#define OPTNOSTICK     19   /* no pun intended */
#endif
#ifdef NOCMDCMD
#define OPTNOCMDCMD    20
#endif
/* XXXXXXXXXXXXXXXXX */
#define OPTDIRPACKG    21
#define OPTVHELP       22
#define OPTVERSION     23
#define OPTDUMPESF     24
#define OPTNOX11       25

#define MAX_opt        32

/* Comments for the Rand call options */
char comt_replay []    = "replay session from this keys file";
char comt_recover []   = "";
char comt_debug []     = " default level = 1\n\
	   debug level 0 : debugging suspended";
char comt_silent []    = "replaying silently";
char comt_help []      = "short help and some checks";
char comt_notracks []  = "do not disturbe the previous session work files";
char comt_inplace []   = "do in-place file updates";
char comt_norecover [] = "do not recover previous aborted session";
char comt_terminal []  = "(terminal type, default : linux, cf TERM)";
char comt_keyboard []  = "(use compiled internal kbd definition, cf EKBD)";
char comt_kbfile []    = "(keyboard file, cf EKBFILE)";
char comt_bullets []   = "turn border bullets on (default on workstation)";
char comt_nobullets [] = "turn border bullets off (only for slow terminal)";
char comt_dtermcap []  = "force use of termcap";

char comt_pattern []   = "set regular expression mode";
char comt_state []     = "state file to be used";
char comt_stdkbd []    = "force the mini keyboard definition (only control charaters)";
char comt_stick []     = "no auto scroll past window right edge";
char comt_nostick []   = "auto scroll past window right edge";
char comt_nocmdcmd []  = "do not allow the 'cmd cmd' command";
char comt_dirpackg []  = "(package directory, cf RAND_PACKG)";
char comt_vhelp []     = "verbose help and kbfiles debugging";
char comt_version []   = "display the Rand version and revision";
char comt_dumpesf []   = "display the content of the state file defined by -state\n\
	   or by the currently used state file";
char comt_nox11 []     = "do not use X11 to get the keyboard mapping";

static struct _opt_desc {
    short type;     /* indic is : -1 = negative logic,
				   0 = flag, 1 = file name, 2 = directory name
				 -99 = default inverted logic, 99 = default */
    void *indic;    /* if NULL, not in use or no indicator */
    char *comment;
    } opt_desc [MAX_opt] = {
	1, &replfname      ,comt_replay   ,  /* OPTREPLAY       0 */
	0, NULL            ,comt_recover  ,  /* OPTRECOVER      1 */
	1, &dbgname        ,comt_debug    ,  /* OPTDEBUG        2 */
	0, &silent         ,comt_silent   ,  /* OPTSILENT       3 */
	0, &helpflg        ,comt_help     ,  /* OPTHELP         4 */
	0, &notracks       ,comt_notracks ,  /* OPTNOTRACKS     5 */
	0, &inplace        ,comt_inplace  ,  /* OPTINPLACE      6 */
	0, &norecover      ,comt_norecover,  /* OPTNORECOVER    7 */
	1, &opttname       ,comt_terminal ,  /* OPTTERMINAL     8 */
	1, &optkname       ,comt_keyboard ,  /* OPTKEYBOARD     9 */
	1, &optkbfile      ,comt_kbfile   ,  /* OPTKBFILE      10 */
	0, &optbullets     ,comt_bullets  ,  /* OPTBULLETS     11 */
       -1, &optbullets     ,comt_nobullets,  /* OPTNOBULLETS   12 */
	0, &optdtermcap    ,comt_dtermcap ,  /* OPTDTERMCAP    13 */
	0, NULL            ,NULL          ,  /*                14 */
	0, &patmode        ,comt_pattern  ,  /* OPTPATTERN     15 */
	1, &statefile      ,comt_state    ,  /* OPTSTATE       16 */
	0, &stdkbdflg      ,comt_stdkbd   ,  /* OPTSTDKBD      17 */
	0, &optstick       ,comt_stick    ,  /* OPTSTICK       18 */
      -99, &optstick       ,comt_nostick  ,  /* OPTNOSTICK     19 */
	0, &optnocmd       ,comt_nocmdcmd ,  /* OPTNOCMDCMD    20 */
	2, &optdirpackg    ,comt_dirpackg ,  /* OPTDIRPACKG    21 */
	0, &verbose_helpflg,comt_vhelp    ,  /* OPTVHELP       22 */
	0, NULL            ,comt_version  ,  /* OPTVERSION     23 */
	0, &dump_state_file,comt_dumpesf  ,  /* OPTDUMPESF     24 */
	0, &noX11opt       ,comt_nox11    ,  /* OPTNOX11       25 */
	0, NULL            , NULL
    };


/* Entries must be alphabetized. */
/* Entries of which there are two in this table must be spelled out. */
S_looktbl opttable[] = {
    "bullets"  , OPTBULLETS  ,
    "debug"    , OPTDEBUG    ,
#ifdef LMCOPT
    "dkeyboard", OPTSTDKBD   ,
#endif
#ifdef TERMCAP
    "dtermcap" , OPTDTERMCAP ,
#endif
    "dump-state-file", OPTDUMPESF,
    "dump_state_file", OPTDUMPESF,
    "help"     , OPTHELP     ,
    "inplace"  , OPTINPLACE  ,
#ifdef  KBFILE
    "kbfile"   , OPTKBFILE   ,
#endif
    "keyboard" , OPTKEYBOARD ,
    "nobullets", OPTNOBULLETS,
#ifdef NOCMDCMD
    "nocmdcmd" , OPTNOCMDCMD ,
#endif
    "norecover", OPTNORECOVER,
#ifdef LMCSRMFIX
    "nostick"  , OPTNOSTICK  ,
#endif
    "notracks" , OPTNOTRACKS ,
    "nox11"    , OPTNOX11    ,
    "package"  , OPTDIRPACKG ,
    "regexp"   , OPTPATTERN  ,  /* Added Purdue CS 2/8/83 */
    "replay"   , OPTREPLAY   ,
    "silent"   , OPTSILENT   ,
#ifdef LMCSTATE
    "state"    , OPTSTATE    ,
#endif
#ifdef LMCSRMFIX
    "stick"    , OPTSTICK    ,
#endif
    "terminal" , OPTTERMINAL ,
    "verbose"  , OPTVHELP    ,
    "version"  , OPTVERSION  ,
    0          , 0
};
static int opttable_maxidx = ((sizeof (opttable) / sizeof (opttable[0])) -1);

#if 0 /*========================= */

Flag optnocmd = NO;          /* YES = -nocmdcmd; <cmd><cmd> functions disabled */

/* XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX */

/* Names of the host running Rand,
 *   from which computer, and display on from_host
 *   from_host and from_display are NULL if running on the local computer
 */
char *myHost, *fromHost, *fromDisplay, *fromHost_alias;
char *ssh_ip, *sshHost, *sshHost_alias; /* host of a ssh session */

/* Assumed Rand editor directory tree structure :
 * ==============================================
 * The following structure is assumed in order to found automaticaly
 *      the various pathes to services files (help, keyboard definition ...)
 *
 * The Rand pacakge directory is the directory which include :
 *       - the Rand Editor installed executable file (which cannot be called a.out)
 *       - the service programs (center, fill, run) executable files
 *       - the help files : errmsg, helpkey, recovermsg, Crashdoc
 *       - the kbfiles directory (directory of keyboard definition files)
 *   Special case : building a new version (compiling and linking)
 *       - the new executable file name must be : a.out
 *      - the help files and kbfiles directory must be at the relative
 *          position defined by helpdir constant string : ../help
 */

static char *dirpackg = NULL;   /* Rand editor package directory */
static Flag buildflg = NO;      /* newly build executable (a.out) */

static const char loopback [] = "127.0.0.1";

/* Rand package directory tree structure related names */
/* --------------------------------------------------- */
    /* default default package dir (used to extract the package sub dir) */
#ifdef DEF_XDIR
extern char def_xdir_dir [];    /* build by NewVersion script (e.r.c) */
#else
static char def_xdir_dir [] = XDIR_DIR;     /* defined in ../include/linux_localenv.h */
#endif
    /* system wide configuration directory */
static char syst_cnfg []  = "/etc";     /* computer configuration directory */
static char alt_local_cnfg []  = "/../etc";  /* local relative to pacakage */
static char local_cnfg [] = "/usr/local/etc";   /* cluster configuration dir */
    /* all-purpose keyboard definition file name */
static char universal_kbf_name [] = "universalkb";
    /* kbfiles directory relative to rand package and configurations directory */
static char kbfilesdir [] = "/kbfiles";
    /* help directory relative to the directory including a.out (just build editor) */
static char helpdir [] = "/../help";
static char execdir [] = "/../fill";
static char buildexec [] = "a.out";
    /* postfix added to the keyboard name (or terminal name) to generate the kbfile name */
static char kbfile_postfix [] = "kb";
static char kbmap_postfix [] = ".kbmap";
static char preference [] = "preferences";
static char debug [] = "debug_output.txt";

static char *full_prgname = NULL;   /* full Rand program name */
static char *prog_fname;        /* actual Rand editor file name */
static char *pwdname = NULL;    /* current working directory */

static char *optdirpackg = NULL;    /* option dir package */
static Flag optusedflg = NO;        /* some program options in use */

char default_workingdirectory [PATH_MAX+1] = "./";

#ifdef VBSTDIO
extern unsigned char _sobuf[];
#endif

Small numargs;
Small curarg;
char *curargchar;
char **argarray;

static char *dbgname = NULL;    /* argument to -debug=xxx option */
static char *replfname = NULL;  /* argument to -replay=xxx option */

#ifdef LMCSTATE
Flag state=NO;      /* -state option */
char *statefile;    /* argument to -state=xxx option */
#endif

char stdk[] = "standard";   /* standard comipled keyboard name */
Flag stdkbdflg;     /* use the standard compiled keyboard definition */
Flag Xterm_global_flg = NO; /* YES for xterm terminal familly */

Flag helpflg,
     verbose_helpflg,
     dbgflg;

static Flag crashed = NO;       /* prev session crashed (or dumped) */
static Flag chosereplay = NO;   /* user chose replay or recovery of a crashed or aborted session */

char *opttname;
char *optkname;     /* Keyboard name */

char *kbfcase;          /* who define kbfile */
char *kbfile = NULL;    /* name of input (keyboard) table file */
char *optkbfile;        /* Keyboard translation file name option */

Flag optbullets = -1;   /* -1 = not set, YES = -bullets, NO = -nobullets */
#ifdef TERMCAP
Flag optdtermcap;       /* YES = force use of termcap */
#endif
static char *tcap_name = NULL; /* name of termcap entry used */
static char tcap_name_default[] = "vt100";

Flag dump_state_file=NO; /* YES = dump the state file defined with -state=<fname> option */
Flag noX11flg=NO;        /* YES = do not use X11 to get the keyboard mapping */
static Flag noX11opt=NO; /* YES = noX11flg defined by -noX11 option */

#endif


/* ++XXXXXXXXXXXXXXXXXXXXXXXXX */
/* Rand package service & configuration files */

static char *packg_subdir = NULL;   /* extracted form program path or XDIR_DIR */

struct cnfg_dir_rec {
	char *path;         /* directory path name */
	Flag existing;      /* existing and read access */
    };

static Flag set_dir_rec (struct cnfg_dir_rec *dir_rec, char *strg);

static struct cnfg_dir_rec user_packg_dir  =    { NULL, NO }; /* $HOME/.Rand */
static struct cnfg_dir_rec user_tmp_dir    =    { NULL, NO }; /* $HOME/tmp */
static struct cnfg_dir_rec user_kbmap_file =    { NULL, NO }; /* keyboard map file name */
static struct cnfg_dir_rec new_kbfile_file =    { NULL, NO }; /* build kb file name */
static struct cnfg_dir_rec user_pref_file  =    { NULL, NO }; /* user preference file name */
static struct cnfg_dir_rec user_debug_file =    { NULL, NO }; /* debug output file name */

static struct cnfg_dir_rec user_cnfg_dir =      { NULL, NO }; /* $HOME/.Rand/kbfiles */
static struct cnfg_dir_rec local_cnfg_dir =     { NULL, NO }; /* /usr/local/etc/Rand/kbfiles */
static struct cnfg_dir_rec alt_local_cnfg_dir = { NULL, NO }; /* <package>/../etc/Rand/kbfiles */
static struct cnfg_dir_rec syst_cnfg_dir =      { NULL, NO }; /* /etc/Rand/kbfiles */
static struct cnfg_dir_rec def_cnfg_dir  =      { NULL, NO }; /* in the package directory <pakage> */

static struct cnfg_dir_rec *cnfg_dir_rec_pt [] = {        /* in order of priority */
	    &user_cnfg_dir,         /* user config */
	    &syst_cnfg_dir,         /* computer config */
	    &alt_local_cnfg_dir,    /* cluster config */
	    &local_cnfg_dir,        /* default cluster config */
	    &def_cnfg_dir           /* default package config */
	};
#define cnfg_dir_rec_pt_sz (sizeof (cnfg_dir_rec_pt) / sizeof (cnfg_dir_rec_pt[0]))

static char no_exist_msg [] = " (not existing or no read access)";

static int max_cnfg_dir_sz = 0;     /* max size of config dir path name */

char * xdir_dir = NULL;     /* package directory */

char * recovermsg;
char * xdir_kr;
char * xdir_help;
char * xdir_crash;
char * xdir_err;
char * xdir_run;
char * xdir_fill;
char * xdir_just;
char * xdir_center;
char * xdir_print;

char * filterpaths[4];
/*
    xdir_fill ,
    xdir_just ,
    xdir_center ,
    xdir_print
*/
/* --XXXXXXXXXXXXXXXXXXXXXXXXX */

       void main1 ();
       void getprogname ();
extern void checkargs ();
static void startup ();
       void showhelp ();
static void dorecov ();
       void gettermtype ();
       void setitty ();
       void setotty ();
extern void makescrfile ();
static Flag getstate ();
extern void getskip ();
static void makestate ();
static void infoinit ();
#ifdef LMCAUTO
static void infoint0 ();
#endif
#ifdef  KBFILE
extern Flag getkbfile ();
#endif

void debug_info (char *func, char *file, int line)
/* must be call like : debug_info (__FUNCTION__, __FILE__, __LINE__); */
{
    if ( dbgflg ) {
	fprintf (dbgfile, "  >>Form function \"%s\" in \"%s\" at line %d<<\n", func, file, line);
    }
}

void getout ();

/* XXXXXXXXXXXXXXXXXXXXXXX */
static void keyedit ();

static void strip_path (char * path)
{
    int sz;

    if ( !path || !*path ) return;
    sz = strlen (path) -1;
    if ( path [sz] == '/' ) path [sz] = '\0';
}

char * get_debug_name ()
{
    return dbgname;
}

static get_id ()
{
    int sz;

    strip_path (default_tmppath);
    userid = getuid ();
    groupid = getgid ();
    mypid = (int) getpid ();
    myname = mypath = NULL;
    (void) getmypath ();    /* gets myname & mypath */
    if ( !myname || !*myname ) myname = "???";
    if ( !mypath || !*mypath ) mypath = &default_tmppath [0];
    strip_path (mypath);
}

static void dbg_general_info ()
{
#ifdef __GNU__
#define TypeSize(t) fprintf (dbgfile, #t " %d, ", sizeof (t));

    fputs ("Main Rand data type size :\n  ", dbgfile);
    TypeSize (Char);
    TypeSize (Short);
    TypeSize (Small);
    TypeSize (ASmall);
    TypeSize (Flag);
    TypeSize (AFlag);
    fputs ("\n  ", dbgfile);
    TypeSize (Cmdret);
    TypeSize (Nlines);
    TypeSize (ANlines);
    TypeSize (Ncols);
    TypeSize (ANcols);
    fputs ("\n---------------------------------------------\n", dbgfile);
#undef TypeSize
#endif /* __GNU__ */
    return;
}

void close_dbgfile (char *str)
{
    if ( dbgflg ) {
	if ( str ) fprintf (dbgfile, "\n=== Exit %s command ====\n", str);
	else fputs ("\n=== Exit ====\n", dbgfile);
    }
    if ( dbgfile ) {
	(void) fflush (dbgfile);
	if ( dbgfile != stdout ) fclose (dbgfile);
	dbgfile = NULL;
    }
    dbgflg = NO;
}

int open_dbgfile (Flag append_flg)
/* append_flg : open the file for write append, else only write
 * return : 1 = not in debug mode
 *          0 = OK
 *         -1 = open file error
 *         -2 = debug output file name not defined
 */
{
    if ( ! dbgflg ) return 1;   /* not in debug mode */
    if ( dbgfile ) return 0;    /* already opened */

    if ( !dbgname || !*dbgname )
	dbgname = get_debug_file_name (!append_flg, NULL);
    if ( !dbgname ) dbgname = "";
    if ( !*dbgname ) return -2;
    dbgfile = fopen (dbgname, append_flg ? "a" : "w");
    if ( dbgfile == NULL ) {
	dbgflg = NO;
	return -1;  /* file error */
    }
    if ( append_flg ) fputc ('\n', dbgfile);
    fprintf (dbgfile, "======= %s Rand debugging session level %d =======\n",
	     append_flg ? "Continue" : "Start", DebugVal);
    if ( DebugVal >= 0 )
	dbg_general_info ();

    if ( DebugVal == 0 ) {
	/* just empty debug file, and wait for 'set debuglevel xx' */
	dbgflg = NO;
	fclose (dbgfile);
	dbgfile = NULL;
    }
    return 0;
}



#ifdef COMMENT
void
main1 (argc, argv)
    int     argc;
    char   *argv[];
.
    All of the code prior to entering the main loop of the editor is
    either in or called by main1.
#endif
void
main1 (argc, argv)
int argc;
char *argv[];
{
    extern Flag get_graph_flg ();
    extern Flag set_reset_graph (Flag);
    extern void save_kbmap (Flag);
    extern void key_resize ();
    extern void history_init ();
    extern int build_lookup_struct (S_lookstruct *);
    extern int build_sorted_cmdtable ();
    extern int build_sorted_itsyms ();
    extern S_lookstruct cmdstruct;
    extern S_lookstruct keyfstruct;
    extern void init_all_lookup_tables ();
    extern void get_preferences (Flag all_flg, Flag show_flg);

    char    ichar;      /* must be a char and can't be a register */
    int cc, infolv;
    Flag grph_flg, new_grph_flg;
    char strg [PATH_MAX+1];

    /* clean various global storage */
    clean_glb_storages ();

    get_id ();  /* get mypath ... */
    fromHost = myHost = fromDisplay = NULL;

    /* user tmp directory */
    sprintf (strg, "%s%s", mypath, tmpdir);
    (void) set_dir_rec (&user_tmp_dir, strg);

    init_all_lookup_tables ();

#if 0
|    cc = build_lookup_struct (&cmdstruct);
|    if ( cc ) getout (YES, "Fatal Error in size of array \"cmd_storage\"");
|    cc = build_lookup_struct (&keyfstruct);
|    if ( cc ) getout (YES, "Fatal Error in size of array \"keyf_storage\"");
#endif

    history_init ();
#ifdef VBSTDIO
    setbuf (stdout, (char *) _sobuf);
#endif
    fclose (stderr);

    numargs = argc;
    argarray = argv;
    getprogname (argv[0]);
    (void) getcwd (default_workingdirectory, sizeof (default_workingdirectory));

    checkargs ();

    if ( dump_state_file ) {
	Flag st_flg;
	/* do not create the working file, just get status file */
	if ( ! state ) st_flg = build_wkfiles (NO);
	if ( st_flg || state ) {
	    (void) getstate (ALL_WINDOWS, YES, NULL, NULL);
	    getout (YES, "");
	} else {
	    getout (YES, "You must use -state=<state_file_name> to dump it");
	}
    }

#ifdef LMCAUTO
    infoint0 ();
#endif
    startup ();

    if ( dbgflg ) {
	int cc;
	cc = open_dbgfile (NO);
	if ( cc < 0 )
	    getout (YES, "Can't open debug file: \"%s\"", dbgname);
    }

    if (helpflg) {
	static void clean_all ();
	static void exit_now ();
	showhelp ();
	helpflg = NO;
	clean_all (NO); /* do not delete change and key stroke files */
	exit_now (-1);
    }

    if ( replaying ) {
	/* Start a replay session : by the -replay=<file name> option
	 * or as a result of handling of previous aborted or crashed session
	 */
	short tmp;      /* must be a short, can't be a register */
	short replterm; /* must be a short, can't be a register */
	struct stat statbuf;

	if ( ! inpfname )
	    getout (YES, "A replay file name must be provided.");
	if ((inputfile = open (inpfname, 0)) < 0)
	    getout (YES, "File error : %s\nCan't open replay file %s.", strerror (errno), inpfname);
	if (read (inputfile, (char *) &tmp, 2) == 0)
	    getout (YES, "Replay file is empty.");
	if (tmp != revision)
	    getout (YES, "Replay file \"%s\" was made by revision %d of %s.",
		 inpfname, -tmp, progname);
	if (   read (inputfile, (char *) &ichar, 1) == 0
	    || read (inputfile, (char *) &replterm, sizeof (short)) == 0
	   )
	    getout (YES, "Replay file is too short.");
	if (replterm != termtype)
	    getout (YES, " Replay file \"%s\" was made by a different type of terminal.", inpfname);
    }
    else {
	/* The session will be resumed from the previous session
	 *  as defined by the state file (.es1) if there is no
	 *  file parameter or if the 1st file parameter is empty.
	 *  (this case can be used within a gdb or dbx code debugging
	 *  to force a run form the state file : ex 'run ""'
	 * If there is 1 or more file parameters, and the first one
	 *  is not empty, the edited files list is defined by the
	 *  parameter list.
	 * If -notracks option is in use, a completely fresh session is
	 *  started, and nothing is comming from any previous session.
	 */
	if ( (curarg < argc) && (*argv[curarg] != '\0') )
	    ichar = NO_WINDOWS;         /* file args follow */
	else
	    ichar = ALL_WINDOWS;        /* put up all old windows and files */
    }

    if ( recovering )
	printf ("\r\n\r\rRecovering from crash...");
    fflush (stdout);

    makescrfile (); /* must come before any calls to editfile () */

    if (!silent) {
	(*term.tt_home) ();
	(*term.tt_clear) ();
    }
#ifdef LMCVBELL
    if (getenv ("VBELL"))
	if (*term.tt_vbell != NULL) {
	    NoBell = NO;
	    VBell = YES;
	}
#endif

#ifndef __linux__
    if ( strcasecmp (tname, "linux") == 0 ) {
	/* we do not know realy what to do automaticaly */
	(void) set_reset_graph (NO);
    }
#endif

    new_grph_flg = grph_flg = get_graph_flg ();
    state_reused_flg = getstate (ichar, NO, &infolv, &new_grph_flg);
    infoinit (infolv);
    if ( new_grph_flg != grph_flg )
	(void) set_reset_graph (new_grph_flg);

    putshort (revision, keyfile);
    putc (ichar, keyfile);          /* for next time */
    putshort ((short) termtype, keyfile);
    key_resize ();  /* dump current screen size in keyfile */

    if ( !replaying && curarg < argc && (*argv[curarg] != '\0') ) {
	char *cp;
	Small cc;
	int i;
	/* The edit file list is in the order :
	 *  Normal, Alt, Next1, Next2 .... (up to the maxi)
	 *  if 'editalt' is defined, it define the alt file.
	 *  The Normal file name must not be an empty string,
	 *  else the session defined by status file (.es1) is restarted.
	 */
	i = argc -1;
	if ( (i - curarg) >= (MAXOPENS - NTMPFILES) )
	    i = (MAXOPENS - NTMPFILES) + curarg -1;
	for ( ; i > curarg ; i-- ) {
	    cp = argv[i];
	    if ( !cp || !*cp ) continue;
	    cc = editfile (cp, (Ncols) 0, 0, 0, NO);
	    keyedit (cp);
	}

	/* the alt file name can be define by environment var 'editalt' */
	cp = getenv ("editalt");
	if ( cp && *cp ) {
	    /* alternate file if any */
	    cc = editfile (cp, (Ncols) 0, 0, 0, NO);
	    keyedit (cp);
	}

	/* do this so that editfile won't be cute about suppressing
	 * a putup on a file that is the same as the altfile
	 */
	curfile = NULLFILE;

	/* normal file, if error error message file */
	cp = argv[curarg];
	cc = editfile (argv[curarg], (Ncols) 0, 0, 1, YES);
	keyedit (argv[curarg]);
	if ( cc <= 0 ) eddeffile (YES);
    }
    else if (!replaying || ichar != NO_WINDOWS) {
	extern void resize_screen ();
	if ( ! curfile ) eddeffile (YES);   /* prevent crash if nothing is up */
	resize_screen ();   /* set up the actual size */
	putupwin ();
    }

    /* get all user preferences if no state file was read */
    get_preferences (!state_reused_flg, NO);
    save_kbmap (NO);
    return;
}

static void set_noX11opt ()
{
    noX11flg = noX11opt = YES;
    set_noX_msg ("\'-noX11\' option in use");
}

#ifdef COMMENT
static void
keyedit (file)
    char   *file;
.
    Write out an edit command to the keys file.
#endif
static void
keyedit (file)
char *file;
{
    char *cp;

    cp = append ("edit ", file);
    writekeys (CCCMD, cp, CCRETURN);
    sfree (cp);
}

#ifdef COMMENT
void
getprogname (arg0)
    char arg0[];
.
    Set the global character pointer 'progname' to point to the actual name
    that the editor was invoked with.
#endif
void
getprogname (arg0)
char arg0[];
{
    char *cp;
    Char lastc;

    lastc = 0;
    progname = cp = arg0;
    for (; *cp; cp++) {
	if (lastc == '/')
	    progname = cp;
	lastc = *cp;
    }
    cp = progname;

    if (loginflg = *progname == '-')
	progname = LOGINNAME;
    return;
}

#ifdef COMMENT
void
checkargs ()
.
    Scan the arguments to the editor.  Set flags and options and check for
    errors in the option arguments.
#endif
void
checkargs ()
{
    char *cp, *fname;
    Flag opteqflg;
    Short tmp;
    char *opterr;

    optusedflg = dbgflg = NO;
    for (curarg = 1; curarg < numargs; curarg++) {
	curargchar = argarray[curarg];
	if ( *curargchar != '-' )
	    break;

	curargchar++;
	if ( *curargchar == '\0' ){
	    /* no more option : a '-' can be used before a file name with a '-' as 1st char */
	    curarg++;
	    break;
	}

	if (*curargchar == '-') curargchar++;   /* allow --help ... */
	optusedflg = YES;
	opteqflg = NO;
	for (cp = curargchar; *cp; cp++)
	    if (*cp == '=') {
		opteqflg = YES;
		*cp = 0;
		break;
	    }
	if (cp == curargchar)
	    goto unrecog;
	tmp = lookup (curargchar, opttable);
	if (opteqflg)
	    *cp++ = '=';
	if (tmp == -1)
	    goto unrecog;
	if (tmp == -2) {
	    opterr = "ambiguous";
	    goto error;
	}
	switch (opttable[tmp].val) {
	case OPTBULLETS:
	    optbullets = YES;
	    break;

	case OPTNOBULLETS:
	    optbullets = NO;
	    break;

	case OPTHELP:
	    helpflg = YES;
	    break;

	case OPTVHELP:  /* verbose help (for debugging) */
	    verbose_helpflg = YES;
	    break;

#ifdef LMCSRMFIX
	case OPTSTICK:
	    optstick = YES;
	    break;

	case OPTNOSTICK:
	    optstick = NO;
	    break;
#endif

#ifdef TERMCAP
	case OPTDTERMCAP:
	    optdtermcap = YES;
	    break;
#endif

#ifdef LMCSTATE
	case OPTSTATE:
	    if (opteqflg && *cp) {
		statefile = cp;
		/* a state file must be given */
		state = (statefile && *statefile);
	    }
	    break;
#endif

	case OPTDEBUG:
	    /* file (and level) for debugging */
	    if ( dbgflg ) {
		opterr = "repeated";
		goto error;
	    }
	    parse_debug_option (opteqflg ? cp : NULL, NO);
	    break;

	case OPTINPLACE:
	    inplace = YES;
	    break;

	case OPTSILENT:
	    silent = YES;
	    break;

	case OPTNORECOVER:
	    norecover = YES;
	    break;

	case OPTNOTRACKS:
	    notracks = YES;
	    break;

	case OPTREPLAY:
	    replaying = YES;
	    if (opteqflg && *cp)
		replfname = cp;
	    break;

	case OPTKEYBOARD:
	    if (opteqflg && *cp)
		optkname = cp;
	    break;

#ifdef  KBFILE
	case OPTKBFILE:
	    if (opteqflg && *cp)
		optkbfile = cp;
	    break;
#endif

	case OPTTERMINAL:
	    if (opteqflg && *cp)
		opttname = cp;
	    break;

	case OPTPATTERN:
	    patmode = YES;
	    break;

#ifdef LMCOPT
	case OPTSTDKBD:
	    stdkbdflg = YES;
	    break;
#endif

#ifdef  NOCMDCMD
	case OPTNOCMDCMD:
	    optnocmd = YES;
	    break;
#endif
	case OPTDIRPACKG:
	    if (opteqflg && *cp)
		optdirpackg = cp;
	    break;

	case OPTVERSION:
	    getout (YES, "");
	    break;

	case OPTDUMPESF:
	    dump_state_file = YES;
	    break;

	case OPTNOX11:
	    set_noX11opt ();
	    break;

	default:
	unrecog:
	    opterr = "not recognized";
	error:
	    {
		S_looktbl *slpt;
		printf ("\n%s option %s\n", argarray[curarg], opterr);
		printf ("\nDefined options :");
		for ( slpt = &opttable[0] ; slpt->str ; slpt ++ )
		    printf (" -%s,", slpt->str);
		printf ("\n\nUse : \"%s --help\" for a short description\n", argarray[0]);
		getout (YES, "");
	    }
	}
    }
    return;
}

/* ++XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX */

/* check_access : check the access right of a file or directory */
/* ------------------------------------------------------------ */

static int check_access (char *fname, int mode, char **str)
    /* return : 0 = ok, 1 = no access right, 2 = not existing */
{
    int cc, err;

    cc = access (fname, mode);
    if ( cc == 0 ) {
	*str = " access ok";
	return (0);
    }
    err = errno;
    cc = access (fname, F_OK);
    if ( cc < 0 ) {
	err = errno;
	*str = " not existing";
	return (2);
    }
    if      ( mode & X_OK ) *str = " no execute access";
    else if ( mode & W_OK ) *str = " no write access";
    else if ( mode & R_OK ) *str = " no read access";
    else                    *str = " no access";
    return (1);
}

#ifdef COMMENT
static char *
packg_file ()
.
    Build the full file name for a Rand package file
#endif
static char *
packg_file (char *fname, char *subdir)
{
    extern int dircheck ();
    char *file, *fulln;
    int sz;

    if ( dirpackg ) {
	file = strrchr (fname, '/');
	if ( ! file ) file = fname;
	sz = strlen (dirpackg) + strlen (file) +2;
	if ( subdir ) sz += strlen (subdir);
	fulln = (char *) calloc (sz, 1);
	if ( fulln ) {
	    (void) strcpy (fulln, dirpackg);
	    if ( subdir ) (void) strcat (fulln, subdir);
	    if ( file[0] != '/' ) (void) strcat (fulln, "/");
	    (void) strcat (fulln, file);
	    return fulln;
	}
    }
    return fname;
}
/* --XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX */

/* get_cwd : get the current working directory */
static char * get_cwd ()
{
    char *sp;
    char cwd_buf [512];

    if ( pwdname ) return (pwdname);

    (void) memset (cwd_buf, 0, sizeof (cwd_buf));
    sp = getcwd (cwd_buf, sizeof (cwd_buf) -1);
    if ( ! sp ) cwd_buf[0] = '.';
    sp = &cwd_buf[strlen (cwd_buf) -1];
    if ( *sp == '/' ) *sp = '\0';
    pwdname = (char *) malloc (strlen (cwd_buf) +1);
    if ( pwdname ) strcpy (pwdname, cwd_buf);
    return (pwdname);
}

/* get_exec_name : try to find the exec file, using the $PATH */
char * get_exec_name (char *fname)
{
    char *sp0, *sp1, *path, *fnm;
    char str[2048];
    int sz, msz;

    if ( ! fname ) return (NULL);

    if ( strchr (fname, '/') ) fnm = fname;
    else {
	path = getenv ("PATH");
	if ( !path ) path = ":/bin:/usr/bin";   /* default PATH value */

	fnm = str;
	msz = sizeof (str);
	for ( sp0 = path ; ; sp0 = sp1 +1 ) {
	    memset (fnm, 0, msz);
	    sp1 = strchr (sp0, ':');    /* next token */
	    if ( !sp1 ) sp1 = &path[strlen (path)];
	    sz = sp1 - sp0;
	    if ( sz == 0 ) break;   /* no more token */
	    if ( sz <= 1 ) continue;
	    if ( sz >= (msz -1) ) break;    /* too large path token */

	    strncpy (fnm, sp0, sz);
	    if ( fnm[sz-1] != '/' ) fnm[sz] = '/';
	    strncat (fnm, fname, msz - sz -1);
	    if ( access (fnm, X_OK) == 0 ) break;
	}
    }
    if ( access (fnm, X_OK) < 0 ) return (NULL);

    /* executable file found */
    sz = strlen (fnm);
    sp0 = (char *) malloc (sz +1);
    if ( sp0 ) strcpy (sp0, fnm);
    return (sp0);
}

/* get_flname : get actual name (follow link) */
char * get_flname (char *fname)
{
    /* try to found the actual directory of the program */
    int cc, nb;
    struct stat lstbuf;
    char pkgnm [1024];
    char lbuf [512];
    char *sp;

    (void) memset (pkgnm, 0, sizeof (pkgnm));
    if ( fname[0] != '/' ) {    /* relative path file name */
	sp = get_cwd ();
	if ( sp ) strncpy (pkgnm, sp, sizeof (pkgnm) -2);
	else pkgnm[0] = '.';
	pkgnm[strlen (pkgnm)] = '/';
    }
    strncat (pkgnm, fname, sizeof (pkgnm));
    pkgnm [sizeof (pkgnm) -1] = '\0';
    for ( ; ; ) {
	cc = lstat (pkgnm, &lstbuf);
	if ( cc ) break;
	if ( ! S_ISLNK (lstbuf.st_mode) ) break;

	nb = readlink(pkgnm, lbuf, sizeof (lbuf));
	if ( nb < 0 ) return (NULL);    /* some error */

	lbuf[nb] = '\0';
	if ( lbuf[0] == '/' ) pkgnm[0] = '\0';    /* absolu path */
	else {      /* relative path, remove the file name */
	    sp = strrchr (pkgnm, '/');
	    if ( sp ) *(sp +1) = '\0';
	    else pkgnm[0] = '\0';
	}
	if ( (strlen (pkgnm) + nb) >= sizeof (pkgnm) ) return (NULL); /* too large name */
	strcat (pkgnm, lbuf);
    }
    if ( ! pkgnm[0] ) return (NULL);    /* not found */
    sp = (char *) malloc (strlen (pkgnm) +1);
    if ( sp ) strcpy (sp, pkgnm);
    return (sp);
}


static Flag set_dir_rec (struct cnfg_dir_rec *dir_rec, char *strg)
{
    int sz;

    if ( !strg ) {
	dir_rec->existing = NO;
	return NO;
    }
    if ( dir_rec->path && (strcmp (dir_rec->path, strg) != 0) ) {
	free (dir_rec->path);
	dir_rec->path = NULL;
    }
    if ( !dir_rec->path ) {
	sz = strlen (strg) +1;
	dir_rec->path = (char *) calloc (sz, 1);
	if ( dir_rec->path ) memcpy (dir_rec->path, strg, sz);
    }
    dir_rec->existing = ( access (strg, R_OK) == 0 );
    return dir_rec->existing;
}

static void get_dirpackage ()
{
    char *fnm, *dir, *cp;
    int i, sz;
    char strg [PATH_MAX+1];
    struct stat stat_local, stat_alt_local;

    strip_path (def_xdir_dir);

    /* get the Rand program directory */
    fnm = get_exec_name (argarray[0]);
    full_prgname = dir = get_flname (fnm);
    if ( full_prgname ) {
	cp = strrchr (full_prgname, '/');
	if ( cp ) {
	    prog_fname = cp+1;
	    sz = cp - full_prgname;
	    dir = (char *) malloc (sz +1);
	    if ( dir ) {
		strncpy (dir, full_prgname, sz);
		dir[sz] = '\0';
	    }
	    else dir = full_prgname;

	    if ( strcmp (prog_fname, buildexec) == 0 ) {
		/* special case for debugging a not yet installed executable */
		buildflg = YES;
		/* the sub directory is defined now by the "packg_file" call */
	    }
	}
    }
    dirpackg = ( optdirpackg ) ? optdirpackg : getenv ("RAND_PACKG");
    if ( dirpackg && !*dirpackg) dirpackg = NULL;
    if ( ! dirpackg ) {
	dirpackg = dir;     /* from the Rand prog directory */
    }
    if ( dirpackg ) {
	for ( cp = &dirpackg[strlen (dirpackg) -1] ; *cp == ' ' ; cp-- ) ;
	if ( *cp == '/' ) *cp = '\0';
    }
    /* If directory package is not defined, take the compiled default */
    xdir_dir = dirpackg ? dirpackg : def_xdir_dir;

    /* package subdirectory */
    cp = strrchr (def_xdir_dir, '/');
    packg_subdir = ( cp ) ? cp : def_xdir_dir;

    /* build the configuration directory pathes */

    (void) memset (&stat_local, 0, sizeof(struct stat));
    (void) memset (&stat_alt_local, 0, sizeof(struct stat));

    if ( buildflg ) sprintf (strg, "%s%s%s", xdir_dir, helpdir, kbfilesdir);
    else sprintf (strg, "%s%s", xdir_dir, kbfilesdir);
    (void) set_dir_rec (&def_cnfg_dir, strg);

    sprintf (strg, "%s%s%s", syst_cnfg, packg_subdir, kbfilesdir);
    (void) set_dir_rec (&syst_cnfg_dir, strg);

    sprintf (strg, "%s%s%s", local_cnfg, packg_subdir, kbfilesdir);
    (void) set_dir_rec (&local_cnfg_dir, strg);
    if ( local_cnfg_dir.existing )
	(void) stat (local_cnfg_dir.path, &stat_local);

    if ( ! buildflg ) {   /* not debugging a new release of the editor */
	sprintf (strg, "%s%s%s%s", xdir_dir, alt_local_cnfg, packg_subdir, kbfilesdir);
	(void) set_dir_rec (&alt_local_cnfg_dir, strg);
	if ( alt_local_cnfg_dir.existing ) {
	    (void) stat (alt_local_cnfg_dir.path, &stat_alt_local);
	    if ( (stat_alt_local.st_dev = stat_local.st_dev)
		 && (stat_alt_local.st_ino = stat_local.st_ino) ) {
		free (alt_local_cnfg_dir.path);
		alt_local_cnfg_dir.existing = NO;
		alt_local_cnfg_dir.path = NULL;
	    }
	}
    }

    if ( mypath ) {
	/* The user HOME directory is known */
	/* build user kbfiles directory and file name */
	char tstrg [128], *tst, *mHost;

	/* user package directory */
	sprintf (strg, "%s/.%s", mypath, packg_subdir+1);
	(void) set_dir_rec (&user_packg_dir, strg);
	if ( ! user_packg_dir.path ) {
	    /* must never append, just to protect again NULL pointer */
	    user_packg_dir.path = mypath;
	    user_packg_dir.existing = YES;
	}

	/* user kbfiles directory and new build kbfile */
	sprintf (strg, "%s%s", user_packg_dir.path, kbfilesdir);
	(void) set_dir_rec (&user_cnfg_dir, strg);

	/* user kbmap file */
	memset (tstrg, '\0', sizeof (tstrg));
	mHost = ( fromHost ) ? fromHost : myHost;
	if ( mHost ) {
	    tstrg[0] = '_';
	    strncpy (&tstrg[1], mHost, sizeof (tstrg) -2);
	    for ( tst = tstrg ; ; ) {
		tst = strchr (tst, '.');
		if ( ! tst ) break;
		*tst = '-';
	    }
	}
	snprintf (strg, sizeof (strg),"%s/%s%s%s", user_packg_dir.path, tname, tstrg, kbmap_postfix);
	(void) set_dir_rec (&user_kbmap_file, strg);
	snprintf (strg, sizeof (strg), "%s/%s_%s%s", user_packg_dir.path, &tstrg[1], tname, kbfile_postfix);
	(void) set_dir_rec (&new_kbfile_file, strg);
	snprintf (strg, sizeof (strg), "%s/%s", user_packg_dir.path, preference);
	(void) set_dir_rec (&user_pref_file, strg);
	snprintf (strg, sizeof (strg), "%s/%s", user_packg_dir.path, debug);
	(void) set_dir_rec (&user_debug_file, strg);
    }

    /* get the max lenth of the config dir path name */
    max_cnfg_dir_sz = 0;
    for ( i = cnfg_dir_rec_pt_sz -1 ; i >= 0 ; i-- ) {
	if ( cnfg_dir_rec_pt[i] && cnfg_dir_rec_pt[i]->path )
	    if ( strlen (cnfg_dir_rec_pt[i]->path) > max_cnfg_dir_sz )
		max_cnfg_dir_sz = strlen (cnfg_dir_rec_pt[i]->path);
    }
}

static Flag mk_user_dir (struct cnfg_dir_rec *user_dir_pt)
{
    int cc;

    if ( ! user_dir_pt->path ) return NO;
    if ( ! user_dir_pt->existing ) {
	cc = mkdir (user_dir_pt->path, 0744);
	user_dir_pt->existing = ( access (user_dir_pt->path, R_OK) == 0 );
    }
    return user_dir_pt->existing;
}

/* get the expected user defined file (keyboard map or new_kbfile) */
static char * get_user_file_name (struct cnfg_dir_rec *user_file,
				  Flag create_dir, Flag *exist)
{
    Flag flg;
    int cc;

    if ( create_dir && user_packg_dir.path && !user_packg_dir.existing ) {
	flg = mk_user_dir (&user_packg_dir);
    }

    user_file->existing = user_file->path
			? ( access (user_file->path, R_OK) == 0 )
			: NO;
    if ( exist ) *exist = user_file->existing;
    return ( user_file->path );
}

char * get_new_kbfile_name (Flag create_dir, Flag *exist)
{
    return (get_user_file_name (&new_kbfile_file, create_dir, exist));
}

char * get_kbmap_file_name (Flag create_dir, Flag *exist)
{
    return (get_user_file_name (&user_kbmap_file, create_dir, exist));
}

char * get_pref_file_name (Flag create_dir, Flag *exist)
{
    return (get_user_file_name (&user_pref_file, create_dir, exist));
}

char * get_debug_file_name (Flag create_dir, Flag *exist)
{
    return (get_user_file_name (&user_debug_file, create_dir, exist));
}

/* Set up utility windows.
 * They have no margins, and they are not on winlist.
 */
void utility_setupwindow ()
{

    /* wholescreen window.  general utility */
    setupwindow (&wholescreen, 0, 0,
		 term.tt_width - 1, term.tt_height - 1, 0, NO);

    /* parameter entry window */
    setupwindow (&enterwin, 0,
		 term.tt_height -NENTERLINES - NINFOLINES,
		 term.tt_width - 1,
		 term.tt_height - 1 - NINFOLINES, 0, NO);
    enterwin.redit = term.tt_width - 1;

    /* info display window. */
    setupwindow (&infowin, 0,
		 term.tt_height - NINFOLINES,
		 term.tt_width - 1,
		 term.tt_height - 1, 0, NO);
}

/* Set up editing window (full screen, must be normaly window [0]) */
void edit_setupwindow (win, flgs)
S_window *win;  /* editing window */
AFlag flgs;     /* value of winflgs */
{
    setupwindow (win, 0, 0,
		 term.tt_width - 1, term.tt_height - 1 - NPARAMLINES, flgs, 1);
}


/* Build the names of the working files */
/*  If bld_flg is False, do not create any file, just get state file
 *      and return False if no state file is found
 */
static Flag build_wkfiles (Flag bld_flg)
{
    extern const char workfileprefix[];
    extern const char tmpnstr[];
    extern const char keystr[];
    extern const char bkeystr[];
    extern const char rstr[];
    extern const int vrschar;
    extern char evrsn;

    struct stat statbuf;
    int indv, fi, cc, cc1, cc2, sz;
    char *str, *str1;
    char * errmsg;  /* for debugging */
    char *name, *sp;
    char pwdname[PATH_MAX];
    Flag flg, mytmp_flg;

    name = NULL;
    memset (pwdname, 0, sizeof (pwdname));
    sp = getcwd (pwdname, sizeof (pwdname));

    if ( sp ) {
	strip_path (sp);
	/* when default_tmppath == tmpdir, this way does only 1 strcmp */
	if ( (strcmp (sp, default_tmppath) == 0) || (strcmp (sp, tmpdir) == 0) ) ;
	else {
	    /* probably not in '/tmp' directory,
	     * check current directory, and build working file prefix and postfix
	     * #define PRIV (S_IREAD | S_IWRITE | S_IEXEC) *
	     */
	    if (   stat (".", &statbuf) != -1
		&& access (".", (R_OK|W_OK|X_OK)) >= 0  /* read, write, exec acces mode */
		&& (   userid != 0
		    || ( (statbuf.st_uid == 0) || (statbuf.st_gid == 0) )
		   )
	       ) {
		/* for "root" user, user or group of the dir must be "root" */
		tmppath = (char *) workfileprefix;  /* use current directory for work files */
		if ( statbuf.st_uid == userid )
		    name = append ("", "");     /* so that name is allocated */
		else
		    name = append (".", myname); /* we will append user name */
	    }
	}
    }
    if ( ! name ) {
	/* no write acces into the current directory or in /tmp
	 * use either the $HOME/tmp or /tmp directory
	 */
	/* build the user tmp directory */
	flg = mk_user_dir (&user_tmp_dir);
	mytmp_flg = YES;
	if ( flg && (access (user_tmp_dir.path, R_OK|W_OK) == 0) ) {
	    tmppath = user_tmp_dir.path;
	} else if ( !tmppath || !*tmppath ) {
	    tmppath = tmpdir;
	    mytmp_flg = NO;
	}
	str1 = (char *) (workfileprefix +1);
	if ( (tmppath [strlen (tmppath) -1] == '/') ) str1++;
	str = append (tmppath, str1);
	tmppath = str;
	str = myname;
	if ( mytmp_flg && pwdname [0] ) {
	    str = strrchr (pwdname, '/');
	    if ( str ) str++;
	    else str = pwdname;
	}
	/* we will append user name or the directory name */
	name = append (".", str);
    }

    /* working file names */
    /* change file : .ec1 */
    str = append (tmpnstr, name);
    la_cfile = append (tmppath, str);
    sfree (str);
    /* state file : .es1 */
    str = append ((char *) rstr, name);
    rfile = append (tmppath, str);
    sfree (str);
    /* keystroke file : .ek1 */
    str = append ((char *) keystr, name);
    keytmp  = append (tmppath, str);
    sfree (str);
    /* alternate key stroke file : .ek1b */
    str = append (bkeystr, name);
    bkeytmp = append (tmppath, str);
    sfree (str);

    sfree (name);

    /* found a free session ('1', ... '9') or clean up aborted one */
    indv = strlen (tmppath) + vrschar;  /* index of the session number char */
    for ( evrsn = '1' ; ; evrsn++ ) {
	if (evrsn > '9') {
	    if ( ! bld_flg ) return NO;
	    la_cfile[indv] = rfile[indv] = keytmp[indv] = bkeytmp[indv] = '1';
	    getout (NO, "\n%s: No free session names left,\n    please delete unsed %s, %s, %s ... files \n",
		    progname, rfile, keytmp, bkeytmp);
	}

	la_cfile[indv] = rfile[indv] = keytmp[indv] = bkeytmp[indv] = evrsn;

	/* test if working files already exist  */
	/* look into the exit help to know what file exit in which condition */
	cc = access (la_cfile, F_OK);   /* change file exit ? */
	errmsg = strerror (errno);  /* for debugging */
	cc1 = access (keytmp, F_OK);    /* key stroke file exist ? */
	errmsg = strerror (errno);  /* for debugging */

	if ( cc1 >= 0 ) {
	    if ( notracks ) continue;
	    /* key stroke file exit, session in use or aborted or dumped */
	    /* check for locked : in use by another session */
	    fi = open (keytmp, O_RDWR);
	    if ( fi < 0 ) continue; /* strange case ! */
	    cc = lockf (fi, F_TLOCK, 0);
	    errmsg = strerror (errno);  /* for debugging */
	    close (fi);
	    if ( cc < 0 ) continue; /* locked : in use, check next one */

	    /* at this point the previous session was aborted or crashed */
	    if ( bld_flg ) {
		crashed = (cc >= 0);    /* crashed if change file exit */
		dorecov (crashed ? 0 : 1);      /* recover, do not clean files */
	    }
	    break;
	}

	/* test state file */
	cc = access (rfile, F_OK);
	errmsg = strerror (errno);  /* for debugging */
	if ( !bld_flg && cc >= 0 )
	    break;  /* state file found, ignore the notracks option */
	if ( !((cc >= 0) && notracks) )
	    break;  /* use this session number */
    }
    return YES;
}


#ifdef COMMENT
static void
startup ()
.
    Various initializations.
#endif
static void
startup ()
{
    extern void marktickfile ();
    extern char * from_host (char **hostname_pt, char **display_pt, const char *loopback_IP, char *** aliases_pt);
    extern char * canon_host (char *host, char *loopback, char ***aliases_pt);
    extern Flag get_keyboard_map ();
    extern char *TI, *KS, *VS;
    char *name, *str;
    char *helpd, *execd;
    char **aliases;

    /* ++XXXXXXXXXXXXXXXXXXXXXXXXXXXXX */

#ifdef DEF_XDIR
    extern char def_xdir_dir    [];
    extern char def_recovermsg  [];
    extern char def_xdir_kr     [];
    extern char def_xdir_help   [];
    extern char def_xdir_crash  [];
    extern char def_xdir_err    [];
    extern char def_xdir_run    [];
    extern char def_xdir_fill   [];
    extern char def_xdir_just   [];
    extern char def_xdir_center [];
    extern char def_xdir_print  [];
#endif

#if 0   /* old fation, version < 19.58 */
| #ifdef  SHORTUID
|     userid = getuid ();
|     groupid = getgid ();
| #else   /* uid is a char (old unix version) */
|     userid  = getuid () & 0377;
|     groupid = getgid () & 0377;
| #endif
#endif

#if 0   /* see get_id () */
|     userid = getuid ();
|     groupid = getgid ();
|     mypid = (int) getpid ();
|     myname = mypath = NULL;
|     (void) getmypath ();    /* gets myname & mypath */
#endif

    fromHost = myHost = fromDisplay = fromHost_alias = NULL;
    ssh_ip = sshHost = sshHost_alias = NULL;
    str = from_host (&myHost, &fromDisplay, loopback, &aliases);
    if ( str && str[0] ) {
	int i;
	fromHost = append (str, "");
	if ( aliases ) {
	    for ( i = 0 ; aliases [i] ; i++ ) ;
	    if ( i > 0 ) {
		fromHost_alias = append (aliases [i-1], "");
		if ( strcmp (fromHost_alias, fromHost) == 0 ) fromHost_alias = NULL;
	    }
	}
    } else fromHost = NULL;

    /* if running in a ssh session, get client info (ref : man ssh) */
    str = getenv ("SSH_CLIENT");
    if ( str ) {
	char *str1, **aliases;
	int sz, i;
	str1 = strchr (str, ' ');
	if ( str1 ) {
	    sz = str1 - str;
	    ssh_ip = salloc (sz +1, YES);
	    memcpy (ssh_ip, str, sz);
	    ssh_ip [sz] = '\0';
	    str = canon_host (ssh_ip, NULL, &aliases);
	    if ( str && *str ) {
		sshHost = ( fromHost && strcmp (str, fromHost) == 0 )
			  ? sshHost = fromHost
			  : append (str, "");
	    }
	    if ( str && aliases ) {
		for ( i = 0 ; aliases [i] ; i++ ) ;
		if ( i > 0 )  sshHost_alias = append (aliases [i-1], "");
	    }
	}
    }

    /* Get the selected terminal type */
    if (    !(tname = opttname)
	 && !(tname = getenv ("TERM") )
       )
	getout (YES, "No TERM environment variable or -terminal argument");

    la_cfile = keytmp = bkeytmp = rfile = inpfname = NULL;
    get_dirpackage ();

    helpd = ( buildflg ) ? helpdir : NULL;
    execd = ( buildflg ) ? execdir : NULL;

#ifdef DEF_XDIR
    recovermsg  = packg_file (def_recovermsg, helpd);
    xdir_kr     = packg_file (def_xdir_kr, helpd);
    xdir_help   = packg_file (def_xdir_help, helpd);
    xdir_crash  = packg_file (def_xdir_crash, helpd);
    xdir_err    = packg_file (def_xdir_err, helpd);
    xdir_run    = packg_file (def_xdir_run, execd);
    xdir_fill   = packg_file (def_xdir_fill, execd);
    xdir_just   = packg_file (def_xdir_just, execd);
    xdir_center = packg_file (def_xdir_center, execd);
    xdir_print  = packg_file (def_xdir_print, execd);

#else
    recovermsg  = packg_file (RECOVERMSG, helpd);
    xdir_kr     = packg_file (XDIR_KR, helpd);
    xdir_help   = packg_file (XDIR_HELP, helpd);
    xdir_crash  = packg_file (XDIR_CRASH, helpd);
    xdir_err    = packg_file (XDIR_ERR, helpd);
    xdir_run    = packg_file (XDIR_RUN, execd);
    xdir_fill   = packg_file (XDIR_FILL, execd);
    xdir_just   = packg_file (XDIR_JUST, execd);
    xdir_center = packg_file (XDIR_CENTER, execd);
    xdir_print  = packg_file (XDIR_PRINT, execd);
#endif /* DEF_XDIR */

    filterpaths[0] = xdir_fill;
    filterpaths[1] = xdir_just;
    filterpaths[2] = xdir_center;
    filterpaths[3] = xdir_print;

    /* --XXXXXXXXXXXXXXXXXXXXXXXXXXXXX */

    {
	Short i;
	for (i=0; i<MAXFILES ; i++) {
		names[i] = "";
/*
		oldnames[i] = "";
*/
	}
     }

#ifdef  SIGNALS
    {
	extern void sig_resize (int);

	Short  i;
	for (i = 1; i <= NSIG; ++i)
	    switch (i) {
	    case SIGINT:
		/* ignore so that replay questionnaire and exiting are
		 * not vulnerable
		 **/
		signal (i, SIG_IGN);
		break;

#ifdef SIGCHLD  /* 4bsd vmunix */
	    case SIGCHLD:
#endif
#ifdef SIGTSTP  /* 4bsd vmunix */
	    case SIGTSTP:
	    case SIGCONT:
#endif
#ifdef CERN     /* while we are debugging */
	    case SIGBUS:
	    case SIGSEGV:
#endif
		/* leave at SIG_DFL */
		break;

#ifdef SIGWINCH /* now I handle window size change (F.P.) */
	    case SIGWINCH:
		(void) signal (i, sig_resize);
		break;
#endif

	    default:
		if (signal (i, SIG_DFL) != SIG_IGN)
		    (void) signal (i, sig);
	    }
    }
#endif  /* SIGNALS */

#ifdef PROFILE
    {
	extern Flag profiling;
	/* sample only 1/8 of all editor runs */
	if (strttime & 7)
	    profil ((char *) 0, 0, 0, 0);
	else
	    profiling = YES;
    }
#endif

    /*
     *  Do this before setitty so that cbreakflg can be cleared if
     *  it is inappropriate for the type of terminal.
     */
    gettermtype ();
    /* stty on input to CBREAK or RAW and ~ECHO ASAP to allow typeahead */
    setitty ();

    {
	extern time_t time ();
	time (&strttime);
    }

    /* XXXXXXXXXXXXXX : do not open files in -help option */
    if (helpflg) {
	Xterm_global_flg = get_keyboard_map (tname, 4, TI, KS, VS, kbinistr);
	return;
    }

    /* stty on output for ~CRMOD and ~XTABS */
    setotty ();

    /* initialize terminal.  must be done before "starting..." message */
    /* so keys are initialized. */
    if (!silent) {
	d_put (VCCICL);
	windowsup = YES;
    }
    Xterm_global_flg = get_keyboard_map (tname, 4, TI, KS, VS, kbinistr);
    if (loginflg)
	printf ("%s is starting...", progname);
#ifdef  DEBUGDEF
    else
	printf ("terminal = %s,  keyboard = %s.\n\r", tname, kname);
    sleep (1);
#endif  /* DEBUGDEF */

#ifdef VBSTDIO
    if (outbuf = salloc (screensize+1, YES)) {
	setbuf (stdout, outbuf);
	stdout->_bufsiz = screensize;
    }
    else
	setbuf (stdout, (char *) _sobuf);
#endif

#ifdef NMN
    {
	Short  i;
	static Ff_buf ffbufhdrs[NUMFFBUFS];
	static char ffbufs[NUMFFBUFS * FF_BSIZE];

	for (i = 0; i < NUMFFBUFS; i++) {
	    ffbufhdrs[i].fb_buf = &ffbufs[i * FF_BSIZE];
	    ff_use ((char *)&ffbufhdrs[i], 0);
	}
    }
#else
    /* Donot use static buffers for OSF/1 ... */
    ff_alloc(NUMFFBUFS,0);
#endif

    la_maxchans = MAXSTREAMS;   /* Essentially no limit.  Limit is dealt */
				/* with in editfile () */

    /* set up cline
     * start with size of first increment
     **/
    cline = salloc ((lcline = icline) + 1, YES);

#ifdef BIGADDR
    la_nbufs = 20;
#else
    la_nbufs = 10;
#endif

    (void) build_wkfiles (YES);

    names [CHGFILE] = la_cfile;
    fileflags [CHGFILE] = INUSE;
    fileflags [NULLFILE] = INUSE;
    marktickfile (CHGFILE, NO);
    marktickfile (NULLFILE, NO);

    /* rename existing key stroke file to backup name */
    mv (keytmp, bkeytmp);
    keysmoved = YES;

    /* inpfname : file name to replay a session :
     *  if replay or recover from previous version
     *       replay from the previous key stroke file
     *  else replay file can be defined by command option : -replay=<filename>
     *  else it will be NUUL
     */
    inpfname = ( chosereplay ) ? bkeytmp : replfname;

    /* open and lock the new key stroke file */
    {
	int fi, cc;
	char * errmsg;

	keyfile = fopen (keytmp, "w");
	if ( keyfile == NULL ) {
	    printf ( "file error : %s\n", strerror (errno));
	    getout (YES, "Can't create keystroke file (%s).", keytmp);
	}
	fi = fileno (keyfile);
	fchmod (fi, 0600);
	cc = lockf (fi, F_TLOCK, 0);
	if ( cc < 0 )
	    errmsg = strerror (errno);  /* for debugging */
    }

    memset (&wholescreen, 0, sizeof (S_window));
    memset (&enterwin, 0, sizeof (S_window));
    memset (&infowin, 0, sizeof (S_window));
    utility_setupwindow ();

    curwin = &wholescreen;
    return;
}


static void display_env_var (char *buf, char *var_name, char *strg) {
    char *tmp;
    tmp = getenv (var_name);
    if ( !tmp || !tmp[0] ) tmp = strg;
    if ( tmp ) sprintf (buf + strlen (buf),
			"    %-7s = %s\n", var_name, tmp);
}

static int wait_cont ()
{
    extern int wait_continue ();
    char ch;
    int cc;

    if ( ! helpflg ) {
	cc = wait_continue (0);
	return (cc);
    }

    printf ("\nPush a key to continue, <Ctrl C> or Escape to end\n");
    ch = getchar ();
    if ( (ch == '\003') || (ch == '\033') ) return (1);
    (*term.tt_clear) ();
    return (0);
}

static void display_fn (char *buf, char *msg, char *fnm)
{
    if ( (strlen (msg) + strlen (fnm)) >= term.tt_width )
	sprintf (buf + strlen (buf), "%s\n  \"%s\"\n", msg, fnm);
    else
	sprintf (buf + strlen (buf), "%s \"%s\"\n", msg, fnm);
}

/* show_one_option : display the info about the option */
static void show_one_option (int idx, Flag full_flg, char *bigbuf, int bigbuf_sz,
			 Flag *opt_flgs)
{
    short val;
    int cc, type;
    char *fname, *cmt, *opt, tmpstrg [256];
    void *indic;
    Flag flag, file_flg;
    char chr, chr1;

    if ( (idx < 0) || (idx >= opttable_maxidx) ) return;
    val = opttable [idx].val;

    if ( (val < 0) || (val >= MAX_opt) ) return;
    if ( opt_flgs[val] ) return;
    opt_flgs[val] = YES;

    type = opt_desc [val].type;
    indic = opt_desc [val].indic;
    file_flg = NO;
    switch (type) {
	case -99 :  /* default, inverted logic */
	case  99 :  /* default, normal logic */
	case  -1 :  /* inverted logic */
	case   0 :
	    flag = NO;
	    if ( indic ) flag = *((Flag *) indic);
	    if ( flag < 0 ) flag = NO;  /* flag not set */
	    else if ( type < 0 ) flag = !flag;
	    if ( !full_flg && !flag ) return;
	    if ( flag && (abs(type) == 99) ) return;    /* default value */
	    chr = flag ? '*' : ' ';
	    break;

	case 1:     /* file name */
	case 2:     /* directory name */
	    fname = NULL;
	    if ( indic ) fname = *((char **) indic);
	    if ( !full_flg && !fname ) return;
	    chr = fname ? '*' : ' ';
	    file_flg = YES;
	    if ( !fname )
		fname = (type == 1) ? "<filename>" : "<directory>";
	    break;

	default:
	    return;
    }

    cmt = opt_desc [val].comment;
    chr1 = 0;

    /* special cases */
    switch (val) {
	case OPTDEBUG :
	    memset (tmpstrg, 0, sizeof (tmpstrg));
	    if ( dbgname ) {
		sprintf (tmpstrg, "%s:%d : level %2$d", dbgname, DebugVal);
		cmt = dbgflg ? "debugging actif" : "debugging suspended";
		chr1 = '=';
	    } else
		strcpy (tmpstrg, "[debug output file name]:[debug level]");
	    fname = tmpstrg;
	    break;
    }

    if ( ! cmt ) cmt = "";
    opt = opttable [idx].str;

    if ( full_flg )
	sprintf (bigbuf + strlen (bigbuf), "%c -%s", chr, opt);
    else
	sprintf (bigbuf + strlen (bigbuf), " -%s", opt);
    if ( file_flg ) {
	sprintf (bigbuf + strlen (bigbuf), "=%s", fname);
    }
    if ( cmt && *cmt )
	sprintf (bigbuf + strlen (bigbuf), " %c %s\n", chr1 ? chr1 : ':', cmt);
    else strcat (bigbuf, "\n");
    return;
}

/* Show Help processing */
static void do_showhelp (full_flg)
Flag full_flg;    /* ful dispaly */
{
    extern char * filestatusString ();
    extern char kbmap_fn[];
    extern char verstr[];
    extern char * la_max_size ();
    extern char * mapping_strg ();
    extern char * get_session_pref (char **stname_pt, Flag *use_flg_pt);

    static char char_str [] =
#ifdef CHAR7BITS
		 "  Built for ASCII (7 bits) characters only\n\n";
#else
		 "  Built for ISO 8859 (8 bits) characters\n\n";
#endif

    char *pref_name, *stname;
    Flag use_flg;
    void getConsoleSize (int *width, int *height);
    static int display_bigbuf ();
    int i, nbli, ctrlc, nb, idx;
    char bigbuf [8192]; /* must be large enough for the message */
    char strg [256];
    char *tmpstrg, *str, *str1, *str2;
    int width, height;
    Flag opt_flgs [MAX_opt];
    nbli = 0;
    memset (bigbuf, 0, sizeof (bigbuf));

    if ( helpflg ) width = height = 2048;  /* large enought */
    else getConsoleSize (&width, &height);

    if ( verbose_helpflg ) printf ("\
----------------------------------------------------------------------------\n");
    else fputc ('\n', stdout);
    if ( full_flg ) 
	sprintf (bigbuf + strlen (bigbuf), "\
Synopsis: %s [options] file [alternate file]\n\
 options are:\n", progname);
    else {
	sprintf (bigbuf + strlen (bigbuf), "\
Overall status of the editor parameters\n\
=======================================\
\n");
	tmpstrg = asctime (localtime (&strttime));
	str = strrchr (tmpstrg, ' ');
	if ( str ) *str = '\0'; /* remove the year */
	str = strrchr (tmpstrg, ' ');
	str = (str) ? str +1 : tmpstrg; /* only the time */
	nb = sprintf (bigbuf + strlen (bigbuf),
		      "Session by \"%s\" at %s on \"%s\"",
		      myname, str, myHost);
	memset (strg, 0, sizeof (strg));
	if ( ssh_ip ) {
	    nb += sprintf (strg, " ssh session from \"%s\"\n",
			   sshHost_alias ? sshHost_alias
					 : sshHost ? sshHost : ssh_ip);
	    if ( nb < width ) {
		strcat (bigbuf, strg);
		memset (strg, 0, sizeof (strg));
		nb = 0;
	    }
	}
	if ( fromHost ) {
	    if ( fromHost != sshHost ) {
		nb += sprintf (&strg[strlen (strg)], " from \"%s\"", fromHost);
		if ( fromHost_alias )
		    nb += sprintf (&strg[strlen (strg)], " (%s)", fromHost_alias);
	    }
	    if ( fromDisplay )
		nb += sprintf (&strg[strlen (strg)], " On display %s", fromDisplay);
	    strg[strlen (strg)] = '\n';
	    if ( nb >= width ) strg[0] = '\n';
	} else strg[0] = '\n';
	strcat (bigbuf, strg);

	pref_name = get_session_pref (&stname, &use_flg);
	if ( state_reused_flg ) {
	    sprintf (bigbuf + strlen (bigbuf),
		     "  Session setup by previous session status file : \"%s\"\n", rfile);
	    if ( pref_name && use_flg )
		sprintf (bigbuf + strlen (bigbuf),
			 "  and by \"[%s]\" section from \"%s\"\n",
			 stname, pref_name);
	} else {
	    if ( pref_name ) {
		if ( use_flg ) {
		    sprintf (bigbuf + strlen (bigbuf),
			     "  Session setup by \"[%s]\" terminal section from user preferences file :\n    \"%s\"\n",
			     stname, pref_name);
		} else {
		    sprintf (bigbuf + strlen (bigbuf),
			     "  No section for \"[%s]\" terminal in user preferences file :\n    \"%s\"\n",
			     stname, pref_name);
		    strcat (bigbuf, "    Use \"set preferences\" command to generate the section\n");
		}
	    }
	}

	strcat (bigbuf, verstr); bigbuf[strlen (bigbuf)] = '\n';
	sprintf (bigbuf + strlen (bigbuf),
		 "  Built for files with %s\n    and max number of edited files : %d, max opened files : %d\n",
		 la_max_size (),
		 (MAXFILES - (FIRSTFILE + NTMPFILES)),
		 (MAXOPENS - NTMPFILES));
	strcat (bigbuf, char_str);
	if ( optusedflg ) sprintf (bigbuf + strlen (bigbuf),
			"In use command line program options :\n");
	else sprintf (bigbuf + strlen (bigbuf),
			"No in use command line program option\n");
    }

    if ( full_flg )
    sprintf (bigbuf + strlen (bigbuf), "\
  - : end the list of option (to be used before a file name starting with '-')\n");

    if ( full_flg || optusedflg ) {
	memset (opt_flgs, 0, sizeof (opt_flgs));
	for ( idx = 0 ; idx < opttable_maxidx ; idx++ ) {
	    show_one_option (idx, full_flg, bigbuf, sizeof (bigbuf), &opt_flgs[0]);
	}
    }

    if ( full_flg && optusedflg )
	sprintf (bigbuf + strlen (bigbuf),
		 "\"*\" means this option is in effect.\n");

    if ( full_flg && dbgflg )
	sprintf (bigbuf + strlen (bigbuf), "\
NB : -debug=debug_file_name option cannot be use by an indirect call\n\
     using a batch file. Only the default debug output can be used !\n");

    sprintf (bigbuf + strlen (bigbuf), "\n\
Environment variables known by Rand editor:\n\
  EKBD, EKBFILE, RAND_PACKG, SHELL, TERM, VBELL and editalt\n");
    display_env_var (bigbuf, "EKBD", NULL);
    display_env_var (bigbuf, "EKBFILE", NULL);
    display_env_var (bigbuf, "RAND_PACKG", NULL);
    display_env_var (bigbuf, "SHELL", NULL);
    display_env_var (bigbuf, "TERM", "(mandatory if \"-terminal\" not used)");
    display_env_var (bigbuf, "VBELL", NULL);
    display_env_var (bigbuf, "editalt", helpflg ? "Alternate edited file name" : NULL);
    bigbuf [strlen (bigbuf)] = '\n';

    ctrlc = display_bigbuf (bigbuf, &nbli, sizeof (bigbuf));
    if ( ctrlc ) return;

    str1 = mapping_strg (&str2);
    if ( Xterm_global_flg ) {
	extern char *emul_name, *emul_class, *wm_name;
	extern void xterm_msg ();

	xterm_msg (bigbuf + strlen (bigbuf));
	sprintf (bigbuf + strlen (bigbuf), "  %s\n", str1);
	if ( str2 ) sprintf (bigbuf + strlen (bigbuf), "  %s\n", str2);
	if ( termtype )
	    sprintf (bigbuf + strlen (bigbuf), "  \"%s\" is using compiled description\n",
		     tname);
	else
	    sprintf (bigbuf + strlen (bigbuf), "  \"%s\" is using terminfo data for \"%s\"\n",
		     tname, tcap_name);
    } else {
	extern void term_msg ();

	term_msg (bigbuf + strlen (bigbuf));
	if ( termtype )
	    sprintf (bigbuf + strlen (bigbuf), "Terminal : \"%s\" (using compiled description), keyboard : %s (%d)\n",
		     tname, kname, kbdtype);
	else
	    sprintf (bigbuf + strlen (bigbuf), "Terminal : \"%s\" (using terminfo data for \"%s\"), keyboard : %s (%d)\n",
		     tname, tcap_name, kname, kbdtype);
	sprintf (bigbuf + strlen (bigbuf), "  %s\n", str1);
	if ( str2 ) sprintf (bigbuf + strlen (bigbuf), "  %s\n", str2);
    }

    if ( kbfile ) {     /* use keyboard definition file */
	extern char * get_keyboard_mode_strg ();
	extern int kbfile_wline;    /* > 0 : duplicated string in kbfile */
	int cc;

	if ( kbfcase )
	    sprintf (strg, "Keyboard definition file (defined by %s):", kbfcase);
	else
	    strcpy (strg, "Keyboard definition file:");
	display_fn (bigbuf, strg, kbfile);
	cc = check_access (kbfile, R_OK, &tmpstrg);
	if ( cc ) {
	    Flag ok;
	    ok = NO;    /* warning not printed */
	    if ( cc == 2 ) {    /* not exiting, check directory */
		char *fpt, *msg;
		fpt = strrchr (kbfile, '/');
		if ( fpt ) {    /* directory specified */
		    *fpt = '\0';
		    if ( check_access (kbfile, R_OK, &msg) ) {
			char *dpt;
			dpt = strrchr (kbfile, '/');
			dpt = ( dpt ) ? dpt+1 : kbfilesdir;
			sprintf (bigbuf + strlen (bigbuf),
				 "    WARNING %s directory :%s\n", dpt, msg);
			ok = YES;
		    }
		    *fpt = '/';     /* rebuild the kfile name */
		}
	    }
	    if ( !ok ) sprintf (bigbuf + strlen (bigbuf),
				"    WARNING kbfile :%s\n", tmpstrg);
	} else if ( kbfile_wline > 0 ) {
	    sprintf (bigbuf + strlen (bigbuf),
		     "    WARNING kbfile : duplicated string at line %d\n", kbfile_wline);
	}
	sprintf (bigbuf + strlen (bigbuf), "Keyboard expected mode : %s\n",
		 get_keyboard_mode_strg ());
    }
    else {      /* use build in compiled keyboard definition */
	if ( stdkbdflg )
	    sprintf (bigbuf + strlen (bigbuf), "Use internal mini definition for %s (forced by \"-dkeyboard\" option)\n", kname);
	else
	    if ( kbfcase )
		sprintf (bigbuf + strlen (bigbuf), "Use internal definition for %s (%s)\n", kname, kbfcase);
	    else
		sprintf (bigbuf + strlen (bigbuf), "No keyboard definition file, use internal definition for %s\n", kname);
    }

    if ( helpflg ) {
	static int get_kbfile_dname (char *, char *, int, char **);
	S_looktbl *slpt;

	sprintf (bigbuf + strlen (bigbuf), " Build in terminals & keyboards :");
	for ( slpt = &termnames[0] ; slpt->str ; slpt ++ )
	    sprintf (bigbuf + strlen (bigbuf), " \"%s\",", slpt->str);
	bigbuf [strlen (bigbuf)] = '\n';
	sprintf (bigbuf + strlen (bigbuf), " Expect \"%s%s\" (term family) or \"%s\" keyboard definition file in :\n",
		 tname, kbfile_postfix, universal_kbf_name);
	if ( buildflg ) {   /* debugging a new release of the editor */
	    for ( i = cnfg_dir_rec_pt_sz -1 ; i >= 0 ; i-- ) {
		if ( cnfg_dir_rec_pt[i] == NULL ) continue;
		if ( cnfg_dir_rec_pt[i]->path )
		    sprintf (bigbuf + strlen (bigbuf), "  %s%s\n",
			     cnfg_dir_rec_pt[i]->path,
			     cnfg_dir_rec_pt[i]->existing ? "" : no_exist_msg);
	    }
	} else {
	    for ( i = 0 ; i < cnfg_dir_rec_pt_sz ; i++ ) {
		if ( cnfg_dir_rec_pt[i] == NULL ) continue;
		if ( cnfg_dir_rec_pt[i]->path )
		    sprintf (bigbuf + strlen (bigbuf), "    %s%s\n",
			     cnfg_dir_rec_pt[i]->path,
			     cnfg_dir_rec_pt[i]->existing ? "" : no_exist_msg);
	    }
	}
	if ( user_packg_dir.path ) {
	    str = user_kbmap_file.path;
	    if ( !str || !*str ) str = "??? not defined !!!";
	    sprintf (bigbuf + strlen (bigbuf), " My (\"%s\") \"%s\" keyboard map file :\n    %s%s\n",
		     myname, tname, str,
		     user_kbmap_file.existing ? "" : no_exist_msg);
	}
    }
    bigbuf [strlen (bigbuf)] = '\n';

    sprintf (strg, "%sRand package %sdirectory path%s:",
	     dirpackg ? ""              : "Default buid in ",
	     buildflg ? ""              : "and HELP ",
	     buildflg ? " (build mode)" : ""
	    );

    display_fn (bigbuf, strg, xdir_dir);
    if ( check_access (xdir_dir, R_OK, &tmpstrg) )
	sprintf (bigbuf + strlen (bigbuf), "    WARNING : %s\n", tmpstrg);
    else {
	static void check_message_file ();
	check_message_file (recovermsg, bigbuf);
	/* check_message_file (xdir_kr   , bigbuf);  -- no more in use */
	check_message_file (xdir_help , bigbuf);
	check_message_file (xdir_crash, bigbuf);
	check_message_file (xdir_err  , bigbuf);
    }

    if ( ! helpflg ) {
	display_fn (bigbuf, "Default directory:",
		    default_workingdirectory);
	tmpstrg = get_cwd ();
	if ( tmpstrg )
	    display_fn (bigbuf, "Current working directory:", tmpstrg);
    }

    /* working files */
    if ( keytmp && rfile && names[CHGFILE] ) {
	sprintf (bigbuf + strlen (bigbuf), "Current Editor status, key stroke, changes files :\n  %s\n  %s %s\n  %s\n",
		rfile, keytmp, bkeytmp, names[CHGFILE]);
    }
    strcat (bigbuf + strlen (bigbuf), "Edited files (use 'help ?file' for details on status value) :\n");
    for ( i = FIRSTFILE + NTMPFILES ; i < MAXFILES ; i++) {
	char *fname;
	if ( !(fileflags[i] & INUSE) ) continue;
	tmpstrg = filestatusString (i, &fname);
	if ( tmpstrg )
	    sprintf (bigbuf + strlen (bigbuf), "  %s%s\n", tmpstrg, fname);
    }

    if ( full_prgname )
	display_fn (bigbuf, "Rand program:", full_prgname);

    sprintf (bigbuf + strlen (bigbuf), "Current user : %s (uid = %d, gid = %d)\n",
	myname, userid, groupid);

    if ( helpflg ) {
	sprintf (bigbuf + strlen (bigbuf),
		 "\nThis is %s : Rand editor version %d release %d\n%s\n",
		 prog_fname ? prog_fname : progname, -revision, subrev, verstr);
    }
    else {
	extern void setoption_msg ();
	char *fname, buf[128];

	sprintf (bigbuf + strlen (bigbuf), "\nCurrent screen size %d lines of %d columns\n", height, width);
	setoption_msg (buf);
	sprintf (bigbuf + strlen (bigbuf), "Current value of SET command paramters (use 'help set' command for details) \n%s\n", buf);

	fname = get_debug_name ();
	if ( fname && *fname ) {
	    /* debugging session */
	    sprintf (bigbuf + strlen (bigbuf), "Debuging output file : \"%s\"\n",
		     fname);
	    sprintf (bigbuf + strlen (bigbuf), "   Debug %s", dbgflg ? "ON" : "OFF");
	    if ( dbgflg )
		sprintf (bigbuf + strlen (bigbuf), " at level %d", DebugVal);
	    strcat (bigbuf, "\n");
	}
	strcat (bigbuf, "\n");
    }

    ctrlc = display_bigbuf (bigbuf, &nbli, sizeof (bigbuf));
    return;
}

static void check_message_file (char *fname, char *buf)
{
    char *strg, *fn;
    int cc;

    cc = check_access (fname, R_OK, &strg);
    if ( cc == 0 ) return;
    fn = strrchr (fname, '/');
    fn = ( fn ) ? fn+1 : fname;
    sprintf (buf + strlen (buf), "    WARNING : Rand package file \"%s\" :%s\n",
	     fn, strg);
}

static int display_bigbuf (char *buf, int *nbli, int bufsz)
{
    int nb, dispsz, ctrlc;
    char ch, *sp0, *sp1;

    ctrlc = NO;
    dispsz = term.tt_height;
    if ( verbose_helpflg ) dispsz = 100000;     /* very very large : do not hold the screen */
    nb = 0;
    for ( sp0 = sp1 = buf ; (ch = *sp1) ; sp1++ ) {
	if ( ch != '\n' ) continue;
	(*nbli)++;
	nb++;
	if ( *nbli < (dispsz -2) ) continue;
	*sp1 = '\0';
	fputs (sp0, stdout);
	fputc ('\n', stdout);
	fputc ('\n', stdout);
	sp0 = sp1 +1;
	ctrlc = wait_keyboard ("Push a key to continue, <Ctrl C> to exit", NULL);
	fputs ("                                         \r", stdout);
	*nbli = 2;  /* overlap size */
	(*term.tt_addr) (dispsz -2, 0);
	if ( ctrlc && !verbose_helpflg ) {
	    *sp0 = '\0';
	    break;
	}
    }
    if ( *sp0 ) fputs (sp0, stdout);
    if ( dbgflg ) {
	fprintf (dbgfile, "-- debug -- message size %d, nb of lines %d\n", strlen (buf), nb);
#ifdef __GNUC__
	debug_info (__FUNCTION__, __FILE__, __LINE__);
#endif
    }
    memset (buf, 0, bufsz);
    if ( verbose_helpflg ) ctrlc = NO;
    return ctrlc;
}

#ifdef COMMENT
void
showhelp ()
.
    Do the help option.
#endif
void
showhelp ()
{
    extern int print_it_escp ();
    extern void print_all_ccmd ();
    extern void display_keymap ();

    void reset_crlf (int);
    int oflag;

    oflag = set_crlf ();
    do_showhelp (YES);
    if ( verbose_helpflg ) {
	extern int help_info (char *, int *);
	if ( print_it_escp () >= 0 ) {  /* print the it table tree (escape seq to ccmd) */
	    print_all_ccmd ();  /* print ccmd assignement (escape seq) */
	    display_keymap (verbose_helpflg);
	    (void) help_info ("Linux_key_mapping", NULL);
	}
    }
    reset_crlf (oflag);
}

void getConsoleSize (int *width, int *height) {
    *width = term.tt_width;
    *height = term.tt_height;
}

void showstatus ()
{
    extern void setoption_msg ();
    int x, y;
    char buf[128];

    do_showhelp (NO);
}


#ifdef COMMENT
static void
dorecov (type)
    int type;
.
    Ask the user how he wants to deal with a crashed or aborted session.
    Type is 0 for crashed, 1 for aborted.
    If the user opts for a recovery or replay, anything which was set by
    checkargs() must be set here to the way we want it for the replay.
#endif
static void
dorecov (type)
int type;
{
    if ( helpflg || norecover )
	return;

    fixtty ();
    printf("\n"); fflush(stdout);

    for ( ;; ) {
	char line [132];
	{
	    int tmp;

	    tmp = dup (1);      /* so that filecopy can close it */
	    if (   (tmp = filecopy (recovermsg, 0, (char *) 0, tmp, NO)) == -1
		|| tmp == -3
	       )
		fatalpr ("Please notify the system administrators\n\
that the editor couldn't read file \"%s\".\n", recovermsg);
	}
	printf("\n\
Type the number of the option you want then hit <RETURN>: ");
	fflush (stdout);

	memset (line, 0, sizeof (line));
	fgets (line, sizeof (line), stdin);
	if ( feof (stdin) ) {
	    /* do nothing, return to shell, equivalent to case '4' */
	    getout (type, "");
	    break;
	}

	switch (*line) {
	case '1':
	case '2':
	    if ( *line == '1' ) {
		/* '1' = silently recover */
		recovering = YES;
		silent = YES;
	    } else {
		/* '2' = replay (emergency recover) */
		recovering = NO;
		silent = NO;
	    }
	    replaying = YES;
	    helpflg = NO;
	    notracks = NO;
	    chosereplay = YES;
	    break;

	case '3':
	    /* ignore the previous session */
	    break;

	case '4':
	    /* silently return to the shell */
	    getout (type, "");

	default:
	    /* any other answer : re ask what to do */
	    continue;
	}
	break;  /* the for loop */
    }
    setitty ();
    setotty ();
    return;
}

/* set, reset the terminal text character set */
/* ------------------------------------------ */
/* customisation of the character set used to display text is
   not implemented. The default terminal char set is in use
*/

static void (*reset_term_proc) () = NULL;

void reset_term ()
{
    if ( reset_term_proc ) (*reset_term_proc) ();
    reset_term_proc = NULL;
}

void set_term ()
{
    extern void (*set_charset (char *)) ();
    reset_term_proc = set_charset (tname);
}


/* build_kbfile_name : build the default keyboard definition file name */
/* ------------------------------------------------------------------- */

static char * get_kbfile_buf (char *term_family, int *buf_sz)
{
    char * kbfname;
    int sz;

    *buf_sz = 0;
    if ( ! xdir_dir ) return (NULL);

    sz = strlen (term_family) + strlen (kbfile_postfix);
    if ( sz < sizeof (universal_kbf_name) ) sz = sizeof (universal_kbf_name);

    sz += max_cnfg_dir_sz +2;
    kbfname = (char *) malloc (sz);
    if ( kbfname ) memset (kbfname, 0, sz);
    *buf_sz = sz;
    return (kbfname);
}

static int get_kbfile_dname (char *term_family,
			     char * kbfile_buf, int kbfile_buf_sz,
			     char **kbfcase_pt)
{
    /* kbfile_buf must be large enough */
    char * kbfname;
    int sz;

    if ( ! xdir_dir ) return (0);

    memset (kbfile_buf, kbfile_buf_sz, 0);
    if ( term_family ) {    /* terminal family default */
	sprintf (kbfile_buf, "%s%s/%s%s",
		 xdir_dir, kbfilesdir, term_family, kbfile_postfix);
	if ( kbfcase_pt )
	    *kbfcase_pt = "package directory and terminal name";
    } else {    /* general purpose default */
	sprintf (kbfile_buf, "%s%s/%s",
		 xdir_dir, kbfilesdir, universal_kbf_name);
	if ( kbfcase_pt )
	    *kbfcase_pt = "package directory and all purpose kb file";
    }
    return (strlen (kbfile_buf));
}

static char * find_kbfile (int buf_sz, char * tname, char * cnfg_dir, char ** cnfg)
{
    memset (kbfile, 0, buf_sz);
    if ( ! cnfg_dir ) return (NULL);

    if ( ! *cnfg ) *cnfg = cnfg_dir;
    sprintf (kbfile, "%s/%s%s", cnfg_dir, tname, kbfile_postfix);
    if ( access (kbfile, R_OK) >= 0 ) return ("terminal name kb file");

    memset (kbfile, 0, buf_sz);
    sprintf (kbfile, "%s/%s", cnfg_dir, universal_kbf_name);
    if ( access (kbfile, R_OK) >= 0 ) return ("all purpose kb file");

    memset (kbfile, 0, buf_sz);
    return (NULL);
}


static void build_kbfile_name ()
{
    int i, buf_sz, sz;
    char *cnfg;

    /* get the terminal type default kbfile name */
    buf_sz = 0;
    kbfile = get_kbfile_buf (tname, &buf_sz);
    if ( ! kbfile ) return;

    cnfg = NULL;
    if ( buildflg ) {   /* debugging a new release of the editor : reversed priority */
	for ( i = cnfg_dir_rec_pt_sz -1 ; i >= 0 ; i-- ) {
	    if ( cnfg_dir_rec_pt[i] == NULL ) continue;
	    if ( ! cnfg_dir_rec_pt[i]->existing ) continue;
	    kbfcase = find_kbfile (buf_sz, tname, cnfg_dir_rec_pt[i]->path, &cnfg);
	    if ( kbfcase ) return;
	}
    } else {
	for ( i = 0 ; i < cnfg_dir_rec_pt_sz ; i++ ) {
	    if ( cnfg_dir_rec_pt[i] == NULL ) continue;
	    if ( ! cnfg_dir_rec_pt[i]->existing ) continue;
	    kbfcase = find_kbfile (buf_sz, tname, cnfg_dir_rec_pt[i]->path, &cnfg);
	    if ( kbfcase ) return;
	}
    }
    sprintf (kbfile, "%s/%s%s", cnfg, tname, kbfile_postfix);
    kbfcase = "file not existing or no read access";
}

/* get_kbfile_name : get or build the kbfile name */
/* ---------------------------------------------- */
static void get_kbfile_name () {

    kbfile = NULL;
    if ( stdkbdflg ) kbfcase = "-dkeyboard";    /* force use of the compiled standard */
    else if ( (kbfile = optkbfile) )
	kbfcase = ( access (kbfile, R_OK) < 0 ) ?
		    "-kbfile option (file not existing or no read access)"
		  : "-kbfile option";
    else if ( (kbfile = getenv ("EKBFILE")) )
	kbfcase = ( access (kbfile, R_OK) < 0 ) ?
		    "EKBFILE (file not existing or no read access)"
		  : "EKBFILE";
    else {
	/* build the default keyboard definition file name */
	build_kbfile_name ();
    }
}


#ifdef COMMENT
void
gettermtype ()
.
    Get the terminal type.  If Version 7, get the TERM environment variable.
    Copy the appropriate terminal and keyboard structures from the tables
    in e.tt.c into the terminal structure to be used for the session.
#endif
void
gettermtype ()
{
    extern int GetCccmd ();

    /* tname must be already got */
    if ( !tname )
	getout (YES, "No TERM environment variable or -terminal argument");
    {
	int ind;
	if ((ind = lookup (tname, termnames)) >= 0) {
	    kbdtype = termnames[ind].val; /* assume same as term for now */
	    kbfcase = "known internal terminal";
	}
	else
	    kbdtype = 0;

	if (   ind >= 0 /* we have the name */
#ifdef  TERMCAP
	    && !optdtermcap /* we aren't forcing the use of termcap */
	    && tterm[termnames[ind].val] != tterm[0] /* not a termcap type */
#endif
	    && tterm[termnames[ind].val] /* we have compiled-in code */
	   )
	    termtype = termnames[ind].val;
#ifdef  TERMCAP
	else {      /* use termcap or terminfo */
	    char *str, *str1;

	    switch (getcap (tname, tcap_name_default, &str1)) {
	    default:
		termtype = 0; /* for terminal defined by termcap, the type is 0 */
		tcap_name = str1;
		break;
	    case -1:
		str = "known";
		goto badterm;
	    case -2:
		str = "usable";
	    badterm:
		getout (YES, "Un%s terminal type: \"%s\"\n  %s", str, tname, str1);
	    }
	}
#else
	else
	    getout (YES, "Unknown terminal type: \"%s\"", tname);
#endif
    }

    /* Get the selected keyboard type */
    if ( stdkbdflg ) kname = stdk;  /* use the standard compiled */
    else if ( (kname = optkname) ) kbfcase = "\"-keyboard\" option";
    else if ( (kname = getenv ("EKBD")) ) kbfcase = "EKBD variable";
    if ( kname ) {
	int ind;
	if (   (ind = lookup (kname, termnames)) >= 0
	    && tkbd[termnames[ind].val] /* we have compiled-in code for it */
	   )
	    kbdtype = termnames[ind].val;
	else
	    getout (YES, "Unknown keyboard type: \"%s\"", kname);
    }
    else if ( kbdtype )
	kname = tname;
    else
	kname = stdk;

    /* select the routines for the terminal type and keyboard type */
    move ((char *) tterm[termtype], (char *) &term, (Uint) sizeof (S_term));
    move ((char *) tkbd[kbdtype],   (char *) &kbd,  (Uint) sizeof (S_kbd));

    if (term.tt_width > MAXWIDTH)
	term.tt_width = MAXWIDTH;

#ifdef  KBFILE
    /** should integrate this stuff as a keyboard type */
    /** Don't document it until it is done that way */
    /* Get the keyboard file if specified */

    kbfile = NULL;  /* DEFAULT : use internal definition */
    get_kbfile_name ();

    if ( kbfile ) {
	extern int in_file();
	extern int nop ();

	set_term ();    /* xlate tables must be filled before processing kbfile */
	if ( getkbfile (kbfile) ) kbd.kb_inlex = in_file;
	else kname = stdk;
	/* -------
	kbd.kb_init  = nop;
	kbd.kb_end   = nop;
	----- */
    }
#endif

    if ( GetCccmd () < 0 )
	getout (YES, "A control character (Ctrl A ... Ctrl Z) must be assigned to <cmd> key function");

    if ( ! helpflg ) {
	d_put (VCCINI); /* initializes display image for d_write */
			/* and selects tt_height if selectable */

	/* initialize the keyboard */
	(*kbd.kb_init) ();
    }

    {
	int tmp;
	tmp = term.tt_naddr;
	tt_lt2 = term.tt_nleft  && 2 * term.tt_nleft  < tmp;
	tt_lt3 = term.tt_nleft  && 3 * term.tt_nleft  < tmp;
	tt_rt2 = term.tt_nright && 2 * term.tt_nright < tmp;
	tt_rt3 = term.tt_nright && 3 * term.tt_nright < tmp;
    }

    if (optbullets >= 0)
	borderbullets = optbullets;
    else if (borderbullets)
	borderbullets = term.tt_bullets;
    return;
}

#ifdef COMMENT
void
setitty ()
.
    Set the tty modes for the input tty.
#endif
void
setitty ()
{
#ifdef SYSIII

#define BITS(start,yes,no) start  = ( (start| (yes) )&( ~(no) )  )
    struct termio temp_termio;

#   ifdef CBREAK
	char ixon = cbreakflg;
#   else
	char ixon = NO;
#   endif

    if ( ioctl (STDIN, TCGETA, &in_termio) < 0 )
	return;
    temp_termio = in_termio;
    temp_termio.c_cc[VMIN]=1;
    temp_termio.c_cc[VTIME]=0;

#ifdef CHAR7BITS
    BITS (temp_termio.c_iflag,
	  IGNPAR|ISTRIP,
	  IGNBRK|BRKINT|PARMRK|INPCK|INLCR|IGNCR|ICRNL|IUCLC|IXOFF
	);
#else
    BITS (temp_termio.c_iflag,
	  IGNPAR,
	  IGNBRK|BRKINT|PARMRK|INPCK|INLCR|IGNCR|ICRNL|IUCLC|IXOFF|ISTRIP
	);
    BITS (temp_termio.c_cflag, CS8, PARENB);
#endif

    if ( ixon ) temp_termio.c_iflag  |= IXON;
    else        temp_termio.c_iflag  &= ~IXON;

    BITS (temp_termio.c_lflag,NOFLSH,ISIG|ICANON|ECHO);

    if ( ioctl (STDIN, TCSETAW, &temp_termio) >= 0 ) istyflg=YES;
    fcntlsave = fcntl (STDIN, F_GETFL, 0);
    return;
#undef BITS


#else /* -SYSIII */

    if ( ioctl (STDIN, TIOCGETP, &instty ) < 0)
	return;

#ifdef  CBREAK
#ifdef  TIOCGETC
    if (cbreakflg) {
	static struct tchars tchars = {
	    0xff,       /* char t_intrc;    * interrupt */
	    0xff,       /* char t_quitc;    * quit */
	    'Q' & 0x1f, /* char t_startc;   * start output */
	    'S' & 0x1f, /* char t_stopc;    * stop output */
	    0xff,       /* char t_eofc;     * end-of-file */
	    0xff,       /* char t_brkc;     * input delimiter (like nl) */
	};
	if (   ioctl (STDIN, TIOCGETC, &spchars) < 0
	    || tchars.t_startc != spchars.t_startc
	    || tchars.t_stopc  != spchars.t_stopc
	    || ioctl (STDIN, TIOCSETC, &tchars) < 0
	   )
	    cbreakflg = NO;
    }
#endif

#ifdef  TIOCGLTC
    if (cbreakflg) {
	static struct ltchars ltchars = {
	    0xff,       /* char t_suspc;    * stop process signal */
	    0xff,       /* char t_dsuspc;   * delayed stop process signal */
	    0xff,       /* char t_rprntc;   * reprint line */
	    0xff,       /* char t_flushc;   * flush output (toggles) */
	    0xff,       /* char t_werasc;   * word erase */
	    0xff,       /* char t_lnextc;   * literal next character */
	};
	if (   ioctl (STDIN, TIOCGLTC, &lspchars) < 0
	    || ioctl (STDIN, TIOCSLTC, &ltchars) < 0
	   ) {
	    (void) ioctl (STDIN, TIOCSETC, &spchars);
	    cbreakflg = NO;
	}
    }
#endif
#endif  /* CBREAK */

    {
	int tmpflags;
	tmpflags = instty.sg_flags;
#ifdef  CBREAK
	if (cbreakflg)
	    instty.sg_flags = CBREAK | (instty.sg_flags & ~(ECHO | CRMOD));
	else
#endif
	    instty.sg_flags = RAW | (instty.sg_flags & ~(ECHO | CRMOD));
	if (ioctl (STDIN, TIOCSETP, &instty) >= 0)
	    istyflg = YES;
	instty.sg_flags = tmpflags;             /* all set up for cleanup */
    }
    return;
#endif /* -SYSIII */
}

#ifdef COMMENT
void
setotty ()
.
    Set the tty modes for the output tty.
#endif
void
setotty ()
{
    set_term ();        /* set terminal character set */

#ifdef SYSIII
    if (ioctl (STDOUT, TCGETA, &out_termio) < 0)
#else
    if (ioctl (STDOUT, TIOCGETP, &outstty) < 0)
#endif /* SYSIII */
	fast = YES;
    else {
	int i;
#ifdef MESG_NO
	struct stat statbuf;
#endif /* MESG_NO */

#ifdef SYSIII

#ifdef COMMENT
/*
The #define of speed below and the comparison, further below, of the
result with B4800 assume a monotinicity of the B's in the speeds they
represent which is true but undocumented for System 3.
In fact the system 3 B's are identical to the version 7 B's.
*/
#endif
/* #define SPEED ((out_termio.c_cflag)&(CBAUD)) */
	i = out_termio.c_oflag;
	out_termio.c_oflag &= ~(OLCUC|ONLCR|OCRNL|ONOCR|ONLRET);
	if( (out_termio.c_oflag & TABDLY) == TAB3)
	    out_termio.c_oflag &= ~TABDLY;
	if(ioctl(STDOUT,TCSETA,&out_termio) >= 0) {
	    ostyflg = YES;
	    out_termio.c_oflag = i;  /* all set up for cleanup */
	ospeed = (out_termio.c_cflag)&(CBAUD);
	}
#else /* - SYSIII */
/* #define SPEED (outstty.sg_ospeed) */
	i = outstty.sg_flags;
	outstty.sg_flags &= ~CRMOD;
	if ((outstty.sg_flags & TBDELAY) == XTABS)
	    outstty.sg_flags &= ~TBDELAY;
	if (ioctl (STDOUT, TIOCSETP, &outstty) >= 0) {
	    ostyflg = YES;
	    outstty.sg_flags = i;             /* all set up for cleanup */
	}
	ospeed = outstty.sg_ospeed
#endif /* SYSIII */

#ifdef MESG_NO
	fstat (STDOUT, &statbuf);        /* save old tty message status and */
	oldttmode = statbuf.st_mode;
#ifdef TTYNAME
	if ((ttynstr = ttyname (STDOUT)) != NULL)
#endif
#ifdef TTYN
	if ((ttynstr[strlen (ttynstr) - 1] = ttyn (STDOUT)) != NULL)
#endif
	    if (ttynstr != NULL) chmod (ttynstr, 0600); /* turn off messages */
#endif

	fast = (ospeed >= B4800);
    }
    /* no border bullets if speed is slow */
    if (!fast && optbullets == -1)
	borderbullets = NO;
    return;
#undef SPEED
}

/* set_crlf : set nl -> nl cr on output to console */
/* ----------------------------------------------- */

int set_crlf ()
{
    int oflag;
#ifdef SYSIII
    struct termio termiobf;

    (void) ioctl (STDOUT, TCGETA, &termiobf);
    oflag = termiobf.c_oflag;
    termiobf.c_oflag |= (ONLCR);    /* map nl into cr nl */
    (void) ioctl (STDOUT, TCSETA, &termiobf);

#else /* - SYSIII */
    struct sgttyb osttyb;

    (void) ioctl (STDOUT, TIOCGETP, &osttyb);
    oflag = osttyb.sg_flags;
    osttyb.sg_flags |= CRMOD;
    (void) ioctl (STDOUT, TIOCSETP, &osttyb);
#endif /* - SYSIII */
    return (oflag);
}

/* reset_crlf : back to initial mode */
/* --------------------------------- */

void reset_crlf (int oflag)
{
#ifdef SYSIII
    struct termio termiobf;

    (void) ioctl (STDOUT, TCGETA, &termiobf);
    termiobf.c_oflag = oflag;
    (void) ioctl (STDOUT, TCSETA, &termiobf);

#else /* - SYSIII */
    struct sgttyb osttyb;

    (void) ioctl (STDOUT, TIOCGETP, &osttyb);
    osttyb.sg_flags = oflag;
    (void) ioctl (STDOUT, TIOCSETP, &osttyb);
#endif /* - SYSIII */
}


#ifdef COMMENT
void
makescrfile ()
.
    Initialize the #o and #c pseudo-files.
    Set up the qbuffers (pick, close, etc.).
#endif
void
makescrfile ()
{
    extern void marktickfile ();
    extern char *cwdfiledir [];
    int j;

    for (j = FIRSTFILE; j < FIRSTFILE + NTMPFILES; j++) {
	names[j] = append (tmpnames[j - FIRSTFILE], ""); /* must be freeable*/
	/*  the firstlas is used for appending lines, and a separate stream
	 *  is cloned for the lastlook wksp so that seeks on it won't disturb
	 *  the seek position of firstlas[j]
	 **/
	cwdfiledir [j] = NULL;
	if (!la_open ("", "nt", &fnlas[j]))
	    getout (YES, "can't open %s", names[j]);
	(void) la_clone (&fnlas[j], &lastlook[j].las);
	lastlook[j].wfile = j;
	fileflags[j] = INUSE | CANMODIFY;
	marktickfile (j, NO);
    }

    for (j = 0; j < NQBUFS; j++)
	(void) la_clone (&fnlas[qtmpfn[j]], &qbuf[j].buflas);
    return;
}

static char * state_file_name = NULL;

static void badstart (FILE * gbuf)
{
    fclose (gbuf);
    getout (YES, "Bad state file: \"%s\"\nDelete it and try again.",
	    state_file_name);
}

static Small get_strg (char * strg, int strg_sz, Flag reset, FILE * gbuf)
/* If not reset : do not clean the strg storage space if nothing to get.
 * The storage must be large enought for the string to read,
 *  if not, the remaining part is skipped.
 * If no strorage is provided, the string is just skipped.
 */
{
    int sz;

    sz = (int) getshort (gbuf);
    if ( strg ) {
	if ( !reset && (sz <= 0) )
	    return sz;
	memset (strg, 0, strg_sz);
    }
    if ( sz > 0 ) {
	if ( strg ) {
	    if ( sz <= strg_sz ) {
		fread (strg, 1, sz, gbuf);
	    } else {
		fread (strg, 1, strg_sz, gbuf);
		getskip ((Small) (sz - strg_sz), gbuf);
	    }
	    strg [strg_sz -1] = '\0';
	} else getskip ((Small) sz, gbuf);
    }
    return (Small) sz;
}


static int file_in_names (Flag dump_state, char *fname, Flag sav_flg)
/* return 0 : no file */
{
    int fi;

    if ( !fname || ! *fname ) return 0;

    for ( fi = 1 ; fi < MAXFILES ; fi++ ) {
	if ( ! names[fi] ) {
	    if ( !sav_flg || !dump_state )
		return 0;  /* not found */
	    names [fi] = salloc (strlen (fname) +1, YES);
	    strcpy (names [fi], fname);
	    break;
	} else {
	    if ( strcmp (names [fi], fname) == 0 ) break;
	}
    }
    return ( fi >= MAXFILES ) ? 0 : fi;
}

static int one_window_file (Char ichar, Flag dump_state, FILE *gbuf, Flag curwin_flg,
			    int *fn, Flag build_flg, Flag alt_flg)
{
    char strg [PATH_MAX+1];
    long lnb;
    Small nletters;
    char *fname;
    Nlines  lin;
    Ncols   col;
    Slines tmplin;
    Scols tmpcol;
    Small cc;
    char *alt_strg;
    int fi;

    cc = 0;
    if ( fn ) *fn = 0;
    alt_strg = alt_flg ? "alternate " : "";

    lnb = ftell (gbuf);
    nletters = get_strg (strg, sizeof (strg), YES, gbuf);
    if ( dump_state ) {
	printf ("  size of %sfile name %d at file pos %07o\n",
		alt_strg, nletters, lnb + sizeof (short));
	if ( nletters <= 0 ) printf ("    NO %sfile\n", alt_strg);
    }

    if ( nletters <= 0 ) return cc;

    /* there is a file */
    if ( feoferr (gbuf) ) badstart (gbuf);

    fname = strg;
    if ( dump_state ) {
	fi = file_in_names (dump_state, fname, YES);
	if ( fn && (fi > 0) ) *fn = fi;
	if ( ! fname [0] ) printf ("    NO %sfile\n", alt_strg);
	else printf ("    %sfile %d \"%s\"\n", alt_strg, fi, fname);
    }

    lin = getlong  (gbuf);
    col = getshort (gbuf);
    tmplin = getc (gbuf);
    tmpcol = getshort (gbuf);

    if ( dump_state ) {
	/* if dummp_state, build_flg cannot be True */
	printf ("    lin %d, col %d, tmplin %d, tmpcol %d\n",
		lin, col, tmplin, tmpcol);
    }
    if ( build_flg ) {
	if ( ichar != ONE_FILE ) {
	    cc = editfile (fname, col, lin, 0, (!alt_flg && !curwin_flg));
	    if ( cc == 1 ) if ( fn ) *fn = curfile;
	    /* this sets them up to get copied into curwksp->ccol & clin */
	    poscursor (curwksp->ccol = tmpcol, curwksp->clin = tmplin);
	}
	else poscursor (0, 0);
    }
    return cc;
}

/* Extract the specification for one window from the state file and act.
 *    It is called with only ALL_WINDOWS, or NO_WINDOWS
 *    If dump_state (display the content of the state file) nothing is build
 */
static void one_window_state (Char ichar, Flag dump_state, FILE *gbuf,
			      Small win_idx, Small curwin_idx,
			      int *nfn, int *afn)
{
    Flag build_flg;
    char chr;
    Small sml;
    Small gf;
    long lnb;
    S_window *window;
    Scols   lmarg;
    Scols   rmarg;
    Slines  tmarg;
    Slines  bmarg;
#ifdef LMCV19
    AFlag   winflgs;
#endif
    Small cc;
    Flag curwin_flg;

    curwin_flg = ( win_idx == curwin_idx );
    build_flg = NO;     /* default : do not build the window */

    if ( ! dump_state ) {
	build_flg = (ichar == ALL_WINDOWS)
		    || ((ichar == ONE_WINDOW) && curwin_flg)
		    || ((ichar == ONE_FILE) && curwin_flg);
    }

    if ( dump_state ) {
	lnb = ftell (gbuf);
	printf ("Window idx %d (at state file position %07o) : %s\n", win_idx, lnb,
		curwin_flg ? " (current active window)" : "");
    }

    chr = getc (gbuf);
    if ( dump_state ) printf ("  prevwin %d\n", chr);
    else if ( build_flg ) {
	window = winlist[win_idx] = (S_window *) salloc (SWINDOW, YES);
	window->prevwin = (ichar == ALL_WINDOWS) ? chr : 0;
    }

    tmarg = getc (gbuf);
    lmarg = getshort (gbuf);
    bmarg = getc (gbuf);
    rmarg = getshort (gbuf);
#ifdef LMCV19
    winflgs = getc (gbuf);
    if ( dump_state ) printf ("  tmarg %d, lmarg %d, bmarg %d, rmarg %d, winflgs %d\n",
			      tmarg, lmarg, bmarg, rmarg, winflgs);
#endif
    if ( ichar != ALL_WINDOWS ) {
	tmarg = 0;
	lmarg = 0;
	bmarg = term.tt_height - 1 - NPARAMLINES;
	rmarg = term.tt_width - 1;
#ifdef LMCSTATE
	winflgs = 0;
#endif
    }

    defplline = getshort (gbuf);
    defmiline = getshort (gbuf);
    defplpage = getshort (gbuf);
    defmipage = getshort (gbuf);
    deflwin   = getshort (gbuf);
    defrwin   = getshort (gbuf);

    if ( build_flg ) {
	setupwindow (window, lmarg, tmarg, rmarg, bmarg, winflgs, 1);
	if ( ! curwin_flg )
	    drawborders (window, curwin_flg ? WIN_ACTIVE : WIN_INACTIVE);
	switchwindow (window);
	poscursor (curwksp->ccol, curwksp->clin);
    }

    gf = 0;     /* file = 0 : nothing, 1 : alternate , 2 : normal */
    /* alternated edited file */
    cc = one_window_file (ichar, dump_state, gbuf, curwin_flg, afn, build_flg, YES);
    if ( cc == 1 ) gf = 1;
    /*  do this so that editfile won't be cute about suppressing a putup on
     *  a file that is the same as the altfile
     */
    curfile = NULLFILE;

    /* main edited file */
    if ( ! curwin_flg ) chgborders = 2;
    cc = one_window_file (ichar, dump_state, gbuf, curwin_flg, nfn, build_flg, NO);
    if ( cc == 1 ) gf = 2;
    if ( build_flg ) {
	chgborders = 1;
	if ( gf < 2 ) {
	    if ( ! curwin_flg  ) chgborders = 2;
	    if ( gf == 0 ) {
		eddeffile (curwin_flg ? NO : YES);
		curwksp->ccol = curwksp->clin = 0;
	    }
	    else if ( ! curwin_flg ) putupwin ();
	    chgborders = 1;
	}
    }
}

static void get_stf_mark (struct markenv *markpt, FILE *gbuf)
{
    if ( feof (gbuf) || !markpt ) return;
    markpt->mrkwinlin = (Nlines)  getlong (gbuf);
    markpt->mrkwincol = (ANcols)  getshort (gbuf);
    markpt->mrklin    = (ASlines) getlong (gbuf);
    markpt->mrkcol    = (Scols)   getshort (gbuf);
}

static void set_posmark (S_wksp *wksp, struct markenv posmark)
{
    wksp->wlin = posmark.mrkwinlin;
    wksp->wcol = posmark.mrkwincol;
    wksp->clin = posmark.mrklin;
    wksp->ccol = posmark.mrkcol;
}

static void one_workspace_cursor (S_wksp *wksp, FILE *gbuf, char *msg, int fn)
{
    struct markenv posmark, prevmark;
    S_wksp *crwksp;

    if ( feof (gbuf) ) return;

    /* get cursor position */
    memset (&posmark, 0, sizeof (posmark));
    get_stf_mark (&posmark, gbuf);
    if ( wksp && (wksp->wfile != NULLFILE) ) {
	set_posmark (wksp, posmark);
    }
    if ( msg ) {
	strcat (msg, "cursor at : ");
	msg_mark (&posmark, NULL, msg, NO, YES);
    }
    /* get cursor previous position */
    memset (&prevmark, 0, sizeof (prevmark));
    get_stf_mark (&prevmark, gbuf);
    if ( wksp && (wksp->wfile != NULLFILE) )
	wksp->wkpos = prevmark;
    if ( msg ) {
	strcat (msg, ", previous at : ");
	msg_mark (&prevmark, NULL, msg, NO, YES);
    }
    if ( !wksp || !fn ) return;

    /* copie also in current window if needed */
    crwksp = curwin->altwksp;
    if ( crwksp->wfile == fn ) {
	set_posmark (crwksp, posmark);
	crwksp->wkpos = prevmark;
    }
    crwksp = curwin->wksp;
    if ( crwksp->wfile == fn ) {
	set_posmark (crwksp, posmark);
	poscursor (crwksp->ccol, crwksp->clin);
	crwksp->wkpos = prevmark;
    }
}

static void one_window_cursor (Short widx, FILE *gbuf, Flag dump_state,
			       Small curwin_idx, int *nfn, int *afn)
{
    int fn;
    S_wksp *wksp;
    char msg [256];

    memset (msg, 0, sizeof (msg));
    sprintf (msg, "Window %d%s\n", widx,
		 (widx == curwin_idx) ? " (Active window)" : "");

    /* alternate work space */
    fn = (int) getshort (gbuf);
    if ( afn ) *afn = fn;
    if ( fn != NULLFILE ) {
	wksp = dump_state ? NULL : winlist[widx]->altwksp;
	strcat (msg, "  Alternate : ");
	one_workspace_cursor (wksp, gbuf, msg, 0);
	strcat (msg, "\n");
    }
    /* normal work space */
    fn = (int) getshort (gbuf);
    if ( nfn ) *nfn = fn;
    if ( fn != NULLFILE ) {
	wksp = dump_state ? NULL : winlist[widx]->wksp;
	strcat (msg, "  Normal    : ");
	one_workspace_cursor (wksp, gbuf, msg, 0);
    }
    if ( dump_state ) puts (msg);
}

#ifdef COMMENT
static Flag
getstate (Char ichar, Flag dump_state, int *infolv_pt, Flag *grph_flg_pt)
.
    /*
    Set up things as per the state file.
    See State.fmt for a description of the state file format.
    The 'ichar' argument is still not totally cleaned up from the way
    this was originally written.
    If ichar ==
      ALL_WINDOWS use all windows and files from state file.
      ONE_WINDOW  set up one full-size window and don't put up any files.
    */
#endif
static Flag
getstate (Char ichar, Flag dump_state, int *infolv_pt, Flag *grph_flg_pt)
{
    extern void new_image ();
    extern void history_reload (FILE *stfile, Flag dump_state);
    extern void set_file_mark (AFn fnb, Flag tick_flg, struct markenv *my_markpt);
    extern int  itswapdeldchar (char *, int, int *);
    extern Flag inputcharis7bits;

    int ncwfi, acwfi;   /* normal and alternate file in current window */
    int ncwf [MAXWINLIST], acwf [MAXWINLIST];
    Slines nlin;
    Scols ncol;
    Short i, widx;
    Small nletters;
    Small cur_widx;
    FILE *gbuf;
    char *ich, dchr;
    char strg [256];
    long sttime;
    char fname_buf [PATH_MAX+1];
    int finb [MAXFILES];    /* where the file are loaded */
    int fidx;               /* number of file in the status file */
    int fi_one, fi_nb;
    Flag grph_flg;

    /* get and set defaults */
    grph_flg = ( grph_flg_pt ) ? *grph_flg_pt : NO;
    acwfi = ncwfi = 0;
    if ( infolv_pt ) *infolv_pt = 0;

    gbuf = NULL;
#ifdef LMCSTATE
    state_file_name = state ? statefile : rfile;
#else
    state_file_name = rfile;
#endif

    if ( dump_state ) {
	switch (ichar) {
	case NO_WINDOWS :
	    ich = "NO_WINDOWS";
	    break;
	case ALL_WINDOWS :
	    ich = "ALL_WINDOWS";
	    break;
	case ONE_WINDOW :
	    ich = "ONE_WINDOW";
	    break;
	case ONE_FILE :
	    ich = "ONE_FILE";
	    break;
	}
	printf ("\n--- Dump of state file \"%s\" for %s ---\n\n", state_file_name, ich);
    }

    if ( ! dump_state ) {
	if ( (ichar == NO_WINDOWS) || notracks ) {
	    /* do not read state file, do not edit scratch file */
	    insmode = YES;  /* by default set in INSERT mode" */
	    makestate (NO);
	    return NO;
	}
    }

    gbuf = fopen (state_file_name, "r");
    if ( ! gbuf ) {
	/* state file cannot be opened (non existing, ...) */
	if ( dump_state ) getout (YES, "  state file cannot be opened");
	else makestate (gbuf == NULL);
	insmode = YES;  /* by default set in INSERT mode" */
	return NO;
    }

    /* Now read the state file */
    /* Rand revision */
    i = getshort (gbuf);
    if ( dump_state )
	printf ("Revision %d, Rand editor revision %d\n", -i, -revision);
    else {
	if ( i != revision ) {
	    if (i >= 0 || feoferr (gbuf)) badstart (gbuf);

	    if (i != -1)            /* always accept -1 */
		getout (YES,
"Startup file: \"%s\" was made by revision %d of %s.\n\
   Delete it or give a filename after the %s command.",
			rfile, -i, progname, progname);
	}
    }

    /* terminal type */
    nletters = get_strg (strg, sizeof (strg), YES, gbuf);
    if (feoferr (gbuf)) badstart (gbuf);
    if ( dump_state ) printf ("terminal name \"%s\"\n", strg);

    nlin = getc (gbuf) & 0377;
    ncol = getc (gbuf) & 0377;
    if ( nlin != term.tt_height || ncol != term.tt_width ) {
	extern void set_tt_size (S_term *, int, int);
	set_tt_size (&term, ncol, nlin);
    }
    if ( dump_state ) printf ("screen size column %d, line %d\n", ncol, nlin);
    else new_image ();

    sttime = getlong (gbuf);
    if ( dump_state ) {
	char *asct;
	asct = asctime (localtime (&sttime));
	asct [strlen(asct) -1] = '\0';
	printf ("edit session time : \"%s\"\n\n", asct);
    }

    ntabs = getshort (gbuf);
    if ( dump_state ) printf ("ntabs %d\n", ntabs);
    if ( ntabs ) {
	int ind;

	if (feoferr (gbuf)) badstart (gbuf);

	stabs = max (ntabs, NTABS);
	tabs = (ANcols *) salloc (stabs * sizeof *tabs, YES);
	for (ind = 0; ind < ntabs; ind++) {
	    tabs[ind] = (ANcols) getshort (gbuf);
	    if ( dump_state ) {
		printf ("tabs[%2d] %3d, ", ind, tabs[ind]);
		if ( ((ind+1) % 5) == 0 ) printf ("\n");
	    }
	}
	if (feoferr (gbuf)) badstart (gbuf);

	if ( dump_state ) printf ("\n");
    }

    linewidth = getshort (gbuf);
    if ( dump_state ) printf ("linewidth %d\n", linewidth);

    if (i = getshort (gbuf)) {
	if (feoferr (gbuf))  badstart (gbuf);

	searchkey = salloc (i, YES);
	fread (searchkey, 1, i, gbuf);
	if (feoferr (gbuf)) badstart (gbuf);

    }
    if ( dump_state ) {
	printf ("size of searchkey string %d\n", i);
	if ( i ) printf ("  searchkey pattern : \"%s\"\n", searchkey);
    }

    insmode = getc (gbuf);
    if ( dump_state ) printf ("insmode %d : %s\n", insmode, insmode ? "YES" : "NO");

    dchr = getc (gbuf);
    if ( dump_state ) printf ("patmode %d : %s\n", dchr, dchr ? "YES" : "NO");
    if ( dchr || patmode )  patmode = YES;  /* Added Purdue CS 10/8/82 MAB */

    dchr = getc (gbuf);
    if ( dump_state ) printf ("curmark %d\n", dchr);
    if (dchr) {  /* curmark */
	int sz;
	sz = sizeof (long)
	     + sizeof (short)
	     + sizeof (char)
	     + sizeof (short);
	getskip (sz, gbuf);
	if ( dump_state ) printf ("skip %d char\n", sz);
    }

#ifdef LMCV19
#ifdef LMCAUTO
    autofill = getc (gbuf);
    if ( dump_state ) printf ("autofill %d\n", autofill);
    autolmarg = getshort (gbuf);
    if ( dump_state ) printf ("autolmarg %d\n", autolmarg);
    if ( !dump_state ) rand_info (inf_auto, 2, autofill ? "WP" : "  ");
#else
    (void) getc (gbuf);
    (void) getshort (gbuf);
    if ( dump_state ) printf ("skip 1 char and 1 short\n");
#endif
#endif

    nwinlist = getc (gbuf);
    if ( dump_state ) printf ("\nnwinlist (Nb of windows) %d\n", nwinlist);
    if ( ferror(gbuf) || nwinlist > MAXWINLIST ) badstart (gbuf);

    cur_widx = getc (gbuf);
    if ( dump_state ) printf ("curwin idx (current window index) %d\n", cur_widx);
    if ( cur_widx < 0 ) cur_widx = 0;
    if ( cur_widx >= nwinlist ) cur_widx = nwinlist -1;

    /* set up window */
    if ( dump_state ) putchar ('\n');
    for ( widx = 0 ; widx < nwinlist ; widx++ ) {
	int nfn, afn;
	nfn = afn = 0;
	one_window_state (ichar, dump_state, gbuf, widx, cur_widx, &nfn, &afn);
	ncwf[widx] = acwf[widx] = -1;
	if ( (widx == cur_widx) ) {
	    ncwfi = nfn;
	    acwfi = afn;
	}
#if 0
|        if ( (widx == cur_widx) && !dump_state ) {
|            /* get the active window edited files */
|            ncwfi = winlist[widx]->wksp->wfile;
|            acwfi = winlist[widx]->altwksp->wfile;
|        }
#endif
    }

    /* Reload the history buffer */
    if ( dump_state ) putchar ('\n');
    history_reload (gbuf, dump_state);

    /* Edited files list (ended by an 0 size name) */
    if ( ! dump_state ) {
	switchwindow (winlist[cur_widx]);
	poscursor (curwksp->ccol, curwksp->clin);
    }
    for ( fidx = 0 ; ; fidx++ ) {
	extern char * fileStatusString ();
	char * fstrg;
	Short fflag;
	Small cc;
	int fi;

	nletters = get_strg (fname_buf, sizeof (fname_buf), YES, gbuf);
	if ( nletters <= 0 ) break;
	fflag = getshort (gbuf);
	if ( dump_state ) {
	    if ( fidx == 0 ) printf ("\nEdited files list :\n");
	    fstrg = fileStatusString (fflag, NULL);
	    fi = file_in_names (dump_state, fname_buf, YES);
	    printf ("%-2d %s %s\n", fi, fstrg, fname_buf);
	    finb [fidx] = fi;   /* where the file is loaded */
	    continue;   /* no more processing */
	}
	if ( fflag & (DELETED | RENAMED) ) continue;

	/* insert in edited files list if needed, and update flag */
	for ( fi = MAXFILES -1 ; fi > 0 ; fi-- ) {
	    if ( !(fileflags [fi] & INUSE) ) continue;
	    if ( strcmp (fname_buf, names [fi]) != 0 ) continue;
	    break;
	}
	if ( fi <= 0 ) {     /* not yet in list, insert it */
	    cc = editfile (fname_buf, (Ncols) -1, (Nlines) -1, 0, NO);
	    fi = curfile;
	} else cc = 1;
	if ( (cc == 1) && (strcmp (fname_buf, xdir_err) == 0) ) {
	    if ( ! (fflag & CANMODIFY) ) fileflags [fi] &= ~CANMODIFY;
	}
	finb [fidx] = fi;   /* where the file is loaded */
    }

    if ( ! feof (gbuf) ) {
	/* Set the <Del> character function assignement */
	extern char * itgetvalue (char *);
	extern char * itsyms_by_val (short);
	extern char del_strg [];
	char *val_pt;
	char del_ch, del_fc;

	del_ch = getc (gbuf);
	del_fc = getc (gbuf);
	if ( feof (gbuf) || (del_ch == '\0') ) {
	    if ( dump_state ) {
		printf ("\nNo function assigned to <Del> key (\\%03o)\n", del_strg [0]);
	    }
	} else {
	    if ( dump_state ) {
		printf ("\n<Del> (\\%03o) key assigned to function \"%s\"\n",
			del_ch, itsyms_by_val ((short) del_fc));
	    } else {
		(void) itswapdeldchar (del_strg, -1, NULL); /* save the init value */
		if ( (del_ch == del_strg [0]) &&
		     ((del_fc == CCBACKSPACE) || (del_fc == CCDELCH)) ) {
		    val_pt = itgetvalue (del_strg);
		    if ( val_pt ) {
			*val_pt = del_fc;
		    }
		}
	    }
	}
	if ( ! feof (gbuf) ) {
	    /* skip end of section mark */
	    (void) getshort (gbuf);
	}
    }

    if ( ! feof (gbuf) ) {
	int infolv, fi, fn;
	Flag tick_flg, title_flg, cs7_flg;
	Flag prv_notracks, prv_norecover, prv_replaying, prv_recovering;
	struct markenv tickmark, *markpt;
	S_wksp *lwksp;
	char msg [256];

	/* various session status */
	prv_notracks = getc(gbuf);
	prv_norecover = getc(gbuf);
	prv_replaying = getc(gbuf);
	prv_recovering = getc(gbuf);
	if ( dump_state ) {
	    printf ("\nSession  state : %s%s%s%s\n",
		    prv_notracks   ? "'-notracks'" : "",
		    prv_norecover  ? "'-norecover'" : "",
		    prv_replaying  ? "replaying" : "",
		    prv_recovering ? "recovering" : "");
	}
	cs7_flg = NO;
	infolv = 0;
	grph_flg = getc (gbuf);           /* graphic char for window edges */
	cs7_flg = getc (gbuf);            /* 7 (or 8) bits characters */
	infolv = (int) getshort (gbuf);   /* info level */
	fi_one = (int) getshort (gbuf);   /* 1st user edited file id */
	fi_nb  = (int) getshort (gbuf);   /* saved idex of 1st edited file */
	if ( dump_state ) {
	    printf ("\nGraphic window edges : %s\n", grph_flg ? "Yes" : "No");
	    printf ("8 bits characters : %s\n", cs7_flg ? "No, 7 bits (cs7)" : "Yes (cs8)");
	    printf ("Info line level : %d\n", infolv);
	    printf ("First user file id : %d\n", fi_one);
	    printf ("Number of edited file in the session : %d (%d)\n", fidx, fi_nb);
	} else {
	    inputcharis7bits = cs7_flg;
	    if ( grph_flg_pt ) *grph_flg_pt = grph_flg;
	    if ( infolv_pt ) *infolv_pt = infolv;
	}
	/* reload file cursor position and tick marks */
	title_flg = NO;
	for ( i = 0 ; i < fidx ; i++ ) {
	    if ( feof (gbuf) ) break;

	    memset (msg, 0, sizeof (msg));
	    fn = finb [i];
	    sprintf (msg, "File %-2d : \"%s\"\n  ", fn, names [fn]);
	    lwksp = &lastlook [fn];
	    lwksp->wfile = fn;
	    one_workspace_cursor (dump_state ? NULL : lwksp, gbuf, msg, fn);

	    /* get file tick */
	    tick_flg = getc (gbuf);
	    memset (&tickmark, 0, sizeof (tickmark));
	    if ( tick_flg ) {
		/* get tick position */
		get_stf_mark (&tickmark, gbuf);
		if ( dump_state ) {
		    strcat (msg, ", tick at : ");
		    msg_mark (&tickmark, NULL, msg, NO, YES);
		}
	    }
	    if ( dump_state ) {
		if ( ! title_flg ) {
		    puts ("\nFiles cursor and tick setting (line x column)");
		    title_flg = YES;
		}
		puts (msg);
	    } else {
		set_file_mark (fn, tick_flg, &tickmark);
	    }
	}
	/* skip end of section mark */
	(void) getshort (gbuf);
    }

    if ( ! dump_state ) {
	/* restaure the active window */
	chgwindow (cur_widx);   /* switch to active window now */
	if ( acwfi > 0 )
	    editfile (names [acwfi], (Ncols) -1, (Nlines) -1, 0, NO);
	if ( ncwfi > 0 )
	    editfile (names [ncwfi], (Ncols) -1, (Nlines) -1, 0, NO);
	else eddeffile (NO);
	if ( acwfi <= 0 )
	    releasewk (curwin->altwksp);
    }

    if ( ! feof (gbuf) ) {
	/* section for the window cursor positions */
	int nfn, afn;
	if ( dump_state )
	    puts ("\nWindow cursor positions");
	for ( widx = 0 ; widx < nwinlist ; widx++ ) {
	    if ( feof (gbuf) ) break;
	    nfn = afn = 0;
	    one_window_cursor (widx, gbuf, dump_state, cur_widx, &nfn, &afn);
	}
	/* skip end of section mark */
	(void) getshort (gbuf);
    } else if ( dump_state )
	puts ("\nNo Window cursor positions info in state file");

    if ( dump_state ) {
	if ( nwinlist > 0 ) {
	    char str [8];
	    printf ("\nCurrent active window : %d\n", cur_widx);
	    if ( acwfi > 0 ) sprintf (str, "%2d \"%s\"", acwfi, names[acwfi]);
	    else strcpy (str, "NO");
	    printf ("  Alternate file : %s\n", str);
	    printf ("  Normal    file : %2d \"%s\"\n", ncwfi, names[ncwfi]);
	}
    }

    if ( ! feof (gbuf) ) {
	int path_max, lpath_max, vers, revi;
	char bld_str [64];

	/* file system info and Rand version, revision */
	path_max = (int) getlong (gbuf);    /* file name max lenth */
	lpath_max = PATH_MAX;
	vers = (int) getshort (gbuf);
	revi = (int) getshort (gbuf);
	nletters = get_strg (bld_str, sizeof (bld_str), YES, gbuf);
	if ( dump_state ) {
	    printf ("\nMax path name lenth (PATH_MAX) : %d, in current OS : %d\n",
		    path_max, lpath_max);
	    if ( nletters )
		printf ("\nRand version E%d.%d, build %s\n", vers, revi, bld_str);
	}

	/* OS and user info */
	nletters = get_strg (strg, sizeof (strg), YES, gbuf);
	if ( nletters > 0 ) {
	    if ( dump_state )
		    printf ("\nOS and user :\n  %s\n", strg);
	} else puts ("\nNo OS and user info\n");

	/* get client session info */
	nletters = get_strg (strg, sizeof (strg), YES, gbuf);   /* fromHost */
	if ( dump_state )
	    if ( strg [0] ) printf ("  Remote session from \"%s\"", strg);
	    else fputs ("  Local session", stdout);
	nletters = get_strg (strg, sizeof (strg), YES, gbuf);   /* fromHost_alias */
	if ( dump_state && strg[0] )
	    printf (" (%s)", strg);
	nletters = get_strg (strg, sizeof (strg), YES, gbuf);   /* fromDisplay */
	if ( dump_state ) {
	    if ( strg[0] ) printf (" On display \"%s\"", strg);
	    else putchar ('\n');
	}

	/* Get ssh session info */
	nletters = get_strg (strg, sizeof (strg), YES, gbuf);   /* ssh_ip */
	if ( nletters > 0 ) {
	    nletters = get_strg (strg, sizeof (strg), NO, gbuf);   /* sshHost */
	    if ( nletters > 0 ) {
		nletters = get_strg (strg, sizeof (strg), NO, gbuf);   /* sshHost_alias */
	    }
	}
	if ( dump_state && strg[0] )
	    printf ("  ssh session from \"%s\" client\n", strg);

	/* skip end of section mark */
	(void) getshort (gbuf);
    }

    if ( ! feof (gbuf) ) {
	/* skip end of file mark */
	(void) getshort (gbuf);
    }

    if ( dump_state ) {
	char * str;
	str = feof (gbuf) ? "\n--- End of a previous version state file ---\n"
			  : "\n--- End of state file dump ---\n";
	fclose (gbuf);
	getout (YES, str);
    }

    if ( ferror (gbuf) ) badstart (gbuf);

    poscursor (curwksp->ccol, curwksp->clin);
    fclose (gbuf);
    return YES;
}

#ifdef COMMENT
void
getskip (num, file)
    int num;
    FILE *file;
.
    Skip 'num' bytes in 'file'.  'Num' must be > 0.
#endif
void
getskip (num, file)
int num;
FILE *file;
{
    do {
	getc (file);
    } while (--num);
    return;
}

/* ------ default tabs setting ------ */
static int default_tabs = DEFAULT_TABS;


#ifdef COMMENT
static void
makestate (nofileflg)
    Flag nofileflg;
.
    Make an initial state without reference to the state file.
    If 'nofileflg' != 0, edit the 'scratch' file.
#endif
static void
makestate (nofileflg)
Flag nofileflg;
{
    S_window *window;

    nwinlist = 1;
    window = winlist[0] = (S_window *) salloc (SWINDOW, YES);

    stabs = NTABS;
    tabs = (ANcols *) salloc (stabs * sizeof *tabs,YES);
    ntabs = 0;

    edit_setupwindow (window, 0);

    switchwindow (window);              /* switched this and next line */
    tabevery ((Ncols) default_tabs, (Ncols) 0, (Ncols) (((NTABS / 2) - 1) * default_tabs),
	      YES);
    drawborders (window, WIN_ACTIVE);
    poscursor (0, 0);
    if ( nofileflg ) {
	edscrfile (NO);
	poscursor (0, 0);
    }
    return;
}

#ifdef LMCAUTO
#ifdef COMMENT
static void
infoint0 ()
.
    Set up initial parameters for the info line.
#endif
static void
infoint0 ()
{
    Scols col;

    inf_insert = 0; col = 7;    /* "INSERT" */

    inf_track = col; col += 4;  /* "TRK", old value was "TRACK" */

    inf_range = col; col += 4;  /* "=RG", old value was "=RANGE" */

    inf_pat = col; col += 3;    /* "RE" */

    inf_auto = col; col += 3;   /* "WP" */

    inf_mark = col; col += 5;   /* "MARK" */

    inf_area = col; col += 7;   /* "30x16" etc. */

    inf_at = col+1; col += 4;   /* space for impl tick flg (*) and "At" or "ch" */
#ifdef LA_LONGFILES
    inf_line = col; col += 10;  /* lineXcolumn  */
#else
    inf_line = col; col += 6;   /* line         */
#endif
    inf_in = col; col += 3;     /* "in"         */
    inf_file = col;             /* filename     */
}

#ifdef COMMENT
static void
infoinit (int infolv)
.
    Set up the displays on the info line.
#endif
static void
infoinit (int infolv)
{
#else
#ifdef COMMENT
static void
infoinit (int infolv)
.
    Set up the displays on the info line.
#endif


static void
infoinit (int infolv)
{
    extern void set_info_level (int level);
    Scols col;

    inf_insert = 0; col = 8;    /* "INSERT" */

    inf_track = col; col += 7;  /* "TRACK" */

    inf_range = col; col += 8;  /* "=RANGE" */

    inf_pat = col; col += 3;    /* "RE" Added 10/18/82 MAB */

    inf_mark = col; col += 5;   /* "MARK" */

    inf_area = col;             /* "30x16" etc. */

    col = 38;
    inf_at = col; col += 3;     /* "At" or "ch" */
#ifdef LA_LONGFILES
    inf_line = col; col += 10;  /* line         */
#else
    inf_line = col; col += 6;   /* line         */
#endif /* LA_LONGFILES */
    inf_in = col; col += 3;     /* "in"         */
    inf_file = col;             /* filename     */
#endif /* - LMCAUTO */

    set_info_level (infolv);    /* default = cursor position */
    rand_info (inf_in, 2, "in");
    infoline = -1;
    infofile = NULLFILE;
    if (insmode) {
	insmode = NO;
	tglinsmode ();
    }
    if (patmode) {
	patmode = NO;
	tglpatmode ();
    }
}

static void clean_all (Flag filclean)
{
    fixtty ();
    if (windowsup)
	screenexit (YES);
    if (filclean && !crashed)
	/* remove change file, keep key stroke files */
	cleanup (YES, NO);
    if (keysmoved)
	/* restaure initial key stroke file from backup (rename file) */
	mv (bkeytmp, keytmp);
    d_put (0);
}

static void exit_now (int status)
{
    close_dbgfile (NULL);
#ifdef PROFILE
    monexit (status);
#else
    exit (status);
#endif
    /* NOTREACHED */
}


#ifdef COMMENT
void
getout (filclean, str, a1,a2,a3,a4,a5,a6)
    Flag filclean;
    char *str;
.
    This is called to exit from the editor for conditions that arise before
    entering mainloop().
#endif
/* VARARGS2 */
void
getout (filclean, str, a1,a2,a3,a4,a5,a6)
Flag filclean;
char   *str;
long a1,a2,a3,a4,a5,a6;
{
    extern char verstr[];

    /* keep key stroke files, delete change file if filclean and !crashed */
    clean_all (filclean);

    if (*str)
	fprintf (stdout, str, a1,a2,a3,a4,a5,a6);

    if ( helpflg ) {
	/* do not exit during a -help option : check all options */
	printf ("\n  --- %s : error during initialization ---\n", progname);
	return;
    }

    printf ("\nThis is %s : Rand editor version %d release %d\n%s\n",
	    prog_fname ? prog_fname : progname, -revision, subrev, verstr);

    exit_now (-1);
    /* NOTREACHED */
}

void save_option_pref (FILE *pref_file)
{
    extern char * get_keyword_strg (S_looktbl *table, int val);
    char *str, *fname, *dfname;

    fname = get_debug_name ();
    if ( fname ) {
	/* debugging is defined */
	str = get_keyword_strg (opttable, OPTDEBUG);
	dfname = get_debug_file_name (NO, NULL);
	if ( dfname && (strcmp (fname, dfname) == 0) )
	    fname = "";  /* default */
	fprintf (pref_file, "option -%s", str);
	if ( *fname || (DebugVal != DEFAULT_DebugVal) )
	    fprintf (pref_file, "=%s,%d", fname, DebugVal);
	fputc ('\n', pref_file);
    }

    if ( noX11opt ) {
	/* do not call X11 server for keyboard mapping */
	str = get_keyword_strg (opttable, OPTNOX11);
	fprintf (pref_file, "option -%s\n", str);
    }

}

int get_debug_default_level ()
{
    return DEFAULT_DebugVal;
}

static void parse_debug_option (char *opt_str, Flag malloc_flg)
/* debug option syntax : -debug=[file name],[level]
 *                  or   -debug=[file name]:[level]
 *   can be called only one time.
 * malloc_flg : do a malloc to store the debug file name
 */
{
    static called_flg = NO;
    char *fname, *str;

    if ( called_flg ) return;

    DebugVal = DEFAULT_DebugVal;
    if ( !opt_str || !*opt_str ) {
	/* use the default file */
	fname = NULL;
    } else {
	str = strchr (opt_str, ':');
	if ( ! str ) str = strchr (opt_str, ',');
	if ( str ) {
	    *(str++) = '\0';
	    if ( *str ) DebugVal = atoi (str);
	}
	fname = opt_str;
    }
    dbgflg = YES;
    dbgname = (malloc_flg) ? append (fname, "") : fname;
    called_flg = YES;
}


void set_option_pref (char *cmd, char *par)
{
    int idx, val, cc;
    char str [256], *str1, *str2;

    if ( *cmd != '-' ) return;
    memset (str, 0, sizeof (str));
    strncpy (str, cmd+1, sizeof (str) -1);

    str1 = strtok (str, "=");
    if ( ! str1 ) return;
    str2 = strtok (NULL, "#");

    idx = lookup (str1, opttable);
    if ( idx < 0 ) return;
    val = opttable[idx].val;
    switch (val) {
	case OPTDEBUG :
	    /* already defined by -debug option ? */
	    if ( dbgflg || dbgname ) break; /* Yes, do not overwrite */

	    parse_debug_option (str2, YES);
	    cc = open_dbgfile (NO);
	    if ( cc < 0 ) dbgflg = NO;
	    break;

	case OPTNOX11 :
	    set_noX11opt ();
	    break;
    }
}

char * get_myhost_name ()
{
    return ( fromHost ) ? fromHost : myHost;
}
