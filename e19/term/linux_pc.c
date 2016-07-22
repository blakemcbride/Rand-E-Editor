/* ==========================================================================
 *
 *   keyboard_map.c : keyboard mapping package :
 *      Assess and test the keyboard mapping for IBM PC 101 keys
 *      keyboard style.
 *      v1 : linux version only
 *   by : Fabien Perriollat : Fabien.Perriollat@cern.ch
 *   version 1.0 Nov 1998
 *   last update 23 Jan 2000 : Fabien.Perriollat@cern.ch
 *                8 Mar 2000 : Fabien.Perriollat@cern.ch
 *                             provide capability for AIX Unix system
 *               11 Jan 2001 : Fabien.Perriollat@cern.ch
 *                             can be compiled without X11 package (ref NOX11)
 *                             can be compiled without X11 KeyBoard extension (ref NOXKB)
 *                             handling of -noX11 option and #!noX11 directive
 * --------------------------------------------------------------------------
 *  Exported routines :
 *      Flag get_keyboard_map (char *terminal, int strg_nb, char *strg1, ...
 *      char * get_keyboard_mode_strg ()
 *      void all_string_to_key (char *escp_seq, ...
 *      void check_keyboard ()
 *      void display_keymap (Flag verbose)
 *
 * --------------------------------------------------------------------------
 *  To be done :
 *      Add handling for X server other than XFree (which is the default for
 *          Linux),
 *      Add keysym to escape seq for other X Terminal emulator than
 *          xterm, nxterm and gnome-terminal.
 * ========================================================================
 */
static const char version[] = "version 2.1 Mar 2000 by Fabien.Perriollat@cern.ch";

/* #define TEST_PROGRAM
 *  Build an autonomous program to test of this code
 *      (available only on PC Linux system)
 */
/* ---------------------------------------------------------------------- */

#include <stdlib.h>

#ifdef TEST_PROGRAM
#define CCUNAS1 0202 /* defined in e.h */
typedef char Flag;
#define NO 0
#define YES 1
#else  /* -TEST_PROGRAM */
#include <signal.h>
#include "e.h"
#include "e.cm.h"
#include "e.it.h"
#include "e.tt.h"
#endif /* TEST_PROGRAM */

#if 0
#include <stdlib.h>
#endif
#include <termios.h>
#include <termio.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/time.h>
#include <setjmp.h>

#ifdef __linux__
#include <linux/kd.h>
#include <linux/keyboard.h>
#endif /* __linux__ */

#ifdef NOX11
#define NOXKB
#include <my_keysymdef.h>
#ifndef NoSymbol
#define NoSymbol 0L
#ifndef KeySym
#define KeySym int
#endif
#endif

#else /* - NOX11 */
#include <X11/Xlib.h>
#include <X11/Intrinsic.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>
#ifndef NOXKB
#include <X11/XKBlib.h>
#include <X11/extensions/XKBfile.h>
#include <X11/extensions/XKBgeom.h>
#else
#define XkbKeyNameLength 128
#endif /* - NOXKB */
#endif /* - NOX11 */

#ifndef K_HOLE
#define K_HOLE -1
#endif


extern Flag verbose_helpflg;                /* from e.c */
extern Flag noX11flg;

extern char * get_kbmap_file_name (Flag create_dir, Flag *exist);

extern S_term term;
extern S_looktbl itsyms[];
extern struct itable *ithead;

extern char *command_msg;

char * get_keyboard_mode_strg ();
static void terminal_type (char *);
char * escseqstrg (char *, Flag);

static jmp_buf jmp_env;


/* structure to define symbolic name to non printing char */
struct _ch_symb {
	char ch;
	char * symb;
	char * symb1;
    } ch_symb [] = {
	{ '\t'  , "<Tab>" ,   NULL   },
	{ '\177', "<Del>" , "(\\177)"},
	{ '\b'  , "<Bksp>", "(^H)"   },
	{ '\033', "<Esc>" ,   NULL   }
    };
#define ch_symb_nb (sizeof (ch_symb) / sizeof (ch_symb[0]))

/* Structure and data used for linux console and xterm familly */
/* ----------------------------------------------------------- */

/* --------------------------------------------------------------------
 *  It is assumed that the modifier keys use the standard mapping :
 *      man keytables(5) :
 *          (By default Shift, AltGr, Control
 *          and Alt are bound to the keys that bear a similar  label;
 *          AltGr may denote the right Alt key.)
 *          Note that you should be very careful when binding the mod-
 *          ifier keys, otherwise you can end up with an unusable key-
 *          board  mapping.
 */

static Flag verbose = NO;       /* verbose printout */

/* keyboard mapping related data */

typedef enum { no_map, user_mapfile, linux_console,
	       X11_default, X11_KBExt, X11_xmodmap } Kb_Mapping;
static const char *kb_mapping_strg [] =
    { "no_map", "user_mapfile", "linux_console",
      "X11_default", "X11_KBExt", "X11_xmodmap" };

static Flag get_map_flg = NO;           /* package init done flag */
static Kb_Mapping kbmap_type = no_map;  /* keyboard mapping type */
static char *get_kbmap_strg = NULL;     /* how the keyboard map was got */
static char *kbmap_fname = NULL;        /* user defined keyboard map file */

/* Loock only in plain, Shift, Control and Alt map */
/*   Ref : man page for keytables(5) for the value of the modifier key */


#ifndef K_NORMTAB
#define K_NORMTAB 0x01
#define K_SHIFTTAB 0x04
#endif


static int keyboard_map [] = {
	K_NORMTAB,      /* plain key map : (0) */
	K_SHIFTTAB,     /* Shift modified map : (0x01) */
	0x04,           /* Control modified map (0x04) */
	-1              /* Alt modified map (0x08) */
	};
#define nb_kbmap (sizeof (keyboard_map) / sizeof (keyboard_map[0]))
#define shifted 1   /* map index of shift modifier */

static char *key_shift_msg [nb_kbmap] = { "", "Shift ", "Ctrl ", "Alt " };


typedef struct _Key_Assign {
    short lkcode;           /* linux keycode (as displayed by "showkey -k" XLATE mode) */
    unsigned short Xkeysym; /* X11 keycode (use xev or showkey progs to display value) */
    char *klabel;           /* label for listing and display */
    int kcode;              /* keycode linux console or X11 */
    int ktfunc [nb_kbmap];  /* key function (retuned by linux key mapping) */
    } Key_Assign;



/* pc 101 style keyboard description */
/* ================================= */

struct _keyboard_struct {
    int idx;    /* index in keyboard_desc of 1st key of the class */
    int nb;     /* nb of key in this class */
    char *label;    /* class label */
    }
    pc101_keyboard_struct [] = {
	{ 0, 12, "Function keys" },
	{ 0,  4, "Cursor key" },
	{ 0,  6, "Edition key pad" },
	{ 0, 17, "Numeric key Pad" },
	{ 0,  2, "Miscellaneous" }
    };
#define pc101_keyboard_struct_nb (sizeof (pc101_keyboard_struct) / sizeof (pc101_keyboard_struct[0]))


/* PC-101 style keyboard assignement description array */
/* --------------------------------------------------- */

static Key_Assign pc_keyboard_desc [] = {
			/* lkcode  Xkeysym label */         /* keycode for */
    /* === Function Keys === */                             /* XFree server*/
    /*  F1          */      {  59, XK_F1 , "F1"  },             /*   67 */
    /*  F2          */      {  60, XK_F2 , "F2"  },             /*   68 */
    /*  F3          */      {  61, XK_F3 , "F3"  },             /*   69 */
    /*  F4          */      {  62, XK_F4 , "F4"  },             /*   70 */
    /*  F5          */      {  63, XK_F5 , "F5"  },             /*   71 */
    /*  F6          */      {  64, XK_F6 , "F6"  },             /*   72 */
    /*  F7          */      {  65, XK_F7 , "F7"  },             /*   73 */
    /*  F8          */      {  66, XK_F8 , "F8"  },             /*   74 */
    /*  F9          */      {  67, XK_F9 , "F9"  },             /*   75 */
    /*  F10         */      {  68, XK_F10, "F10" },             /*   76 */
    /*  F11         */      {  87, XK_F11, "F11" },             /*   95 */
    /*  F12         */      {  88, XK_F12, "F12" },             /*   96 */

    /* === Cursor Key === */
    /*  UP ARROW    */      { 103, XK_Up   , "UP"    },         /*   98 */
    /*  LEFT ARROW  */      { 105, XK_Left , "LEFT"  },         /*  100 */
    /*  DOWN ARROW  */      { 108, XK_Down , "DOWN"  },         /*  104 */
    /*  RIGHT ARROW */      { 106, XK_Right, "RIGHT" },         /*  102 */

    /* === Extended Key Pad === */
    /*  INSERT      */      { 110, XK_Insert , "INSERT"  },     /*  106 */
    /*  HOME        */      { 102, XK_Home   , "HOME"    },     /*   97 */
    /*  PAGE UP     */      { 104, XK_Page_Up, "PAGE-UP" },     /*   99 */

    /*  DELETE      */      { 111, XK_Delete   , "DELETE"   },  /*  107 */
    /*  END         */      { 107, XK_End      , "END"      },  /*  103 */
    /*  PAGE DOWN   */      { 109, XK_Page_Down, "PAGE-DOWN"},  /*  105 */

    /* === Numeric Key Pad === */
    /*  KP 0        */      {  82, XK_KP_0 , "KP-0" },          /*   90 */
    /*  KP 1        */      {  79, XK_KP_1 , "KP-1" },          /*   87 */
    /*  KP 2        */      {  80, XK_KP_2 , "KP-2" },          /*   88 */
    /*  KP 3        */      {  81, XK_KP_3 , "KP-3" },          /*   89 */
    /*  KP 4        */      {  75, XK_KP_4 , "KP-4" },          /*   83 */
    /*  KP 5        */      {  76, XK_KP_5 , "KP-5" },          /*   84 */
    /*  KP 6        */      {  77, XK_KP_6 , "KP-6" },          /*   85 */
    /*  KP 7        */      {  71, XK_KP_7 , "KP-7" },          /*   79 */
    /*  KP 8        */      {  72, XK_KP_8 , "KP-8" },          /*   80 */
    /*  KP 9        */      {  73, XK_KP_9 , "KP-9" },          /*   81 */

    /*  KP +        */      {  78, XK_KP_Add    ,  "KP-PLUS" }, /*   86 */
    /*  KP ENTER    */      {  96, XK_KP_Enter  , "KP-ENTER" }, /*  108 */
    /*  KP .        */      {  83, XK_KP_Decimal,   "KP-DOT" }, /*   91 */

    /*  KP NUM LOCK */      {  69, XK_Num_Lock   , "KP-NLOCK" }, /*   77 */
    /*  KP /        */      {  98, XK_KP_Divide  , "KP-DIV"   }, /*  112 */
    /*  KP *        */      {  55, XK_KP_Multiply, "KP-MUL"   }, /*   63 */
    /*  KP -        */      {  74, XK_KP_Subtract, "KP-MINUS" }, /*   82 */

    /* === Miscellaneous keys */
    /*  BACK SPACE  */      {  14, XK_BackSpace, "BKSP" },      /*    22 */
    /*  TAB         */      {  15, XK_Tab      , "TAB"  }       /*    23 */

    };

#define pc_keyboard_desc_nb (sizeof (pc_keyboard_desc) / sizeof (pc_keyboard_desc[0]))
static const int nb_key = pc_keyboard_desc_nb;

#undef COMMENTS
#ifdef COMMENTS
/* -------------------------------------------------------------
    Usefull reading : "The Linux Keyboard and Console HOWTO"
	in Linux doc. (file : /usr/doc/HOWTO/Keyboard-and-Console-HOWTO)
	man pages : console_codes(5), keytables(5)

    ----- Extract from this "Keyboard and Console HOWTO" :

	3.  Keyboard generalities

	You press a key, and the keyboard controller sends scancodes to the
	kernel keyboard driver. Some keyboards can be programmed, but usually
	the scancodes corresponding to your keys are fixed.  The kernel
	keyboard driver just transmits whatever it receives to the application
	program when it is in scancode mode, like when X is running.
	Otherwise, it parses the stream of scancodes into keycodes,
	corresponding to key press or key release events.  (A single key press
	can generate up to 6 scancodes.)  These keycodes are transmitted to
	the application program when it is in keycode mode (as used, for
	example, by showkey).  Otherwise, these keycodes are looked up in the
	keymap, and the character or string found there is transmitted to the
	application, or the action described there is performed.  (For
	example, if one presses and releases the a key, then the keyboard
	produces scancodes 0x1e and 0x9e, this is converted to keycodes 30 and
	158, and then transmitted as 0141, the ASCII or latin-1 code for `a';
	if one presses and releases Delete, then the keyboard produces
	scancodes 0xe0 0x53 0xe0 0xd3, these are converted to keycodes 111 and
	239, and then transmitted as the 4-symbol sequence ESC [ 3 ~, all
	assuming a US keyboard and a default keymap. An example of a key
	combination to which an action is assigned is Ctrl-Alt-Del.)

	The translation between unusual scancodes and keycodes can be set
	using the utility setkeycodes - only very few people will need it.
	The translation between keycodes and characters or strings or actions,
	that is, the keymap, is set using the utilities loadkeys and
	setmetamode.  For details, see getkeycodes(8), setkeycodes(8),
	dumpkeys(1), loadkeys(1), setmetamode(1). The format of the files
	output by dumpkeys and read by loadkeys is described in keytables(5).

	Where it says `transmitted to the application' in the above
	description, this really means `transmitted to the terminal driver'.
	That is, further processing is just like that of text that comes in
	over a serial line.  The details of this processing are set by the
	program stty.

    -----

    Linux Keyboard Driver analysis :
    --------------------------------

    WARNING : scan code translation (modified by setkeycodes)
	      and keymap (modified by loadkeys or setmetamode)
	      can change the behaviour of the keyboard.

	In the following analysis it is assumed that the standard
	translation and mapping is not modified.
	The best way to add functionality to the keyboard is to
	define a mapping into a function (or a user string) for
	un-mapped keycode. Changing scan code translation is
	not recommanded.

    Linux keyboard driver (/usr/src/linux/drivers/char/keyboard.c)
    build in string generation for keypad and cursor keys.

	The value (and order) of Kxxx functions is defined by the
	include file : /usr/include/linux/keyboard.h

	To know the binding between a function defined in the dumpkeys
	listing and a actual result by the keyboard handler, you must
	found the binding between the loadkeys / dumpkeys function name
	and Kxxx symbol of keyboard.h include file.
	This can be done by :
	    1 - loock into the output of "dumpkeys --long-info" to
		found the function value of a given function name
		using synonyms if needed.
	    2 - found in /usr/include/linux/keyboard.h to found
		the Kxxx symbol for the function value.

    The string generated by a Function key, and Editing key is defined
    by the "char func_buf[]" array (function value in the range 0x01xx),
    the index in func_buf[] array is the value xx.
    For example the key which is mapped to "Remove" (0x0116), like normaly
    the Delete key in the Editing key pad, the string generated is
    func_buf[(0x0166 & 0x00ff)] which is : '\033', '[', '3', '~', '\0'
    See "KT_FN descriptor" arrays later in this file.
    The default map arrays can be produced by
    "loadkeys --mktable defkeymap.map"
    The map used to build the kernel is defined in
    /usr/src/linux-2.2.12/drivers/char/defkeymap.c

    The string generated for the numerical key pad and the cursor keys
    is build in the keyboard driver and cannot be redefined by "loadkeys"
    It is only possible to assigne the key to another function.

    The build in string depend upon the keyboard mode :
	Cursor mode (normal or alternate)
    and Key Pad mode (numeric or application).

    General terminal reset : "\033c" (numeric mode and normal cursor mode)
    Alternate cursor mode is set by :             "\033[?1h" (DECCKM)
	reset to the default (normal cursor) by : "\033[?1l"
    Appliction mode is set by :           "\033="  (DECPAM)
	reset to default (Numerical) by : "\033>"  (DECPNM)
	ref : man console_codes(4)

    In alternate cursor mode the string generated are :
	<Esc>Ox in stead of <Esc>[x (where x is a char).


    ---- analysis of linux keyboard driver (/usr/src/linux/drivers/char/keyboard.c)
    The string generated by a given Numerical Key Pad key is build
    by the routine "do_pad" according to the arrays :
	pad_chars (Num Lock mode)
	app_map (application mode)
    the index in this arrays is the low byte of the K_Pxxx key function
    code (defined in /usr/include/linux/keyboard.h)

	static const char *pad_chars = "0123456789+-*/\015,.?";
	static const char *app_map   = "pqrstuvwxylSRQMnn?";

    The Numerical Key Pad (K_P0 .... K_PPLUSMINUS) can produce :
	numbers (corresponding to the Major" key label) according to "pad_chars"
	application strings, according to "app_map"
	remapped into cursor and service (corresponding to alternate key label)

    Build in logic with depend upon mode (applic or cursor) and Num Lock state is :
	if (applic)  if (no Shift) applic string
		     else if (Num Lock) number (= major key label)
			  else remaped into cursor (with application string)
						   (= alternate key label)
	else (= cursor mode)
	    if (Num Lock) number (= major key label)
	    else remaped into cursor (with cursor string)
				     (= alternate key label)

    The remapping is defined by :
	K_P0     -> K_INSERT
	K_P1     -> K_SELECT
	K_P2     -> K_DOWN
	K_P3     -> K_PGDN
	K_P4     -> K_LEFT
	K_P5     -> <Esc>[G (numerical mode), <Esc>OG (application)
	K_P6     -> K_RIGHT
	K_P7     -> K_FIND
	K_P8     -> K_UP
	K_P9     -> K_PGUP
	K_PCOMMA -> K_REMOVE
	K_PDOT   -> K_REMOVE

	K_PPLUS, K_PMINUS, K_PSTAR, K_PSLASH, K_PENTER, K_PPLUSMINUS
	are mapped into the numerical value (according to pad_chars array)

    Special case of the Num Lock key (upper left key of the Numerical Key Pad)
	Function value : K_NUM
	By default is is handled by the "num" routine :
	    which builds <esc>OP in application mode
	    or which flips the Num Lock state (and keyboard led), "bare_num" routine.
	If the key is bind to K_BARENUMLOCK it will always
	    flips the Num Lock state (and keyboard led), "bare_num" routine.


    The case of the Cursor Keys is handle by "do_cur" routine.
    The string is buils according to the "cur_chars" array
	and cursor / applic mode.
	static const char *cur_chars = "BDCA";
	(order : K_DOWN, K_LEFT, K_RIGHT, K_UP)

  -------------------------------------------------------------- */
#endif



/* structure to describe the escape sequences */

struct KTdesc {
    int ktfunc;             /* key function (kt or keysym, retuned by linux key mapping) */
    char *tcap;             /* termcap / terminfo ident or label for mapping */
    char *strg;             /* string currently generated by this key function */
    char *norstrg;          /* string for normal (or numeric) mode, NULL : not sensitive */
    char *altstrg;          /* string for alternate (or applic) mode, NULL : not sensitive */
    };

struct _all_ktdesc {
    int kt_type;            /* KT type for linux / console, not used for xterm */
    char *type_name;        /* label for the group */
    struct KTdesc **ktdesc_array;   /* pointer to the descriptor array */
    int *ktdesc_nb;         /* number of elements in the array */
    };

/* Terminal related global variables */
/* --------------------------------- */

static char *terminal_name = NULL;  /* pointer to TERM variable */
static Flag Xterminal_flg = NO;     /* X terminal family */
static Flag Xterm_flg = YES;        /* special xterm processing : to be set by option para */

static Flag keypad_appl_mode = NO; /* application (Yes) or numeric (NO) */
static Flag cursor_alt_mode  = NO; /* alternate (Yes) or normal (NO) */

static Flag assess_cursor_flg, assess_keypad_flg;   /* terminal mode to be assessed */
static int kt_void;             /* not in use entry in kt description array */

static struct _all_ktdesc *inuse_ktdesc = NULL; /* all_ktdesc or all_Xktdesc */
static int inuse_ktdesc_nb = 0; /* nb of elements in inuse_ktdesc [] */

static char *init_msg = NULL;   /* string used to initialize the terminal */

/* -------------------------------------------------------------- */

/* Terminal control setting escape sequence (for initialisation) */
    /* only a copy of this structure must be used for re-ordering */
static struct Init_Sequence {
    int type;
    int ini;    /* index of the string or -1 : used dynamicaly */
    char *ref;
    char *info;
    } init_seq [] = {
	{ 0, -1, "\033c"   , "Terminal and Keyboard reset" },
	{ 1, -1, "\033>"   , "Numeric Key Pad mode       " },   /* DECPNM */
	{ 2, -1, "\033="   , "Application Key Pad mode   " },   /* DECPAM */
	{ 3, -1, "\033[?1l", "Normal Cursor mode         " },   /* DECCKM */
	{ 4, -1, "\033[?1h", "Alternate Cursor mode      " }    /* DECCKM */
    };
#define init_seq_nb (sizeof (init_seq) / sizeof (init_seq[0]))
#define key_pad_num 1
#define cursor_norm 3

/* Description of escape squences generated by keyboard */
/* ---------------------------------------------------- */
/* ref to terminfo man page for the termcap / terminfo ident string */

/* Descriptor to be used for mapping defined by user map file */

static int any_mydesc_nb = 0;
static struct _all_ktdesc all_myktdesc [] = {
    { 0,  "Any key", NULL, &any_mydesc_nb },
};
#define all_myktdesc_nb (sizeof (all_myktdesc) / sizeof (all_myktdesc[0]))

/* storage for user defined keyboard mapping */
#define ESCSEQ_SZ 2048  /* must be large enough for the keyboard description */
/* normal storage */
static char nor_escseq [ESCSEQ_SZ];  /* escape sequence strings */
static int  nor_escseq_idx = 0; /* index of free space in myescseq */
static short nor_esc_idx [pc_keyboard_desc_nb][nb_kbmap];   /* index into nor_escseq [] */

/* alternate storage : to capture the keyboard mapping */
static char alt_escseq [ESCSEQ_SZ];  /* escape sequence strings */
static int  alt_escseq_idx = 0; /* index of free space */
static short alt_esc_idx [pc_keyboard_desc_nb][nb_kbmap];   /* index into alt_escseq [] */

/*  pointer to the current set */
static int escseq_sz = sizeof (nor_escseq); /* size of what is pointed by mykb_escseq */
static char *mykb_escseq = nor_escseq;
static int *mykb_escseq_idx = &nor_escseq_idx;
static short (* mykb_esc_idx) [pc_keyboard_desc_nb][nb_kbmap] = &nor_esc_idx;
/*
static short *mykb_esc_idx = nor_esc_idx;
*/

#if 0
/* for fast search */
static int mykb_escseq_nb = 0;
static char *mykb_escseq_pt [pc_keyboard_desc_nb * nb_kbmap];
#endif

/* ------------------------------------------------------------------------ */
#ifdef __linux__


/* The "linux" (or "console") terminal */
/* ----------------------------------- */

/* Cursor Key Pad */
/* build in string, sensitive to cursor / applic mode */
/*  warning : in terminfo the cursor value seem to be defined */

/* cursor normal mode */
static struct KTdesc kt_desc_cur_norm [] = {
	{ K_DOWN  , "kcud1", NULL, "\033[B", "\033OB" },
	{ K_LEFT  , "kcub1", NULL, "\033[D", "\033OD" },
	{ K_RIGHT , "kcuf1", NULL, "\033[C", "\033OC" },
	{ K_UP    , "kcuu1", NULL, "\033[A", "\033OA" },
    };
#define KT_CUR_nb (sizeof (kt_desc_cur_norm) / sizeof (struct KTdesc))

/* cursor alternated mode */
static struct KTdesc kt_desc_cur_alt [] = {
	{ K_DOWN  , "kcud1", "\033OB" },
	{ K_LEFT  , "kcub1", "\033OD" },
	{ K_RIGHT , "kcuf1", "\033OC" },
	{ K_UP    , "kcuu1", "\033OA" },
    };

/* Numeric Key Pad */
/* build in string, sensitive to cursor / applic mode */
/*  warning : in terminfo the numeric key pad seems to be not defined */

static struct KTdesc kt_desc_pad [] = {
	{ K_P0      ,NULL,  "\033Op" },
	{ K_P1      ,NULL,  "\033Oq" },
	{ K_P2      ,NULL,  "\033Or" },
	{ K_P3      ,NULL,  "\033Os" },
	{ K_P4      ,NULL,  "\033Ot" },
	{ K_P5      ,NULL,  "\033Ou" },
	{ K_P6      ,NULL,  "\033Ov" },
	{ K_P7      ,NULL,  "\033Ow" },
	{ K_P8      ,NULL,  "\033Ox" },
	{ K_P9      ,NULL,  "\033Oy" },
	{ K_PPLUS   ,NULL,  "\033Ol" }, /* key-pad plus */
	{ K_PMINUS  ,NULL,  "\033OS" }, /* key-pad minus */
	{ K_PSTAR   ,NULL,  "\033OR" }, /* key-pad asterisk (star) */
	{ K_PSLASH  ,NULL,  "\033OQ" }, /* key-pad slash */
	{ K_PENTER  ,NULL,  "\033OM" }, /* key-pad enter */
	{ K_PCOMMA  ,NULL,  "\033On" }, /* key-pad comma: kludge... */
	{ K_PDOT    ,NULL,  "\033On" }, /* key-pad dot (period): kludge... */
	{ K_PPLUSMINUS,NULL,"\033O?" }, /* key-pad plus/minus */
    };
#define KT_PAD_nb (sizeof (kt_desc_pad) / sizeof (struct KTdesc))

/* translation table for Key Pad */

/* special case for K_P5 */
static char kp5_trans_num_strg[]  = "\033[G";   /* numeric mode */
static char kp5_trans_appl_strg[] = "\033OG";   /* application mode */
static char kp5_trans_appl1_strg[]= "\033Ou";   /* application mode */

static short kp_translate [] = {
	K_P0     , K_INSERT ,
	K_P1     , K_SELECT ,
	K_P2     , K_DOWN   ,
	K_P3     , K_PGDN   ,
	K_P4     , K_LEFT   ,
	K_P6     , K_RIGHT  ,
	K_P7     , K_FIND   ,
	K_P8     , K_UP     ,
	K_P9     , K_PGUP   ,
	K_PPLUS  , '+'      ,
	K_PMINUS , '-'      ,
	K_PSTAR  , '*'      ,
	K_PSLASH , '/'      ,
	K_PENTER , '\r'     ,
	K_PCOMMA , K_REMOVE ,
	K_PDOT   , K_REMOVE ,
	K_PPLUSMINUS, '?'
    };
#define kp_translate_nb (sizeof (kp_translate) / sizeof (kp_translate[0]))

/* Special case keys */

static struct KTdesc kt_desc_spec [] = {
	/* special case keys */
	{ K_NUM     ,NULL,  "\033OP" }, /* key-pad Num Lock */
    };
#define KT_SPEC_nb (sizeof (kt_desc_spec) / sizeof (struct KTdesc))

	/* end of linux build in string generation zone */

/* KT_FN descriptor */

static struct KTdesc kt_desc_fn [] = {
	/* key pad service keys */
	{ K_FIND   , "khome", "\033[1~" }, /* home key label */
	{ K_INSERT , "kich1", "\033[2~" },
	{ K_REMOVE , "kdch1", "\033[3~" },
	{ K_SELECT , "kend" , "\033[4~" }, /* end key label */
	{ K_PGUP   , "kpp"  , "\033[5~" }, /* PGUP is a synonym for PRIOR */
	{ K_PGDN   , "knp"  , "\033[6~" }, /* PGDN is a synonym for NEXT */
	{ K_MACRO  ,  NULL  , "\033[M~" },
	{ K_HELP   ,  NULL  ,  NULL     },
	{ K_DO     ,  NULL  ,  NULL     },
	{ K_PAUSE  ,  NULL  , "\033[P~" },

	/* function keys */
	{ K_F1,  "kf1",  "\033[[A"  },
	{ K_F2,  "kf2",  "\033[[B"  },
	{ K_F3,  "kf3",  "\033[[C"  },
	{ K_F4,  "kf4",  "\033[[D"  },
	{ K_F5,  "kf5",  "\033[[E"  },
	{ K_F6,  "kf6",  "\033[17~" },
	{ K_F7,  "kf7",  "\033[18~" },
	{ K_F8,  "kf8",  "\033[19~" },
	{ K_F9,  "kf9",  "\033[20~" },
	{ K_F10, "kf10", "\033[21~" },

	{ K_F11, "kf11", "\033[23~" },
	{ K_F12, "kf12", "\033[24~" },
	{ K_F13, "kf13", "\033[25~" },
	{ K_F14, "kf14", "\033[26~" },
	{ K_F15, "kf15", "\033[28~" },
	{ K_F16, "kf16", "\033[29~" },
	{ K_F17, "kf17", "\033[31~" },
	{ K_F18, "kf18", "\033[32~" },
	{ K_F19, "kf19", "\033[33~" },
	{ K_F20, "kf20", "\033[34~" },

	{ K_F21, "kf21",  NULL },
	{ K_F22, "kf22",  NULL },
	{ K_F23, "kf23",  NULL },
	{ K_F24, "kf24",  NULL },
	{ K_F25, "kf25",  NULL },
	{ K_F26, "kf26",  NULL },
	{ K_F27, "kf27",  NULL },
	{ K_F28, "kf28",  NULL },
	{ K_F29, "kf29",  NULL },
	{ K_F30, "kf30",  NULL },

	/* empty slot for 10 additional function keys */
	{ 0    ,  NULL ,  NULL },
	{ 0    ,  NULL ,  NULL },
	{ 0    ,  NULL ,  NULL },
	{ 0    ,  NULL ,  NULL },
	{ 0    ,  NULL ,  NULL },
	{ 0    ,  NULL ,  NULL },
	{ 0    ,  NULL ,  NULL },
	{ 0    ,  NULL ,  NULL },
	{ 0    ,  NULL ,  NULL },
	{ 0    ,  NULL ,  NULL },
    };
#define KT_FN_nb (sizeof (kt_desc_fn) / sizeof (struct KTdesc))


/* KT_LATIN descriptor (latin characters) */

static struct KTdesc kt_desc_latin [] = {
	{ '\t'  ,  NULL ,  "\t"   },  /* horizontal tab */
	{ '\b'  ,  NULL ,  "\b"   },  /* <Ctrl H> back space */
	{ '\177',  NULL ,  "\177" },  /* back space */
    };
#define KT_LATIN_nb (sizeof (kt_desc_latin) / sizeof (struct KTdesc))


static struct KTdesc *ktcur_desc_pt = kt_desc_cur_norm;
static int ktcur_desc_nb = KT_CUR_nb;

static struct KTdesc *ktpad_desc_pt = kt_desc_pad;
static int ktpad_desc_nb = KT_PAD_nb;

static struct KTdesc *ktspec_desc_pt = kt_desc_spec;
static int ktspec_desc_nb = KT_SPEC_nb;

static struct KTdesc *ktfn_desc_pt = kt_desc_fn;
static int ktfn_desc_nb = KT_FN_nb;

static struct KTdesc *ktlatin_desc_pt = kt_desc_latin;
static int ktlatin_desc_nb = KT_LATIN_nb;

static struct _all_ktdesc all_ktdesc [] = {
    { KT_CUR,  "Cursor key",          &ktcur_desc_pt,   &ktcur_desc_nb  },
    { KT_PAD,  "Num key pad",         &ktpad_desc_pt,   &ktpad_desc_nb  },
    { KT_SPEC, "Special key",         &ktspec_desc_pt,  &ktspec_desc_nb },
    { KT_FN,   "Function key",        &ktfn_desc_pt,    &ktfn_desc_nb   },
    { KT_LATIN,"Latin character key", &ktlatin_desc_pt, &ktlatin_desc_nb}
};
#define all_ktdesc_nb (sizeof (all_ktdesc) / sizeof (all_ktdesc[0]))

#else  /* - __linux__ */
#define all_ktdesc all_myktdesc
#define all_ktdesc_nb all_myktdesc_nb
#endif /* __linux__ */

/* ------------------------------------------------------------------------ */

/* xterm terminal family */
/* --------------------- */

/* It is assumed thet the VT100 emulation is in use, the VT52 mode is not
 *      supported and the 3270 is not handle.
 *
 *  In the old version of xterm (or in nxterm ...) the string generated
 *      for XK_Home, XK_KP_Home, XK_End, XK_KP_End, XK_Begin, XK_KP_Begin
 *      are not properly defined. In this case it is necessary to
 *      override the translation table for XTerm  with :
 *
 *
 *  Ref :
 *      "/usr/include/X11/keysymdef.h"  for symbol definition.
 *      XFree xterm source : "input.c" file for the way use by xtrem
 *          terminal emulator to generate the escape sequence.
*/

/*
------------------------------------------------------------------------------
It is assumed that this translation and keymapping is done on Linux X server
#-----------------------------------------------------------------------------
# Linux specific case :
# ---------------------
# The following translation override is assumed to be done for nxterm
#   application in "/usr/X11R6/lib/X11/app-defaults/NXTerm"

# ! The Home and End are not defined in XTerm by default
# !   Overriding the translation for the key in the numerical key pad
# !       KP_Home, KP_End, KP_Delete, KP_Begin
# !       (the right side key pad) disturbe the normal behaviour of these
# !       keys as defined by the shift modifier.
# !   The new translation must be applied only to shift and lock_shift
# !       case (see the ':' in the translation defintion).
#
# *VT100.Translations: #override \n\
#         <Key>Home:             string(0x1b) string("[H")\n\
#        :<Key>KP_Home:          string(0x1b) string("[H")\n\
#        :<Key>KP_End:           string(0x1b) string("[F")\n\
#         <Key>End:              string(0x1b) string("[F")\n\
#         <Key>Begin:            string(0x1b) string("[E")\n\
#        :<Key>KP_Begin:         string(0x1b) string("[E")\n\
#         <Key>Delete:           string(0x1b) string("[3~")

# Expected keyboard X mapping :
#
# ! /usr/local/X11R6/lib/X11/xinit/Xmodmap.sf :
# !   105 keys US keymaping. by Fabien.Perriollat@cern.ch Dec 98
# !   This is an good compromise setting fo the rand editor.
# !   A link to this file is expected to be defined in
# !       "/usr/X11R6/lib/X11/xinit/.Xmodmap"
# ! The keycode values are given for a XFree86 X11 server,
# !     they must be replaced by the actual values of the X server vendor.
#
# ! keycode  22 = BackSpace
# ! keycode  23 = Tab KP_Tab
#
# keycode  67 = F1  F11
# keycode  68 = F2  F12
# keycode  69 = F3  F13
# keycode  70 = F4  F14
# keycode  71 = F5  F15
# keycode  72 = F6  F16
# keycode  73 = F7  F17
# keycode  74 = F8  F18
# keycode  75 = F9  F19
# keycode  76 = F10 F20
#
# ! keycode  77 = Num_Lock F17 Num_Lock
# ! keycode  78 = Multi_key
#
# keycode  79 = KP_7 KP_Home KP_7 KP_Home
# keycode  80 = KP_8 Up KP_8 Up
# keycode  81 = KP_9 Prior KP_9 Prior
# keycode  82 = KP_Subtract
# keycode  83 = KP_4 Left KP_4 Left
# keycode  84 = KP_5 Begin KP_5 Begin
# keycode  85 = KP_6 Right KP_6 Right
# keycode  86 = KP_Separator KP_Add KP_Separator KP_Add
# keycode  87 = KP_1 KP_End KP_1 KP_End
# keycode  88 = KP_2 Down KP_2 Down
# keycode  89 = KP_3 Next KP_3 Next
# keycode  90 = KP_0 Insert KP_0 Insert
# keycode  91 = KP_Decimal KP_Delete KP_Decimal KP_Delete
#
# ! keycode  92 = 0x1007ff00
#
# ! keycode 107 = Delete

#-----------------------------------------------------------------------------
*/

/* X Terminal emulator parameters */
/* ------------------------------ */

typedef enum { unknow, xterm, gnome } Terminal_Class;

static const char unknow_dpy_name [] = "--Unknown--";
static const char undef_dpy_name [] = "--Undefined--";
static const char unknow_emul_name []  = "unknown";
static const char unknow_emul_class [] = "Unknown";

char *emul_name = NULL, *emul_class = NULL;

#ifndef NOX11
static Display *dpy = NULL;
static Window emul_window;
static XClassHint class_hints;
#ifndef NOXKB
static XkbDescRec * xkb_rec = NULL;
#endif
#endif

static KeySym *x_keyboard_map = NULL;
static char *dpy_name = NULL;
static char *dpy_msg = NULL;
static char *noX_msg = NULL;
static char *Xvendor_name = NULL;
static int min_keycode = 0, max_keycode = 0;
static int max_shift_level = 0;
static int keysym_per_keycode;
static Flag use_XKB_flg = NO;
static Terminal_Class x_terminal_class = unknow;
static Flag gnome_swap_flg = NO;
static Flag xterm_backarrow_flg = NO;
static char *xkb_ext_msg = NULL;    /* why XKB is not used */
static char *keycodes_str, *geometry_str, *symbols_str,
	    *types_str, *compat_str, *phys_symbols_str;

/* Keyboard structure : keyboard sections (for XFree sever)
 *   The "Alpha" section must be the last one.
static unsigned char *section_names [] = {
 */
static char *section_names [] = {
	"Function", "Editing", "Keypad", "Alpha"
    };
#define num_section_names (sizeof (section_names) / sizeof (section_names[0]))
static int section_idx [num_section_names];


/* Cursor control & motion */
/*      sensitive to cursor mode (DECCKM) : normal or alternate */
/*      The XK_KP_Home ... XK_KP_Begin are remapped on XK_Home ... XK_Begin */

static struct KTdesc Xkt_desc_cursor [] = {
					    /* normal      alternate */
    { XK_Insert   , "Insert"   , "\033[2~",   NULL     ,   NULL     },
    { XK_Delete   , "Delete"   , "\033[3~",   NULL     ,   NULL     },

    { XK_Home        , "Home"        ,  NULL     , "\033[H"  ,  "\033OH"  },
    { XK_Left        , "Left"        ,  NULL     , "\033[D"  ,  "\033OD"  },   /* Move left, left arrow */
    { XK_Up          , "Up"          ,  NULL     , "\033[A"  ,  "\033OA"  },   /* Move up, up arrow */
    { XK_Right       , "Right"       ,  NULL     , "\033[C"  ,  "\033OC"  },   /* Move right, right arrow */
    { XK_Down        , "Down"        ,  NULL     , "\033[B"  ,  "\033OB"  },   /* Move down, down arrow */
    { XK_Prior       , "Prior"       , "\033[5~" ,  NULL     ,   NULL     },   /* Prior, previous */
    { XK_Page_Up     , "Page_Up"     , "\033[5~" ,  NULL     ,   NULL     },
    { XK_Next        , "Next"        , "\033[6~" ,  NULL     ,   NULL     },   /* Next */
    { XK_Page_Down   , "Page_Down"   , "\033[6~" ,  NULL     ,   NULL     },
    { XK_End         , "End"         ,  NULL     , "\033[F"  ,  "\033OF"  },   /* EOL */
    { XK_Begin       , "Begin"       ,  NULL     , "\033[E"  ,  "\033OE"  },   /* BOL */

	/* XK_KP_Home ... XK_KP_Begin are mapped into XK_Home ... XK_Begin by xterm routines */
						 /* normal      alternate */
    { XK_KP_Home     , "KP_Home"     ,  NULL     , "\033[H"  ,  "\033OH"  },
    { XK_KP_Left     , "KP_Left"     ,  NULL     , "\033[D"  ,  "\033OD"  },   /* Move left, left arrow */
    { XK_KP_Up       , "KP_Up"       ,  NULL     , "\033[A"  ,  "\033OA"  },   /* Move up, up arrow */
    { XK_KP_Right    , "KP_Right"    ,  NULL     , "\033[C"  ,  "\033OC"  },   /* Move right, right arrow */
    { XK_KP_Down     , "KP_Down"     ,  NULL     , "\033[B"  ,  "\033OB"  },   /* Move down, down arrow */
    { XK_KP_Prior    , "KP_Prior"    , "\033[5~" ,  NULL     ,   NULL     },   /* Prior, previous */
    { XK_KP_Page_Up  , "KP_Page_Up"  , "\033[5~" ,  NULL     ,   NULL     },
    { XK_KP_Next     , "KP_Next"     , "\033[6~" ,  NULL     ,   NULL     },   /* Next */
    { XK_KP_Page_Down, "KP_Page_Down", "\033[6~" ,  NULL     ,   NULL     },
    { XK_KP_End      , "KP_End"      ,  NULL     , "\033[F"  ,  "\033OF"  },   /* EOL */
    { XK_KP_Begin    , "KP_Begin"    ,  NULL     , "\033[E"  ,  "\033OE"  },   /* BOL */
};
#define XKT_CUR_nb (sizeof (Xkt_desc_cursor) / sizeof (struct KTdesc))


/* Keypad Functions, keypad numbers cleverly chosen to map to ascii */
/*      sensitive to key pad mode (DECKPAM) : numeric or application */

static struct KTdesc Xkt_desc_pad [] = {
    { XK_KP_F1,     "KP_F1" ,    "\033OP" , NULL , NULL },  /* PF1, KP_A, ... */
    { XK_KP_F2,     "KP_F2" ,    "\033OQ" , NULL , NULL },
    { XK_KP_F3,     "KP_F3" ,    "\033OR" , NULL , NULL },
    { XK_KP_F4,     "KP_F4" ,    "\033OS" , NULL , NULL },

    { XK_KP_Insert, "KP_Insert", "\033[2~", NULL , NULL },
    { XK_KP_Delete, "KP_Delete", "\033[3~", NULL , NULL },

					   /* num    applic */
    { XK_KP_0        , "KP_0"        , NULL , "1" , "\033Op" },
    { XK_KP_1        , "KP_1"        , NULL , "2" , "\033Oq" },
    { XK_KP_2        , "KP_2"        , NULL , "3" , "\033Or" },
    { XK_KP_3        , "KP_3"        , NULL , "4" , "\033Os" },
    { XK_KP_4        , "KP_4"        , NULL , "5" , "\033Ot" },
    { XK_KP_5        , "KP_5"        , NULL , "6" , "\033Ou" },
    { XK_KP_6        , "KP_6"        , NULL , "7" , "\033Ov" },
    { XK_KP_7        , "KP_7"        , NULL , "8" , "\033Ow" },
    { XK_KP_8        , "KP_8"        , NULL , "9" , "\033Ox" },
    { XK_KP_9        , "KP_9"        , NULL , "0" , "\033Oy" },

    { XK_KP_Tab      , "KP_Tab"      , NULL , "\t", "\033OI" },
    { XK_KP_Enter    , "KP_Enter"    , NULL , "\r", "\033OM" },  /* enter */
    { XK_KP_Equal    , "KP_Equal"    , NULL , "=" , "\033OX" },  /* equals */
    { XK_KP_Multiply , "KP_Multiply" , NULL , "*" , "\033Oj" },
    { XK_KP_Add      , "KP_Add"      , NULL , "+" , "\033Ok" },
    { XK_KP_Separator, "KP_Separator", NULL , "," , "\033Ol" },  /* separator, often comma */
    { XK_KP_Subtract , "KP_Subtract" , NULL , "-" , "\033Om" },
    { XK_KP_Decimal  , "KP_Decimal"  , NULL , "." , "\033On" },
    { XK_KP_Divide   , "KP_Divide"   , NULL , "/" , "\033Oo" },
};
#define XKT_PAD_nb (sizeof (Xkt_desc_pad) / sizeof (struct KTdesc))


/* Auxilliary Functions */

static struct KTdesc Xkt_desc_fn [] = {
    /* function keys */
    { XK_F1,  "F1" , "\033[11~" },
    { XK_F2,  "F2" , "\033[12~" },
    { XK_F3,  "F3" , "\033[13~" },
    { XK_F4,  "F4" , "\033[14~" },
    { XK_F5,  "F5" , "\033[15~" },
    { XK_F6,  "F6" , "\033[17~" },
    { XK_F7,  "F7" , "\033[18~" },
    { XK_F8,  "F8" , "\033[19~" },
    { XK_F9,  "F9" , "\033[20~" },
    { XK_F10, "F10", "\033[21~" },

    { XK_F11, "F11", "\033[23~" },
    { XK_F12, "F12", "\033[24~" },
    { XK_F13, "F13", "\033[25~" },
    { XK_F14, "F14", "\033[26~" },
    { XK_F15, "F15", "\033[28~" },
    { XK_F16, "F16", "\033[29~" },
    { XK_F17, "F17", "\033[31~" },
    { XK_F18, "F18", "\033[32~" },
    { XK_F19, "F19", "\033[33~" },
    { XK_F20, "F20", "\033[34~" },
};
#define XKT_FN_nb (sizeof (Xkt_desc_fn) / sizeof (struct KTdesc))


/* KT_LATIN descriptor (latin characters) */

static struct KTdesc Xkt_desc_latin [] = {
    { XK_Tab,       "Tab",        "\t"  ,  NULL, NULL },
    { XK_BackSpace, "BackSpace",  "\b"  ,  NULL, NULL },
    { XK_Delete,    "Delete",     "\177",  NULL, NULL },
};
#define XKT_LATIN_nb (sizeof (Xkt_desc_latin) / sizeof (struct KTdesc))


/* Overwrite XTerm KTdesc for various classes of X terminal emulator */

/* This description must correspond to the normal setting
 *  of back space and delete keys (see in menu :
 *      Setting, Preferences, Swap DEL/Backspace
 * See routine "zvt_term_key_press" in gnome_terminal source,
 *      in file ".../gnome/lib/zvt/zvtterm.c"
 */
static struct KTdesc Xkt_desc_GnomeTerminal [] = {
    { XK_BackSpace, "BackSpace", "\b"      , NULL,  NULL },
    { XK_Delete   , "Delete"   , "\177"    , NULL , NULL },
    { XK_End      , "End"      , "\033[4~" , NULL , NULL },
    { XK_Home     , "Home"     , "\033[1~" , NULL , NULL },
    { XK_KP_End   , "End"      , "\033[4~" , NULL , NULL },
    { XK_KP_Home  , "Home"     , "\033[1~" , NULL , NULL },
    { XK_F1       , "F1"       , "\033OP"  , NULL , NULL },
    { XK_F2       , "F2"       , "\033OQ"  , NULL , NULL },
    { XK_F3       , "F3"       , "\033OR"  , NULL , NULL },
    { XK_F4       , "F4"       , "\033OS"  , NULL , NULL },
};
#define XKT_GnomeTerminal_nb (sizeof (Xkt_desc_GnomeTerminal) / sizeof (struct KTdesc))

/* XTerm class when BackarrowKey is "false" (default is "True"). */
static struct KTdesc Xkt_desc_XTerm [] = {
    { XK_BackSpace, "BackSpace", "\177"     , NULL,  NULL },
};
#define XKT_XTerm_nb (sizeof (Xkt_desc_XTerm) / sizeof (struct KTdesc))

static struct _terminal_overwrite {
	char * class_name;
	struct KTdesc *Xkt_desc_overwrite;
	int overwrite_nb;   /* number of elements in the overwrite array */
	Terminal_Class class;
    } terminal_overwrite [] = {
	{ "GnomeTerminal", Xkt_desc_GnomeTerminal, XKT_GnomeTerminal_nb, gnome },
	{ "XTerm", Xkt_desc_XTerm, XKT_XTerm_nb, xterm },
	{  NULL, NULL, 0, unknow }
    };


static struct KTdesc *Xktcur_desc_pt = Xkt_desc_cursor;
static int Xktcur_desc_nb = XKT_CUR_nb;

static struct KTdesc *Xktpad_desc_pt = Xkt_desc_pad;
static int Xktpad_desc_nb = XKT_PAD_nb;

static struct KTdesc *Xktfn_desc_pt = Xkt_desc_fn;
static int Xktfn_desc_nb = XKT_FN_nb;

static struct KTdesc *Xktlatin_desc_pt = Xkt_desc_latin;
static int Xktlatin_desc_nb = XKT_LATIN_nb;

static struct _all_ktdesc all_Xktdesc [] = {
    { 0, "Cursor key",          &Xktcur_desc_pt,   &Xktcur_desc_nb },
    { 0, "Key Pad",             &Xktpad_desc_pt,   &Xktpad_desc_nb },
    { 0, "Function key",        &Xktfn_desc_pt,    &Xktfn_desc_nb  },
    { 0, "Latin character key", &Xktlatin_desc_pt, &Xktlatin_desc_nb }
};
#define all_Xktdesc_nb (sizeof (all_Xktdesc) / sizeof (all_Xktdesc[0]))

static char *get_kt_strg (int, char **);

/* array to be used for quick search */
static struct KTdesc *(*Xkt_desc_pt) [] = NULL;
static int Xkt_desc_pt_nb;

/* special case of badly handle key in old xterm version and in nxterm */
/*  The handling for these key is correctly done in new version of xterm,
 *      we assume that a "#VT100.Translation: #override" is done
 *      in app_defaults ("/usr/X11R6/lib/X11/app_defaults/NXTerm")
 *      to provide the escape sequence of the new xterm. But this
 *      cannot provide the adequate handling of the cursor mode.
 *      A special case in "string2key_label" routine is done to go
 *      arround this problem.
 */

static struct _nxterm_special {
    int ktfunc;
    struct KTdesc *Xktdesc_special_pt;
    } nxterm_special [] = {
	{ XK_Home    , NULL },
	{ XK_End     , NULL },
	{ XK_Begin   , NULL },
	{ XK_KP_Home , NULL },
	{ XK_KP_End  , NULL },
	{ XK_KP_Begin, NULL }
    };
#define nxterm_special_nb (sizeof (nxterm_special) / sizeof (nxterm_special[0]))

/* In Linux xterm, which is compiled with the OPT_VT52_MODE keys F1-F4
 *  are interpreted as PF1-PF4 (see input.c in Input routine.
 *  A back translation as to be done for a correct handling of F1-F4
 *  see routine 'overwrite_PF1PF4' called by 'getkbfile' during initialization.
 */

static struct _xterm_F1F4_keys {
    char *PFstring;
    char  *Fstring;
    } xterm_F1F4_keys [] = {
	{ "\033OP", "\033[11~" },
	{ "\033OQ", "\033[12~" },
	{ "\033OR", "\033[13~" },
	{ "\033OS", "\033[14~" }
    };
#define xterm_F1F4_keys_nb (sizeof (xterm_F1F4_keys) / sizeof (xterm_F1F4_keys[0]))

/* ------------------------------------------------------------------------ */

#ifdef __linux__

/* Routines specific for the linux / console terminal */
/* -------------------------------------------------- */

static void kbf_linux_string (int idx, char *strg)
{
    char *cp;

    cp = (char *) malloc ( strlen(strg)+1);
    if ( cp ) {
	strcpy (cp,strg);
	ktfn_desc_pt[idx].strg = cp;
    }
}

/* build_linux_ktfn_string : build the descriptors of string function (KT_FN type) */

static int build_linux_ktfn_string (int fn, int *nbpt, int *szpt,
				    struct KTdesc *ktfn_pt, char *ktfnstrg_pt)
{
    int i, cc;
    int nb, sz;
    char *sp;
    struct kbsentry iokbstr;

    nb = sz = 0;
    for ( i = 0 ; i <= 255 ; i++ ) {
	iokbstr.kb_func = i;
	iokbstr.kb_string[0] = '\0';
	cc = ioctl (fn, KDGKBSENT, &iokbstr); /* read the string */
	if ( cc < 0 ) return (-2);  /* error : not a linux console */
	if ( iokbstr.kb_string[0] == '\0' ) continue; /* not assigned */
	if ( ktfn_pt && ktfnstrg_pt ) {
	    sp = &ktfnstrg_pt[sz];
	    strcpy (sp, iokbstr.kb_string);
	    ktfn_pt[nb].ktfunc = K (KT_FN, i);
	    ktfn_pt[nb].strg = sp;
	}
	nb++;
	sz += strlen (iokbstr.kb_string) +1;
    }
    if ( ktfn_pt && ktfnstrg_pt ) {
	if ( (nb != *nbpt) || (sz != *szpt) )
	    return (-1);   /* error */
    }
    else {
	*nbpt = nb;
	*szpt = sz;
    }
    return (nb);
}

/* get_linux_kbmap_string : get the defined function strings and build descriptors */

static void get_linux_kbmap_string (int fn)
{
    int i, cc;
    int nb, sz;
    struct kbsentry *kbstr;
    struct KTdesc *ktds;
    char *ktstrg_buf;

    ktfn_desc_pt = (struct KTdesc *) NULL;
    ktfn_desc_nb = 0;

    ktds = (struct KTdesc *) NULL;
    kbstr = (struct kbsentry *) NULL;
    nb = sz = 0;
    cc = build_linux_ktfn_string (fn, &nb, &sz, NULL, NULL); /* get size */
    if ( (cc <= 0) || (nb == 0) || (sz == 0) ) return;   /* error or nothing */

    ktds = (struct KTdesc *) calloc (nb, sizeof (struct KTdesc));
    if ( ktds ) {
	ktstrg_buf = (char *) calloc (sz, sizeof (char));
	if ( ktstrg_buf ) {
	    cc = build_linux_ktfn_string (fn, &nb, &sz, ktds, ktstrg_buf); /* build */
	    if ( cc > 0 ) {
		ktfn_desc_pt = ktds;
		ktfn_desc_nb = nb;
		return;
	    }
	}
    }
    if ( ktds ) free (ktds);
    if ( ktstrg_buf ) free (ktstrg_buf);
    return;
}

static void get_linux_map ()
{
    Flag something;
    int i, j;
    short val;
    int fn, cc;
    struct kbentry kbetr;
    struct kbsentry kbstr;

    something = NO;
    fn = 0;     /* ioctl on console input */
    /* get the function string */
    get_linux_kbmap_string (fn);

    /* get the map */
    for ( j = 0 ; j < nb_kbmap ; j++ ) {
	val = kbetr.kb_table = keyboard_map[j];
	if ( val < 0 ) continue;    /* do not read */
	for ( i = 0 ; i < nb_key ; i++ ) {
	    kbetr.kb_index = pc_keyboard_desc[i].kcode;
	    kbetr.kb_value = 0;
	    cc = ioctl (fn, KDGKBENT, &kbetr);  /* read map entry */
	    if ( cc < 0 ) break;    /* error : this not the linux console */
	    val = kbetr.kb_value;
	    if ( val == K_NOSUCHMAP) break; /* no such map */
	    if ( val == K_HOLE ) continue;  /* nothing mapped */
	    pc_keyboard_desc[i].ktfunc[j] = val;
	    something = YES;
	}
    }
    if ( something ) {
	get_kbmap_strg = "Keyboard map got from linux internal description";
	kbmap_type = linux_console;
    }
}

/* get_linux_ktpad_strg : special case to get the string for Key Pad */

static char * get_linux_ktpad_strg (int *ktcodept, int shift, char **tcap_strg)
{
    char *strg;
    int i;

    strg = get_kt_strg (*ktcodept, NULL);
    if ( !keypad_appl_mode || (shift == shifted) ) {
	/* translate */
	if ( *ktcodept == K_P5 ) {
	    strg = ( keypad_appl_mode ) ? kp5_trans_appl_strg : kp5_trans_num_strg;
	} else {
	    for ( i = 0 ; i < kp_translate_nb ; i +=2 ) {
		if ( kp_translate[i] == *ktcodept ) break;
	    }
	    if ( i < kp_translate_nb ) {
		*ktcodept = kp_translate[i+1];
		strg = get_kt_strg (*ktcodept, NULL);
	    }
	}
    }
    return (strg);
}

/* get_linux_ktspec_strg : special case to get the string for special key */

static char * get_linux_ktspec_strg (int ktcode, char **tcap_strg)
{
    char *strg;

    switch (ktcode) {
	case K_NUM :
	    if ( ! keypad_appl_mode ) return (NULL);
	    break;
	default :
	}
    strg = get_kt_strg (ktcode, NULL);
    return (strg);
}

/* get_linux_kt_escseq : get the escape sequence for a given kt code */

static char *get_linux_kt_escseq (int ktcode, char **tcap_strg)
{
    int i, aridx, nb;
    int idx, kttype;
    struct KTdesc *ktpt;

    if ( ! inuse_ktdesc ) return (NULL);

    kttype = KTYP (ktcode);
    for ( idx = 0 ; idx < inuse_ktdesc_nb ; idx++ ) {
	if ( kttype == inuse_ktdesc [idx].kt_type ) break;
    }
    if ( idx >= inuse_ktdesc_nb ) return (NULL);
    if ( ! inuse_ktdesc [idx].ktdesc_array ) return (NULL);

    nb = *(inuse_ktdesc [idx].ktdesc_nb);
    for ( i = 0 ; i < nb ; i++ ) {
	ktpt = *(inuse_ktdesc [idx].ktdesc_array) + i;
	if ( ktpt->ktfunc != ktcode ) continue;
	if ( tcap_strg ) *tcap_strg = ktpt->tcap;
	return (ktpt->strg );
    }
    return (NULL);
}

static char *get_linux_kt_strg (int *ktcode_pt, unsigned int shift)
{
    static char latin[2];
    char *strg;
    int ktcode;

    ktcode = *ktcode_pt;
    switch ( KTYP (ktcode) ) {
	case KT_FN :
	case KT_CUR :
	    strg = get_linux_kt_escseq (ktcode, NULL);
	    break;

	case KT_PAD :
	    /* can modify ktcode according to the console state */
	    strg = get_linux_ktpad_strg (&ktcode, shift, NULL);
	    break;

	case KT_SPEC :
	    strg = get_linux_ktspec_strg (ktcode, NULL);
	    break;

	case KT_LATIN :
	    strg = get_linux_kt_escseq (ktcode, NULL);
	    if ( ! strg ) {
		latin[0] = KVAL (ktcode);
		latin[1] = '\0';
		strg = latin;
	    }
	    break;

	default :
	    strg = NULL;
	}
    *ktcode_pt = ktcode;
    return (strg);
}

#endif /* __linux__ */
/* ------------------------------------------------------------------------ */

static void get_console_map ()
{
#ifdef __linux__
    get_linux_map ();
#endif /* __linux__ */
    if ( kbmap_type == no_map ) {
	/* something else can be try ? */
	;
    }
}

/* ------------------------------------------------------------------------ */

/* Routines specific for the xterm family terminal */
/* ----------------------------------------------- */

#ifdef NOstrcasecmp
static int strcase (char *st1, char *st2, int (*cmp)(), int n)
{
    char *str1, *str2, *st, *str;
    int sz, cc;
    char c;

    sz = strlen (st1) + strlen (st2) +2;
    if ( sz == 0 ) return 0;
    str1 = malloc (sz);
    if ( ! str1 ) return -1;
    memset (str1, 0, sz);
    str2 = &str1[strlen (st1) +1];
    for ( st = st1, str = str1; c = *st; st++, str++ ) *str = toupper(c);
    for ( st = st2, str = str2; c = *st; st++, str++ ) *str = toupper(c);
    if ( n == 0 ) cc = (*cmp) (str1, str2);
    else cc = (*cmp) (str1, str2, n);
    free (str1);
    return cc;
}

static int strcasecmp (char *st1, char *st2)
{
    return strcase (st1, st2, strcmp, 0);
}

static int strncasecmp (char *st1, char *st2, int n)
{
    return strcase (st1, st2, strncmp, n);
}

#endif /* NOstrcasecmp */

#ifndef NOX11

static int compare_Key_Assign_keysym (Key_Assign **kdesc1, Key_Assign **kdesc2)
{
    return ((*kdesc1)->Xkeysym - (*kdesc2)->Xkeysym );
}

static int compare_Xkt_desc_tcap (struct KTdesc **Xkt_desc1, struct KTdesc **Xkt_desc2)
{
    return (strcasecmp ((*Xkt_desc1)->tcap, (*Xkt_desc2)->tcap) );
}

#endif /* - NOX11 */

static int compare_Xkt_desc_keysym (struct KTdesc **Xkt_desc1, struct KTdesc **Xkt_desc2)
{
    return ((*Xkt_desc1)->ktfunc - (*Xkt_desc2)->ktfunc);
}

/* Build and sort the Ktdesc according to keysym (ktfunc) value */
static void sort_keysym_desc ()
{
    struct KTdesc **Xktdesc_ptpt, *Xktdesc_pt, Xktdesc;
    int i, j, idx, nb;

    if ( Xkt_desc_pt ) return;  /* already done */

    /* array for keysymb description */
    for ( i = Xkt_desc_pt_nb = 0 ; i < all_Xktdesc_nb ; i++ ) {
	Xkt_desc_pt_nb += *(all_Xktdesc [i].ktdesc_nb);
    }
    Xkt_desc_pt = (struct KTdesc *(*)[]) calloc (Xkt_desc_pt_nb, sizeof (struct KTdesc **));
    if ( ! Xkt_desc_pt ) return;    /* nothing can be done */

    for ( i = idx = 0 ; i < all_Xktdesc_nb ; i++ ) {
	nb = *(all_Xktdesc [i].ktdesc_nb);
	for ( j = 0 ; j < nb ; j++ ) {
	    (*Xkt_desc_pt) [idx++] = &((*(all_Xktdesc [i].ktdesc_array))[j]);
	}
    }
    /* sort Xkt_desc_pt by ktfunc (keysym) values */
    qsort ((void *)Xkt_desc_pt , Xkt_desc_pt_nb,
	   sizeof (struct KTdesc *), (int (*)()) compare_Xkt_desc_keysym);
}


#ifdef NOX11
static void get_xterm_map ()
{
    return;
}
#else /* - NOX11 */

static void get_xterm_map ()
{
    Key_Assign *keyboard_desc_pt [pc_keyboard_desc_nb];
    Key_Assign **kbdesc_ptpt, *kbdesc_pt, *kbdesc_val_pt, kbdesc;
    struct KTdesc **Xktdesc_ptpt, *Xktdesc_pt, Xktdesc;
    int i, j, idx, nb, val, sz;
    FILE *xmodmap_file;
    char line [256], *sp, *sp1;
    char * ksym_strg [nb_kbmap];
    int nb_ksym;
    KeySym keysym, keysym_sh0;

#ifdef USE_XMODMAP
    char * xmodmap_filename;
    char strg [PATH_MAX + 64];
    int cc;
#endif

    sort_keysym_desc ();

    /* build arrays of keyboard descriptor for quick search */
    for ( i = 0 ; i < pc_keyboard_desc_nb ; i++ )
	keyboard_desc_pt [i] = &pc_keyboard_desc [i];
    qsort ((void *)keyboard_desc_pt , pc_keyboard_desc_nb,
	   sizeof (Key_Assign *), (int (*)()) compare_Key_Assign_keysym);

#ifndef USE_XMODMAP
    /* parse the keyboard mapping */
    nb_ksym = max_keycode - min_keycode +1;
    if ( x_keyboard_map ) {
	for ( i = 0 ; i < nb_ksym ; i++ ) {
	    kbdesc_val_pt = &kbdesc;
	    kbdesc_val_pt->kcode = i + min_keycode;

	    /* setup the keyboard descriptor (for the case where
	     * X Keyboard extension is not available */
	    kbdesc_pt = NULL;
	    for ( j = 0 ; j < max_shift_level ; j ++ ) {
		keysym = x_keyboard_map [keysym_per_keycode*i +j];
		if ( kbmap_type == X11_KBExt ) {
		    /* special case : see also "display1Xkey" */
		    if ( j == 0 ) keysym_sh0 = keysym;  /* default keysym */
		    else if ( keysym == NoSymbol ) keysym = keysym_sh0;
		}
		if ( keysym == NoSymbol ) continue; /* nothing assigned */
		Xktdesc.ktfunc = keysym;
		Xktdesc.tcap = XKeysymToString (keysym);
		Xktdesc_pt = &Xktdesc;
		Xktdesc_ptpt = (struct KTdesc **) bsearch (&Xktdesc_pt,
				*Xkt_desc_pt, Xkt_desc_pt_nb,
				sizeof (struct KTdesc *),
				(int (*)()) compare_Xkt_desc_keysym);
		if ( ! Xktdesc_ptpt ) continue;

		Xktdesc_pt = *Xktdesc_ptpt;
		if ( j == 0 ) {
		    /* the unshifted assignement is supposed to be 'regular' */
		    /* get the key for this value */
		    kbdesc_val_pt->Xkeysym = Xktdesc_pt->ktfunc;    /* keysymb value */
		    kbdesc_ptpt = (Key_Assign **) bsearch (&kbdesc_val_pt, keyboard_desc_pt,
				    pc_keyboard_desc_nb, sizeof (Key_Assign *),
				    (int (*)()) compare_Key_Assign_keysym);
		    if ( ! kbdesc_ptpt ) {
			/* not found, try for the key shifted symbol */
			kbdesc_val_pt->Xkeysym = x_keyboard_map [keysym_per_keycode*i +1];
			kbdesc_ptpt = (Key_Assign **) bsearch (&kbdesc_val_pt, keyboard_desc_pt,
					pc_keyboard_desc_nb, sizeof (Key_Assign *),
					(int (*)()) compare_Key_Assign_keysym);
		    }
		    if ( kbdesc_ptpt ) {
			kbdesc_pt = *kbdesc_ptpt;
			kbdesc_pt->kcode = kbdesc_val_pt->kcode;
		    }
		}
		if ( kbdesc_pt ) {
		    kbdesc_pt->ktfunc [j] = Xktdesc_pt->ktfunc;
		}
	    }
	}
    }
#endif /* - USE_XMODMAP */

#ifdef USE_XMODMAP
    memset (strg, 0, sizeof (strg));
    strcpy (strg, "xmodmap -pke > ");
    xmodmap_filename = tmpnam (strg + strlen (strg));
    if ( xmodmap_filename ) {
	cc = system (strg);
	if ( cc != 0 ) {
	    unlink (xmodmap_filename);
	    return;
	}
    }

    /* parse the output of "xmodmap -pke" to get the mapping */
    qsort ((void *)Xkt_desc_pt , Xkt_desc_pt_nb,
	   sizeof (struct KTdesc *), (int (*)()) compare_Xkt_desc_tcap);

    xmodmap_file = fopen (xmodmap_filename, "r");
    if ( xmodmap_file == NULL ) return;

    while ( fgets (line, sizeof line, xmodmap_file) != NULL ) {
	if ( strncmp ("keycode ", line, 8) != 0 ) continue;
	sp = strchr (line, '=');
	if ( ! sp ) continue;

	kbdesc_val_pt = &kbdesc;

	/* get the keycode value */
	*(sp++) = '\0';
	val = atoi (&line[8]);
	kbdesc_val_pt->kcode = val;

	sz = strlen (sp);
	if ( sz <= 1 ) continue;    /* nothing assigned */

	/* get the keysym strings */
	memset (ksym_strg, 0, sizeof (ksym_strg));
	sp [sz-1] = '\0';   /* remove the trailling '\n' */
	sp1 = sp;
	for ( j = 0 ; j < nb_kbmap ; j ++ ) {
	    for ( ; *sp1 == ' ' ; sp1++ ) ; /* skip over headding space */
	    if ( *sp1 == '\0' ) break;  /* nothing more */
	    ksym_strg [j] = sp1;
	    sp1 = strchr (ksym_strg [j], ' ');
	    if ( ! sp1 ) break;
	    *(sp1++) = '\0';    /* remove trailling space */
	}

	/* setup the keyboard descriptor */
	kbdesc_pt = NULL;
	for ( j = 0 ; j < nb_kbmap ; j ++ ) {
	    if ( ! ksym_strg [j] ) continue;    /* nothing assigned */

	    Xktdesc.tcap = ksym_strg [j];
	    Xktdesc_pt = &Xktdesc;
	    Xktdesc_ptpt = (struct KTdesc **) bsearch (&Xktdesc_pt,
			    *Xkt_desc_pt, Xkt_desc_pt_nb,
			    sizeof (struct KTdesc *),
			    (int (*)()) compare_Xkt_desc_tcap);
	    if ( ! Xktdesc_ptpt ) continue;

	    Xktdesc_pt = *Xktdesc_ptpt;
	    if ( j == 0 ) {
		/* the unshifted assignement is supposed to be 'regular' */
		/* get the key for this value */
		kbdesc_val_pt->Xkeysym = Xktdesc_pt->ktfunc;    /* keysymb value */
		kbdesc_ptpt = (Key_Assign **) bsearch (&kbdesc_val_pt, keyboard_desc_pt,
				pc_keyboard_desc_nb, sizeof (Key_Assign *),
				(int (*)()) compare_Key_Assign_keysym);
		if ( kbdesc_ptpt ) {
		    kbdesc_pt = *kbdesc_ptpt;
		    kbdesc_pt->kcode = kbdesc_val_pt->kcode;
		}
	    }
	    if ( kbdesc_pt ) {
		kbdesc_pt->ktfunc [j] = Xktdesc_pt->ktfunc;
	    }
	}
    }
    fclose (xmodmap_file);
    (void) unlink (xmodmap_filename);
    get_kbmap_strg = "Keyboard map got from the output of \"xmodmap -pke\"";
    kbmap_type = X11_xmodmap;
#endif /* USE_XMODMAP */

    /* re-order the array to be used by "xterm_keysym2string" */
    qsort ((void *)Xkt_desc_pt, Xkt_desc_pt_nb,
	   sizeof (struct KTdesc *), (int (*)()) compare_Xkt_desc_keysym);

    if ( x_terminal_class == xterm ) {
	/* build the special case array of cursor key for nxterm */
	for ( i = 0 ; i < nxterm_special_nb ; i++ ) {
	    struct KTdesc Xktdesc, *Xktdesc_pt, **Xktdesc_ptpt;
	    Xktdesc.ktfunc = nxterm_special [i].ktfunc;
	    Xktdesc_pt = &Xktdesc;
	    Xktdesc_ptpt = (struct KTdesc **) bsearch (&Xktdesc_pt,
			    *Xkt_desc_pt, Xkt_desc_pt_nb,
			    sizeof (struct KTdesc *),
			    (int (*)()) compare_Xkt_desc_keysym);
	    if ( ! Xktdesc_ptpt ) continue;
	    nxterm_special [i].Xktdesc_special_pt = *Xktdesc_ptpt;
	}
    }
}
#endif /* - NOX11 */

/* xterm_keysym2string : get the terminal emulator string generated
 *   for the given keysym code
 */

static char *xterm_keysym2string (int keysym, char **tcap_strg)
{
    static char latin[2];
    struct KTdesc Xktdesc, *Xktdesc_pt, **Xktdesc_ptpt;

    if ( tcap_strg ) *tcap_strg = NULL;

    if ( (keysym >= XK_space) && (keysym <= XK_asciitilde) ) {
	latin [0] = (keysym - XK_space) + ' ';
	latin [1] = '\0';
	return (latin);
    }

    /* -------------- old XTerm version --------------
    if ( (keysym >= XK_KP_Home) && (keysym <= XK_KP_Begin) ) {
	keysym += XK_Home - XK_KP_Home;
    }
    */

    if ( keysym == XK_BackSpace )
	latin [0] = '\b';
    if ( keysym == XK_Delete )
	latin [0] = '\177';

    Xktdesc.ktfunc = keysym;
    Xktdesc_pt = &Xktdesc;
    Xktdesc_ptpt = (struct KTdesc **) bsearch (&Xktdesc_pt,
		    *Xkt_desc_pt, Xkt_desc_pt_nb,
		    sizeof (struct KTdesc *),
		    (int (*)()) compare_Xkt_desc_keysym);
    if ( ! Xktdesc_ptpt ) return (NULL);

    Xktdesc_pt = *Xktdesc_ptpt;
    if ( tcap_strg ) *tcap_strg = Xktdesc_pt->tcap;
    return (Xktdesc_pt->strg);
}

Flag get_xterm_name (char **class_pt, char ** name_pt)
/* if it is and xterm, return YES and class and emulator name
 * else return NO
 */
{
    if ( class_pt ) *class_pt = NULL;
    if ( name_pt )  *name_pt  = NULL;
#ifndef NOX11
    if ( class_pt && emul_class && (strcmp (emul_class, unknow_emul_class) != 0) ) *class_pt = emul_class;
    if ( name_pt  && emul_name &&  (strcmp (emul_name,  unknow_emul_name)  != 0) ) *name_pt  = emul_name;
#endif
    return (*class_pt || *name_pt);
}
/* ------------------------------------------------------------------------ */

/* routine to get escape seq and key for user defined mapping */

static char * get_my_kt_strg (int ktcode)
{
    if ( (ktcode >= *mykb_escseq_idx) || (ktcode < 0) ) return NULL;
    return (&mykb_escseq [ktcode]);
}

static char * get_my_kt_escseq (int ktcode, char **tcap_strg)
{
    char * str;
    int i, j;

    str = get_my_kt_strg (ktcode);
    if ( ! str ) return NULL;

    for ( i = 0 ; i < nb_key ; i++ ) {
	for ( j = 0 ; j < nb_kbmap ; j++ ) {
	    if ( pc_keyboard_desc[i].ktfunc[j] == ktcode )
		if ( tcap_strg ) *tcap_strg = pc_keyboard_desc[i].klabel;
	}
    }
    return str;
}

/* ------------------------------------------------------------------------ */

static char * get_ktcode2escseq (int ktcode, char **tcap_strg)
{
    switch ( kbmap_type ) {
	case user_mapfile :
	    return (get_my_kt_escseq (ktcode, tcap_strg));

#ifdef __linux__
	case linux_console :
	    return (get_linux_kt_escseq (ktcode, tcap_strg));
#endif /* __linux__ */

	case X11_default :
	case X11_KBExt :
	case X11_xmodmap :
	    return (xterm_keysym2string (ktcode, tcap_strg));

	default :
	    return NULL;
    }
}

static char * get_ktcode2strg (int ktcode, char **tcap_strg, unsigned int shift)
{
    int ktc;

    switch ( kbmap_type ) {
	case user_mapfile :
	    return (get_my_kt_strg (ktcode));

#ifdef __linux__
	case linux_console :
	    ktc = ktcode;
	    return (get_linux_kt_strg (&ktc, shift));
#endif /* __linux__ */

	case X11_default :
	case X11_KBExt :
	case X11_xmodmap :
	    return (xterm_keysym2string (ktcode, tcap_strg));

	default :
	    return NULL;
    }
}

static void set_cursor_mode ()
{
    static void switch_mode (struct KTdesc *, int, Flag);

    switch ( kbmap_type ) {
	case user_mapfile :
	    return;

#ifdef __linux__
	case linux_console :
	    switch_mode ( ktcur_desc_pt, ktcur_desc_nb, cursor_alt_mode);
	    return;
#endif /* __linux__ */

	case X11_default :
	case X11_KBExt :
	case X11_xmodmap :
	    switch_mode ( Xktcur_desc_pt, Xktcur_desc_nb, cursor_alt_mode);
	    return;

	default :
	    return;
    }

}


/* Try to assert the terminal mode (according to the initialisation string) */
/* ------------------------------------------------------------------------ */

static char * get_last_occurence (char *cs, char *ct)
{
    char *str, *lstr;

    lstr = NULL;
    for ( str = cs ; str ; str++ ) {
	str = strstr (str, ct);
	if ( ! str ) break;
	lstr = str;
    }
    return (lstr);
}

/* analyse the initialisation string */

static int compare_init_seq (struct Init_Sequence *sq1, struct Init_Sequence *sq2)
{
    return ( sq1->ini - sq2->ini );
}

static void get_terminal_mode (char *init_strg, Flag *app_mode, Flag *alt_cursor_mode)
{
    int i;
    char *sp;
    struct Init_Sequence initseq [init_seq_nb];

    if ( ! init_strg ) return;

    memcpy (initseq, init_seq, sizeof (init_seq));
    for ( i = 0 ; i < init_seq_nb ; i++ ) {
	sp = get_last_occurence (init_strg, initseq[i].ref);
	initseq[i].ini = ( sp ) ? sp - init_strg : -1;
    }
    qsort ((void *)initseq, init_seq_nb, sizeof (initseq[0]), (int (*)()) compare_init_seq);
    /* build the status */
    for ( i = 0 ; i < init_seq_nb ; i++ ) {
	if ( initseq[i].ini < 0 ) continue;
	switch (initseq[i].type) {
	    case 0 :        /* reset (normal cursor and numerical */
		*app_mode = *alt_cursor_mode = NO;
		break;
	    case 1 :        /* numeric key pad mode */
		*app_mode = NO;
		break;
	    case 2 :        /* application key pad mode */
		*app_mode = YES;
		break;
	    case 3 :        /* normal cursor mode */
		*alt_cursor_mode = NO;
		break;
	    case 4 :        /* alternate cursor mode */
		*alt_cursor_mode = YES;
		break;
	}
    }
}

/* set_term_mode : set terminal mode (cursor and numerical key pad) */
/* ---------------------------------------------------------------- */

static void switch_mode (struct KTdesc *ktdesc_pt, int nb, Flag mode_flg)
{
    int i;
    char *sp;

    for ( i = 0 ; i < nb ; i++ ) {
	sp = ( mode_flg ) ? ktdesc_pt [i].altstrg : ktdesc_pt [i].norstrg ;
	if ( sp ) ktdesc_pt [i].strg = sp;
    }
}

static void set_appl_mode ()
{
    if ( ! Xterminal_flg ) return;  /* nothing for linux terminal */

    switch_mode (Xktpad_desc_pt, Xktpad_desc_nb, keypad_appl_mode);
}


static void set_term_mode (Flag keypad_mode, Flag cursor_mode, Flag set)
{
    int idx;

    keypad_appl_mode = keypad_mode;
    cursor_alt_mode  = cursor_mode;
    set_appl_mode ();
    set_cursor_mode ();
    if ( !set ) return;

    idx = ( keypad_appl_mode ) ? 2 : 1;
    fputs (init_seq[idx].ref, stdout);
    idx = ( cursor_alt_mode ) ? 4 : 3;
    fputs (init_seq[idx].ref, stdout);
}

/* load_kbmap_file : read a user defined keyboard mapping file */
/* ----------------------------------------------------------- */

static int switch_kbmap_storage (Flag alt, Flag reset)
{
    int sz;

    escseq_sz       = alt ?  sizeof (alt_escseq) : sizeof (nor_escseq);
    mykb_escseq     = alt ?  alt_escseq     :  nor_escseq;
    mykb_escseq_idx = alt ? &alt_escseq_idx : &nor_escseq_idx;
    mykb_esc_idx    = alt ? &alt_esc_idx    : &nor_esc_idx;
    sz              = alt ?  sizeof (alt_esc_idx) : sizeof (nor_esc_idx);

    if ( reset ) {
	memset (mykb_escseq, 0, escseq_sz);
	*mykb_escseq_idx = 0;
	memset (mykb_esc_idx, -1, sz);
    }
    return sz;
}


#if 0
static int compare_escseq (char **str1, char **str2)
{
    return ( strcmp (*str1, *str2) );
}
#endif

static char * kbmap_escp (char *str)
{
    static char tstrg [128];    /* large enough */
    char tch [4];
    char ch, *tstr, *str1, *str2;
    int i, sz;

    if ( !str || !*str ) return NULL;

    memset (tstrg, 0, sizeof (tstrg));
    tstr = tstrg;
    for ( str1 = str ; (ch = *str1) ; str1++ ) {
	if ( ch == '<' ) {
	    /* predefined char */
	    for ( i = ch_symb_nb -1 ; i >= 0 ; i-- ) {
		sz = strlen (ch_symb [i].symb);
		if ( strncmp (ch_symb [i].symb, str1, sz) == 0 ) {
		    ch = ch_symb [i].ch;
		    str1 += sz -1;
		    break;
		}
	    }
	} else if ( ch == '^' ) {
	    /* control char */
	    str1++;
	    ch = *str1 & 037;
	} else if ( ch == '\\' ) {
	    memcpy (tch, str1 +1, 3);
	    tch [3] = '\0';
	    ch = (unsigned char) strtol (tch, NULL, 0);
	    str1 += 3;
	}
	*tstr++ = ch;
    }
    return tstrg;
}

static Flag parse_kbmapline (char *line)
{
    Flag something;
    int kidx, klvl, sz;
    char *str, *str1, *str2, *esc_pt;
    char ch;

    for ( str = line ; *str && (*str == ' ') ; str++ ) ;
    if ( !*str ) return NO;

    /* get the key name */
    str1 = str;
    str2 = str = strchr (str1, ':');
    if ( !str ) return NO;

    *str2 = '\0';
    for ( str-- ; *str == ' ' ; str-- ) *str = '\0';
    for ( kidx = pc_keyboard_desc_nb -1 ; kidx >= 0 ; kidx-- ) {
	if ( strcmp (str1, pc_keyboard_desc [kidx].klabel) == 0 ) break;
    }
    if ( kidx < 0 ) return NO;

    /* get the key strings */
    something = NO;
    for ( klvl = 0 ; klvl < nb_kbmap ; klvl++ ) {
	if ( !str2 ) break;                 /* nothing more */
	str1 = str2 +1;
	str2 = str = strchr (str1, ',');    /* next separator */
	if ( str2 ) *str2 = '\0';
	for ( ; *str1 == ' ' ; str1++ ) ;   /* remove heading space */
	str = strchr (str1, ' ');
	if ( str ) *str = '\0';             /* remove trailling space */
	str = kbmap_escp (str1);
	if ( verbose_helpflg ) printf ("%s %-10s",
				       klvl ? "," : "                  ",
				       str ? escseqstrg (str, NO) : ""
				      );
	if ( str && *str ) {
	    sz = strlen (str) +1;
	    if ( (*mykb_escseq_idx + sz) < escseq_sz ) {
		esc_pt = &mykb_escseq [*mykb_escseq_idx];
		strcpy (esc_pt, str);
		(*mykb_esc_idx) [kidx][klvl] = *mykb_escseq_idx;
		*mykb_escseq_idx += sz;
		something = YES;

	    }
	}
    }
    return something;
}

static Flag read_kbmap_file (char *kbmap_fname, Flag alt)
{
    FILE *kfile;
    char buf [512], *line, *str;
    Flag result, something;
    int lnb, lbsz, lsz, sz;

    result = NO;

    (void) switch_kbmap_storage (alt, YES);

    kfile = fopen (kbmap_fname, "r");
    if ( kfile == NULL ) return result;

    if ( verbose_helpflg )
	printf ("\nGet keymap from \"%s\"\n\n", kbmap_fname);
    memset (buf, 0, sizeof (buf));
    line = &buf[1];
    lbsz = sizeof (buf) -3;
    lnb = 0;
    while ( fgets (line, lbsz, kfile) != NULL ) {
	lnb++;
	lsz = strlen (line);
	if ( lsz && (line [lsz-1] == '\n') ) line [lsz-1] = '\0';
	if ( verbose_helpflg ) printf ("%3d %s\n", lnb, line +1);
	for ( str = line ; *str && (*str == ' ') ; str++ ) ;
	if ( !*str || (*str == '#') ) continue; /* empty or comment line */
	something = parse_kbmapline (str);
	if ( something ) {
	    if ( verbose_helpflg ) fputc ('\n', stdout);
	    result = YES;
	}
    }
    mykb_escseq [escseq_sz -1] = '\0';
    fclose (kfile);

    if ( alt ) (void) switch_kbmap_storage (NO, NO);
    return result;
}

static Flag load_kbmap_file (char *kbmap_fname)
{
    Flag result;
    int idx, lv;
    char *esc_pt;

    result = read_kbmap_file (kbmap_fname, NO);
    if ( !result ) return NO;

    for ( idx = 0 ; idx < pc_keyboard_desc_nb ; idx++ ) {
	for ( lv = 0 ; lv < nb_kbmap ; lv++ ) {
	    pc_keyboard_desc [idx].ktfunc [lv] = (*mykb_esc_idx) [idx][lv];
	}
	pc_keyboard_desc [idx].kcode = pc_keyboard_desc [idx].lkcode;
    }
#if 0
    /* build array for fast search */
    memset (mykb_escseq_pt, 0, sizeof (mykb_escseq_pt));
    mykb_escseq_nb = 0;
    for ( esc_pt = mykb_escseq ; *esc_pt ; esc_pt += (strlen (esc_pt) +1) ) {
	mykb_escseq_pt [mykb_escseq_nb++] = esc_pt;
    }
    qsort (mykb_escseq_pt, mykb_escseq_nb, sizeof (mykb_escseq_pt[0]),
	   (int (*)(const void *,const void *)) compare_escseq);
#endif
    Xterminal_flg = NO;
    return result;
}

/* get_kb_map : get the current active keyboard mapping */
/* ---------------------------------------------------- */

static void get_kb_map (char *terminal, char *init_strg,
			Flag *app_mode,
			Flag *alt_cursor_mode)
{
    int i, j, idx, k;
    short val;
    int fn, cc;
    char * xmodmap_filename;

    if ( get_map_flg ) return;  /* init already done */

    kbmap_type = no_map;

    /* reset user defined keyboard mapping storage */
    (void) switch_kbmap_storage (NO, YES);
#if 0
    memset (mykb_escseq_pt, 0, sizeof (mykb_escseq_pt));
    mykb_escseq_nb = 0;
#endif

    /* init the PC101 description structure */
    for ( idx = k = 0 ; k < pc101_keyboard_struct_nb ; k++ ) {
	pc101_keyboard_struct [k].idx = idx;
	idx += pc101_keyboard_struct [k].nb;
    }

    /* check for terminal type and build xmodmap file if needed */
    terminal_name = terminal;
    terminal_type (terminal);

    /* reset the keyboard description according to the terminal class */
    kt_void = ( Xterminal_flg ) ? XK_VoidSymbol : K_HOLE;
    for ( i = 0 ; i < nb_key ; i++ ) {
	pc_keyboard_desc[i].kcode = ( Xterminal_flg ) ? NoSymbol : pc_keyboard_desc[i].lkcode;
	for ( j = 0 ; j < nb_kbmap ; j++ ) {
	    /* reset to no action mapped */
	    pc_keyboard_desc[i].ktfunc[j] = kt_void;
	}
    }

    if ( init_strg ) get_terminal_mode (init_strg, app_mode, alt_cursor_mode);

    if ( Xterminal_flg ) get_xterm_map ();
    if ( kbmap_type == no_map ) get_console_map ();

    if ( kbmap_type == no_map ) {
	/* look for a user keyboard mapping file */
	Flag exist;

	kt_void = -1;
	for ( i = 0 ; i < nb_key ; i++ ) {
	    for ( j = 0 ; j < nb_kbmap ; j++ ) {
		/* reset to no action mapped */
		/* in this case ktfunc is the index into mykb_escseq [] */
		pc_keyboard_desc[i].ktfunc[j] = kt_void;
	    }
	}

	kbmap_fname = get_kbmap_file_name (NO, &exist);
	if ( kbmap_fname && *kbmap_fname ) {
	    get_kbmap_strg = ( exist )
			   ? "Keyboard map defined by my file :"
			   : "Keyboard map expected from the file : (built by \"build-kbmap\" command)";
	}
	if ( exist ) {
	    Flag cc;
	    /* get the user defined map */
	    cc = load_kbmap_file (kbmap_fname);
	    if ( cc ) kbmap_type = user_mapfile;
	    else get_kbmap_strg = "Keyboard map file does not define any mapping";
	}
    }

    /* init according to the keyboard mapping type */
    switch ( kbmap_type ) {
	case user_mapfile :
	    inuse_ktdesc = all_myktdesc;
	    inuse_ktdesc_nb = all_myktdesc_nb;
	    break;
	case linux_console :
	    inuse_ktdesc = all_ktdesc;
	    inuse_ktdesc_nb = all_ktdesc_nb;
	    break;
	case X11_default :
	case X11_KBExt :
	case X11_xmodmap :
	    inuse_ktdesc = all_Xktdesc;
	    inuse_ktdesc_nb = all_Xktdesc_nb;
	    break;
	default :
	    inuse_ktdesc = NULL;
	    inuse_ktdesc_nb = 0;
    }

    /* initialisation done */
    get_map_flg = YES;
}

static void dump_usr_kbmap (FILE *kfile, Flag alt)
{
    Key_Assign *kap;
    int iesc, i, j, k, j1, j2, nb_shf, keycode, ktcode;
    char *strg, *escst, *st, *sp;
    int nb, sz, cmd;
    char strg1 [256], *cmdstrg;
    unsigned char cmdv [nb_kbmap][2];
    int cmd_sz [nb_kbmap];
    Flag something;

    memset ( strg1, 0, sizeof (strg1));
    nb_shf = Xterminal_flg ? max_shift_level : nb_kbmap;
    for ( i = k = 0 ; i < nb_key ; i++ ) {
	if ( i == pc101_keyboard_struct [k].idx ) {
	    fprintf (kfile, "\n# %-15s", pc101_keyboard_struct [k].label);
	    for ( j = 0 ; j < nb_shf ; j++ ) {
		fprintf (kfile, " %-7s    ", key_shift_msg [j]);
	    }
	    (void) fputc ('\n', kfile);
	    k++;
	    if ( k >= pc101_keyboard_struct_nb ) k = 0;
	}
	kap = &pc_keyboard_desc[i];
	keycode = kap->kcode;
	fprintf (kfile, "  %-10s : ", kap->klabel);

	memset (cmd_sz, 0, sizeof (cmd_sz));
	memset (cmdv, 0, sizeof (cmdv));
	something = NO;
	for ( j = 0 ; j < nb_shf ; j++ ) {
	    strg = "";
	    escst = NULL;
	    if ( ! alt ) {
		if ( keyboard_map [j] >= 0 ) {
		    ktcode = kap->ktfunc[j];
		    escst = get_ktcode2strg (ktcode, NULL, (unsigned int) j);
		}
	    } else {
		iesc = alt_esc_idx [i][j];
		if ( iesc >= 0 ) escst = &alt_escseq [iesc];
	    }
	    strg = escseqstrg (escst, NO);
	    if ( (st = strchr (strg, '(')) ) *st = '\0';
	    fprintf (kfile, " %-10s,", strg);
	    if ( escst ) {
		/* get the allocated key function */
		nb = strlen (escst);
		sp = escst;
		sz = itget (&sp, &nb, ithead, strg1);
		if ( sz > 0  ) {
		    if ( sz > 2 ) sz = 2;
		    memcpy (&cmdv [j][0], strg1, sz);
		    cmd_sz [j] = sz;
		    something = YES;
		}
	    }
	}
	(void) fputc ('\n', kfile);
	if ( something ) {
	    fputs ("             # " , kfile);
	    for ( j = 0 ; j < nb_shf ; j++ ) {
		strg = "";
		memset (strg1, 0, sizeof (strg1));
		for ( sz = cmd_sz [j], j1 = 0 ; sz > 0 ; sz--, j1++ ) {
		    cmd = (unsigned char) cmdv [j][j1];
		    for ( j2 = 0 ; itsyms [j2].str ; j2++ ) {
			if ( itsyms [j2].val != cmd ) continue;
			strcat (strg1, itsyms [j2].str);
			strg1 [strlen (strg1)] = ' ';
			break;
		    }
		}
		fprintf (kfile, " %-10s,", strg1);
	    }
	    (void) fputc ('\n', kfile);
	}
    }
}

/* save_my_kbmap : save the user defined keyboard mapping */
/* ------------------------------------------------------ */

/* 1st line in kbmap file signature */
static char kfile_type [] = "Keymap file type :";
static char separ_line [] = "#---------------------------------------------------------------------------\n";


static char * rename_kbmapfile (char *kfname, Kb_Mapping file_kbmap_type)
{
    static char bup_kfname [PATH_MAX];

    memset (bup_kfname, 0, sizeof (bup_kfname));
    strcpy (bup_kfname, kfname);
    if ( file_kbmap_type == user_mapfile ) {
	strcat (bup_kfname, ".bak");
    } else {
	strcat (bup_kfname, ".old");
    }
    (void) unlink (bup_kfname);
    (void) rename (kfname, bup_kfname);
    return bup_kfname;
}

static Kb_Mapping get_kbmap_filetype (char ** kfname_pt, Flag *exist_pt)
{
    int sz;
    Kb_Mapping file_kbmap_type;
    Flag exist;
    char * kfname;
    FILE * kfile;
    char *asct;
    char line [256], *str;

    if ( exist_pt ) *exist_pt = NO;
    if ( kfname_pt ) *kfname_pt = NULL;
    file_kbmap_type = no_map;
    kfname = get_kbmap_file_name (YES, &exist);
    if ( exist_pt ) *exist_pt = exist;
    if ( !kfname || !*kfname ) return file_kbmap_type;

    if ( kfname_pt ) *kfname_pt = kfname;
    kfile = fopen (kfname, "r");
    if ( ! kfile ) return file_kbmap_type;

    /* keyboard map file is existing, gets its type */
    memset (line, 0, sizeof (line));
    for ( ; ; ) {
	if ( fgets (line, sizeof (line), kfile) == NULL ) break;
	str = &line [0];
	if ( *str++ != '#' ) break;
	for ( ; (*str == ' ') ; str++ ) ;
	if ( *str == '-' ) continue;    /* separator line */
	sz = strlen (kfile_type);
	if ( strncmp (str, kfile_type, sz) == 0 ) {
	    file_kbmap_type = (Kb_Mapping) atoi (str + sz);
	    break;
	}
    }
    fclose (kfile);
    return file_kbmap_type;
}

static void write_kbmap_file (char * kfname, Kb_Mapping map_type,
			      char * old_kfname, Flag alt)
{
    extern char *kbfile;
    static char cmd_msg [128];
    FILE * kfile, * old_kfile;
    char *asct;
    time_t sttime;
    struct tm *tm_local;
    char ch, line [512], *str;

    kfile = fopen (kfname, "w");
    if ( ! kfile ) {
	sprintf (cmd_msg, "File \"%s\" error : %s", kfile, strerror (errno));
	command_msg = cmd_msg;
	return;
    }

    old_kfile = ( old_kfname ) ? fopen (old_kfname, "w") : NULL;

    (void) time (&sttime);
    asct = asctime (localtime (&sttime));
    asct [strlen(asct) -1] = '\0';

    fputs (separ_line, kfile);
    fprintf (kfile, "# %s %d (%s) please, do not modify this line\n",
	     kfile_type, map_type, kb_mapping_strg [map_type]);
    if ( ! old_kfile ) {
	/* generate a new map file */
	fprintf (kfile, "# User \"%s\", \"%s\" terminal keyboard mapping file\n",
		 myname, tname);
	fprintf (kfile, "# \"%s\" generated on %s\n", kfname, asct);
	fprintf (kfile, "# %s\n", get_kbmap_strg);
	fprintf (kfile, "# Keyboard mode : %s\n", get_keyboard_mode_strg ());
	if ( map_type == user_mapfile )
	    fprintf (kfile, "# Edited by \"%s\" on %s\n", myname, asct);

    } else {
	/* update an old version */
	memset (line, 0, sizeof (line));
	(void) fgets (line, sizeof (line), old_kfile); /* skip the 1st line */
	for ( ; ; ) {
	   str = fgets (line, sizeof (line), old_kfile);
	   if ( ! str ) break;     /* error, do not read any more */
	   ch = line [0];
	   if ( ch && (ch != '#') ) break;
	   if ( strncmp (line, "#---------", 10) == 0 ) break;
	   fputs (line, kfile);
	}
	fprintf (kfile, "# Updated by \"%s\" on %s\n", myname, asct);
    }
    fputs (separ_line, kfile);
    fprintf (kfile, "# Keyboard function allocation is defined by :\n");
    fprintf (kfile, "# \"%s\"\n", kbfile ? kbfile : "???");
    fputs (separ_line, kfile);

    dump_usr_kbmap (kfile, alt);

    fclose (kfile);
    if ( old_kfile ) fclose (old_kfile);
}


void save_kbmap (Flag force)
{
    extern char *kbfile;
    int sz;
    char * kfname;
    FILE * kfile;
    Flag kbf_exist;
    char *asct;
    time_t  sttime;
    char line [256], *str;
    Kb_Mapping file_kbmap_type;  /* existing keyboard mapping file type */

    if ( kbmap_type == no_map ) return;
    if ( !force && (kbmap_type == user_mapfile) ) return;

    file_kbmap_type = get_kbmap_filetype (&kfname, &kbf_exist);

    if ( kbf_exist ) {
	switch ( kbmap_type ) {
	    case X11_default :
	    case X11_xmodmap :
		if ( file_kbmap_type == X11_KBExt ) return;
		if ( file_kbmap_type == user_mapfile )
		    (void) rename_kbmapfile (kfname, file_kbmap_type);
		break;

	    case X11_KBExt :
		if ( file_kbmap_type == X11_KBExt ) break;
	    case user_mapfile :
		(void) rename_kbmapfile (kfname, file_kbmap_type);
		break;

	    default :
		if ( file_kbmap_type == user_mapfile )
		    (void) rename_kbmapfile (kfname, file_kbmap_type);
	}
    }
    write_kbmap_file (kfname, kbmap_type, NULL, NO);
}

/* X KEYBOARD Extension processing */
/* ------------------------------- */

static void swap_backspace_del (struct KTdesc **Xktdesc_ptpt, int nb,
				char *bksp_str, char *del_str,
				Flag swap_flg )
{
    int i;

    for ( i = 0 ; i < nb ; i++, Xktdesc_ptpt++ ) {
	if ( (*Xktdesc_ptpt)->ktfunc == XK_BackSpace ) {
	    (*Xktdesc_ptpt)->strg = swap_flg ? del_str : bksp_str;
	}
	else if ( (*Xktdesc_ptpt)->ktfunc == XK_Delete ) {
	    (*Xktdesc_ptpt)->strg = swap_flg ? bksp_str : del_str;
	}
    }
}

/* Specific X terminal emulator setting */
/* ------------------------------------ */

static void get_gnome_normal_bksp_del_strg (char **bksp_strg_pt,
					    char **del_strg_pt)
{
    int i;

    for ( i = 0 ; i < XKT_GnomeTerminal_nb ; i++ ) {
	if ( Xkt_desc_GnomeTerminal[i].ktfunc == XK_BackSpace )
	    *bksp_strg_pt = Xkt_desc_GnomeTerminal[i].strg;
	if ( Xkt_desc_GnomeTerminal[i].ktfunc == XK_Delete )
	    *del_strg_pt = Xkt_desc_GnomeTerminal[i].strg;
    }
}

/* Swap in cmd list backspace and delete cmd */
void swap_cmd_back_del ( char *bksp_str, char *del_str, Flag swap_flg)
{
    extern int itgetleave (char *strg, struct itable **it_leave, struct itable *head);

    static char bksp_cmd, del_cmd;
    static Flag init_done = NO;

    char *bksp_strg, *del_strg;
    struct itable *it_leave_bksp, *it_leave_del;

    bksp_strg = bksp_str;
    del_strg = del_str;
    if ( !bksp_strg || !del_strg )
	get_gnome_normal_bksp_del_strg (&bksp_strg, &del_strg);
    if ( !bksp_strg || !del_strg ) return;

    it_leave_bksp = it_leave_del = NULL;
    (void) itgetleave (bksp_strg, &it_leave_bksp, ithead);
    (void) itgetleave (del_strg,  &it_leave_del,  ithead);

    if ( !it_leave_bksp || !it_leave_del ) return;
    if (   (it_leave_bksp->it_len != 1)
	|| (it_leave_del->it_len  != 1) ) return;

    if ( ! init_done ) {
	bksp_cmd = *(it_leave_bksp->it_val);
	del_cmd  = *(it_leave_del->it_val);
	init_done = YES;
    }
    *(it_leave_bksp->it_val) = swap_flg ? del_cmd  : bksp_cmd;
    *(it_leave_del->it_val)  = swap_flg ? bksp_cmd : del_cmd;
}


#ifndef NOX11
static void gnome_setting ()
{
    static char gnome_term_conf [] = "/.gnome/Terminal";
    static char swap [] = "swap_del_and_backspace";
    static char conf [] = "[Config]";
    static char true [] = "true";

    char fname [PATH_MAX], line[256];
    FILE *file;
    Flag config, swap_flg;
    char *str, *bksp_str, *del_str;
    int i;

    gnome_swap_flg = NO;
    if ( x_terminal_class != gnome ) return ;

    memset (fname, 0, sizeof (fname));
    memset (line, 0, sizeof (line));
    str = getenv ("HOME");
    if ( !str && !*str ) return;
    strcpy (fname, str);
    strcat (fname, gnome_term_conf);
    file = fopen (fname, "r");
    if ( file == NULL ) return;

    config = NO;
    while ( fgets (line, sizeof (line), file) != NULL ) {
	if ( line[0] == '[' ) config = (strncmp (line, conf, sizeof (conf) -1) == 0);
	if ( ! config ) continue;

	if ( strncmp (swap, line, sizeof (swap) -1) != 0 ) continue;
	str = strchr (line, '=');
	if ( ! str ) continue;
	gnome_swap_flg = swap_flg = (strncasecmp (str+1, true, sizeof (true) -1) == 0);
    }
    fclose (file);

    /* swap DEL and Backspace keysym to string convertion */
    bksp_str = del_str = NULL;
    get_gnome_normal_bksp_del_strg (&bksp_str, &del_str);
    if ( !bksp_str || !del_str ) return;

    swap_backspace_del (&(*Xkt_desc_pt) [0], Xkt_desc_pt_nb,
			bksp_str, del_str, swap_flg);

    /* swap also in the cmd list */
    swap_cmd_back_del (bksp_str, del_str, swap_flg);
}
#endif /* -NOX11 */

#ifndef NOX11
/* Get the X terminal emulator class */
static Terminal_Class get_terminal_class ()
{
    struct _terminal_overwrite *tover;

    x_terminal_class = unknow;
    if ( ! emul_class ) return;

    for ( tover = &terminal_overwrite [0] ; tover->class_name  ; tover++ ) {
	if ( strncasecmp (tover->class_name, emul_class, strlen (tover->class_name)) == 0 ) break;
    }
    if ( ! tover->class_name ) return;
    x_terminal_class = tover->class;
}
#endif /* -NOX11 */


#ifndef NOX11
/* Overwrite entry in the KTdesc for the given X terminal emulator */
static void overwrite_ktdesc (char * emul_class)
{
    int ksym, i, j, k, nb;
    struct _terminal_overwrite *tover;
    struct KTdesc *ktdsc;

    if ( x_terminal_class == unknow ) (void) get_terminal_class ();

    for ( tover = &terminal_overwrite [0] ; tover->class_name  ; tover++ ) {
	if ( tover->class == x_terminal_class ) break;
    }
    if ( (tover->overwrite_nb == 0) || !tover->Xkt_desc_overwrite ) return;

    for ( i = 0 ; i < all_Xktdesc_nb ; i++ ) {
	nb = *(all_Xktdesc [i].ktdesc_nb);
	for ( j = 0 ; j < nb ; j++ ) {
	    ktdsc = &((*(all_Xktdesc [i].ktdesc_array))[j]);
	    ksym = ktdsc->ktfunc;
	    for ( k = tover->overwrite_nb -1 ; k >= 0 ; k-- ) {
		if ( tover->Xkt_desc_overwrite[k].ktfunc != ksym ) continue;
		*ktdsc = tover->Xkt_desc_overwrite[k];
		break;
	    }
	}
    }

    if ( Xkt_desc_pt ) {
	/* force to rebuild the KTdesc */
	free (Xkt_desc_pt);
	Xkt_desc_pt = NULL;
    }
    sort_keysym_desc ();
    if ( x_terminal_class == gnome ) gnome_setting ();
}
#endif /* -NOX11 */



#ifndef NOX11
/* Provide for 'status' command some info */
static void xkb_info (char *buf)
{
    char *str1, *str2, *str3, *str4;
    if ( xkb_ext_msg )
	sprintf (buf, "  %s\n", xkb_ext_msg);
    else {
	if ( keycodes_str ) {
	    sprintf (buf, "  Keyboard \"%s, %s, %s",
		     keycodes_str, geometry_str, symbols_str);
	    if ( phys_symbols_str ) sprintf (&buf[strlen(buf)], ", engraved %s)\"\n",
					     phys_symbols_str);
	    else sprintf (&buf[strlen(buf)], "\"\n");
	}
    }
    if ( x_terminal_class  == xterm ) {
	sprintf (&buf[strlen(buf)], "  When XTerm was started Backspace key generate %s\n",
		 xterm_backarrow_flg ? "\"<Bksp>(^H)\" (default value)"
				     : "\"<Del>('\\177')\"");
	sprintf (&buf[strlen(buf)], "  see in XTerm 'Main Menu' entry 'Backarrow Key' %s\n",
		 xterm_backarrow_flg ? "on (checked)"
				     : "off (un-checked)");
    } else if ( x_terminal_class  == gnome ) {
       if ( gnome_swap_flg )
	    sprintf (&buf[strlen(buf)], "  Backspace and Delete keys are swapped\n");
    }
}
#endif /* -NOX11 */

#ifndef NOX11
/* Get the X Keyboard data */
static void get_xkb_data (Display *dpy)
{
#ifndef NOXKB
    Status cc1, cc2, cc3;
    char *sName;
    int i, j;

    keycodes_str = geometry_str = symbols_str = types_str = compat_str = phys_symbols_str = NULL;
    xkb_rec = NULL;
    if ( ! use_XKB_flg ) return;    /* cannot do any thing */

    /* I do not understand how XkbGetKeyboard is working !!
     * xkb_rec = XkbGetKeyboard (dpy, XkbNamesMask, XkbUseCoreKbd);
     */
    xkb_rec = XkbGetMap (dpy, XkbAllMapComponentsMask, XkbUseCoreKbd);
    if ( ! xkb_rec ) {
	xkb_ext_msg = "Cannot read XKB mapping";
	use_XKB_flg = NO;
	return;
    }
    cc1 = XkbGetNames (dpy, XkbAllNamesMask, xkb_rec);
    cc2 = XkbGetGeometry (dpy, xkb_rec);

    if ( xkb_rec->names ) {
	keycodes_str = XkbAtomGetString (dpy, xkb_rec->names->keycodes);
	geometry_str = XkbAtomGetString (dpy, xkb_rec->names->geometry);
	symbols_str  = XkbAtomGetString (dpy, xkb_rec->names->symbols);
	types_str    = XkbAtomGetString (dpy, xkb_rec->names->types);
	compat_str   = XkbAtomGetString (dpy, xkb_rec->names->compat);
	phys_symbols_str   = XkbAtomGetString (dpy, xkb_rec->names->phys_symbols);
    }

    /* get the keyboard section order */
    memset (section_idx, -1, sizeof (section_idx));
    for ( i = 0 ; i < xkb_rec->geom->num_sections ; i++ ) {
	sName = XkbAtomGetString (xkb_rec->dpy, (xkb_rec->geom->sections)[i].name);
	for ( j = 0 ; j < num_section_names ; j++ ) {
	    if ( strcasecmp (sName, section_names [j]) != 0 ) continue;
	    section_idx [j] = i;
	}
    }
    get_kbmap_strg = "Keyboard map got from X11 Keyboard Extension";
    kbmap_type = X11_KBExt;
#endif /* NOXKB */
}
#endif /* - NOX11 */

#ifndef NOX11
/* Check and init the XKB extension library */
Flag CheckXKBExtention (Display *dpy, int *reason,
				  Flag *xkb_extension)
#ifdef NOXKB
{
    xkb_ext_msg = "Not compiled with XKeyboard Extension";
    if ( xkb_extension ) *xkb_extension = False;
    return False;
}


#else /* - NOXKB */
{
    int  major_num, minor_num;
    int  *major_rtrn, *minor_rtrn;
    int  ev_rtrn, err_rtrn;
    char *dpyName;
    Flag flg;
    FILE *out;

    xkb_ext_msg = NULL;
    out = stdout;
    if ( xkb_extension ) *xkb_extension = True;

    if ( dpy == NULL ) {
	if ( reason ) *reason = XkbOD_ConnectionRefused;
	return False;
    }

    dpyName = DisplayString (dpy);
    major_num = XkbMajorVersion;
    minor_num = XkbMinorVersion;
    major_rtrn = &major_num;
    minor_rtrn = &minor_num;
    if ( ! XkbLibraryVersion (major_rtrn, minor_rtrn) ) {
	if ( reason ) *reason = XkbOD_BadLibraryVersion;
	xkb_ext_msg = "X library supports incompatible version";
	if ( verbose_helpflg ) {
	    fprintf (out, "%s was compiled with XKB version %d.%02d\n",
		     progname, XkbMajorVersion, XkbMinorVersion);
	    fprintf (out, "X library supports incompatible version %d.%02d\n",
		     major_num, minor_num);
	}
	return False;
    }

    major_num = XkbMajorVersion;
    minor_num = XkbMinorVersion;
    flg = XkbQueryExtension (dpy, NULL, &ev_rtrn, &err_rtrn,
			     major_rtrn, minor_rtrn);
    if ( ! flg ) {
	if ( *major_rtrn || *minor_rtrn ) {
	    if ( reason ) *reason= XkbOD_BadServerVersion;
	    xkb_ext_msg = "X server uses incompatible X Keyboard Extension version";
	    if ( verbose_helpflg ) {
		fprintf (out, "%s was compiled with XKB version %d.%02d\n",
			 progname, XkbMajorVersion, XkbMinorVersion);
		fprintf (out, "X server \"%s\" uses incompatible version %d.%02d\n",
			 dpyName, major_num, minor_num);
	    }
	} else {
	    xkb_ext_msg = "X Keyboard Extension not present on your X11 server";
	    if ( reason ) *reason= XkbOD_NonXkbServer;
	    if ( xkb_extension ) *xkb_extension = False;
	    if ( verbose_helpflg )
		fprintf (out, "XKB Extension not present on your X11 server %s\n",
			 dpyName);
	}
	return False;
    }
    if ( reason ) *reason= XkbOD_Success;
    return True;
}
#endif /* - NOXKB */
#endif /* - NOX11 */


/* terminal_type : test for X terminal case and get the keyboard mapping */
/* --------------------------------------------------------------------- */
/*  If the terminal name is neither "linux" nor "console" (standard names for linux console)
 *    - Try to know which class of X Terminal emulator is used
 *      by the value of  "WINDOWID" and window properties.
 *    - Get keyboard mapping by a call to "XGetKeyboardMapping"
 *    - Try to found emulator environemnt for sensitive value, like
 *      the BackSpace and Delete key transaltion.
 *
 *  Old style (use by version less than 19.52)
 *      use a call to "xmodmap -pke" to produce the file which will be
 *      parsed to get the keyboard mapping.
 *      If the call to xmodmap return an error, it assumes that the user
 *      terminal is not an X terminal.
 */


#ifndef NOX11
static Flag get_xterm_backarrow (Display *dpy, Window wind, char *name, char *class)
{
    XtAppContext app_context;
    XrmDatabase db;
    int nb, argc;
    char name_str[128], class_str[128], *str_type;
    XrmValue value;
    Bool cc;
    Flag backarrow_flg;
    Status status;
    XTextProperty tp;

    backarrow_flg = YES;    /* XTerm default */
    argc = 0;
    XtToolkitInitialize ();
    app_context = XtCreateApplicationContext ();
    XtDisplayInitialize (app_context, dpy, "rand", "Rand", NULL, 0, &argc, NULL);
    db = XtScreenDatabase (DefaultScreenOfDisplay (dpy));
    if ( ! db ) return backarrow_flg;

    strcpy (name_str, name);
    strcat (name_str, ".vt100.backarrowKey");
    strcpy (class_str, class);
    strcat (class_str, ".VT100.BackarrowKey");
    cc = XrmGetResource (db, name_str, class_str, &str_type, &value);
    if ( cc )
	backarrow_flg = ( strcasecmp (value.addr, "false") != 0 );
#ifdef __linux__
    XrmDestroyDatabase (db);
#endif

    /* get the emulator command and options */
    if ( XGetTextProperty (dpy, wind, &tp, XA_WM_COMMAND) ) {
	nb = tp.nitems;
	/* for the time be no option define the Backarrow state */
    }
    return backarrow_flg;
}
#endif /* -NOX11 */


#ifndef NOX11


void xopen_alarm (int val)
{
    longjmp (jmp_env, val);
}

/* get_Xdisplay : try to open a connection to X server, with time out protection */
void get_Xdisplay (char *str)
{
    static char dpy_msg_buf [128];

#ifdef SYSIII
    struct termio saved_termio, temp_termio;
    Flag termio_flg;
#endif

    int jmp_val;

    dpy = (Display *) NULL;
    if ( str ) {
	dpy_name = malloc (strlen (str) +1);
	if ( dpy_name ) strcpy (dpy_name, str);
	else dpy_name = (char *) unknow_dpy_name;
    } else {
	dpy_msg = "X11 not called : \"DISPLAY\" variable is not defined\n";
	dpy_name = (char *) undef_dpy_name;
	Xterminal_flg = NO; /* no access to X11 */
	return;
    }
    dpy_msg = dpy_msg_buf;

    /* call XOpenDisplay with protection */
#ifdef SYSIII
    termio_flg = NO;
    if ( ioctl (STDIN, TCGETA, &saved_termio) >= 0 ) {
	temp_termio = saved_termio;
	temp_termio.c_lflag |= ISIG;
	(void) ioctl (STDIN, TCSETAW, &temp_termio);
	termio_flg = YES;
    }
#endif
    (void) signal (SIGALRM, xopen_alarm);
    (void) signal (SIGINT, xopen_alarm);
    alarm (2);
    if ( (jmp_val = setjmp (jmp_env)) == 0 ) {
	dpy = XOpenDisplay (NULL);
	alarm (0);
	if ( dpy ) dpy_msg = NULL;
	else sprintf (dpy_msg_buf, "X11 not used : X11 connection is refused to \"DISPLAY\" %s\n", str);
    } else {
	/* XOpenDisplay interrupted */
	alarm (0);
	if ( dpy ) XCloseDisplay (dpy);
	dpy = (Display *) NULL;
	if ( str ) sprintf (dpy_msg_buf, "X11 not used : %s during connection to \"DISPLAY\" %s\n",\
			    (jmp_val == SIGINT) ? "interrupted" : "time out", str);
    }
    (void) signal (SIGALRM, SIG_IGN);
    (void) signal (SIGINT, SIG_IGN);

#ifdef SYSIII
    if ( termio_flg ) {
	(void) ioctl (STDIN, TCSETAW, &saved_termio);
    }
#endif
    Xterminal_flg = (dpy != (Display *) NULL);
}
#endif /* -NOX11 */



static void terminal_type (char *terminal)
{
    char tnm [64];
    int cc;
    char *str, *str1;
    long int lival;
    Flag overwrite;
    int nchildren;

#ifndef NOX11
    Window wind, root, parent, *children;
    XClassHint class_hints;
    XrmDatabase db;
#endif

    /* default */
    kbmap_type = no_map;
    use_XKB_flg = NO;
    x_terminal_class  == unknow;
    emul_name  = (char *) unknow_emul_name;
    emul_class = (char *) unknow_emul_class;
    max_shift_level = keysym_per_keycode = nb_kbmap;
    min_keycode = max_keycode = 0;
    dpy_name = Xvendor_name = (char *) unknow_dpy_name;
    dpy_msg = NULL;
    x_keyboard_map = (KeySym *) NULL;
#ifndef NOX11
    dpy = NULL;
#endif /* - NOX11 */

    /* By default we assume to be an X terminal emulator */
    Xterminal_flg = YES;

    if ( ! terminal ) return;

    if (   (strcasecmp  (terminal, "linux")   == 0)
	|| (strcasecmp  (terminal, "console") == 0) ) {
#ifdef __linux__
	/* It can be the linux console */
	Xterminal_flg = NO;
#endif
	return;
    }

    str = getenv ("DISPLAY");

#ifdef NOX11
    noX11flg = YES;
    Xterminal_flg = NO;
    if ( str ) dpy_name = str;
    return;

#else /* -NOX11 */

    if ( noX11flg ) {
	dpy_msg = "X11 is not called to get keyboard mapping\n";
	Xterminal_flg = NO; /* no access to X11 */
	return;
    }

    if ( ! str ) {
	dpy_msg = "X11 not called : \"DISPLAY\" variable is not define\n";
	Xterminal_flg = NO; /* no access to X11 */
	return;
    }

    get_Xdisplay (str);
    if ( ! dpy ) return;

    /* Running on a X server, set default terminal type */
    Xterminal_flg = YES;
    x_terminal_class  == xterm;
    max_shift_level = nb_kbmap;

    /* display name and vendor name */
    str = DisplayString (dpy);
    dpy_name = malloc (strlen (str) +1);
    if ( dpy_name ) strcpy (dpy_name, str);
    else dpy_name = (char *) unknow_dpy_name;

    str = XServerVendor (dpy);
    Xvendor_name = malloc (strlen (str) +1);
    Xvendor_name = malloc (strlen (str) +1);
    if ( Xvendor_name ) strcpy (Xvendor_name, str);
    else Xvendor_name = (char *) unknow_dpy_name;

    /* get the keyboard mapping */
    (void) XDisplayKeycodes (dpy, &min_keycode, &max_keycode);
    x_keyboard_map = XGetKeyboardMapping (dpy, min_keycode,
					  max_keycode - min_keycode +1,
					  &keysym_per_keycode);
    if ( ! x_keyboard_map ) {
	get_kbmap_strg = "X11 Keyboard map cannot be read";
	dpy_msg = "The X Keyboard mapping cannot be read\n";
	XCloseDisplay (dpy);
	dpy = NULL;
	return;
    }
    kbmap_type = X11_default;
    get_kbmap_strg = "Keyboard map got from X11 call";
    max_shift_level = (nb_kbmap < keysym_per_keycode) ? nb_kbmap : keysym_per_keycode;
    sort_keysym_desc ();

    /* Try to get more info on the X terminal emulator in use */
    str = getenv ("WINDOWID");
    if ( !str || !*str ) {
	XCloseDisplay (dpy);
	dpy = NULL;
	dpy_msg = "Define the environment variable \"WINDOWID\" with the window id\n  (using \"xwininfo\" or \"xprop\") for better result\n";
	return;
    }

    /* running with a XTerm familly terminal emulator */
    lival = strtol (str, &str1, 0);
    if ( *str1 ) {  /* invalid value */
	XCloseDisplay (dpy);
	dpy = NULL;
	return;
    }
    emul_window = (Window) lival;
    root = (Window) 0;
    cc = 0;
    for ( wind = emul_window ; wind != root ; wind = parent ) {
	cc = XGetClassHint (dpy, wind, &class_hints);
	if ( cc ) break;
	if ( ! XQueryTree (dpy, wind, &root, &parent, &children, &nchildren) ) break;
    }
    if ( cc ) {
	emul_name  = class_hints.res_name;
	emul_class = class_hints.res_class;
    }
    str = strchr (emul_class, ':');
    if ( str ) *str ='\0';

    /* try to get the keyboard keycode to key name mapping with X Keyboard extension */
    use_XKB_flg = CheckXKBExtention (dpy, NULL, NULL);
    if ( use_XKB_flg ) get_xkb_data (dpy);

    (void) get_terminal_class ();

    overwrite = YES;
    if ( x_terminal_class  == xterm ) {
	xterm_backarrow_flg = get_xterm_backarrow (dpy, wind, emul_name, emul_class);
	overwrite = ! xterm_backarrow_flg;  /* only if it is not the default */
    }
    if ( overwrite ) overwrite_ktdesc (emul_class);

    sort_keysym_desc ();
    XCloseDisplay (dpy);
    dpy = NULL;
    return;

#endif /* -NOX11 */
}

/* get_keyboard_map : load the key map according to the init escape sequence */
/* ------------------------------------------------------------------------- */

Flag get_keyboard_map (char *terminal, int strg_nb,
		       char *strg1, char *strg2, char *strg3,
		       char *strg4, char *strg5, char *strg6)
{
    int i, sz;
    Flag app_mode, alt_cursor_mode;
    char *strg[6];

    if ( strg_nb > (sizeof (strg) / sizeof (strg[0])) )
	strg_nb = (sizeof (strg) / sizeof (strg[0]));
    strg [0] = strg1;
    strg [1] = strg2;
    strg [2] = strg3;
    strg [3] = strg4;
    strg [4] = strg5;
    strg [5] = strg6;

    if ( init_msg ) return (Xterminal_flg);     /* already done */

    for ( sz = i = 0 ; i < strg_nb ; i++ )
	if ( strg[i] ) sz += strlen (strg[i]);
    if ( sz ) {
	init_msg = malloc (sz+1);
	if ( init_msg ) memset (init_msg, 0, sz+1);
    }
    if ( init_msg ) {
	for ( i = 0 ; i < strg_nb ; i++ )
	    if ( strg[i] ) strcat (init_msg, strg[i]);
    } else init_msg = "Not defined";

    app_mode = alt_cursor_mode = NO;    /* assume default mode */
    get_kb_map (terminal, init_msg, &app_mode, &alt_cursor_mode);
    set_term_mode (app_mode, alt_cursor_mode, NO);
    return (Xterminal_flg);
}

/* get_keyboard_mode_strg : return the current keyboard mode */
/* --------------------------------------------------------- */

static void get_kbmode_strgs (char **appl_strg, char **curs_strg)
{
    *appl_strg = ( keypad_appl_mode ) ? "Application" : "Numeric";
    *curs_strg = ( cursor_alt_mode )  ? "Alternate"   : "Normal";
}

char * get_keyboard_mode_strg ()
{
    static char kbmode_strg [80];
    char *appl_strg, *curs_strg;

    get_kbmode_strgs (&appl_strg, &curs_strg);
    sprintf (kbmode_strg, "%s Key Pad, %s Cursor", appl_strg, curs_strg);
    return (kbmode_strg);
}

static char * get_key_bycode (int ktfunc)
{
    static char strg[128];
    int i, j;
    short val;

    memset (strg, 0, sizeof (strg));
    if ( ktfunc == kt_void ) return (strg);

    for ( i = 0 ; i < nb_key ; i++ ) {
	for ( j = 0 ; j < nb_kbmap ; j++ ) {
	    val = pc_keyboard_desc[i].ktfunc[j];
	    if ( val != ktfunc ) continue;
	    sprintf (&strg[strlen (strg)], "%s%s, ",
		     key_shift_msg[j], pc_keyboard_desc[i].klabel);
	}
    }
    return (strg);
}

/* get_ktfunc_by_string : while *i and *j >= 0 : can be more entry */
/*  return kt_void : string is existing but no KT function attached */

static int get_ktfunc_by_string (char *strg, int *i, int *j)
{
    char *sp;
    int ktfunc;
    struct KTdesc *ktpt;

    if ( ! inuse_ktdesc ) return (kt_void);

    ktfunc = kt_void;
    if ( (*i >= 0) && (*j >= 0) ) {
	for (  ; *i < inuse_ktdesc_nb ; (*i)++ ) {
	    if ( ! inuse_ktdesc [*i].ktdesc_array ) continue;
	    for ( ; *j < *(inuse_ktdesc [*i].ktdesc_nb) ; (*j)++ ) {
		ktpt = *(inuse_ktdesc [*i].ktdesc_array) + (*j);
		if ( ktpt->ktfunc == 0 ) continue;
		if ( ktpt->ktfunc == kt_void ) continue;
		sp = ktpt->strg;
		if ( !sp ) continue;
		if ( strcmp (sp, strg) == 0 ) {
		    ktfunc = ktpt->ktfunc;
		    return (ktfunc);
		}
	    }
	}
    }
    *i = *j = -1;
    return (ktfunc);
}


/* get_kt_strg : get the string for a given kt (or keysym) code */
/* ------------------------------------------------------------ */

static char *get_kt_strg (int ktcode, char **tcap_strg)
{
    char *sp;

    if ( tcap_strg ) *tcap_strg = NULL;
    if ( ktcode == kt_void ) return (NULL);

    sp = get_ktcode2escseq (ktcode, tcap_strg);
    return (sp);
}

/* ------------------------------------------------------------------------
  In Linux xterm, which is compiled with the OPT_VT52_MODE keys F1-F4
    the string generated for the keys F1-F4 are translated into
    the string for the keys PF1-PF4
    Ref to input.c file of xterm, routine : Input
  -------------------------------------------------------------------------
*/

/* xterm_terminal : check for xterm terminal type */
/* ---------------------------------------------- */

static Flag xterm_name ()
{
    if ( !Xterm_flg ) return (NO);
    return (strcasecmp (tname, "xterm") == 0);
}

static Flag xterm_terminal ()
{
    extern char *tname;

    if ( !Xterm_flg ) return (NO);
    if ( ! xterm_name () ) return NO;
    if ( emul_class && (strcasecmp (emul_class, "xterm") != 0) ) return NO;
    return YES;
}


/* overwrite_PF1PF4 : overwrite the cmd for PF1-PF4 with the cmd for F1-F4 */
/* ----------------------------------------------------------------------- */

void overwrite_PF1PF4 ()
{
    extern int itoverwrite ();

    int i, cc, nb;
    char **strg;
    unsigned char cmd;

    if ( ! xterm_terminal () ) return;

    for ( i = xterm_F1F4_keys_nb -1 ; i >= 0 ; i-- ) {
	cc = itoverwrite (xterm_F1F4_keys[i].Fstring, xterm_F1F4_keys[i].PFstring, ithead);
    }
}


/* reverse_xterm_F1F4 : reverse the F1-F4 to PF1-PF4 interpretation for xterm */
/* -------------------------------------------------------------------------- */

static void reverse_xterm_F1F4 (char *escp)
{
    int i;

    if ( ! xterm_terminal () ) return;

    for ( i = xterm_F1F4_keys_nb -1 ; i >= 0 ; i-- ) {
	if ( strcmp (xterm_F1F4_keys[i].PFstring, escp) != 0 ) continue;
	strcpy (escp, xterm_F1F4_keys[i].Fstring);
	return;
    }
}

/* String to key using XKB extension */
/* --------------------------------- */

/* must be in alphabetic order of name */
static struct _pretty_label {
	char *name;
	char *label;
    } pretty_label [] = {
	{ "DELE",   "Delete" },
	{ "INS",    "Insert" },
	{ "KPAD",   "KP-Add" },
	{ "KPDL",   "KP-Dot" },
	{ "KPDV",   "KP-Div" },
	{ "KPEN",   "KP-Enter"  },
	{ "KPMU",   "KP-Mul" },
	{ "KPSU",   "KP-Minus" },
	{ "NMLK",   "KP-NumLock" },
	{ "PAUS",   "Pause"  },
	{ "PGDN",   "PageDown" },
	{ "PGUP",   "PageUp" },
	{ "PRSC",   "PrintScreen" },
	{ "SCLK",   "ScrollLock" }
    };
#define pretty_label_nb (sizeof (pretty_label) / sizeof (pretty_label[0]))

#ifndef NOX11
static int compare_pretty_label (struct _pretty_label *prname1, struct _pretty_label *prname2)
{
    return (strcasecmp (prname1->name, prname2->name));
}


static char * pretty_key_label (char *keyname)
{
    static char keylabel [XkbKeyNameLength+16];
    int i;
    char *chpt;
    struct _pretty_label prname, *prname_pt;

    memset (keylabel, 0, sizeof (keylabel));
    memcpy (keylabel, keyname, XkbKeyNameLength);

    prname.name = keylabel;
    prname_pt = (struct _pretty_label *) bsearch (&prname,
		    pretty_label, pretty_label_nb,
		    sizeof (struct _pretty_label),
		    (int (*)()) compare_pretty_label);
    if ( prname_pt ) return (prname_pt->label);

    if ( strncmp (keylabel, "FK", 2) == 0 ) {
	i = ( keylabel[2] == '0' ) ? 2 : 1;
	keylabel[i] = 'F';
	return (&keylabel[i]);
    }
    if ( strncmp (keylabel, "KP", 2) == 0 ) {
	keylabel[3] = keylabel[2];
	keylabel[2] = '-';
	return (keylabel);
    }
    for ( chpt = keylabel +1 ; *chpt ; chpt++ ) *chpt = tolower (*chpt);
    return (keylabel);
}
#endif /* - NOX11 */

static char * keycode2keylabel (int keycode)
{
    char *str;
    int i;

    if ( (keycode < min_keycode) ||
	 (keycode > max_keycode) ) return NULL;

#ifndef NOXKB
    if ( use_XKB_flg && xkb_rec->names ) {
	/* X Keyboard extension available */
	str = pretty_key_label ((char *) &xkb_rec->names->keys [keycode]);
	return str;
    }
#endif /* - NOXKB */

    /* X Keyboard extension is not available */
    for ( i = 0 ; i < pc_keyboard_desc_nb ; i++ ) {
	if ( pc_keyboard_desc[i].kcode == keycode )
	    return (pc_keyboard_desc[i].klabel);
    }
    return "???";
}

/* Walk across keymap for all <keycde, shift> for a given keysym
 *  start with *keycode = 0
 *  end when return 0;
 */
static int keysymb2keycode (KeySym keysym, int *keycode, int *shift)
{
    int i, j;
    KeySym ksym;

    if ( keysym == NoSymbol ) return 0;
    if ( ! x_keyboard_map ) return 0;

    if ( *keycode < min_keycode ) {
	*keycode = min_keycode;
	*shift = -1;
    }
    j = *shift;
    i = *keycode - min_keycode;
    for ( ; i <= max_keycode ; i++ ) {
	for (  ; j < max_shift_level ; j++ ) {
	    ksym = x_keyboard_map [keysym_per_keycode*i +j];
	    if ( (ksym == NoSymbol) ) continue;
	    if ( ksym == keysym ) {
		*keycode = i + min_keycode;
		*shift = j;
		return *keycode;
	    }
	}
	j = 0;
    }
    return 0;
}

/* Walk across excape strings for all keysym
 *  start with *idx <= 0;
 *  end when return NoSymbol;
 */
static KeySym string2keysym (char *strg, int *idx)
{
    struct KTdesc *ktd;

    if ( !strg || !*strg ) return NoSymbol;
    if ( *idx < 0 ) *idx = 0;
    for ( ; *idx < Xkt_desc_pt_nb ; (*idx)++ ) {
	ktd = (*Xkt_desc_pt)[*idx];
	if ( (ktd->ktfunc == NoSymbol) ||
	     !ktd->strg ) continue;
	if ( strcmp (ktd->strg, strg) != 0 ) continue;
	return ktd->ktfunc ;
    }
    return NoSymbol;
}

/* string2key_label, x_string2key_label : return the key label
 * -----------------------------------------------------------
 *  Get the key and modifier for a given string (escape sequence)
 *  To start a new scan ; *keycode = -1
 *  Return key label and modifier string,
 *      or NULL if nothing or no more key
 */

static char * x_string2key_label (char *strg, int *keycode, int *idx,
				  int *shift, char **modstrg)
{
    KeySym keysym, ks;
    int kc;
    char *str;

    if ( !strg || !*strg ) return NULL;
    if ( *keycode < 0 ) *idx = *keycode = *shift = 0;
    else (*shift)++;
    for ( ks = NoSymbol ; ; (*idx)++ ) {
	keysym = string2keysym (strg, idx);
	if ( keysym == NoSymbol ) break;
	if ( keysym == ks ) continue;   /* to prevent duplication */
	ks = keysym;
	kc = keysymb2keycode (keysym, keycode, shift);
	if ( kc != 0 ) {
	    if ( modstrg ) *modstrg = key_shift_msg [*shift];
	    str = keycode2keylabel (*keycode);
	    return str;
	}
	*keycode = *shift = 0;
    }
    return NULL;
}


/* special processing for old xterm emulator */
static Flag nxterm_special_case (int ktf, char *strg)
{
    int i;
    char *strg1;

    if ( ! Xterminal_flg ) return (NO);

    /* try for special cursor case (in case of translation overrride) */
    for ( i = nxterm_special_nb ; i >= 0 ; i-- ) {
	if ( nxterm_special [i].ktfunc != ktf ) continue;
	if ( ! nxterm_special [i].Xktdesc_special_pt ) continue;
	strg1 = ( cursor_alt_mode )
		? nxterm_special [i].Xktdesc_special_pt->norstrg
		: nxterm_special [i].Xktdesc_special_pt->altstrg;
	if ( strcmp (strg, strg1) == 0 ) return (YES);
    }
    return (NO);
}


static char * string2key_label (char *strg, int *key, int *idx,
				int *shift, char **modstrg)
{
    static char * kcode2string (int, unsigned int, Flag, Flag, int *, char **, char **);

    int i, j;
    int ktf;
    char *ktstrg;
    char *klb, *shf;


    if ( *key < 0 ) {
	*key = pc_keyboard_desc [0].kcode;
	*shift = -1;
    }
    for ( i = 0 ; i < nb_key ; i++ ) {
	if ( *key == pc_keyboard_desc [i].kcode ) break;
    }
    j = *shift +1;
    if ( j >= nb_kbmap ) {
	i++ ; j = 0;
    }
    for ( ; i < nb_key ; i++ ) {
	for (  ; j < nb_kbmap ; j++ ) {
	    if ( keyboard_map[j] < 0 ) continue;
	    ktf = pc_keyboard_desc [i].ktfunc[j];
	    if ( ktf == kt_void ) continue;
	    ktstrg = kcode2string (pc_keyboard_desc [i].kcode, (unsigned int) j,
		     keypad_appl_mode, cursor_alt_mode,
		     &ktf, &klb, &shf);
	    if ( ! ktstrg ) continue;
	    if ( strcmp (strg, ktstrg) != 0 ) {
		if ( !Xterminal_flg || !nxterm_special_case (ktf, strg) ) continue;
	    }
	    if ( modstrg ) *modstrg = key_shift_msg [j];
	    *key = pc_keyboard_desc [i].kcode;
	    *shift = j;
	    *idx = i;
	    return (pc_keyboard_desc [i].klabel);
	}
	j = 0;
    }
    if ( i >= nb_key ) *key = -1;
    return (NULL);
}


/* all_string_to_key : all keys label for a given escape sequence */
/* -------------------------------------------------------------- */
/*  The buffer (buf) must be large enough to be split into "nbkbmap"
	(the value "nbkbmap" provided by the caller can be reduce
	to the best value for the usage of the buffer),
	substrings which will receive the key labels for the
	corresponding modifier.
    On return keys is init with the pointer to label string of the modifiers,
	if no keys, the string is empty.

    Can be call more that one time without re-init of keys_pt to add
	keys label for various escape sequences. Between 2 of these calls
	nbkbmap, buf and buf_sz must not be modified.
	The first call is flag by NULL value in the keys (char *) array.
*/

void all_string_to_key (char *escp_seq,
	char *(*keys_pt) [],    /* pointer to array of nb_kbmap pointer to char */
	int  *keys_nb,          /* nb of members, return usable nb */
	char *buf, int buf_sz)  /* buffer for key label and its size */
{
    int i, idx, nb, sz;
    int nbkbmap, label_sz;
    int key_code, shift;
    Flag kfnd_flg [pc_keyboard_desc_nb][nb_kbmap];

    char *key_label, *modf_label;
    char **keys;

    if ( !get_map_flg ) return;
    if ( *keys_nb <= 0 ) return;    /* nothing can be done */

    keys = (char **) *keys_pt;
    if ( ! *keys ) {
	/* keys is NULL : initialisation to be done */
	memset (*keys_pt, 0, *keys_nb * sizeof((*keys_pt)[0]));
	memset (buf, 0, buf_sz);
	memset (kfnd_flg, 0, sizeof (kfnd_flg));

	/* get the best value for the number of usefull key maps */
	if ( Xterminal_flg ) nbkbmap = max_shift_level;
	else {
	    nbkbmap = *keys_nb;
	    if ( nbkbmap > nb_kbmap ) nbkbmap = nb_kbmap;
	    for ( i = nb = 0 ; i < nbkbmap ; i++ ) {
		if ( keyboard_map [i] < 0 ) continue;
		nb++;
	    }
	    if ( nb < nbkbmap ) nbkbmap = nb;   /* usefull number of map */
	}
	*keys_nb = nbkbmap;

	label_sz = buf_sz / nbkbmap;
	if ( label_sz < 16 ) return;    /* too small buffer */
	/* init the array of pointer to key label strings */
	for ( i = 0 ; i < nbkbmap ; i++ ) {
	    keys [i] = &buf [i*label_sz];
	}
    } else {
	nbkbmap = *keys_nb;
	label_sz = buf_sz / nbkbmap;
    }

    for ( key_code = shift = -1 ; ; ) {
	key_label = ( Xterminal_flg )
		    ? x_string2key_label (escp_seq, &key_code, &idx,
					  &shift, &modf_label)
		    :   string2key_label (escp_seq, &key_code, &idx,
					  &shift, &modf_label);
	if ( ! key_label ) break;
	if ( shift >= nbkbmap ) continue;
	if ( ! *key_label ) continue;
	if ( !keys[shift][0] && modf_label && *modf_label ) {
	    strcpy (keys[shift], modf_label);
	    sz = strlen (modf_label);
	    if ( modf_label [sz -1] == ' ' ) sz--;
	    strcpy (&keys[shift][sz], ": ");
	}

	if ( !Xterminal_flg )
	    if ( kfnd_flg [idx][shift] ) continue;  /* already catched */

	/* leave space at the end for an extra separator character */
	if ( (strlen (key_label) + strlen (keys[shift]) +2) < (label_sz -1) )
	    sprintf (&keys[shift][strlen (&keys[shift][0])], "%s ", key_label);

	if ( !Xterminal_flg ) kfnd_flg [idx][shift] = YES;
    }
}

/* kcode2string : get the string for a given key and console state */
/* --------------------------------------------------------------- */

static char * kcode2string (int kcode, unsigned int shift,
		     Flag applic_mode, Flag curs_mode,
		     int *ktcodept,
		     char **shift_strg, char **key_label)
{
    static char latin[2];
    int i;
    int ktcode;
    char *strg;

    if ( Xterminal_flg ) return NULL;

    *shift_strg = *key_label = NULL;
    *ktcodept = kt_void;
    if ( (shift >= nb_kbmap) && (keyboard_map [shift] < 0) )
	return (NULL);
    *shift_strg = key_shift_msg [shift];
    for ( i = nb_key-1 ; i >= 0 ; i-- ) {
	if ( pc_keyboard_desc [i].kcode == kcode ) break;
    }
    if ( i < 0 ) return (NULL);     /* no key descriptor */

    *key_label = pc_keyboard_desc [i].klabel;
    ktcode = pc_keyboard_desc [i].ktfunc[shift];
    if ( ktcode == kt_void ) return (NULL);  /* nothing assigned to this key */

    strg = get_ktcode2strg (ktcode, NULL, shift);
    *ktcodept = ktcode;
    return (strg);
}

/* checkkeyb : interactive check for keyboard state */
/* ------------------------------------------------ */

static void checkkeyb (Flag echo)
{
    static void print_keys (char *escp, Flag *nl_pt);
    static Flag build_escape_seq (char ch, char *escp, int idx);

    static char msg[] = "type \"Ctrl C\" exit, \"Ctrl A\" switch App mode, \"Ctrl B\" switch cusor mode\n";
    static char int_msg[] = " Interrupted control sequence\n";
    Flag app_mode, alt_cursor_mode;
    unsigned char ch, escp[32];
    int idx, sz, tmode, retval;
    char *st, *st1;
    Flag nl, esc_seq;
    struct timeval tv, *tv_pt;
    fd_set rfds;

    fputs (msg, stdout); nl = YES;
    sz = sizeof (escp);
    esc_seq = NO;
    for ( idx = 0 ; ch != '\003' ; idx++ ) {  /* exit on <Ctrl>C */
	if ( idx >= (sz -1) ) idx = sz -2;  /* must never appear */

	if ( ! esc_seq ) {
	    idx = 0; memset (escp, 0, sz);
	    tv_pt = NULL;
	} else {
	    /* arm the escape sequence read time-out */
	    tv.tv_sec = 0;
	    tv.tv_usec = 100000;
	    tv_pt = &tv;
	}

	FD_ZERO(&rfds);
	FD_SET(0, &rfds);   /* read from stdin */
	retval = select (1, &rfds, NULL, NULL, tv_pt);
	if ( retval ) (void) read (0, &ch, 1);
	else ch = '\n';

	if ( (idx == 0) && (ch == '\033') ) {
	    esc_seq = YES;
	    escp [idx] = ch;
	    continue;
	}
	if ( esc_seq ) {
	    if ( ! build_escape_seq (ch, escp, idx) ) continue;
	    esc_seq = NO;
	    /* a sequence is completed, display it */
	    reverse_xterm_F1F4 (escp);
	    print_keys (escp, &nl);
	    if ( ch == '\033' ) {
		/* interrupted sequence */
		if (echo) {
		    fputs (escseqstrg (escp, NO), stdout);
		    fputs (int_msg, stdout); nl = YES;
		}
	    }
	    continue;
	}
	if ( (ch == '\r') || (ch == '\n') ) {
	    /* new line */
	    if ( echo ) {
		if ( !nl ) putchar ('\n');
		fputs (msg, stdout); nl = YES;
	    }
	    continue;
	}

	if ( (ch== '\001') || (ch == '\002') ) {
	    /* change terminal mode */
	    if ( ch == '\001' ) {
		app_mode = ! keypad_appl_mode;
		tmode = ( app_mode ) ? 2 : 1;
	    }
	    else {
		alt_cursor_mode = ! cursor_alt_mode;
		tmode = ( alt_cursor_mode ) ? 4 : 3;
	    }
	    set_term_mode (app_mode, alt_cursor_mode, YES);
	    get_kbmode_strgs (&st, &st1);
	    printf ("%s = New terminal mode, send : \"%s\"\n--- new terminal mode : %s key pad, %s cursor ----\n",
		    init_seq[tmode].ref,
		    escseqstrg (init_seq[tmode].ref, NO),
		    st, st1);
	    nl = YES;
	    continue;
	} else {
	    if ( (ch == '\t') || (ch == '\177') || (ch == '\b') ) {
		escp [idx] = ch;
		print_keys (escp, &nl);
		continue;
	    }
	    else if ( echo ) {
		printf(" '\\%03o'", ch);
		if ( ch < ' ' ) {
		    putchar ('^'); putchar (ch+'@');
		} else putchar (ch);
	    nl = NO;
	    fflush (stdout);
	    }
	}
    }
}

/* Build a message for status like command */

void term_msg (char *buf)
{
#ifdef NOX11
    strcat (buf, "Rand Editor is not build with X11 library\n");
    if ( ! xterm_name () )
	return;

    strcat (buf, "It is assumed to be a X-Terminal emulator\n");
    sprintf (buf + strlen (buf), "X-Terminal \"%s\"", tname);

    if ( strcmp (dpy_name, unknow_dpy_name) != 0 ) {
	sprintf (buf + strlen (buf), " on X11-Server DISPLAY \"%s\"\n", dpy_name);
    }
    strcat (buf, "\n");

#endif
    if ( ! dpy_msg ) return;
    strcat (buf, dpy_msg);
    if ( noX_msg ) {
	if ( buf[strlen (buf) -1] == '\n' ) buf[strlen (buf) -1] = '\0';
	strcat (buf, " : ");
	strcat (buf, noX_msg);
    }
    if ( buf[strlen (buf) -1] != '\n' ) strcat(buf, "\n");
}

void xterm_msg (char *buf)
{
#ifdef NOX11
    term_msg (buf);
#else /* - NOX11 */
    if ( ! x_keyboard_map ) {
	strcat (buf, "It is assumed to be a X-Terminal emulator\n");
	if ( dpy_msg ) strcat (buf, dpy_msg);
    }
    sprintf (buf + strlen (buf), "X-Terminal \"%s\" on X11-Server DISPLAY \"%s\"\n",
	     tname, dpy_name);
    sprintf (buf + strlen (buf), "  by vendor \"%s\"\n", Xvendor_name);
    sprintf (buf + strlen (buf), "  Terminal emulator name \"%s\", class \"%s\"\n",
	     emul_name, emul_class);
    if ( x_keyboard_map && dpy_msg ) strcat (buf, dpy_msg);
    xkb_info (buf + strlen (buf));
#endif /* - NOX11 */
}

void set_noX_msg (char * msg) {
    if ( msg ) noX_msg = msg;
}


#ifndef TEST_PROGRAM
/* check_keyboard : interactive check fo keyboard state */
/* ---------------------------------------------------- */

void check_keyboard (Flag charset_flg, Flag gr_flg)
{

static char xmapping_warning [] = "\n\
    If the mapping seem to be strange, check the xmodemap mapping\n\
    and you can use xkeycaps or xev to see if the mapping is 'regular' :\n\
	F1 mapped to F1 key ...\n\n\
";
static char nomapping_warning [] = "\n\
    I do not know anything about the keyboard keys to escape sequences\n\
    mapping; for better results you can capture or update the mapping with\n\
    the \"build-kbmap\" command.\n\n\
";

    extern char *TI, *KS, *VS;
    extern void put_graph_char ();
    extern char * graph_char_txt ();

    char *st, *st1, *st2, buf [1024];
    int applmod, cursmod;
    int i, i0, i1;
    Flag g_flg, utf_flg;

#ifdef CHAR7BITS
    st2 = "ANSII (7 bits)";
    i1 = 0176;
#else
    st2 = "ISO 8859 (8 bits)";
    i1 = 0377;
#endif

    /* save the initial terminal mode */
    applmod = keypad_appl_mode;
    cursmod = cursor_alt_mode;

    if ( (kbmap_type == no_map) && !charset_flg )
	fputs (nomapping_warning, stdout);
    set_term_mode (applmod, cursmod, NO);

    g_flg = gr_flg;
    if ( ! charset_flg ) {
	g_flg = NO;
	get_kbmode_strgs (&st, &st1);
	printf ("\n--- terminal mode : %s key pad, %s cursor ----\n", st, st1);
	printf ("    Initialisation string : %s\n", escseqstrg (init_msg, NO));
	printf ("    <kbinit> (from kbfile) : %s\n", escseqstrg (kbinistr, NO));
	if ( TI && *TI ) printf ("    <ti> (from terminfo) : %s\n\n", escseqstrg (TI, NO));
	if ( KS && *KS ) printf ("    <ks> (from terminfo) : %s\n\n", escseqstrg (KS, NO));
	if ( VS && *VS ) printf ("    <vs> (from terminfo) : %s\n\n", escseqstrg (VS, NO));
    }
    if ( g_flg ) {
	st1 = graph_char_txt (&utf_flg);
	printf ("\nGraphical character set used for window edges from  :\n%s\n", st1);
    } else {
	printf ("Character set to display text : %s\n", st2);
    }
    printf ("\n\
    .    .   0...............1...............\n\
    hexa .   0123456789ABCDEF0123456789ABCDEF\n\
    .    .   0.......1.......2.......3.......\n\
    .    oct 01234567012345670123456701234567\n\
    .    .   ................................");

    i0 = (g_flg && utf_flg) ? 0 : ' ';
    for ( i = i0 ; i <= i1 ; i++ ) {
	if ( (!g_flg || !utf_flg) && (i == 0200) ) i = 0237;
	else {
	    if ( (i & 0037) == 0 ) printf ("\n    0x%02X %03o ", i, i);
	    put_graph_char (i, g_flg);
	}
    }
    putchar ('\n');
    putchar ('\n');

    if ( charset_flg ) {
	set_term_mode (applmod, cursmod, YES);
	/* clear any input char */
	while ( getkey (PEEK_KEY) != NOCHAR ) {
	    keyused = YES;
	    (void) getkey (WAIT_KEY);
	}
	return;
    }

    if ( Xterminal_flg ) {
	xterm_msg (buf + strlen (buf));
	fputs (buf, stdout);
    }
    if ( Xterm_flg ) {
	if ( (kbmap_type == X11_default) || (kbmap_type == X11_xmodmap) )
	    fputs (xmapping_warning, stdout);
	if ( xterm_terminal () )
	    printf ("    Warning : for XTerm Emulator key PF1 .. PF4 are mapped to F1 .. F4\n\n");
    }

    checkkeyb (YES);
    /* restaure terminal mode */
    set_term_mode (applmod, cursmod, YES);
}
#endif /* - TEST_PROGRAM */

char * escseqstrg (char *escst, Flag kbfile_format)
{
    static char st[128];
    char *sp, *str, ch;
    int i;
    Flag quote;

    quote = NO;
    memset (st, 0, sizeof (st));
    if ( !escst || !escst[0] ) return st;

    str = st;
    for ( sp = escst ; (ch = *sp) ; sp++ ) {
	str = &st[strlen(st)];
	if ( isprint (ch) ) {
	    if ( kbfile_format && !quote ) *str++ = '"';
	    quote = YES;
	    *str++ = ch;
	    continue;
	}
	if ( ! kbfile_format ) {
	    for ( i = ch_symb_nb -1 ; i >= 0 ; i-- ) {
		if ( ch != ch_symb [i].ch ) continue;
		strcpy (str, ch_symb [i].symb);
		if ( ch_symb [i].symb1 ) strcat (str, ch_symb [i].symb1);
		str = &st[strlen(st)];
		break;
	    }
	    if ( i >= 0 ) continue;
	} else if ( ch == '\177' ) {
	    if ( quote ) *str++ = '"';
	    quote = NO;
	    strcpy (str, "<0177>");
	    continue;
	}
	if ( ch < ' ' ) {
	    if ( kbfile_format && quote ) *str++ = '"';
	    quote = NO;
	    *str++ = '^';
	    *str++ = '@' + ch;
	}
    }
    if ( kbfile_format && quote ) *str++ = '"';
    return (st);
}


static void print_keys (char *escp, Flag *nl_pt)
{
    int i, sz;
    char label_buf [nb_kbmap * 128];
    char *labels_strgs [nb_kbmap];
    int labels_strgs_nb;

    if ( ! *nl_pt ) putchar ('\n');
    printf ("\"%s\" by: ", escseqstrg (escp, NO));

    labels_strgs_nb = sizeof (labels_strgs) / sizeof (labels_strgs [0]);
    memset (labels_strgs, 0, sizeof (labels_strgs));
    all_string_to_key (escp, &labels_strgs, &labels_strgs_nb,
		       label_buf, sizeof (label_buf));
    for ( i = 0 ; i < labels_strgs_nb ; i++ ) {
	if ( labels_strgs [i] &&  labels_strgs [i][0] ) {
	    sz = strlen (labels_strgs [i]);
	    if ( labels_strgs [i][sz -1] == ' ' ) {
		labels_strgs [i][sz -1] = ';';
		labels_strgs [i][sz] = ' ';
	    }
	    fputs (labels_strgs [i], stdout);
	}
    }
    putchar ('\n');
    *nl_pt = YES;
}

static void assess_term_mode (char *escp)
{
#ifdef __linux__
    int i;
    char *st, *st1;
    Flag cur_flg;

    if ( ! (assess_cursor_flg || assess_keypad_flg) ) return;

    if ( assess_cursor_flg ) {
	/* try to found cursor mode */
	cur_flg = NO;
	for ( i = 0 ; i < KT_CUR_nb ; i++ ) {
	    if ( strcmp (escp, kt_desc_cur_norm [i].strg) == 0 ) {
		assess_cursor_flg = NO;
		break;
	    }
	}
	if ( assess_cursor_flg ) {
	    cur_flg = YES;
	    for ( i = 0 ; i < KT_CUR_nb ; i++ ) {
		if ( strcmp (escp, kt_desc_cur_alt [i].strg) == 0 ) {
		    assess_cursor_flg = NO;
		    break;
		}
	    }
	}
	if ( ! assess_cursor_flg ) cursor_alt_mode = cur_flg;
    }
    if ( assess_keypad_flg ) {
	/* try to found key pad mode (using KP-5 key) */
	assess_keypad_flg = NO;     /* assume it will be found */
	if ( strcmp (escp, kp5_trans_num_strg) == 0 ) keypad_appl_mode = NO;
	else if ( strcmp (escp, kp5_trans_appl_strg) == 0 ) keypad_appl_mode = YES;
	else if ( strcmp (escp, kp5_trans_appl1_strg) == 0 ) keypad_appl_mode = YES;
	else     assess_keypad_flg = YES;
    }
    if ( ! (assess_cursor_flg || assess_keypad_flg) ) {
	set_term_mode (keypad_appl_mode, cursor_alt_mode, NO);

	get_kbmode_strgs (&st, &st1);
	printf ("\n--- assessed terminal mode : %s key pad, %s cursor ----\n\n", st, st1);
    }
#endif /* __linux__ */
}

#ifndef TEST_PROGRAM

static int sort_looktb (S_looktbl *obj1, S_looktbl *obj2)
{
    char *p1, *p2;
    int cmp;

    p1 = ( isalpha (obj1->str[0]) ) ? obj1->str : &obj1->str[1];
    p2 = ( isalpha (obj2->str[0]) ) ? obj2->str : &obj2->str[1];
    cmp = strcmp (p1, p2);
    if ( cmp == 0 ) cmp = strcmp (obj1->str, obj2->str);
    return (cmp);
}

static void looktbl_print (S_looktbl *tbl, char *line_pt, int idx)
{
    (void) sprintf (line_pt + 2, "%3d %-9s ", tbl[idx].val, tbl[idx].str);
}

static char * symbs_strg (unsigned char ch)
{
    extern char * itsyms_by_val (short);
    char *sp;
    char str [2];

    sp = itsyms_by_val (ch);
    if ( ! sp ) {
	if ( isprint (ch) ) {
	    str [0] = ch;
	    str [1] = '\0';
	    sp = str;
	} else sp = "???";
    }
    return sp;
}

/* print_sorted : routine to print in columns a list of strings */

static void print_sorted (tbl, print_func, item_nb, nb_clm, nb_chr, nb_spc, my_out)
void *tbl;
void (*print_func)();
int item_nb;    /* number of items in sorted array */
int nb_clm;     /* number of columns */
int nb_chr;     /* nb of charcaters printed for each item */
int nb_spc;     /* number of spaces between each item */
FILE *my_out;

{
    char line[81];
    int lnb, lastln;
    int itemidx[16];
    int i, j, k, l, idx;
    int psiz;
    FILE *out;

    out = (my_out) ? my_out : stdout;
    if ( item_nb <= 0 ) return;
    psiz = nb_chr + nb_spc;
    if ( (nb_spc < 1) || (nb_clm > 16)
	 || ((nb_clm * psiz) >= sizeof (line)) ) {
	fputs ("\n--- Bad parameter to 'print_sorted' which results in a too long string ---\n", out);
	return;
    }

    lnb = item_nb / nb_clm;
    lastln = item_nb % nb_clm;
    for ( i = 0, k = 0 ; i < nb_clm ; i++ ) {
	itemidx[i] = k;
	k += ( i < lastln ) ? lnb +1 : lnb;
	}
    for ( i = 0 ; i <= lnb ; i ++ ) {
	memset (line, ' ', sizeof (line));
	l = (i == lnb) ? lastln : nb_clm;
	if ( l == 0 ) continue;
	for ( j = 0 ; j < l ; j ++ ) {
	    (* print_func) ((char *) tbl, &line[psiz*j], itemidx[j]);
	    itemidx[j] +=1;
	    }
	for ( idx = 0, k = sizeof (line) -1 ; k >= 0 ; k-- ) {
	    if ( line[k] == '\0' ) line[k] = ' ';
	    if ( (idx == 0) && (line[k] != ' ') ) idx = k+1;
	    }
	if ( idx == 0 ) idx = sizeof (line) -2;
	line[idx++] = '\n'; line[idx] = '\0';
	fputs (line, out);
	}
    return;
}

static void print_comment (FILE *out, char *cmt, char cch, Flag nl_flg)
{
    char str [128], str1 [128];
    int sz;

    sprintf (str, "%c %s\n", cch, cmt);
    memset (str1, '-', sizeof (str1));
    if ( cch ) {
	str1 [0] = cch;
	str1 [1] = ' ';
    }
    sz = strlen (str) -1;
    if ( sz >= sizeof (str1) -2) sz = sizeof (str1) -3;
    str1 [sz] = '\n';
    str1 [sz +1] = '\0';
    if ( nl_flg ) fputc ('\n', out);
    fputs (str, out);
    if ( out != stdout ) fputs (str1, out);
}

/* ctrl_asg_array : array of control character key functions assigned */
/*  The array is :
 *      2 control char : if it is a single Ctrl key, the second is null
 *      2 cmd char, the second one is CCUNAS1 if it is a single char command
 */
#define CTRL_asg_cel 4

static void ctrlkey_print (unsigned char *ctrl_asg, char *line_pt, int idx)
{
    int i;
    unsigned char ch, ch1;
    char *ctl, *sp;

    i = idx * CTRL_asg_cel;
    ch = ctrl_asg [i];
    ch1 = ctrl_asg [i+1];
    ctl = "Ctrl";
    if ( ch > 'Z' ) {
	if ( ch == '\177' ) ctl = "Del";
	else ctl = "???";
	(void) sprintf (line_pt, "<%s>    ", ctl, ch);
    } else {
	if ( ch1 == '\0' ) (void) sprintf (line_pt, "<%s %c> ", ctl, ch);
	else (void) sprintf (line_pt, "<%1$s %2$c><%1$s %3$c> ", ctl, ch, ch1);
    }
    strcat (line_pt, symbs_strg (ctrl_asg [i+2]));

    if ( ctrl_asg [i+3] != CCUNAS1 ) {
	strcat (line_pt, " ");
	strcat (line_pt, symbs_strg (ctrl_asg [i+3]));
    }
}

void output_ctrlkey (unsigned char *ctrl_asg, FILE * out)
{
    int i;
    unsigned char ch, ch1;

    for ( i = 0 ; (ch = ctrl_asg [i]) != '\0' ; i+= CTRL_asg_cel ) {
	if ( ch == CCUNAS1 ) break;
	ch1 = ctrl_asg [i+1];
	if ( ch > 'Z' ) {
	    if ( ch != '\177' ) continue;
	    fputs ("<0177>", out);
	} else {
	    if ( ch1 == '\0' ) fprintf (out, "^%c", ch);
	    else fprintf (out, "^%c^%c", ch, ch1);
	}
	fprintf(out, ":<%s>", symbs_strg (ctrl_asg [i+2]));
	if ( ctrl_asg [i+3] != CCUNAS1 )
	    fprintf(out, ":<%s>", symbs_strg (ctrl_asg [i+3]));
	putc ('\n', out);
    }
}

static int set_crtl_asg_cel (unsigned char *ctrl_asg_arr, int *idx_pt,
			     unsigned char *cmd, int *lnb_pt)
{
    extern char * itgetvalue (char * strg);

    int i, j, nb;
    unsigned char ctrl[4], ch, *val_pt;

    if ( ! idx_pt ) return 0;
    if ( !lnb_pt || *lnb_pt > 2 ) return 0;

    j = *lnb_pt;
    ctrl [j--] = '\0';
    for ( ; j >= 0 ; j-- ) {
	ch = cmd [j];
	ctrl [j] = (ch > '\037') ? ch : ch + '@';
    }

    val_pt = itgetvalue (cmd);
    if ( ! val_pt) return 0;

    i = *idx_pt * CTRL_asg_cel;
    ctrl_asg_arr [i++] = ctrl[0];
    ctrl_asg_arr [i++] = ctrl[1];
    ctrl_asg_arr [i++] = *val_pt;
    ctrl_asg_arr [i++] = CCUNAS1;
    (*idx_pt)++;
    return 1;
}

static int get_ctrl_asg (unsigned char ** ctrl_asg_pt, int *sz1,
			 unsigned char ** ctrl_asg_ext_pt, int* sz2)
{
    /* maxi is 32 single and 32 extended control assigment */
    static unsigned char ctrl_asg_arr [65*CTRL_asg_cel];
    static int ctrl_asg_arr_sz = 0;     /* number of control char assigned */
    static int ctrl_asg_simple_sz = 0;  /* number of simple control char */
    static int ctrl_asg_extend_sz = 0;  /* number of extended control char */

    unsigned char cmd[256];
    Flag free_flgs [32];    /* max 32 control char (^A ... ^Z) */
    int i, i0, i1, j, lnb, nb;

    if ( ctrl_asg_pt ) *ctrl_asg_pt = ctrl_asg_arr;
    if ( ctrl_asg_arr_sz <= 0 ) {
	/* not yet initialized */

	memset (cmd, 0, sizeof (cmd));
	memset (free_flgs, 0, sizeof (free_flgs));
	memset (ctrl_asg_arr, 0, sizeof (ctrl_asg_arr));
	ctrl_asg_arr_sz = i = 0;
	i0 = 'A' & '\037';
	i1 = 'Z' & '\037';
	for ( i = i0 ; i <= i1 ; i++ ) {
	    cmd[0] = i; cmd[1] = '\0';
	    lnb = 1;
	    nb = set_crtl_asg_cel (ctrl_asg_arr, &ctrl_asg_arr_sz, cmd, &lnb);
	    if ( nb <= 0 ) free_flgs [i] = YES;
	}
	/* add also delete char */
	cmd[0] = '\177'; cmd [1] = '\0';
	lnb = 1;
	nb = set_crtl_asg_cel (ctrl_asg_arr, &ctrl_asg_arr_sz, cmd, &lnb);
	ctrl_asg_simple_sz = ctrl_asg_arr_sz;

	/* extended control case (2 control chars) */
	for ( j = i1 ; j >= i0 ; j-- ) {
	    if ( ! free_flgs [j] ) continue;
	    for ( i = i0 ; i <= i1 ; i++ ) {
		cmd[0] = j; cmd [1] = i; cmd[2] = '\0';
		lnb = 2;
		nb = set_crtl_asg_cel (ctrl_asg_arr, &ctrl_asg_arr_sz, cmd, &lnb);
		if ( nb <= 0 ) continue;
		if ( ctrl_asg_arr_sz >= ((sizeof (ctrl_asg_arr) / CTRL_asg_cel) -1) )
		    break;
	    }
	}
	ctrl_asg_extend_sz = ctrl_asg_arr_sz - ctrl_asg_simple_sz;
    }

    if ( sz1 ) *sz1 = ctrl_asg_simple_sz;
    if ( ctrl_asg_ext_pt )
	*ctrl_asg_ext_pt = ctrl_asg_arr + (ctrl_asg_simple_sz * CTRL_asg_cel);
    if ( sz2 ) *sz2 = ctrl_asg_extend_sz;
    return ctrl_asg_arr_sz;
}

void all_ctrl_key_by_func (char *msg, int msg_sz, int fcmd)
{
    unsigned char * ctrl_asg, ch, ch1;
    int sz, i, i1, i2, sz1;

    sz = get_ctrl_asg (&ctrl_asg, NULL, NULL, NULL);
    i1 = sz * CTRL_asg_cel;
    for ( i = 0 ; i < i1 ; i += CTRL_asg_cel ) {
	if ( (ctrl_asg[i+2] == fcmd) || (ctrl_asg[i+3] == fcmd) ) {
	    i2 = strlen (msg);
	    sz1 = msg_sz - i2;
	    if ( sz1 <= 10 ) break;
	    ch = ctrl_asg[i];
	    ch1 = ctrl_asg[i+1];
	    if ( ch <= 'Z' )
		if ( ch1 == '\0' ) sprintf (&msg[i2], "<Ctrl %c>; ", ch);
		else sprintf (&msg[i2], "<Ctrl %c><Ctrl %c>; ", ch, ch1);
	    else strcat (msg, "<Del>; ");
	}
    }
}

/* print the Control characters assignement */

static void display_ctrl (FILE *kb_file, Flag cmt_format)
{
    unsigned char *ctrl_asg, *ctrl_asg_ext;
    char *comt;
    int sz, sz1, sz2;
    FILE *out;
    Flag format_flg;

    out = (kb_file) ? kb_file : stdout;
    format_flg = (kb_file) ? cmt_format : YES;

    /* get the assignement of control characters */
    sz = get_ctrl_asg (&ctrl_asg, &sz1, &ctrl_asg_ext, &sz2);
    comt = (format_flg) ? "" : "! ";
    print_comment (out, "Control Key assignement", comt[0], YES);
    if ( sz > 0 )
	if ( format_flg ) {
	    print_sorted ((void *) ctrl_asg, ctrlkey_print, sz1, 4, 19, 1, out);
	    if ( sz2 > 0 ) {
		fputc ('\n', out);
		print_sorted ((void *) ctrl_asg_ext, ctrlkey_print, sz2, 3, 25, 1, out);
	    }
	} else
	    output_ctrlkey (ctrl_asg, out);
    else
	fprintf (out, "\n%s    No Control key is assigned to key function", comt);
    fputc ('\n', out);
}

/* linux_keymap : display the current key map */
/* ------------------------------------------ */


static int new_page (int *nbnl, int min_size, FILE *out)
{
    int ctrlc;

    if ( out != stdout ) return NO;
    if ( verbose || ((*nbnl + min_size +2) <= term.tt_height) ) return NO;

    ctrlc = wait_keyboard ("\nPush a key to continue, <Ctrl C> to exit", NULL);
    fputs                 ("\r                                        \r", stdout);
    *nbnl = 1;
    return ctrlc;
}


static char * print1key_func (int *nbnl, int ktcode, int kcode, int shift,
			      char *klabel, char *strg, Flag skip,
			      int *ctrlc_pt, FILE *out, char *esc_strg)
{
    static char cmdstrg [64];
    int i, k, nb, cc, cmd, ctrlc;
    char strg1 [256], *sp, *esc, *tstr;
    Flag esc_flg;

    esc_flg = esc_strg && (out != stdout) && !verbose;
    esc = ( esc_flg ) ? esc_strg + strlen (esc_strg) : NULL;
    if ( ctrlc_pt ) *ctrlc_pt = NO;
    memset (cmdstrg, 0, sizeof (cmdstrg));
    if ( strg && (keyboard_map [shift] >= 0) ) {
	nb = strlen (strg);
	sp = strg;
	cc = itget (&sp, &nb, ithead, strg1);
	if ( cc > 2 ) cc = 2;
	for ( i = 0 ; cc > 0 ; cc--, i++ ) {
	    cmd = (unsigned char) strg1[i];
	    for ( k = 0 ; itsyms [k].str ; k++ ) {
		if ( itsyms [k].val != cmd ) continue;
		if ( *cmdstrg ) cmdstrg [strlen (cmdstrg)] = ' ';
		strcat (cmdstrg, itsyms [k].str);
		break;
	    }
	}
    }
    if ( !*cmdstrg && skip ) return NULL;

    if ( !strg ) strg = "";
    if ( shift == 0 ) {
	ctrlc = new_page (nbnl, 1, out);
	if ( ctrlc && !verbose && ctrlc_pt ) {
	    *ctrlc_pt = ctrlc;
	    return NULL;
	}

	tstr = escseqstrg (strg, YES);
	if ( Xterminal_flg ) {
	    if ( kcode != NoSymbol )
		fprintf (out, "\n%+13s %3d :", klabel, kcode);
	    else
		fprintf (out, "\n%+13s     :", klabel);
	    if ( esc ) sprintf (esc, "%+13s      ", tstr);
	}
	else {
	    fprintf (out, "\n%+14s :", klabel);
	    if ( esc ) sprintf (esc, "%+14s  ", tstr);
	}
	(*nbnl)++;
    }
    if ( verbose ) {
	if ( Xterminal_flg ) {
	    char strg1[24];
	    (void) get_ktcode2strg (ktcode, &sp, 0);
	    sprintf (strg1, "0x%4X %s", ktcode, (sp) ? sp : "");
	    fprintf (out, " ((%-19s) -> %-9s)", strg1, (strg) ? escseqstrg (strg, NO) : "null");
	} else
	    fprintf (out, " ((%d,%3d) -> %-9s)", (ktcode/256), (ktcode%256), (strg) ? escseqstrg (strg, NO) : "null");
    }
    if ( verbose ) fprintf (out, " %-9s, ", cmdstrg);
    else {
	fprintf (out, "%+10s, ", cmdstrg);
	if ( esc ) sprintf (esc, "%+10s  ", escseqstrg (strg, YES));
    }
    return *cmdstrg ? cmdstrg : NULL;
}


/* Display 1 key mapping (all level) */

static void display1Xkey (int keycode, char *keylabel, Flag skip,
			  int *nbnl, FILE *out, char *esc_strg)
{
    int j, ctrlc;
    char *strg, *cmdstrg;
    KeySym keysym, keysym_sh0;

    if ( ! x_keyboard_map ) return;

    for ( j = 0 ; j < max_shift_level ; j++ ) {
	keysym = x_keyboard_map [keysym_per_keycode * (keycode - min_keycode) +j];
	if ( keysym == XK_Escape && (j == 0) ) return;  /* do not display ESC key */
	/* special case : see also "get_xterm_map" */
	if ( j == 0 ) keysym_sh0 = keysym;  /* default keysym */
	else if ( keysym == NoSymbol ) keysym = keysym_sh0;
	strg = get_ktcode2strg (keysym, NULL, 0);
	cmdstrg = print1key_func (nbnl, keysym, keycode, j, keylabel,
				  strg, skip, &ctrlc, out, esc_strg);
	if ( ctrlc ) return;
	if ( !cmdstrg && (j == 0) && skip )
	    return;      /* do not print all Alpha section keys */
    }
}


/* Display the linux console (non X11) keyboard mapping,
 *  or for X terminal emulator if X Keyboard Extension (or geometry)
 *  is not available.
 */
static void display_keyboard_map (int *nbnl, FILE *out)
{
    Key_Assign *kap;
    int i, j, k, nb, keycode, ktcode, ctrlc;
    char *strg, *shift_strg, *klabel, *cmdstrg;
    char esc_strg [512];

    for ( i = 0 ; i < nb_key ; i++ ) {
	memset (esc_strg, 0, sizeof (esc_strg));
	for ( k = 0 ; k < pc101_keyboard_struct_nb ; k++ ) {
	    if ( pc101_keyboard_struct [k].idx != i ) continue;
	    ctrlc = new_page (nbnl, pc101_keyboard_struct [k].nb +4, out);
	    if ( ctrlc && !verbose ) return;

	    fprintf (out, "\n\n%-15s", pc101_keyboard_struct [k].label);
	    (*nbnl)++;
	    nb = Xterminal_flg ? max_shift_level : nb_kbmap;
	    for ( j = 0 ; j < nb ; j++ ) {
		if ( !Xterminal_flg && keyboard_map [j] < 0 ) continue;
		if ( verbose ) fprintf (out, "                %-7s         ", key_shift_msg [j]);
		else fprintf (out, "     %+7s", key_shift_msg [j]);
	    }
	    break;
	}
	kap = &pc_keyboard_desc[i];
	keycode = kap->kcode;
	if ( ! Xterminal_flg ) {
	    for ( j = 0 ; j < nb ; j++ ) {
		if ( keyboard_map [j] < 0 ) continue;
		strg = kcode2string (keycode, j,
				     keypad_appl_mode, cursor_alt_mode,
				     &ktcode, &shift_strg, &klabel);
		cmdstrg = print1key_func (nbnl, ktcode, keycode, j, klabel,
					  strg, NO, &ctrlc, out, esc_strg);
		if ( ctrlc ) return;
	    }
	} else {
	    klabel = pc_keyboard_desc [i].klabel;
	    display1Xkey (keycode, klabel, NO, nbnl, out, esc_strg);
	}
	if ( (out != stdout) && *esc_strg )
	    fprintf (out, "\n%-15s %s\n", "", esc_strg);
    }
    fputc ('\n', out);
}

#ifndef NOXKB
/* Display the X11 current terminal emulator keyboard mapping
 *   using XKB Extension (need geometry description)
 */
static void display_xkb_map (int *nbnl, FILE *out)
{
    Flag something;
    XkbGeometryPtr geom;
    XkbSectionPtr xsec;
    XkbRowPtr xrow;
    XkbKeyRec *xkey;
    KeySym keysym, keysym_l0;
    char esc_strg [512];

    char *strg, *cmdstrg;
    int i, j, si, ri, ki, kk, ctrlc;
    char *kname, *klabel;

    if ( ! use_XKB_flg ) return;
    geom = xkb_rec->geom;

    /* print the keys mapping */
    something = NO;
    for ( i = 0 ; i < num_section_names ; i++ ) {
	si = section_idx [i];
	if ( si < 0 ) continue;

	fputc ('\n', out);
	(*nbnl)++;
	ctrlc = new_page (nbnl, 1, out);
	if ( ctrlc && !verbose ) return;

	fprintf (out, "\n%-23s", section_names [i]);
	(*nbnl)++;
	for ( j = 0 ; j < max_shift_level ; j++ ) {
	    if ( verbose ) fprintf (out, "                %-7s         ", key_shift_msg [j]);
	    else fprintf (out, "%-7s      ", key_shift_msg [j]);
	}

	something = YES;
	xsec = &geom->sections[si];
	for ( ri = 0 ; ri < xsec->num_rows ; ri++ ) {
	    xrow = &xsec->rows[ri];
	    for ( ki = 0 ; ki < xrow->num_keys ; ki++ ) {
		xkey = &xrow->keys[ki];
		kname = xkey->name.name;
		kk = XkbFindKeycodeByName (xkb_rec, kname, True);
		if ( kk == NoSymbol ) continue;

		klabel = pretty_key_label (kname);
		memset (esc_strg, 0, sizeof (esc_strg));
		display1Xkey (kk, klabel, (i == (num_section_names -1)),
			      nbnl, out, esc_strg);
		if ( (out != stdout) && *esc_strg )
		    fprintf (out, "\n%-15s    %s\n", "", esc_strg);
	    }
	}
    }
    fputc ('\n', out);
}
#endif /* - NOXKB */

/* Return the string about the way in which the keyboard mapping was got */
char * mapping_strg (char ** str)
{
    if ( str ) *str = kbmap_fname;
    return ( get_kbmap_strg ? get_kbmap_strg : "No Keyboard map can be read");
}



static void print_keymap (Flag prg_verbose, int lnb, FILE *kb_file)
{
    static const char nomap_strg [] = "\
No keyboard map is available in this configuration.\
";
    static const char nomap_strg1 [] = "\n\
  For a better result you can capture or update the map file for your\n\
  keyboard with the \"build-kbmap\" command.\n\
";

    int i, j, k, idx, nbnl;
    Key_Assign *kap;
    int ktcode, cc, nb, cmd;
    int ctrlc;
    char *shift_strg, *klabel;
    char *strg, strg1 [256], *cmdstrg, *cmdstrg0, *sp;
    char buf [1024];
    int sz;
    char *str, *str1, *str2;
    FILE *out;

    out = (kb_file) ? kb_file : stdout;

    ctrlc = 0;
    nbnl = lnb;
    str1 = mapping_strg (&str2);
    verbose = prg_verbose;
    memset (buf, 0, sizeof (buf));

    if ( kbmap_type == no_map ) {
	fprintf (out, "Terminal \"%s\" (value of TERM environment variable)\n", terminal_name);
	fputs (get_kbmap_strg ? get_kbmap_strg : nomap_strg, out);
	fputs (nomap_strg1, out);
	str = get_kbmap_file_name (NO, NULL);
	if ( !str || !*str ) str = "??? not defined !!!";
	fprintf (out, "\"%s\"\n  is the expected keyboard map file\n", str);
	if ( ! verbose ) fputs ("\n\n\n", out);
	return;
    }

    /* a keyboard mapping is available */
    switch ( kbmap_type ) {
	case user_mapfile :
	    fprintf (out, "\n-- \"%s\" terminal is assumed to have a PC-101 style keyboard --\n    In mode : %s\n", terminal_name, get_keyboard_mode_strg ());
	    nbnl +=2;
	    sprintf (&buf[strlen(buf)], "  %s\n", str1);
	    nbnl++;
	    if ( str2 ) {
		sprintf (&buf[strlen(buf)], "  %s\n", str2);
		nbnl++;
	    }
	    break;

	case linux_console :
	    fprintf (out, "\n-- \"%s\" PC-101 style keyboard map, (%s) --\n", terminal_name, get_keyboard_mode_strg ());
	    nbnl++;
	    sprintf (&buf[strlen(buf)], "  %s\n", str1);
	    nbnl++;
	    if ( str2 ) {
		sprintf (&buf[strlen(buf)], "  %s\n", str2);
		nbnl++;
	    }
	    break;

#ifndef NOX11
	case X11_default :
	case X11_KBExt :
	case X11_xmodmap :
	    sprintf (buf, "X-Terminal \"%s\" keyboard map\n", emul_class);
	    nbnl++;
	    sprintf (&buf[strlen(buf)], "  %s\n", str1);
	    nbnl++;
	    if ( str2 ) {
		sprintf (&buf[strlen(buf)], "  %s\n", str2);
		nbnl++;
	    }

	    if ( ! x_keyboard_map ) {
		strcat (buf, "  Warning : keyboard mapping is not available !!\n");
		nbnl++;
	    }
	    if ( use_XKB_flg && !xkb_ext_msg ) {
		sprintf (&buf[strlen(buf)], "    \"%s, %s, %s", keycodes_str, geometry_str,
			 symbols_str);
		if ( phys_symbols_str ) sprintf (&buf[strlen(buf)], ", engraved %s)\"\n",
						 phys_symbols_str);
		else sprintf (&buf[strlen(buf)], "\"\n");
		nbnl++;
#ifndef NOXKB
		if ( xkb_rec && xkb_rec->geom )
		    strcat (buf, "    Using X Keyboard Extension geometry description.\n");
		else
		    strcat (buf, "    X Keyboard Extension does not provide geometry description.\n");
		nbnl++;
#endif /* - NOXKB */
	    } else if ( xkb_ext_msg ) {
		strcat (buf, xkb_ext_msg);
		buf [strlen (buf)] = '\n';
		nbnl++;
	    }
	    break;
#endif /* - NOX11 */

	default :
	    ;
    }
    if ( buf[0] ) fputs (buf, out);

    if ( verbose ) {
	/* complementary info for verbose output */
	fprintf (out, "\n  %s", str1);
	switch ( kbmap_type ) {
	    case user_mapfile :
		fprintf (out, " \"%s\"\n", get_kbmap_strg, str2);
		nbnl += 1;
		break;

	    case linux_console :
		fputs ("\n  Keyboard mapping is normaly defined by a call to \"loadkeys\" during startup.", out);
		nbnl += 1;
		fputs ("\n  Ref \"The Linux Keyboard and Console HOWTO\" for more info.", out);
		nbnl += 1;
		break;

	    default :
		fputc ('\n', out);
		nbnl += 1;
	}

	fprintf (out, "\n    Initialisation string : %s", escseqstrg (init_msg, NO));
	fprintf (out, "\n    For each key : key code (ref /usr/include/linux/keyboard.h),\n                  Escape Sequence, Rand key function");
	nbnl += 2;
    }

    /* now display the mapping */
    switch ( kbmap_type ) {
	case user_mapfile :
	case linux_console :
	    display_keyboard_map (&nbnl, out);
	    break;

#ifndef NOX11
#ifndef NOXKB
	case X11_KBExt :
	    if ( xkb_rec->geom ) {
		display_xkb_map (&nbnl, out);
		break;
	    }
#endif /* - NOXKB */
	case X11_default :
	case X11_xmodmap :
	    display_keyboard_map (&nbnl, out);
	    break;
#endif /* - NOX11 */

	default :
	    return;     /* nothing can be done */
    }
}

/* Generate a template kb file for the current terminal */

static int sort_stbl (S_looktbl **stbl1, S_looktbl **stbl2)
{
    char *st1, *st2;
    char c1, c2;
    int v;

    c1 = c2 = '\0';
    st1 = (*stbl1)->str;
    st2 = (*stbl2)->str;
    c1 = st1[0];
    c2 = st2[0];
    if ( ! isalpha (c1) ) st1++;
    if ( ! isalpha (c2) ) st2++;
    v = strcmp (st1, st2);
    if ( v == 0 ) v = c1 - c2;
    return v;
}

static void print_it_str (int fcmd, FILE *out)
{
    extern char * itsyms_by_val (short);
    extern int it_value_to_string (int, Flag, char *, int);
    int nb;
    char *str, *str1;
    char buf [128];

    memset (buf, 0, sizeof (buf));
    str = itsyms_by_val (fcmd);
    nb = 1;
    if ( fcmd == KBINIT ) strcpy (buf, kbinistr);
    else if ( fcmd == KBEND ) strcpy (buf, kbendstr);
    else nb = it_value_to_string (fcmd, NO, buf, sizeof (buf));
    if ( nb ) {
	str1 = strchr (buf, ' ');
	if ( str1 ) *str1 = '\0';
	fprintf (out, "<%s>:%s\n", str, escseqstrg (buf, YES));
    } else fprintf (out, "! <%s>: Not defined\n", str);
}


static void generate_kbfile (Flag prg_verbose, char *my_kbf_name)
{
    extern time_t time ();
    extern void display_xlate (char *, FILE *);
    extern void display_miscsymbs (FILE *);
    extern int key_by_func (int, char *, int, char **, char **, int,
		 char *, int, Flag, Flag);
    extern char * get_directive (unsigned int);
    extern char * itsyms_by_val (short);
    extern char * get_new_kbfile_name (Flag, Flag *);
    void display_keymap (Flag);
    extern char *myHost;
    extern Flag noX11flg;
    extern char *kbfile;

    static char default_new_kbfile [] = "/tmp/mykb";

    static char intro [] = "! File generated by \"build-kbfile\" command\n";
    static char separ [] = "! ================================================================\n";
    static char cmd_msg [128];

    FILE * termkb_file;
    int i, i0, j, nb, escp_nb, fcmd;
    char *str, *kbf_name;
    S_looktbl **stbl;
    char ch, buf [256];
    char *escp_str [16], *escp_labels [16];
    char labels [128];
    time_t strttime;
    Flag kbf_exist;

    memset (cmd_msg, 0, sizeof (cmd_msg));

    if ( my_kbf_name && *my_kbf_name ) {
	kbf_name = my_kbf_name;
    } else {
	kbf_name = get_new_kbfile_name (YES, &kbf_exist);
	if ( !kbf_name ) kbf_name = default_new_kbfile;
    }
    termkb_file = fopen (kbf_name, "w");
    if ( ! termkb_file ) {
	sprintf (cmd_msg, "File \"%s\" error : %s", kbf_name, strerror (errno));
	command_msg = cmd_msg;
	return;
    }

    (void) time (&strttime);
    str = asctime (localtime (&strttime));
    str [strlen(str) -1] = '\0';
    str [11] = '\0';
    fputs (separ, termkb_file);
    fputs (intro, termkb_file);
    fprintf (termkb_file, "! Session by \"%s\" at %s%s on \"%s\"\n!\n",
	     myname, str, &str[20], myHost);
    if ( kbfile ) {
	fprintf (termkb_file, "! Initial keyboard definition file :\n!    %s\n!\n", kbfile);
    }
    if ( Xterminal_flg )
	fputs ("!   look at vt200kbn file for recomanded X terminal setting\n", termkb_file);
    fputs (separ, termkb_file);

    fputc ('\n', termkb_file);
    fputs (get_directive (1), termkb_file);
    fputs (  " --- Keyboard mapping overview -----------\n\n", termkb_file);
    print_keymap (prg_verbose, 0, termkb_file);
    display_ctrl (termkb_file, YES);
    fputc ('\n', termkb_file);
    fputs (get_directive (2), termkb_file);
    fputs (" ----------------------------------------\n\n", termkb_file);

    /* #include directive */
    print_comment (termkb_file, "Major kb file directives", '!', YES);
    fputs ("\n! #include <vt200kbn>\n", termkb_file);

    /* NoX11 directive */
    if ( ! noX11flg ) fputs ("! ", termkb_file);
    fputs (get_directive (0), termkb_file);
    fputc ('\n', termkb_file);

    /* terminal initialisation and end escape sequence */
    print_comment (termkb_file, "Terminal initialisation and exit escape sequences", '!', YES);
    print_it_str (KBINIT, termkb_file);
    print_it_str (KBEND, termkb_file);

    /* sort the list of key functions */
    for ( i0 = 0 ; (str = itsyms[i0].str) != NULL ; i0++ ) ;
    stbl = (S_looktbl **) calloc (i0, sizeof (S_looktbl));
    if ( ! stbl ) return;
    for ( i = 0 ; i < i0 ; i++ ) stbl [i] = &(itsyms[i]);
    qsort (stbl, i0, sizeof (S_looktbl **),
	   (int (*)(const void *,const void *)) sort_stbl);

    print_comment (termkb_file, "Keyboard keys function assignement", '!', YES);
    escp_nb = sizeof (escp_str) / sizeof (escp_str [0]);
    for ( i = 0 ; i < i0 ; i++ ) {
	fcmd = stbl[i]->val;
	if ( (fcmd == KBINIT) || (fcmd == KBEND) ) continue;
	nb = key_by_func (fcmd, buf, sizeof (buf),
			  escp_str, escp_labels, escp_nb,
			  labels, sizeof (labels), NO, YES);
	if ( nb ) {
	    fputc ('\n', termkb_file);
	    for ( j = 0 ; j < nb ; j++ ) {
		ch = *(escp_str [j]);
		if ( (ch >= ('A' & '\037')) && (ch <= ('Z' & '\037')) || (ch == '\177') ) continue;
		fprintf (termkb_file, "! %s\n", escp_labels[j] ? escp_labels[j] : "???");
		fprintf (termkb_file, "%s:<%s>\n",
			 escseqstrg (escp_str [j], YES), stbl[i]->str);
	    }
	}
	else fprintf (termkb_file, "\n! :<%s> Not assigned\n", stbl[i]->str);
    }

    display_ctrl (termkb_file, NO);
    display_miscsymbs (termkb_file);
    display_xlate (tname, termkb_file);
    fputs (separ, termkb_file);

    fclose (termkb_file);
    sprintf (cmd_msg, "Kb file \"%s\" build", kbf_name);
    command_msg = cmd_msg;
}

/* Display the current keyboard mapping
 *   verbose flag must be used only for "-help -verbose" option case
 */

void display_keymap (Flag prg_verbose)
{
    extern void display_xlate (char *, FILE *);

    int ctrlc, gk;
    char *str1, *str2;
    int sz, nbnl;
    S_looktbl *keyftable;

    ctrlc = 0;
    str1 = mapping_strg (&str2);
    verbose = prg_verbose;

    /* display the header */
    if ( verbose ) fputc ('\n', stdout);
    else fputs ("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n", stdout);
    nbnl = 1;

    print_keymap (prg_verbose, nbnl, NULL);
    if ( ! verbose ) {
	ctrlc = wait_keyboard ("Push a key to continue with Control keys, <Ctrl C> to exit", NULL);
	fputs (              "\r                                                          ", stdout);
	nbnl = 0;
	if ( ctrlc ) return;
    }

    /* display control key mapping */
    display_ctrl (NULL, YES);

    /* display the border char set */
    if ( ! verbose ) {
	ctrlc = wait_keyboard ("Push a key to continue with Border character set, <Ctrl C> to exit", NULL);
	fputs (              "\r                                                                  ", stdout);
	if ( ctrlc ) return;
    }
    display_xlate (tname, NULL);

    if ( ! verbose ) {
	ctrlc = wait_keyboard ("Push 'Y' to generate a kb file template, else return to edition session", &gk);
	if ( ctrlc || !((gk == 'y') || (gk == 'Y')) )
	    return;

	generate_kbfile (prg_verbose, NULL);
	return;
    }

    /* "verbose" case : display the defined Key Functions */
    for ( sz = 0 ; itsyms [sz].str ; sz++ ) ;
    keyftable = (S_looktbl *) calloc (sz, sizeof (keyftable[0]));
    if ( keyftable ) {
	memcpy (keyftable, itsyms, sizeof (keyftable[0]) * sz);
	qsort (keyftable, sz, sizeof (keyftable[0]),
	       (int (*)(const void *,const void *)) sort_looktb);
	printf ("Defined (%d) Rand Key Functions\n", sz);
	print_sorted (keyftable, looktbl_print, sz, 5, 14, 1, NULL);
	putchar ('\n');
	free (keyftable);
    }

}

/* buildkbfile : command to interactively build a mapping file */
/* ----------------------------------------------------------- */

static Flag build_escape_seq (char ch, char *escp, int idx)
{
    /* return YES when a sequence is completed */

    if ( (ch < ' ') || (ch == '\177') )
	return YES;

    if ( idx == 1 ) {
	if ( (ch == '[') || (ch == 'O') ) {
	    escp [idx] = ch;
	    return NO;
	} else if ( (ch >= 'A') && (ch <= 'D') ) {
	    /* VT50 cursor control key */
	    escp [idx] = ch;
	    return YES;
	} else return YES;
    }
    escp [idx] = ch;
    if ( (ch != '~') && !isalpha (ch) ) return NO;
    return YES;
}

/* global data for display of key map */
/* ---------------------------------- */

static char PC101_style [] = "It is assumed to be a PC-101 style keyboard";

#define TL_X 2
#define TL_Y 4;
#define INC_X 13

/* top left corner and delta x for levels */
static int tl_x, tl_y, inc_x;
/* position for the current key message */
static int kstr_x, kstr_y;

/* empty key label */
static char empty_kstrg [64];

static char get_key_strg (char *escp, int sz)
{
    int i, j;
    char ch;
    Flag esc_seq;

    memset (escp, 0, sz);
    esc_seq = NO;
    for ( i = 0, ch = '\0' ; ch != '\003' ; i++ ) {  /* exit on <Ctrl>C */
	if ( i >= (sz -1) ) i = sz -2;
	ch = (char) (getchar () & 0177);
	if ( (i == 0) && (ch == '\033') ) {
	    esc_seq = YES;
	    escp [i] = ch;
	    continue;
	}
	if ( esc_seq ) {
	    if ( build_escape_seq (ch, escp, i) )
		return '\n';
	} else if ( ch == ' ' ) {
	    escp [0] = '\0';    /* erase */
	    break;
	} else {
	    if ( (ch == '\n') || (ch == '\r') ) break;
	    escp [i] = ch;
	    if ( (i == 0) && (ch == ' ') ) return '\n';
	    if ( (ch < ' ') || (ch == '\177') ) break;
	    if ( i == 0 ) {
		/* erase the key value string */
		fputs (empty_kstrg, stdout);
		for ( j = strlen (empty_kstrg) ; j > 0; j-- )
		    (*term.tt_left) (); /* back-space */
	    }
	    fputc (ch, stdout); fflush (stdout);
	}
    }
    return ch;
}

static int store_escp (char * escp, char *alt_escseq, int sz,
		       int *alt_escseq_idx, int idx, int lv)
{
    int i;
    char ch, *str;

    /* remove heading space char */
    for ( str = escp ; *str == ' ' ; str++ ) ;
    ch = *str;
    if ( !ch || isprint (ch) ) return -1;

    for ( i = 0 ; i < *alt_escseq_idx ;  ) {
	if ( strcmp (str, &alt_escseq [i]) == 0 ) break;
	i += strlen (&alt_escseq [i]) +1;
    }
    if ( i >= *alt_escseq_idx ) {
	/* not found */
	if ( (i + strlen (str) +2) >= sz ) return -1; /* no more room */
	strcpy (&alt_escseq [i], str);
	*alt_escseq_idx += strlen (&alt_escseq [i]) +1;
    }
    return i;
}

static char * print_escp (short *esc, char *alt_escseq, int lv)
{
    int i, j;
    char *str;

    for ( j = 0 ; j <= lv ; j++ ) {
	i = esc [j];
	str = ( i >= 0 ) ? escseqstrg (&alt_escseq [i], NO) : "";
	printf ("%-10s , ", str);
    }
}

static void display_level_name (int lv, Flag inv_video)
{
    (*term.tt_addr)  (tl_y -1, tl_x + (lv * inc_x) +2);
    (*term.tt_video) (inv_video);
    fputs (key_shift_msg [lv], stdout);
    (*term.tt_video) (NO);
    fflush (stdout);
}

static void display_strg (char *str, Flag inv_video)
{
    (*term.tt_video) (inv_video);
    if ( str ) fputs (str, stdout);
    (*term.tt_video) (NO);
}

static void display_key_name (int idx, int idx1, Flag inv_video)
{
    char strg [64], *str, *str_k;
    int px;

    memset (strg, ' ', sizeof (strg));
    str_k = pc_keyboard_desc [idx].klabel;
    px = tl_x - strlen (str_k) - 2;
    if ( px < 0 ) px = 0;
    strg [px] = '\0';
    (*term.tt_addr) (tl_y + (idx - idx1), 0);
    fputs (strg, stdout);
    display_strg (str_k, inv_video);
    fputs (" =", stdout);
    fflush (stdout);
}

static void display_key_value (int idx, int idx1, int lv, Flag inv_video)
{
    char strg [64], *escp, *str1, *str2;
    int i, px;

    memset (strg, ' ', sizeof (strg));
    strg [sizeof (strg) -1] = '\0';
    i = alt_esc_idx[idx][lv];
    if ( i < 0 ) {
	strg [inc_x -2] = '\0';
	str1 = empty_kstrg;
	str2 = "";
    } else {
	escp = escseqstrg (&alt_escseq [i], NO);
	px = inc_x - strlen (escp) -2;
	if ( px < 0 ) px = 0;
	strg [px] = '\0';
	str1 = escp;
	str2 = strg;
    }
    (*term.tt_addr)  (tl_y + (idx - idx1), tl_x + (inc_x * lv) +1);
    display_strg (str1, inv_video);
    printf ("%s,", str2);
    fflush (stdout);
}

static void display_section (int sec, int max_lv,
			     char * top_msg, char * wait_msg)
{
    int lv, nb, idx, idx1, idx2;
    char *str;

    idx1 = pc101_keyboard_struct [sec].idx;
    idx2 = idx1 + pc101_keyboard_struct [sec].nb -1;

    /* write the section header */
    ( *term.tt_clear ) ();
    ( *term.tt_home ) ();
    if ( top_msg ) {
	fputs (top_msg, stdout);
	fputc ('\n', stdout);
    }
    kstr_y = 1;
    (*term.tt_addr)  (kstr_y, 0);
    kstr_x = printf ("Capture the mapping for \"%s\" : depress ",
		     pc101_keyboard_struct [sec].label);

    for ( lv = 0 ; lv <= max_lv ; lv++ ) {
	display_level_name (lv, NO);
    }

    for ( idx = idx1 ; idx <= idx2 ; idx++ ) {
	display_key_name (idx, idx1, NO);
    }

    if ( wait_msg ) {
	for ( nb = 0, str = wait_msg ; *str ; str++ ) if ( *str == '\n' ) nb++;
	(*term.tt_addr)  (term.tt_height -1 -nb, 0);
	fputs (wait_msg, stdout);
	fflush (stdout);
    }

    /* display the curent key values */
    for ( lv = 0 ; lv <= max_lv ; lv++ ) {
	for ( idx = idx1 ; idx <= idx2 ; idx ++ ) {
	    display_key_value (idx, idx1, lv, NO);
	}
    }
}


static Flag get_kbmap ()
{
static char sec_msg [] = "\
<Ctrl C> abort capture, <Ctrl A> skip to next section, <Ctrl D> end capture\n\
<Ctrl B> previous key, <Return> next key, <Space> erase current value\
";
    int i, sz, sz1, iesc, sec, lv, max_lv, max_nb;
    int idx, idx0, idx1, idx2;
    char ch, escp [32];
    int sec_lv  [pc101_keyboard_struct_nb];
    int sec_idx [pc101_keyboard_struct_nb];

    max_nb = Xterminal_flg ? max_shift_level : nb_kbmap;
#ifdef __linux__
    if ( max_nb >= (nb_kbmap -1) ) max_nb = nb_kbmap -1;
#endif
    memset (sec_lv, 0, sizeof (sec_lv));
    for ( sec = 0 ; sec < pc101_keyboard_struct_nb ; sec++ )
	sec_idx [sec] = idx1 = pc101_keyboard_struct [sec].idx;

    for ( sec = 0 ; sec < pc101_keyboard_struct_nb ; sec++ ) {
	max_lv = max_nb -1;
	display_section (sec, max_lv, PC101_style, sec_msg);
	idx1 = pc101_keyboard_struct [sec].idx;
	idx2 = idx1 + pc101_keyboard_struct [sec].nb -1;
	lv = sec_lv  [sec];
	idx = sec_idx [sec];
	for ( ; lv <= max_lv ; lv++ ) {
	    sec_lv  [sec] = lv;
	    for ( ; idx <= idx2 ; idx++ ) {
		sec_idx [sec] = idx;
		display_level_name (lv, YES);
		display_key_name (idx, idx1, YES);
		display_key_value (idx, idx1, lv, YES);

		(*term.tt_video) (YES);
		(*term.tt_addr) (kstr_y, kstr_x);
		fputs (key_shift_msg [lv], stdout);
		fputs (pc_keyboard_desc [idx].klabel, stdout);
		(*term.tt_addr) (tl_y + (idx - idx1), tl_x + (inc_x * lv) +1);
		fflush (stdout);
		ch = get_key_strg (escp, sizeof (escp));
		(*term.tt_video) (NO);
		fflush (stdout);

		if ( ch == '\003' ) return NO;  /* <Ctrl C> : abort */
		if ( ch == '\004' ) return YES; /* <Crtl D> : done */
		if ( ch == '\001' ) break;  /* <Ctrl A> : skip to next section */
		if ( ch > '\004' ) {
		    if ( escp [0] ) {
			iesc = store_escp (escp, alt_escseq, sizeof (alt_escseq),
					   &alt_escseq_idx, idx, lv);
			if ( iesc >= 0 ) alt_esc_idx [idx][lv] = iesc;
		    } else if ( ch == ' ' ) {
			/* erased string */
			alt_esc_idx [idx][lv] = -1;
			idx--;  /* re-display the same key */
			continue;
		    }
		}
		(*term.tt_video) (NO);
		(*term.tt_addr) (kstr_y, kstr_x);
		(*term.tt_clreol) ();
		display_key_value (idx, idx1, lv, NO);
		display_level_name (lv, NO);
		display_key_name (idx, idx1, NO);
		if ( ch == '\002' ) idx -=2;    /* <Ctrl B> : previous */
		if ( idx < (idx1 -1) ) {
		    /* previous level last key */
		    lv -=2;
		    idx = idx2;
		    break;
		} else if ( idx >= idx2 ) {
		    /* next level first key */
		    idx = idx1;
		    break;
		}
	    }
	    if ( ch == '\001' ) break;  /* <Ctrl A> : skip to next section */
	    if ( lv < -1 ) lv = max_lv -1;
	    else if ( lv >= max_lv ) lv = -1;
	}
	if ( sec < -1 ) sec = pc101_keyboard_struct_nb -2;
	else if ( sec >= pc101_keyboard_struct_nb -1 ) sec = -1;
    }
    return YES;
}

static int dummy_video (Flag inverted)
{
    return 0;
}

static Cmdret process_buildkbmap ()
{
    extern char * tparm ();
    extern char *MR, *ME;
    extern char *myHost, *fromHost, *fromDisplay;

    static char cmd_msg [128];
    Flag cc, result;
    int idx, lv, sz, sz1, ctrlc;
    char * kfname, * old_kfname;
    char fname [PATH_MAX], *str;
    FILE * kfile;
    Flag kbf_exist;
    Kb_Mapping file_kbmap_type;  /* existing keyboard mapping file type */

    memset (cmd_msg, 0, sizeof (cmd_msg));
    if ( ! term.tt_video ) term.tt_video = dummy_video;

    for ( sz = 0, idx = 0 ; idx < pc_keyboard_desc_nb ; idx++ ) {
	sz1 = strlen (pc_keyboard_desc [idx].klabel);
	if ( sz1 > sz ) sz = sz1;
    }
    tl_x = sz +TL_X;
    tl_y = TL_Y;
    inc_x = INC_X;

    memset (empty_kstrg, ' ', sizeof (empty_kstrg));
    sz = inc_x -2;
    if ( sz >= sizeof (empty_kstrg) ) sz = sizeof (empty_kstrg) -1;
    empty_kstrg [sz] = '\0';

    memset (alt_esc_idx, -1, sizeof (alt_esc_idx));
    memset (alt_escseq, 0, sizeof (alt_escseq));
    alt_escseq_idx = 0;

    file_kbmap_type = get_kbmap_filetype (&kfname, &kbf_exist);
    if ( kbf_exist ) {
	/* load the current value */
	result = read_kbmap_file (kfname, YES);
    }

    cc = get_kbmap ();

    (*term.tt_video) (NO);
    ( *term.tt_clear ) ();
    ( *term.tt_home ) ();
    if ( cc ) {
	memset (fname, 0, sizeof (fname));
	if ( kbf_exist && (file_kbmap_type != user_mapfile) ) {
	    strcpy (fname, kfname);
	    strcat (fname, ".user");
	}

	printf ("Capture keyboard map for terminal \"%s\" ", tname);
	if ( fromHost ) printf (" on \"%s\" ", fromHost);
	printf ("well done.\nI will %s the keyboard map file :\n    \"%s\"\n",
		(*fname || !kbf_exist) ? "generate" : "update",
		 *fname ? fname : kfname);
	ctrlc = wait_keyboard ("<Ctrl C> : abort; any other key : save file", NULL);
	if ( ctrlc ) return CROK;

	/* save the new map file */
	old_kfname = NULL;
	if ( kbf_exist ) {
	    if ( file_kbmap_type == user_mapfile )
		old_kfname = rename_kbmapfile (kfname, file_kbmap_type);
	    else kfname = fname;
	}
	write_kbmap_file (kfname, user_mapfile, old_kfname, YES);

	if ( kbmap_type == user_mapfile ) {
	    /* update the current mapping */
	    memcpy (nor_escseq, alt_escseq, sizeof (alt_escseq));
	    memcpy (nor_esc_idx, alt_esc_idx, sizeof (alt_esc_idx));
	    for ( idx = 0 ; idx < pc_keyboard_desc_nb ; idx++ ) {
		for ( lv = 0 ; lv < nb_kbmap ; lv++ ) {
		    pc_keyboard_desc [idx].ktfunc [lv] = (*mykb_esc_idx) [idx][lv];
		}
	    }
	}
    }
    sprintf (cmd_msg, "Keymap file \"%s\" build", kfname);
    command_msg = cmd_msg;
    return CROK;
}

Cmdret buildkbfile (char *my_kbf_name)
{
    generate_kbfile (NO, my_kbf_name);
    return CROK;
}

Cmdret buildkbmap ()
{
    extern Cmdret do_fullscreen (Cmdret (*) (), void *, void *, void *);
    Cmdret cc;

    cc = do_fullscreen (process_buildkbmap, NULL, NULL, NULL);
    return cc;
}

#endif /* - TEST_PROGRAM */

/* ==================================================================== */

#ifdef TEST_PROGRAM

#ifndef __linux__
#error "keyboard stand-alone program can be build only on Linux system"
#endif

/* -------------------------------------------------------------------- */

/* Global variables for the autonomus keyboard test program */
/* -------------------------------------------------------- */

    Flag listing_flg, help_flg, acursor_flg, pkeypad_flg;
    Flag noinit_flg, mode_flg;

    static char def_init [] = "\033c";  /* reset */
    /* ------- for debugging of the init string parsing -----
    static char def_init [] = "\033c\033=\033[?1hTrikTrak\033c\033>\033[?1l";
    static char def_init [] = "\033=\033[2l";
    */

static char * init_strg = def_init;

static struct termios termiobf;


void prog_help (char *pname)
{
    int i;

    printf ("\
Synopsis : %s [option ...]\n\
    options :\n\
	-h   --help    : this on-line help\n\
	-l   --listing : listing of the keyboard mapping on standard output\n\
	-num --numkey  : set key pad in numeric mode\n\
	-app --appkey  : set key pad in application mode\n\
	-nor --norcur  : set in normal cursor mode\n\
	-alt --altcur  : set in alternate cursor mode\n\
	-n   --noinit  : do not set the terminal mode, use the current on\n\
	-init <init_string> : string to be used for terminal and keyboard init\n\
	    major init string :\n\
",
	    pname);

    for ( i = 0 ; i < init_seq_nb ; i++ )
	printf ("\
		%s : %s\n", init_seq[i].info, escseqstrg (init_seq[i].ref, NO));
    printf ("\
\n\
Program to assess the keyboard mapping of Function and Key Pad keys\n\
    %s\n\n",
	    version);
    exit (1);
}

static void reset_term ()
{
    /* reset keyboard input mode */
    (void) tcsetattr (0, TCSANOW, &termiobf);

    /* reset the terminal in default mode */
    fputs (init_seq [key_pad_num].ref, stdout);
    fputs (init_seq [cursor_norm].ref, stdout);
    (void) fflush (stdout);
}

static void intr_handler (int signb)
{
    if ( signb != SIGINT ) printf ("Ended by signal %d\n", signb);
    reset_term ();
    exit (0);
}

static void program_option (int argc, char *argv[])
{
    static char user_init [128];
    int i, j, sz, v;
    char ch, *strg;

    listing_flg = help_flg = acursor_flg = pkeypad_flg = noinit_flg = mode_flg = NO;
    assess_cursor_flg = assess_keypad_flg = NO;
    if ( argc <= 1 ) return;

    for ( i = 1 ; i < argc ; i++ ) {
	strg = argv [i];
	if (   (strcasecmp ("-h", strg) == 0)
	    || (strcasecmp ("--help", strg) == 0) ) {
	    help_flg = YES;
	} else if ( (strcasecmp ("-l", strg) == 0)
	    || (strcasecmp ("--listing", strg) == 0) ) {
	    listing_flg = YES;
	} else if ( (strcasecmp ("-alt", strg) == 0)
	    || (strcasecmp ("--altcur", strg) == 0) ) {
	    mode_flg = acursor_flg = YES;
	} else if ( (strcasecmp ("-nor", strg) == 0)
	    || (strcasecmp ("--norcur", strg) == 0) ) {
	    mode_flg = YES;
	    acursor_flg = NO;
	} else if ( (strcasecmp ("-num", strg) == 0)
	    || (strcasecmp ("--numkey", strg) == 0) ) {
	    mode_flg = YES;
	    pkeypad_flg = NO;
	} else if ( (strcasecmp ("-app", strg) == 0)
	    || (strcasecmp ("--appkey", strg) == 0) ) {
	    mode_flg = pkeypad_flg = YES;
	} else if ( (strcasecmp ("-n", strg) == 0)
	    || (strcasecmp ("--noinit", strg) == 0) ) {
	    mode_flg = noinit_flg = YES;
	} else if ( (strcasecmp ("-init", strg) == 0)
	    || (strcasecmp ("--init", strg) == 0) ) {
	    mode_flg = noinit_flg = NO;
	    if ( (i+1) < argc ) {
		memset (user_init, 0, sizeof (user_init));
		strg = argv[++i];
		for ( sz = 0 ; ch = *strg ; strg++ ) {
		    if ( ch == '\\' ) {
			ch = *(++strg);
			if ( ! ch ) {
			    printf ("%s : invalid init string\n", argv[i]);
			    prog_help (argv[0]);
			}
			if ( isdigit (ch) ) {
			    /* the check is not righ enough : '8' and '9' and taken ! */
			    sscanf (strg, "%o%", &v);
			    ch = v & 0177;
			    for ( ; isdigit (*strg) ; strg++ ) ;
			    strg--;
			}
			/*
			else if ( ch == 'a' ) ch = '\a';
			else if ( ch == 'b' ) ch = '\b';
			else if ( ch == 'f' ) ch = '\f';
			else if ( ch == 'n' ) ch = '\n';
			else if ( ch == 'r' ) ch = '\r';
			else if ( ch == 't' ) ch = '\t';
			else if ( ch == 'v' ) ch = '\v';
			else if ( ch == '\\' ) ch = '\\';
			*/
			else switch (ch) {
			    case 'a' :
				ch = '\a';
				break;
			    case 'b' :
				ch = '\b';
				break;
			    case 'f' :
				ch = '\f';
				break;
			    case 'n' :
				ch = '\n';
				break;
			    case 'r' :
				ch = '\r';
				break;
			    case 't' :
				ch = '\t';
				break;
			    case 'v' :
				ch = '\v';
				break;
			    case '\\' :
				ch = '\\';
				break;
			}
		    }
		    user_init [sz++] = ch;
		}
		init_strg = user_init;
	    } else {
		printf ("%s : missing initialisation string\n", strg);
		prog_help (argv[0]);
	    }
	} else {
	    printf ("Unknown option : %s\n", strg);
	    prog_help (argv[0]);
	}
    }
}

main (int argc , char *argv[])
{
    static user_init [64];

    int i, j;
    Key_Assign *kap;
    char *st, *st1;
    struct KTdesc *ktpt;
    int ktcode, tmode;
    Flag app_mode, alt_cursor_mode;
    char *shift_strg, *klabel, *terminal;
    char *strg, *terminal;
    int lflag;

    /* read the console driver state */
    (void) tcgetattr (0, &termiobf);
    lflag = termiobf.c_lflag;
    termiobf.c_lflag &= ~(ICANON);
    (void) tcsetattr (0, TCSANOW, &termiobf);
    termiobf.c_lflag = lflag;

    program_option (argc, argv);
    if ( help_flg ) prog_help (argv[0]);

    signal (SIGINT, intr_handler);

    if ( ! mode_flg ) {
	strg = init_strg;
	app_mode = alt_cursor_mode = NO;
    } else {
	strg = NULL;
	app_mode = pkeypad_flg;
	alt_cursor_mode = acursor_flg;
    }
    if ( strg ) {
	printf ("%s\nInit string: %s\n", strg, escseqstrg (strg, NO));
	}
    terminal = getenv ("TERM");
    get_kb_map (terminal, strg, &app_mode, &alt_cursor_mode);

    if ( mode_flg && noinit_flg ) {
	st = st1 = "?????";
	fputs ("\nTerminal mode is not set : use the current unknown mode.\n", stdout);
	fputs ("Push a \"Cursor Key\" and \"Key-Pad 5\" key to try to assess the current mode.\n", stdout);
	assess_cursor_flg = assess_keypad_flg = YES;
    } else {
	set_term_mode (app_mode, alt_cursor_mode, mode_flg);
	get_kbmode_strgs (&st, &st1);
    }
    printf ("\n--- %s terminal mode : %s key pad, %s cursor ----\n\n",
	    terminal, st, st1);

    if ( listing_flg ) {
	printf ("\n---- keyboard map ----\n");
	for ( i = 0 ; i < nb_key ; i++ ) {
	    kap = &pc_keyboard_desc[i];
	    for ( j = 0 ; j < nb_kbmap ; j++ ) {
		strg = kcode2string (kap->kcode, j, 1, 1, &ktcode, &shift_strg, &klabel);
		if ( j == 0 ) printf ("\nkcode %3d %+12s :", kap->kcode, klabel);
		if ( !strg ) continue;
		printf ( " %s 0x%03x, ",  shift_strg, ktcode);
		/*
		if ( kap->ktfunc[j] == -1 ) continue;
		printf ( " %s 0x%03x, ",  key_shift_msg[j], kap->ktfunc[j]);
		*/
	    }
	}

	printf ("\n\n---- Function strings ----\n");
	for ( i = 0 ; i < inuse_ktdesc_nb ; i++ ) {
	    printf ("  -- %d %s --\n", *(inuse_ktdesc [i].ktdesc_nb), inuse_ktdesc [i].type_name);
	    for ( j = 0 ; j < *(inuse_ktdesc [i].ktdesc_nb) ; j++ ) {
		ktpt = *(inuse_ktdesc [i].ktdesc_array) + j;
		if ( ktpt->ktfunc == 0 ) continue;
		if ( !ktpt->strg ) continue;
		st = escseqstrg (ktpt->strg, NO);
		st1 = get_key_bycode (ktpt->ktfunc);
		printf ("%#05x %+10s = %-10s : %s\n",
			ktpt->ktfunc, (ktpt->tcap) ? ktpt->tcap : "", st, st1);
	    }
	}
	printf ("\n--------\n");
    }

    else {      /* interractive usage */
	checkkeyb (NO);
    }
    reset_term ();
}
#endif /* TEST_PROGRAM */
/* -------------------------------------------------------------------- */
