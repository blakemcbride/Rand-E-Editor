/*
 * file e.tt.h - terminal types header file
 *
 **/

#ifdef COMMENT
	Copyright abandoned, 1983, The Rand Corporation
#endif

#define C(c) ((c) & 31)

extern
char *kname;            /* name of keyboard type */
extern
char *tname;            /* name of terminal type */
extern
Small kbdtype;          /* which kind of keyboard */
extern
Small termtype;         /* which kind of terminal */
extern
Short screensize;       /* term.tt_width * term.tt_height */
extern
Flag    fast;           /* running at a fast baud rate (>= 4800) */
/* XXXXXXXXXXXXXXXXXXXXXXXXXXX */

#ifdef __linux__
extern
int     ospeed;                 /* tty output baud rate */
#else
extern
short   ospeed;                 /* tty output baud rate */
#endif

extern
Flag    borderbullets;  /* Enable bullets on borders */
extern
S_looktbl kbdnames[];   /* names of known keyboards */
extern
S_looktbl termnames[];  /* names of known terminals */
extern
Scols ocol;
extern
Slines olin;
extern
Scols icol;
extern
Slines ilin;

extern
Flag tt_lt2;            /* 2 lefts take fewer characters than an addr */
extern
Flag tt_lt3;            /* 3 lefts take fewer characters than an addr */
extern
Flag tt_rt2;            /* 2 rights take fewer characters than an addr */
extern
Flag tt_rt3;            /* 3 rights take fewer characters than an addr */

typedef struct kbd {
/*  extern */ int  (*kb_inlex  ) ();
/*  extern */ int  (*kb_init   ) ();
/*  extern */ int  (*kb_end    ) ();
} S_kbd;

typedef struct term {
/*  extern */ int  (*tt_ini0   ) ();
/*  extern */ int  (*tt_ini1   ) ();
/*  extern */ int  (*tt_end    ) ();
/*  extern */ int  (*tt_left   ) ();
/*  extern */ int  (*tt_right  ) ();
/*  extern */ int  (*tt_dn     ) ();
/*  extern */ int  (*tt_up     ) ();
/*  extern */ int  (*tt_cret   ) ();
/*  extern */ int  (*tt_nl     ) ();
/*  extern */ int  (*tt_clear  ) ();
/*  extern */ int  (*tt_home   ) ();
/*  extern */ int  (*tt_bsp    ) ();
/*  extern */ int  (*tt_addr   ) ();
/*  extern */ int  (*tt_lad    ) ();
/*  extern */ int  (*tt_cad    ) ();
/*  extern */ int  (*tt_xlate  ) ();
/*  extern */ int  (*tt_insline) ();
/*  extern */ int  (*tt_delline) ();
/*  extern */ int  (*tt_inschar) ();
/*  extern */ int  (*tt_delchar) ();
/*  extern */ int  (*tt_clreol ) ();
/*  extern */ int  (*tt_vscroll) ();
/*  extern */ int  (*tt_deflwin) ();
/*  extern */ int  (*tt_erase  ) ();
#ifdef LMCHELP
/*  extern */ int  (*tt_so     ) ();
/*  extern */ int  (*tt_soe    ) ();
/*  extern */ int  (*tt_help   ) ();
#endif
#ifdef LMCVBELL
/*  extern */ int  (*tt_vbell  ) ();
#endif
    char             tt_nleft;
    char             tt_nright;
    char             tt_ndn;
    char             tt_nup;
    char             tt_nnl;
    char             tt_nbsp;
    char             tt_naddr;
    char             tt_nlad;
    char             tt_ncad;
    char             tt_wl;
    char             tt_cwr;
    char             tt_pwr;
    char             tt_axis;
    char             tt_bullets;
    char             tt_prtok;
    short            tt_width;
    char             tt_height;

/*  extern */ int  (*tt_video) (Flag normal);
} S_term;

extern S_term term;
extern S_term *tterm[];
extern S_kbd  kbd;
extern S_kbd  *tkbd[];

/* output control characters for the "standard" windowing terminal */
/**/
#define VCCNUL 000 /* @ move the terminal cursor to where it belongs	*/
#define VCCINI 001 /* a initialize                                      */
#define VCCRES 002 /* b reset                                           */
#define VCCEND 003 /* c restore terminal                                */
#define VCCICL 004 /* c clear physical terminal screen                  */
#define VCCBEL 007 /* g bell						*/
#define VCCLEF 010 /* h left arrow					*/
#define VCCHOM 013 /* k cursor home within window			*/
#define VCCCLR 014 /* l clr current window				*/
#define VCCRET 015 /* m carriage return: moves cursor to left limit	*/
#define VCCUP  016 /* n up arrow					*/
#define VCCARG 021 /* q arg or qualification key			*/
#define VCCDWN 022 /* r down arrow					*/
#define VCCAAD 024 /* t absolute cursor address 			*/
#define VCCBKS 036 /* ^ backspace					*/
#define VCCRIT 037 /* _ right arrow					*/
/* fine VCCWIN 025  *   define window */
/* fine VCCINL 026  *   insert line */
/* fine VCCDLL 027  *   delete line */
