#ifdef COMMENT
--------
file e.h.c
    support  standard help functions.
#endif

# include "e.h"
# include "e.cm.h"
# include "e.m.h"
# include "e.tt.h"
# include "e.h.h"
#include <string.h>

#ifdef SYSSELECT
#include <sys/time.h>
#else
#include SIG_INCL
#endif

void print_sort_table (S_lookstruct *);

/* these 2 values are primarily defined in e.it.h */
#define KBINIT  040     /* Must not conflict with CC codes */
#define KBEND   041

/* these flags are set by in_file routine from e.it.c */
static int Esc_flg = 0;     /* Escape was just pushed flag, not yet used by the UNIX version*/
       int CtrlC_flg = 0;   /* Ctrl C was just pushed flag */

static Flag by_flg = NO; /* add the "by ..." info */
static int nl_nb = 0;   /* number of line in the call to help_description_ext */

/* registered resize screen routine for full screen info display */
static void (*resize_service) () = NULL;    /* specifique resize routine */
static void (*resize_param) (int *, int *) = NULL;  /* get current term size */

/* browse_keyboard : display the description of the pushed key assigned function */
/* ----------------------------------------------------------------------------- */
/*  return when :
	- CR is pushed
	- any non assigned key is pushed
*/

static void browse_keyboard (char *msg)
{
    extern char * itsyms_by_val (short val);
    extern void ignore_quote ();
    static int help_description ();
    char blank [128];
    char *str;
    int qq, ctrlc, sz, nbli;

    sz = (msg) ? strlen (msg) : 0;
    if ( sz > (sizeof (blank) -2) ) sz = sizeof (blank) -2;
    memset (blank, ' ', sizeof (blank));
    blank [sz] = '\r';
    blank [sz +1] = '\0';
    for ( ; ; ) {
	ignore_quote ();
	ctrlc = wait_keyboard (msg, &qq);
	if ( ctrlc || (qq == CCRETURN) ) break;
	if ( (qq >= ' ') && (qq <= '~') ) putchar ('\007');
	else {
	    str = itsyms_by_val (key);
	    if ( !str ) str = "???";
	    fputs (blank, stdout);
	    (*term.tt_home) ();
	    (*term.tt_clear) ();
	    by_flg = YES;
	    (void) help_description ('~', -1, key, NULL, str, NULL);
	    sz = term.tt_height -nl_nb -2;
	    if ( sz > 0 ) for ( ; sz >= 0 ; sz -- ) fputc ('\n', stdout);
	}
    }
}

/* ------------------------------------------------------------ */

/* open_help_file : local to open the help file */
/* -------------------------------------------- */

extern char * xdir_help;

static FILE * open_help_file () {
    FILE *helpin; 

    if ((helpin = fopen (xdir_help, "r")) == NULL) {
	printf ("\r\n\nCan't open %s\n\r", xdir_help);
    }
    return (helpin);
}

static int check_new_page (int *nbli, int nl)
{
    int sz, ctrlc;

    sz = term.tt_height -2;
    if ( nl ) sz++;
    if ( *nbli < sz ) return NO;

    ctrlc = wait_keyboard ("Push a key to complete the description, <Ctrl C> to exit", NULL);
    fputs               ("\r                                                        \r", stdout);
    *nbli = 0;
    (*term.tt_addr) (term.tt_height -1, 0);
    return ctrlc;
}

/* help_description :
 *   Displays a message describing from the help file
 *   whose lexed value is passed in "val".
 *   The command major name is given in cmd_str.
 */

static int help_description_ext (sep, nbli, val, cmd_str, str, abreviation, nlnb_pt)
char sep;   /* separator character : ~ (keyfunc) ` (coomand) # (info) */
int nbli;   /* number of line already displayed on the screen (-1 : nocheck) */
int val;    /* message value : command or key function value */
char *cmd_str;  /* if defined, the message is a command description */
char *str, *abreviation;    /* current command and mini abreviation */
int *nlnb_pt;   /* when not NULL just count the number of line in the text, no output */
{
    extern Flag verbose_helpflg;
    extern char * it_strg ();
    extern char * escstrg ();

    FILE *helpin, *fopen();
    int go, n, sz, nl, ctrlc;
    char buf [256];
    char *type;
    Flag flg;

    if ( cmd_str && !str ) str = cmd_str;
    if ( !str ) return 0;

    type = "???";
    ctrlc = NO;

    helpin = open_help_file ();
    if ( helpin == NULL) {
	return (0);   /* no more message */
    }

    go = nl = 0;
    nl_nb = 0;
    if ( (nbli == 0) && !nlnb_pt ) (*term.tt_home) ();

    if ( (sep != '#') && str ) {
	/* display the header for keyfunc and command */
	if ( sep == '`' ) {    /* message by string name */
	    type = "Command";
	    if ( !nlnb_pt ) {
		if ( strcmp (str, cmd_str) == 0 )
		    printf ("%s (%d) : Editor %s", str, val, type);
		else
		    printf ("%s (%d) : Editor %s \"%s\"", str, val, type, cmd_str);
	    }
	    if ( abreviation )
		if ( !nlnb_pt ) {
		    char *cmt;
		    cmt = (strchr (abreviation, ',')) ? "aliases" : "alias";
		    printf (", %s : %s", cmt, abreviation);
		}
	} else if ( sep == '~' ) {
	    type = (flg = (val == KBINIT) || (val == KBEND))
		 ? "Defined String" : "Key Function";
	    if ( !nlnb_pt ) printf ("%s (%d) : %s", str, val, type);
	    if ( !flg && by_flg ) {
		printf (" by \"%s\"", escstrg (it_strg ()));
	    }
	}
	if ( !nlnb_pt ) puts ("\n");    /* 2 nl ! */
	nbli +=2;
	nl_nb +=2;
	nl = YES;
    }

    if ( !nlnb_pt ) putchar ( (nbli >= 0 ) ? '\r' : '\n');
    while ( fgets (buf, sizeof buf, helpin) != NULL ) {
	if ( go ) {
	    if ( buf [0] == sep ) break;
	    if ( nl && (buf [0] == '\n') ) continue;
	    if ( !nlnb_pt ) fputs (buf, stdout);
	    nl = (buf [0] == '\n');
	    nl_nb +=1;
	    if ( (nbli >= 0) && !nlnb_pt ) {
		nbli++;
		ctrlc = check_new_page (&nbli, nl);
		if ( ctrlc && !verbose_helpflg ) break;
	    }
	}
	else if ( buf [0] == sep ) {
            sz = strlen(buf);
            if ( buf[sz -1] == '\n' ) buf[sz -1] = '\0';
	    if ( cmd_str ) {    /* message by string name */
		if ( sep == '#' ) {
		    if ( strncmp (buf + 1, cmd_str, strlen (cmd_str)) == 0) go = YES;
		} else {
		    if ( strcmp (buf + 1, cmd_str) == 0) go = YES;
		}
	    }
	    else {      /* message by value */
		if ( (n = atoi (buf + 1)) == val ) go = YES;
	    }
        }
    }

    if ( !nl ) {
	if ( !nlnb_pt ) fputc ('\n', stdout);
	nl_nb +=1;
    }
    if ( (sep != '#') && !go && !ctrlc ) {
	if ( !nlnb_pt ) printf ("A description is not available for this %s in\n    %s\n\n",
				type, xdir_help);
	nl_nb +=3;
    }
    if ( nl ) go--;
    fclose (helpin);
    if ( !nlnb_pt ) fflush (stdout);
    if ( nlnb_pt ) *nlnb_pt = nl_nb;
    return (go);
}

static int help_description (sep, nbli, val, cmd_str, str, abreviation)
char sep;   /* separator character : ~ (keyfunc) ` (coomand) # (info) */
int nbli;   /* number of line already displayed on the screen (-1 : nocheck) */
int val;    /* message value : command or key function value */
char *cmd_str;  /* if defined, the message is a command description */
char *str, *abreviation;    /* current command and mini abreviation */
{
    (void) help_description_ext (sep, nbli, val, cmd_str, str, abreviation, NULL);
    by_flg = NO;
}


/* get the number of line in the help output */
static int help_description_check (sep, nbli, val)
char sep;   /* separator character : ~ (keyfunc) ` (comand) # (info) */
int nbli;   /* number of line already displayed on the screen (-1 : nocheck) */
int val;    /* message value : command or key function value */
{
    int nlnb;

    (void) help_description_ext (sep, nbli, val, NULL, NULL, NULL, &nlnb);
    return (nlnb);
}

/* help_cmd :
 *   Displays a message describing the effects of the command
 *   whose lexed value is passed in "cmd". 
 *   The command major name is given in cmd_str.
 */

static int help_cmd (cmd, cmd_str, str, abreviation)
int cmd;
char *cmd_str, *str, *abreviation;
{
    int go;

    by_flg = NO;
    go = help_description ('`', 0, cmd, cmd_str, str, abreviation);
    return (go);
}

int help_info (char *str, int *nbli)
{
    int go;
    int nb;

    nb = ( nbli ) ? *nbli : -1;
    by_flg = NO;
    go = help_description ('#', nb, 0, str, NULL, NULL);
    if ( nbli ) *nbli += go;
    return (go);
}

/* ------------------------------------------------------------------------ */

static void not_yet ()
{
    printf ("Not yet implemented\r\n");
}

static void keymap_term ()
{
    extern void display_keymap ();

    display_keymap (NO);
}

/* key_by_func : Get keys list by key function value
 * -------------------------------------------------
 *      escp_str and escp_labels must be the same size : escp_nb
 *      funcval_flg : yes return just the function command string
 *      escp_labels_flg : return on key labels for each escp string
*/

static int concat_labels (char ** shift_key, int shift_key_nb,
			  char * msg, int msg_sz,
			  char * keys_label, int keys_label_sz)
{
    int i, sz;

    for ( i = 0 ; i < shift_key_nb ; i++ ) {
	if ( !shift_key[i] || !shift_key[i][0] ) continue;
	sz = msg_sz - strlen (msg);
	if ( sz < 1 ) break;
	strncat (msg, shift_key[i], sz);
	sz = strlen (msg);
	if ( sz < (msg_sz -2) ) {
	    msg [sz-1] = ';';
	    msg [sz] = ' ';
	}
    }
    memset (shift_key, 0, shift_key_nb * sizeof (char *));
    memset (keys_label, 0, keys_label_sz);
    return ( strlen (msg));
}

int key_by_func (int fcmd, char *buf, int buf_sz,
		 char **escp_str, char **escp_labels, int escp_nb,
		 char *labels, int labels_sz,
		 Flag funcval_flg,
		 Flag escp_labels_flg)
{
    extern int it_value_to_string ();
    extern void all_string_to_key ();
    int i, nb, sz, msg_sz;
    char ch;
    char *sp, *sp1, *msg;
    char *shift_key[4];
    char keys_label [4*128];
    int shift_key_nb;

    memset (buf, 0, buf_sz);        /* buffer for escape sequences */
    nb = it_value_to_string (fcmd, NO, buf, buf_sz);
    if ( nb <= 0 ) return 0;
    if ( nb > escp_nb ) nb = escp_nb;
    if ( funcval_flg ) return nb;

    memset (escp_str, 0, escp_nb * sizeof (char *));
    if ( escp_labels ) memset (escp_labels, 0, escp_nb * sizeof (char *));
    memset (labels, 0, labels_sz);  /* storage for keys labels */

    sp = buf;
    for ( i = 0 ; i < nb ; i++ ) {
	sp1 = strchr (sp, ' ');
	escp_str [i] = sp;
	if ( !sp1 ) break;
	*sp1 = '\0';
	sp = sp1 +1;
    }
    if ( i < nb ) nb = i+1;

    /* assigned keys */
    memset (shift_key, 0, sizeof (shift_key));
    memset (keys_label, 0, sizeof (keys_label));
    shift_key_nb = sizeof (shift_key) / sizeof (shift_key[0]);

    msg = labels;
    for ( i = 0 ; i < nb ; i++ ) {
	ch = escp_str[i][0];
	if ( ch >= '\040' && (ch != '\177') ) continue;
	all_string_to_key (escp_str[i], shift_key, &shift_key_nb,
			   keys_label, sizeof (keys_label));
	if ( escp_labels_flg ) {
	    msg_sz = labels_sz - (msg - labels);
	    sz = concat_labels (shift_key, shift_key_nb, msg, msg_sz,
				keys_label, sizeof (keys_label));
	    if ( sz ) {
		if ( escp_labels ) escp_labels [i] = msg;
		msg += (strlen (msg) +1);
	    }
	}
    }
    if ( ! escp_labels_flg ) {
	/* a single string for all key labels */
	msg_sz = labels_sz;
	(void) concat_labels (shift_key, shift_key_nb, msg, msg_sz,
			      keys_label, sizeof (keys_label));
    }
    return nb;
}


/* key_assigned : local to generate the string of key assigned to the given key function */
/* ------------------------------------------------------------------------------------- */

static void key_assigned (msg, msg_sz, fcmd, fstr, funcval_flg)
char *msg;
int msg_sz;
int fcmd;
char *fstr;
Flag funcval_flg;
{

    extern Flag verbose_helpflg;
    extern char * escstrg ();
    char buf[256];
    char *escp[16];
    int sz;

    sz = strlen (msg);
    (void) key_by_func (fcmd, buf, sizeof (buf),
		 escp, NULL, sizeof (escp) / sizeof (escp [0]),
		 msg+sz, msg_sz-sz, funcval_flg, NO);
    if ( verbose_helpflg || funcval_flg ) printf ("%s\n", escstrg (buf));
}

/* ------------------------------------------------------------------------ */

/* keyfunc_ibmpc : find the key for a given key command */
/* ---------------------------------------------------- */

static Cmdret keyfunc_ibmpc (helparg)
char *helparg;
{
    extern void all_ctrl_key_by_func (char *msg, int msg_sz, int fcmd);
    extern Flag verbose_helpflg;
    extern S_looktbl itsyms[];
    int idx, sz;
    char msg [256];
    char *hlparg;
    int fcmd;
    char *str;
    Flag tflg;

    if ( ! *nxtop )
        return CRNEEDARG;

    str ="";
    hlparg = getword (&nxtop);
    idx = lookup (hlparg, itsyms);
    fcmd = itsyms[idx].val;
    str  = itsyms[idx].str;
    if ( *nxtop && (fcmd == CCCMD) ) {
	if ( *hlparg ) sfree (hlparg);
	hlparg = getword (&nxtop);
	idx = lookup (hlparg, itsyms);
	if ( idx  >= 0 ) {
	    fcmd = itsyms[idx].val;
	    str  = itsyms[idx].str;
	}
    }

    if ( idx  < 0 ) {
	mesg (TELALL + 2, hlparg,
	      (idx != -2) ? " : Unknown Key Function" : " : Ambiguous Key Function");
	if ( *hlparg ) sfree (hlparg);
	loopflags.hold = YES;
	return RE_CMD;
    }
    if ( *hlparg ) sfree (hlparg);

    hlparg = getword (&nxtop);
    if ( *hlparg ) sfree (hlparg);
    if ( *nxtop ) 
	return CRTOOMANYARGS;

    memset (msg, 0, sizeof (msg));
    sz = sprintf (msg, "%s: ", str);
    all_ctrl_key_by_func (&msg[sz], sizeof (msg) -sz, fcmd);
    sz = strlen (msg);
    tflg = verbose_helpflg;
    verbose_helpflg = NO;
    key_assigned (&msg[sz], sizeof (msg) - sz, fcmd, str, NO);
    verbose_helpflg = tflg;
    msg [term.tt_width -3] = '\0';
    mesg (TELALL + 1, msg);
    loopflags.hold = YES;
    return CROK;
}

/* Does some processing in full screen mode */
Cmdret do_fullscreen (Cmdret (*process) (), void *p1, void *p2, void *p3)
{
    extern void reset_crlf ();
    extern char *command_msg;
    Cmdret cc;
    int oflag, col, lin;

    col = cursorcol; lin = cursorline;
    savecurs ();
    ( *term.tt_clear ) ();
    ( *term.tt_home ) ();
    poscursor (0, term.tt_height -1);
    oflag = set_crlf ();
    command_msg = NULL;

    cc = (*process) (p1, p2, p3);

    reset_crlf (oflag);
    fflush (stdout);
    fresh ();
    restcurs ();
    if ( !command_msg ) {
	mesg (TELALL +1, "");
	loopflags.hold = NO;
    }
    return cc;
}

/* ======================================================================== */

/* New help for Unix console */
/* ------------------------- */
/*
    The argument provided by a <cmd><help> is defined by the value
	char *boardstg = "status\0";
	defined in the file : e.m.c in "mainloop" routine
*/

static char retmsg []  = "--- Press ENTER (CR or RETURN) or <Ctrl C> to return to the Edit session ---";
static char contmsg [] = "--- Press ENTER (CR or RETURN) to navigate, <Ctrl C> to return to the Edit session ---";
static char keyfmsg [] = "--- Press a key for Key function, <Ctrl C> to return to the Edit session ---";

#define HELP_CMD    1
#define HELP_KBCK   2
#define HELP_KEY    3
#define HELP_KEYF   4
#define HELP_KMAP   5
#define HELP_NEW    6
#define HELP_STAT   7
#define HELP_CHARSET 8
#define HELP_GRAPHSET 9

S_looktbl helptable [] = {
    "charset"           , HELP_CHARSET,
    "cmd"               , HELP_CMD  ,
    "commands"          , HELP_CMD  ,
    "graphset"          , HELP_GRAPHSET,
    "kbchk"             , HELP_KBCK ,
    "key"               , HELP_KEY  ,
    "keyboard_check"    , HELP_KBCK ,
    "keyf"              , HELP_KEYF ,
    "keyfunctions"      , HELP_KEYF ,
    "keymap"            , HELP_KMAP ,
    "new"               , HELP_NEW  ,
    "newfeatures"       , HELP_NEW  ,
    "status"            , HELP_STAT ,
    0                   , 0
    };

/* "cmd" and "commands" are ambiguous with the key function and editor command */
#define CMD_IDX 1
#define COM_IDX 2

/* short description of help command */
static char helpmsg [] = "\
\"help status\" .   .   .   .   .   .   Display the actual editor parameters value\n\
\"help NewFeatures\" or \"help new\"  .   Display the new features of the editor\n\
\"help cmd\"  or \"help commands\".   .   Navigate in Editor Commands description\n\
\"help keyf\" or \"help keyfunctions\".   Navigate in Key Functions description\n\
\"help <command or function>\"  .   .   Info on Editor Command and/or Key Function\n\
\"help cmd <command>\"  .   .   .   .   Info on the given Editor Command\n\
\"help keyf <function>\"    .   .   .   Info on the given Key Function\n\
\"help keymap\" .   .   .   .   .   .   Global keyboard mapping info\n\
\"help key <function>\" .   .   .   .   Info on the keys assigned to function\n\
\"help charset\".   .   .   .   .   .   Display the current character set\n\
\"help graphset\"   .   .   .   .   .   Display the graphical character set used\n\
					for window edges\n\
\"help kbchk\" or \"help keyboard_check\" Interractively check keyboard mapping\n\
";

void show_help_msg ()
{
    putc ('\n', stdout);
    fputs (helpmsg, stdout);
}

/* help special command string to display ambiguous list " */
static char ambig_str []  = "ambiguous";
static char allcmds_str [] = "allcommands";

char * help_ambiguous_str (Flag ambig_flg)
{
    return ( ambig_flg ? ambig_str : allcmds_str );
}

char * help_cmd_str ()
{
    return ( helptable [CMD_IDX].str );
}

int expand_help (char *strg, int strg_sz, int *pos)
/*
 *  Return :
 *          -3 : no more argument in strg
 *          -2 : ambiguous argument in strg
 *          -1 : not a help argument (can be a command or a key function)
 *           0 : nothing to be done
 *           1 : well done
 */
{
    extern int expand_keyword_name (int pos, S_looktbl *table, int *val);
    extern int getccmd (char *strg, int strg_sz, int *ccmdpos);
    extern char *getparam (char *str, int *beg_pos, int *end_pos);
    extern S_looktbl cmdtable[];
    extern S_looktbl itsyms[];
    int cc, pos1, pos2, val, sz;
    char * kwd;

    if ( !strg || !*strg ) return -2; /* no more argument */
    pos1 = ( pos && (*pos > 0) ) ? *pos : 0;
    kwd = getparam (strg, &pos1, &pos2);
    if ( !kwd ) return -3;  /* no more argument */

    cc = expand_keyword_name (pos1, helptable, &val);
    if ( cc == -1 ) return -1;    /* keyword not found */
    if ( cc == -2 ) return -2;    /* ambiguous keyword */

    if ( cc == 1 ) {
	/* reload the expanded command string */
	sz = getccmd (strg, strg_sz, NULL);
	kwd = getparam (strg, &pos1, &pos2);
	if ( !kwd ) return -2;  /* must not occure at this point */
    }

    /* more argument ? */
    pos1 = pos2;
    kwd = getparam (strg, &pos1, &pos2);
    if ( !kwd ) return 1;   /* nothing more to be done */

    if ( pos ) *pos = pos1;
    if ( val == HELP_KEYF )
	cc = expand_keyword_name (pos1, itsyms, &val);
    else if ( val == HELP_CMD )
	cc = expand_keyword_name (pos1, cmdtable, &val);

    return 1;   /* well done */
}



static Cmdret help_completion (int cc, char *helparg)
{
    extern char *command_msg;
    static char err_msg [128];

    if ( cc >= 0 ) return CROK;

    if ( cc == -2 ) {
	sprintf (err_msg ,"---- \"%s\" : Ambiguous Editor command or Key function", helparg);
	command_msg = err_msg;
	return RE_CMD;
    } else {
	sprintf (err_msg, "---- \"%s\" : Unknow Editor Command, Key function or HELP parameter", helparg);
	command_msg = err_msg;
    }
    return CROK;
}

static Cmdret process_help_std (helparg, nextarg)
char *helparg, *nextarg;

{
    extern char *nxtop;
    extern S_looktbl cmdtable[];
    static int browse_cmdhelp  (char *, char *);
    static int browse_keyfhelp (char *, char *);
    extern int get_ctrlc_fkey ();

static char stmsg [] = "\n\
For info on a particular key action, press that key combination.\n\
\n";

static char stmsg1 [] = "\
Now press a key (or keys combination) for description of the assigned action.\
\n";

    static int stmsg_nbln = 0;  /* number of lines in stmsg string */

    Short qq;
    int i, idx, fkey, hcmd;
    int cmd, cc;
    char *cmd_str, *str;
    int oflag;
    int col, lin, nbli, nb, sz, ctrlc;
    Flag ctrlc_flg;

    extern void showstatus ();
    extern int set_crlf ();
    extern void reset_crlf ();
    extern void check_keyboard ();
    extern char verstr[];
    extern S_looktbl itsyms[];
    extern void set_ambiguous_param (S_looktbl *table, char *str, Flag abv);
    static Cmdret show_ambiguous ();
    Cmdret help_ambiguous (Flag *ctrlc_flg_pt, Flag ambig_flg);

    if (   (helparg == NULL) || (*helparg == '\0')
        || ( strcmp (helparg, "keyhelp") == 0) ) {
	if ( ! stmsg_nbln ) {
	    char ch, *cp;
	    for ( cp = helpmsg ; (ch = *cp) ; cp++ ) if ( ch == '\n' ) stmsg_nbln++;
	    for ( cp = stmsg   ; (ch = *cp) ; cp++ ) if ( ch == '\n' ) stmsg_nbln++;
	    }

	nbli = stmsg_nbln;
	fputs (helpmsg, stdout);
	fputs (stmsg, stdout);
	fkey = get_ctrlc_fkey ();
	if ( fkey != CCRETURN ) {
	    nb = help_description_check ('~', -1, fkey);
	    sz = term.tt_height -(nbli + nb) -3;
	    if ( sz >= 0 ) {
		for ( ; sz >= 0 ; sz -- ) fputc ('\n', stdout);
		fputs ("<Ctrl C> is assigned to :", stdout);
		by_flg = YES;
		(void) help_description ('~', -1, fkey, NULL, NULL, NULL);
	    }
	}
	browse_keyboard (retmsg);
	return CROK;
    }
    if ( strcmp (helparg, ambig_str) == 0 ) {
	/* special to display list of ambiguous command (see e.m.c) */
	return ( help_ambiguous (&ctrlc_flg, YES));
    }
    if ( strcmp (helparg, allcmds_str) == 0 ) {
	/* special to display list of command (see e.m.c) */
	cc = help_ambiguous (&ctrlc_flg, NO);
	return ( ctrlc_flg ? cc : RE_CMD );
    }

    hcmd = lookup (helparg, helptable);
    if ( hcmd >= 0 ) {
	int sz;

	if ( *nextarg && (helptable [hcmd].val != HELP_KEYF)
		      && (helptable [hcmd].val != HELP_CMD) )
	    return CRTOOMANYARGS;

	cc = 0;
	switch (helptable [hcmd].val) {
	    case HELP_NEW :
		fprintf (stdout, "%s\n\n", verstr);
		nbli = 2;
		(void) help_info ("New_features_Linux", &nbli);
		ctrlc = check_new_page (&nbli, 1);
		ctrlc = wait_keyboard (retmsg, NULL);
		break;

	    case HELP_STAT :
		showstatus ();
		ctrlc = wait_keyboard (retmsg, NULL);
		break;

	    case HELP_KMAP :
		keymap_term ();
		break;

	    case HELP_CMD :
		sz = strlen (paramv);
		if ( sz > 0 ) {
		    /* command line is not blank */
		    if ( strncmp (paramv, helptable [CMD_IDX].str, sz) == 0 ) {
			/* help cmd is ambiguous with the cmd key function */
			cc = browse_keyfhelp (paramv, NULL);
			break;
		    }
		    if ( strncmp (paramv, helptable [COM_IDX].str, sz) == 0 ) {
			/* help command is ambiguous with the editor command */
			cc = browse_cmdhelp (paramv, NULL);
			break;
		    }
		}
		/* help key on command line results in a
		   'help command' with "cmd <command>" parameter
		   paramv = <command>, nextarg = <command>
		*/
		cc = browse_cmdhelp (nextarg, NULL);
		break;

	    case HELP_KEYF :
		cc = browse_keyfhelp (nextarg, NULL);
		break;

	    case HELP_KBCK :
		check_keyboard (NO, NO);
		break;

	    case HELP_CHARSET :
		check_keyboard (YES, NO);
		ctrlc = wait_keyboard (retmsg, NULL);
		break;

	    case HELP_GRAPHSET :
		check_keyboard (YES, YES);
		ctrlc = wait_keyboard (retmsg, NULL);
		break;

	}
	if ( cc >= 0 ) return CROK ;
	return (help_completion (cc, nextarg));
    }

    else if ( hcmd == -2 ) {
	/* must be a strick match or at least 3 chars in help argument */
	extern char *command_msg;
	static char err_msg [128];
	sprintf (err_msg ,"---- \"%s\" : Ambiguous Help argument", helparg);
	command_msg = err_msg;
	return RE_CMD ;
    }

    /* can be a help <cmd> or <keyf> */
    idx = lookup (helparg, cmdtable);
    i = lookup (helparg, itsyms);
    if ( idx >= 0 ) {
	cc = browse_cmdhelp (helparg, (i >= 0) ? keyfmsg : NULL);
	if ( CtrlC_flg ) return CROK;
	if ( i >= 0 ) {
	    /* also a key function */
	    (*term.tt_clear) ();
	    cc = browse_keyfhelp (helparg, NULL);
	    if ( cc >= 0 ) return CROK;
	}
	return CROK;
    }

    if ( idx != -2 ) {
	/* can be a key function */
	cc = browse_keyfhelp (helparg, NULL);
	if ( cc >= 0 ) return CROK;
    }
    help_completion (idx, helparg);
    if ( idx = -1 ) {
	set_ambiguous_param (helptable, NULL, NO);
	return CRUNRECARG;
    }
    return CROK ;
}


Cmdret help_std (char * helparg)
{
    Cmdret cc;
    int i, hcmd;
    char ch;

    if ( helparg ) {
	for ( i = 0 ; (ch = helparg[i]) ; i++ )
	    helparg[i] = tolower (ch);

	if ( ! *nxtop ) {
	    /* check for "help ?" */
	    if ( strcmp (helparg, ambig_str) != 0 ) {
		/* do not overwrite the ambiguous parameters */
		cc = help_cmd_arguments (helparg, helptable);
		if ( cc != CRUNRECARG )
		    return cc;
	    }
	}
	hcmd = lookup (helparg, helptable);
	if ( (hcmd >= 0) && (helptable [hcmd].val == HELP_KEY) )
	    return (keyfunc_ibmpc () );
    }
    cc = do_fullscreen (process_help_std, (void *) helparg, (void *)nxtop, NULL);
    if ( (cc == -1) || (cc == -2) ) mesg (ERRSTRT + 3, "\"", helparg, "\"");
    return cc;
}

static Cmdret show_ambiguous_files (Flag *ctrlc_flg_pt)
{
    extern void clear_ambiguous_param ();
    extern void show_namelist ();
    int cc;

    show_namelist ();
    clear_ambiguous_param ();
    cc = wait_keyboard (retmsg, NULL);
    if ( ctrlc_flg_pt ) *ctrlc_flg_pt = cc;
    return CROK;
}

static Cmdret show_ambiguous (Flag *ctrlc_flg_pt, Flag *ambig_flg_pt)
{
    extern int get_ambiguous_param (char **amb_kwd, S_looktbl **table_pt, S_looktbl *(**sorted_pt)[],
				    int * sorted_sz_pt, Flag *abv_pt, char **comt_pt);
    extern void clear_ambiguous_param ();
    S_looktbl *table, *(*sorted)[];
    char ch, *ambiguous_kwd, *str, *comt;
    int i, j, k, cc, sz, nb, type;
    int col, max_col, h, height;
    Flag abv_flg;
    S_looktbl *tbl_pts [256];

    if ( ambig_flg_pt && !*ambig_flg_pt ) {
	/* not a command : display just the editor commands list */
	extern S_lookstruct cmdstruct;
	print_sort_table (&cmdstruct);
    } else {
	/* realy help ambiguous call */
	if ( ctrlc_flg_pt ) *ctrlc_flg_pt = NO;
	type = get_ambiguous_param (&ambiguous_kwd, &table, &sorted, &nb, &abv_flg, &comt);
	if ( type == 0 ) return CRNOVALUE;
	else if ( type == 2 ) return show_ambiguous_files (ctrlc_flg_pt);
	else if ( type != 1 ) return CRNOVALUE;
	else if ( ! ambiguous_kwd ) return CRNOVALUE;

	ch = ( *ambiguous_kwd ) ? ' ' : 's';
	if ( comt ) printf ("%s%c\n  ", comt, ch);
	if ( *ambiguous_kwd ) {
	    if ( abv_flg )
		printf ("Too short abreviation \"%s\"\n\n", ambiguous_kwd);
	    else
		printf ("Keywords matching \"%s\"\n\n", ambiguous_kwd);
	} else putchar ('\n');

	sz = strlen (ambiguous_kwd);
	if ( sz > 0 ) {
	    /* extract the matching keywords */
	    memset (tbl_pts, 0, sizeof (tbl_pts));
	    for ( i = 0, nb = 0; str = table [i].str ; i++ ) {
		cc = strncmp (str, ambiguous_kwd, sz);
		if ( cc < 0 ) continue;
		else if ( cc == 0 ) tbl_pts [nb++] = &table [i];
		else if ( cc > 0 ) break;
		if ( nb > (sizeof (tbl_pts) /  sizeof (tbl_pts[0]) ) ) break;
	    }
	    sorted = &tbl_pts;
	}
	if ( ! sorted ) return;

	if ( nb > 0 ) {
	    height = term.tt_height -5;
	    max_col = ((term.tt_width -2) / 18);
	    if ( height <= 0 ) height = 1;
	    h = (nb -1) / max_col +1;   /* nb of lines for max columns */
	    if ( h < 5 ) h = 5;
	    if ( height > h ) height = h;
	    col = (nb -1) / height +1;
	    if ( col > max_col ) col = max_col;
	    if ( col == 1 ) height = nb;
	    for ( i = 0 ; i < height ; i++ ) {
		for ( j = 0 ; j < col ; j++ ) {
		    k = j * height + i;
		    if ( k >= nb ) continue;
		    printf ("%3d %-14s  ", (*sorted) [k]->val, (*sorted) [k]->str);
		}
		putchar ('\n');
	    }
	}
    }
    clear_ambiguous_param ();
    cc = wait_keyboard (retmsg, NULL);
    if ( ctrlc_flg_pt ) *ctrlc_flg_pt = cc;
    return CROK;
}

Cmdret help_ambiguous (Flag *ctrlc_flg_pt, Flag ambig_flg)
{
    int cc;
    cc = do_fullscreen (show_ambiguous, ctrlc_flg_pt, &ambig_flg, NULL);
    return cc;
}

/* Display the arguments of a command */

Cmdret help_cmd_arguments (char *str, S_looktbl *table)
{
    void set_ambiguous_param (S_looktbl *, char *, Flag);
    Cmdret retval;

    set_ambiguous_param (table, NULL, NO);  /* for display args with help key */
    if ( strcmp (str, "!") != 0 )
	return -1;
    retval = help_ambiguous (NULL, NO); /* display the arguments in table */
    return retval;
}


/* Utility to display some info and wait for keyboard */
/* -------------------------------------------------- */

static Cmdret process_show_info (
		Flag (*(*my_info)())(int, int *),
		int *val_ret, char *msg)
{
    int ctrlc, gk, val, i;
    char val_strg [16], *str;
    char msg_str [128];
    Flag (*my_kbhandler) (int, int *);

    if ( my_info ) my_kbhandler = (*my_info) ();

    memset (msg_str, 0, sizeof (msg_str));
    str = ( msg ) ? msg : retmsg;
    strncpy (msg_str, str, sizeof (msg_str)-1);
    if ( val_ret ) {
	val = 0;
	memset (val_strg, 0, sizeof (val_strg));
	for ( i = 0 ; i < sizeof (val_strg) -1 ; ) {
	    ctrlc = wait_keyboard (msg_str, &gk);
	    if ( ctrlc || (gk == CCCMD) || (gk == CCINT) ) {
		/* cancel */
		val = 0;
		break;
	    }
	    if ( gk == CCRETURN ) {
		/* done */
		if ( ! val ) val = atoi (val_strg);
		break;
	    }
	    if ( (gk == CCMOVELEFT) || (gk == CCDELCH) || (gk == CCBACKSPACE) ) {
		if ( i > 0 ) i--;
		msg_str [i] = str [i];
		val_strg [i] = '\0';
		continue;
	    } else if ( my_kbhandler ) {
		if ( my_kbhandler (gk, &val) ) continue;
	    }
	    if ( (gk < '0') || (gk > '9') ) continue;
	    val = 0;
	    val_strg [i++] = (char) gk;
	    memcpy (msg_str, val_strg, strlen (val_strg));
	}
	*val_ret = val;
    } else {
	ctrlc = wait_keyboard (msg_str, NULL);
    }
    return CROK;
}

void show_info (void (*my_info) (), int *val_ret, char *msg,
		void (*my_resize) (), void (*my_size) ())
{
    resize_service = my_resize;
    resize_param = my_size;
    (void) do_fullscreen (process_show_info, (void *) my_info, (void *) val_ret, (void *) msg);
    resize_param = resize_service = NULL;
}

/* ----------------------------------------------------------------------- */

S_looktbl *keyftable = NULL;
int keyftable_sz = 0;   /* nb of entry in keyftable */

static FILE *outst = NULL;  /* stdout or the file stream */


static int sort_looktb (obj1, obj2)
S_looktbl *obj1;
S_looktbl *obj2;

{
char *p1, *p2;
int cmp;
    p1 = ( isalpha (obj1->str[0]) ) ? obj1->str : &obj1->str[1];
    p2 = ( isalpha (obj2->str[0]) ) ? obj2->str : &obj2->str[1];
    cmp = strcmp (p1, p2);
    if ( cmp == 0 ) cmp = strcmp (obj1->str, obj2->str);
    return (cmp);
}


/* print_sorted : routine to print in columns a list of strings */
/* ------------------------------------------------------------ */

/* amximum nb of column printed by print_sorted */
#define MAX_PRT_COL 16

/* Build the item_idx array
 *  Return the number of line -1 or
 *         -1 if item_idx array is too samll (must be > item_nb)
 */
static int col_struct (int item_nb, int col_nb,
		       int *item_idx, int item_idx_sz, /* 1st index in each col */
		       int *lastln_pt)
{
    int lnb, lastln;
    int i, k;

    if ( item_idx_sz <= col_nb ) return -1;

    lnb = item_nb / col_nb;
    lastln = item_nb % col_nb;
    for ( i = 0, k = 0 ; i < col_nb ; i++ ) {
	item_idx[i] = k;
	k += ( i < lastln ) ? lnb +1 : lnb;
    }
    item_idx [i] = item_nb;
    if ( lastln_pt ) * lastln_pt = lastln;
    return lnb;
}

static void print_sorted (tbl, print_func, item_nb, col_nb, nb_data,
			  nb_spc, colsz_arr)
void *tbl;
void (*print_func)();
int item_nb;    /* number of items in sorted array */
int col_nb;     /* number of columns */
int nb_data;    /* nb of charcaters printed for each item */
int nb_spc;     /* number of spaces between each item */
int *colsz_arr;

{
    char line[256];
    int line_sz, width;
    int lnb, lastln;
    int item_idx [MAX_PRT_COL +1];
    int line_idx [MAX_PRT_COL +1];
    int i, j, k, l, idx;
    Flag stdout_flg;

    width  = term.tt_width;
    line_sz = (sizeof (line) > width) ? width -1 : sizeof (line) -2;
    if ( outst == NULL ) outst = stdout;
    stdout_flg = ( outst == stdout );

    lnb = col_struct (item_nb, col_nb, item_idx,
		      (sizeof (item_idx) / sizeof (item_idx[0])), &lastln);
    memset (line_idx, 0, sizeof (line_sz));
    for ( i = idx = 0 ; i < col_nb ; i++ ) {
	line_idx [i] = idx;
	idx += colsz_arr [i] + nb_data + nb_spc;
    }
    for ( i = 0 ; i <= lnb ; i ++ ) {
	memset (line, ' ', sizeof (line));
	l = (i == lnb) ? lastln : col_nb;
	if ( l == 0 ) continue;
	for ( j = 0 ; j < l ; j ++ ) {
	    (* print_func) (tbl, item_nb, &line[line_idx[j]], item_idx[j],
			    colsz_arr[j], nb_data);
	    item_idx[j] +=1;
	}
	for ( idx = 0, k = line_sz +1 ; k >= 0 ; k-- ) {
	    if ( line[k] == '\0' ) line[k] = ' ';
	    if ( (idx == 0) && (line[k] != ' ') ) idx = k+1;
	}
	if ( idx == 0 ) idx = sizeof (line) -2;
	line[idx++] = '\n'; line[idx] = '\0';
	if ( stdout_flg ) fputs (line+2, outst);
	else {
	    if ( (line [0] == ' ') && (line [1] == ' ') ) line [0] = '#';
	    fputs (line, outst);
	}
    }
    return;
}


/* table_print : local to print 1 item from table */
/* ---------------------------------------------- */

static void sort_table_print (S_looktbl  *(*tbl)[], int tbl_sz, char *line_pt,
			      int idx, int fld_sz, int data_sz)
{
    if ( idx >= tbl_sz ) return;
    (void) sprintf (line_pt + 2, "%*d %*s ",  data_sz -1,
	     (*tbl)[idx]->val, -fld_sz, (*tbl)[idx]->str);
}

/* get_all_aliases : construct a string with all the aliases of a command */
/* ---------------------------------------------------------------------- */

static char * get_all_aliases (S_lookstruct *tblstruct, int cmd_val, Flag sep_flg)
{
    static char aliases [128];
    int nb, j, sz;
    char *format;

    j = tblstruct->alias_table_sz;
    if ( !tblstruct->as_alias || (j <= 0) || !tblstruct->alias_table )
	return NULL;

    /* check and get aliases */
    memset (aliases, 0 , sizeof (aliases));
    format = sep_flg ? "%s\"%s\"" : "%s%s";
    for ( ; j >= 0 ; j-- ) {
	if ( (*tblstruct->alias_table)[j]->val != cmd_val ) continue;
	sz = strlen (aliases);
	nb = (sizeof (aliases) -1) - sz;
	if ( nb > 0 ) snprintf (&aliases[sz], nb, format,
	  aliases[0] ? ", " : "",
	  (*tblstruct->alias_table)[j]->str);
    }
    return ( aliases[0] ? aliases : NULL );
}


/* browse_sortedtbl : common processing to browse commands or key functions */
/* ------------------------------------------------------------------------ */

static int browse_sortedtbl (S_lookstruct *tblstruct, char * mystr, char *waitmsg)
{
    static int waitkb (short);

    char *all_aliases;
    int nb, i, di, idx, cmd_val, cc;
    char *str, *cmd_str;

    if ( mystr && *mystr ) {
	/* a single entry */
	idx = lookup (mystr, tblstruct->table);
	if ( idx == -2 ) {  /* ambiguous */
	    show_ambiguous (NULL, NULL);
	    return 0;
	}
	if ( idx < 0 ) return idx;
	cc = (* tblstruct->one_help) (&(tblstruct->table [idx]), mystr);
	if ( cc ) cc = wait_keyboard (waitmsg ? waitmsg : retmsg, NULL);
	return 0;
    }

    /* browse all entries */
    print_sort_table (tblstruct);
    cc = wait_keyboard (contmsg, NULL);
    if ( cc > 0 ) return;
    (*term.tt_clear) ();

    for ( i = 0, di = 1 ;  ; i += di ) {
	if ( i < 0 ) i = tblstruct->sorted_table_sz -1;
	if ( i >= tblstruct->sorted_table_sz ) i = 0;

	/* display the help message */
	cc = (* tblstruct->one_help) ((*tblstruct->sorted_table)[i], NULL);
	if ( CtrlC_flg) break;
	cc = waitkb (NO);
        if ( cc > 0 ) break;
        di = ( cc < 0 ) ? -1 : 1;
    }
    return cc;
}

/* browse_cmdhelp : local to browse all editor commands ("help cmd" processing) */
/* ---------------------------------------------------------------------------- */

int one_help_cmd (S_looktbl *cmd, char *str)
/* return 1 : ok, 0 : error */
{
    extern S_lookstruct cmdstruct;

    char *all_aliases, *cmd_str;
    int idx, cmd_val;

    if ( !cmd || !cmd->str || !*cmd->str ) return 0;

    /* found the major name, not alias */
    cmd_val = cmd->val;
    for ( idx = cmdstruct.sorted_table_sz -1 ; idx >= 0 ; idx-- ) {
	if ( (*cmdstruct.sorted_table)[idx]->val == cmd_val ) break;
    }
    if ( idx < 0 ) return 0;

    cmd_str = (*cmdstruct.sorted_table)[idx]->str;
    all_aliases = get_all_aliases (&cmdstruct, cmd->val, YES);
    (void) help_cmd (cmd_val, cmd_str, str, all_aliases);
    return 1;
}


static int browse_cmdhelp (char * cmdstr, char *waitmsg)
{
    extern S_lookstruct cmdstruct;
    int cc;

    cc = browse_sortedtbl (&cmdstruct, cmdstr, waitmsg);
    return cc;
}

/* help_keyf :
 *   Displays a message describing the effects of the key function
 *   whose lexed value is passed in "fcnt". 
 *   The command major name is given in fcnt_str.
 */

static int help_keyf (fcnt, fcnt_str)
int fcnt;
char *fcnt_str;
{
    int go;

    go = help_description ('~', 0, fcnt, NULL, fcnt_str, NULL);
    return (go);
}

/* waitkb : local to wait on keyboard input */
/* ---------------------------------------- */
/*
    return : -1 = UP or PAGE-UP
	      0 = any other key
	      1 = CCCMD or CCINT
*/

static void wait_msg (short nl) {
    extern int GetCccmd ();
    char ch;

    (*term.tt_addr)  (term.tt_height -1, 0);
    ch = '@' + GetCccmd ();
    if ( nl ) putchar ('\n');
    printf ("Push a key: next; <Ctrl C>, <Ctrl %c>, <cmd>Key: exit; <UP>, <DOWN>: navigate\r", ch);
    fflush (stdout);
}

static int waitkb (short nl)

{
unsigned Short qq;

    wait_msg (nl);
    keyused = YES;
    qq = getkey (WAIT_KEY);
    (*term.tt_clear) ();
    if ( (qq == CCINT) || (qq == CCCMD) || CtrlC_flg || Esc_flg ) {
	if ( keyfile != NULL ) {
	    putc (CCINT, keyfile);
	    if ( numtyp > MAXTYP ) {
		flushkeys ();
		numtyp = 0;
		}
	    }
	return (1);
	}
    if ( (qq == CCMIPAGE) || (qq == CCMOVEUP) ) return (-1);
    return (0);
}

/* wait_key : local to wait on keyboard input, never wait if output is on file */
/* --------------------------------------------------------------------------- */
/*
    return : -1 = UP or PAGE-UP
	      0 = any other key (continue)
	      1 = CCCMD or INT
*/

static int wait_key ()
{
    if ( outst == NULL ) outst = stdout;
    if ( outst != stdout ) return (0);
    return ( waitkb (YES) );
}

/* wait_keyboard : wait for any key pushed on the keyboard */
/* ------------------------------------------------------- */
/*
    return the result of getkey in *gk_pt (if not NULL)
    return True (YES) if <Ctrl C> key was pushed
*/

static void update_msg (int width, int height, void *para)
{
    char * msg, ch;

    if ( ! para ) return;
    msg = (char *) para;
    ch = '\0';
    if ( width < strlen (msg) ) {
	ch = msg [width];
	msg [width] = '\0';
    }
    fputc ('\r', stdout); fputs (msg, stdout); fputc ('\r', stdout);
    (void) fflush (stdout);
    if ( ch ) msg [width] = ch;
}

static void resize_update_msg (int width, int height, void *para)
{
    char * msg, ch;

    if ( resize_service ) (*resize_service) (width, height);
    update_msg (width, height, para);
}

int wait_keyboard (char *msg, int *gk_pt)
{
    extern void set_alt_resize (void (* my_resize) (int width, int height, void *para), void *para);
    extern void switch_ctrlc ();
    int qq;
    char str [256], *strg;
    int width, height;

    if ( CtrlC_flg ) return CtrlC_flg;

    memset (str, 0, sizeof (str));
    if ( msg ) {
	if ( resize_param ) (* resize_param) (&width, &height);
	else {
	    width  = term.tt_width;
	    height = term.tt_height;
	}
	(*term.tt_addr)  (height -1, 0);
	strncpy (str, msg, sizeof (str) -1);
	update_msg (width, height, (void *) str);
    }
    keyused = YES;
    switch_ctrlc (YES);
    set_alt_resize (resize_update_msg, (void *) str);
    qq = getkey (WAIT_KEY);
    set_alt_resize (NULL, NULL);
    if ( (qq == CCINT) && keyfile ) putc (CCINT, keyfile);
    if ( gk_pt ) *gk_pt = qq;
    switch_ctrlc (NO);
    return CtrlC_flg;
}

/* call 'reset_ctrlc' to be sure that 'wait_keyboard' will wait for input */
void reset_ctrlc () {
    CtrlC_flg = NO;
}

Flag test_ctrlc () {
    return (CtrlC_flg);
}

int call_waitkeyboard (char *msg, int *gk_pt)
{
    int cc;

    reset_ctrlc ();
    cc = wait_keyboard (msg ? msg : retmsg, gk_pt);
    return (CtrlC_flg ? CROK : RE_CMD);
}

/* wait_continue : wait for a key pushed on the keyboard */
/* ----------------------------------------------------- */
/*
    return :  0 = any other key
	      1 = CCCMD or CCINT
*/

int wait_continue (any_flg)
int any_flg;    /* ! 0 : wait in any case */
		/*   0 : wait only if the screen size < 43 rows */
{
    if ( !any_flg && (term.tt_height >= 43) ) return (0);
    (*term.tt_addr)  (term.tt_height -1, 0);
    return ( waitkb (NO) > 0 );
}

/* browse_keyfhelp : local to browse all key functions ("help keyf" processing) */
/* ---------------------------------------------------------------------------- */

int one_help_keyf (S_looktbl *kf, char *str)
/* return 1 : ok, 0 : error */
{
    extern Flag verbose_helpflg;
    extern Flag noX11flg;

    int nb, di, fcnt, cc, sz;
    Flag funcval_flg, tflg;
    char *fcnt_str, *sp;
    char msg[256];

    if ( ! kf ) return 0;

    fcnt = kf->val;
    fcnt_str = kf->str;
    if ( fcnt == 0177 ) fcnt = CCBACKSPACE; /* special case for del char */
    funcval_flg = ( (fcnt == KBINIT) || (fcnt == KBEND) );
    by_flg = NO;
    (void) help_keyf (fcnt, fcnt_str);
    for ( sp = fcnt_str ; *sp ; sp++ ) putchar (toupper (*sp));
    funcval_flg = ( (fcnt == KBINIT) || (fcnt == KBEND) );
    tflg = verbose_helpflg;
/*  ?!?
    verbose_helpflg = noX11flg;
*/
    sp = ( funcval_flg ) ? " Current value: " :
			   ( verbose_helpflg || funcval_flg )
				? " Current Key Assignement: "
				: " Current Key Assignement:\n";
    fputs (sp, stdout);
    memset (msg, 0, sizeof (msg));
    all_ctrl_key_by_func (msg, sizeof (msg), fcnt);
    sz = strlen (msg);
    key_assigned (&msg[sz], sizeof (msg) -sz, fcnt, fcnt_str, funcval_flg);
    verbose_helpflg = tflg;
    puts (msg);
    return 1;
}

static int browse_keyfhelp (char * kfstr, char *waitmsg)
{
    extern S_lookstruct keyfstruct;
    int cc;

    cc = browse_sortedtbl (&keyfstruct, kfstr, waitmsg);
    return cc;
}


/* print_cmdtable : local to print in column the command names */
/* ----------------------------------------------------------- */

/* Build the column size, and return the sum of all columns size */

static int columns_size (
    int *item_sz, int item_nb,  /* item_nb : nb of element in item_sz array */
    int col_nb,                 /* nb of columns */
    int *colsz_arr, int colsz_sz)   /* max item size for each column */
{
    int i, i1, j;
    int item_idx [MAX_PRT_COL+1];
    int sz, c_nb, t_sz, lnb;
    int max_sz;

    /* set default : maxi size for all columns */
    for ( i = item_nb -1 ; i >= 0 ; i-- ) {
	sz = item_sz [i];
	if ( sz > max_sz ) max_sz = sz;
    }
    c_nb = ( col_nb <= colsz_sz ) ? col_nb : colsz_sz;
    memset (colsz_arr, 0, colsz_sz * sizeof (colsz_arr[0]));
    for ( i = c_nb -1 ; i >= 0 ; i-- ) colsz_arr [i] = max_sz;

    lnb = col_struct (item_nb, c_nb, item_idx,
			  (sizeof (item_idx) / sizeof (item_idx[0])), NULL);
    if (lnb < 0 ) return -1;

    t_sz = max_sz = 0;
    for ( i = j = 0, i1 = item_idx [j +1] ; ; i++ ) {
	if ( i >= i1 ) {
	    /* next column */
	    colsz_arr [j++] = t_sz;
	    max_sz += t_sz;
	    i1 = item_idx [j+1];
	    t_sz = 0;
	}
	if ( i >= item_nb ) break;
	sz = item_sz [i];
	if ( sz > t_sz ) t_sz = sz;
    }
    return max_sz;
}

/* Find the best number of columns,
 *   Return the numer of columns and the size of each one
 */


static int check_columns_size ( S_looktbl *(*table)[], int table_sz,
    int item_sep, int item_data,    /* size of separator and data field */
    int *colsz_arr, int colsz_sz)   /* size of columns, size of the array */
{
    int i, sz, max_sz, col_nb, line_nb, width;
    int item_nb;
    int item_sz [256];
    char *str;

    max_sz = 0;
    item_nb = table_sz;
    for ( i = item_nb -1 ; i >= 0 ; i-- ) {
	if ( i > (sizeof (item_sz) / sizeof (item_sz[0])) ) break;
	sz = strlen ((*table)[i]->str);
	item_sz [i] = sz;
	if ( sz > max_sz ) max_sz = sz;
    }
    width  = term.tt_width;
    col_nb = ((width + item_sep) -1) / (max_sz + item_data + item_sep);
    if ( col_nb >= colsz_sz ) col_nb = colsz_sz -1;
    line_nb = ((item_nb -1) / col_nb) +1;
    if ( line_nb < 5 ) {
	/* too few lines */
	line_nb = 5;
	col_nb  = ((item_nb -1) / line_nb) +1;
	line_nb = ((item_nb -1) / col_nb) +1;
    } else for ( col_nb++ ;  ; col_nb++ ) {
	/* try more columns */
	line_nb = ((item_nb -1) / col_nb) +1;
	sz = columns_size (item_sz, item_nb, col_nb, colsz_arr, colsz_sz);
	sz += ((item_sep + item_data) * col_nb) - item_sep;
	if ( (line_nb < 5) || (sz >= width) || (col_nb > colsz_sz) ) {
	    col_nb--;
	    break;
	}
    }
    sz = columns_size (item_sz, item_nb, col_nb, colsz_arr, colsz_sz);
    return col_nb;
}

void print_sort_table (S_lookstruct *tblstruct)
{
    static int print_alias_table (S_lookstruct *);
    char *cmt1, *cmt2;
    int width, height;
    int i, sz;
    int item_nb, item_sep, item_data, item_sz, col_nb, line_nb, max_sz;
    int colsz_arr [MAX_PRT_COL];

    switch (tblstruct->type) {
	case 1 :    /* commands */
	    cmt1 = "Editor commands (without aliases)";
	    cmt2 = "Help on a specific command with \"help cmd <command name>\"";
	    break;

	case 2 :    /* key functions */
	    cmt1 = "Keyboard functions";
	    cmt2 = "\
Help on a given keyboard function with \"help keyf <function name>\"\n\
Help on keyboard keys assigned functions can be displayed with \"help\"\n\
Keys assigned to a given function can be displayed with\n\
    \"help key <key function name>\" or \"help ? <key function name>\"\n\
";
	    break;

	default :
	    cmt1 = cmt2 = NULL;
    }

    /* The separator (space char(s) after the string) can be outside
     * the window size. The line must include a 'new line' before the
     * within the terminal window size */
    memset (colsz_arr, 0, sizeof (colsz_arr));
    max_sz = 0;
    item_nb = tblstruct->sorted_table_sz;
    width  = term.tt_width;
    height = term.tt_height;
    item_sep = 1;
    item_data = 4;
    col_nb = check_columns_size (tblstruct->sorted_table, tblstruct->sorted_table_sz,
				 item_sep, item_data, colsz_arr,
				 sizeof(colsz_arr) / sizeof(colsz_arr[0]));
    line_nb = ((item_nb -1) / col_nb) +1;
    if ( cmt1 ) {
	printf ("%d %s\n", tblstruct->sorted_table_sz, cmt1);
	line_nb++;
    }
    print_sorted (tblstruct->sorted_table, sort_table_print,
		  tblstruct->sorted_table_sz, col_nb, item_data, item_sep,
		  colsz_arr);

    if ( tblstruct->type == 1 )
	line_nb += print_alias_table (tblstruct);

    if ( cmt2 ) {
	if ( line_nb < (height - 2) ) putchar ('\n');
	puts (cmt2);
	if ( line_nb < (height - 3) ) putchar ('\n');
    }
}

static int print_alias_table (S_lookstruct *tblstruct)
{
    int i, nb, lnb, sz, sz1, width;
    char *all_aliases, str[128];
    S_looktbl *(*sorted_table)[];

    if ( !tblstruct->as_alias || !tblstruct->alias_table_sz )
	return 0;

    width = term.tt_width;
    sorted_table = tblstruct->sorted_table;
    nb = tblstruct->sorted_table_sz;
    printf ("\n%d Editor commands aliases\n", tblstruct->alias_table_sz);
    lnb = 2;
    for ( i = sz = 0 ; i < nb ; i++ ) {
	all_aliases = get_all_aliases (tblstruct, (*sorted_table)[i]->val, NO);
	if ( ! all_aliases ) continue;
	sprintf (str, "%s: %s", (*sorted_table)[i]->str, all_aliases);
	sz1 = strlen (str);
	if ( (sz + sz1 +4) >= width ) {
	    putchar ('\n');
	    sz = 0;
	    lnb++;
	}
	sz += printf ("%s%s", sz ? ";  " : "", str);
    }
    putchar ('\n');
    return (lnb +1);
}
/* ======================================================================== */
