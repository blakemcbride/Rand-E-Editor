#ifdef COMMENT
--------
file e.tt.c
    Terminal-dependent code and data declarations
#endif

#ifdef COMMENT
	Copyright abandoned, 1983, The Rand Corporation
#endif

#include "e.h"
#include "e.tt.h"
#include "e.sg.h"

/* the following is to save code space */
#ifndef BIGADDR
#undef putchar
#define putchar(c) fputc(c,stdout)
#endif

char   *kname;                  /* name of keyboard type */
char   *tname;                  /* name of terminal type */
Small   kbdtype;                /* which kind of keyboard */
Small   termtype;               /* which kind of terminal */
Short   screensize;             /* term.tt_width * term.tt_height */
Flag    fast;                   /* running at a fast baud rate (>= 4800) */
/* XXXXXXXXXXXXXXXXXXXXX */
#ifdef __linux__
int     ospeed;                 /* tty output baud rate */
#else
short   ospeed;                 /* tty output baud rate */
#endif
Flag    borderbullets = YES;    /* Enable bullets on borders */
S_term  term;
S_kbd   kbd;
Flag tt_lt2;            /* 2 lefts take fewer characters than an addr */
Flag tt_lt3;            /* 3 lefts take fewer characters than an addr */
Flag tt_rt2;            /* 2 rights take fewer characters than an addr */
Flag tt_rt3;            /* 3 rights take fewer characters than an addr */

#ifdef COMMENT
   TO ADD A TERMINAL TYPE:
	 Increment NTERMINALS by 1
	 Define a T_xx manifest constant for the terminal
	 Add the names of the terminal to the termnames table.
	 Add an include for 'term/xx.c' in this file.
	 Add the TM_xx entry to the "tterm" structure initialization
	   at the end of this file.
	 Add the KB_xx entry to the "tkbd" structure initialization
	   at the end of this file.
	 See term/Doc for more instructions.
	 Write a "term/xx.c" file for the terminal.
	 Add term/xx.c to Makefile dependencies

   Terminal #1 is the default if you are on a system without environment
   variables.

#endif

#ifdef TERMCAP
#define T_tcap      0   /* tcap.c       Termcap terminal -- must be 0 */
#endif
#define T_std       0   /* stdinlex.c   Standard keyboard */
/* fine T_aa        1    * annarbor.c   Ann Arbor (Models 4080 and Q2878) */
/* fine T_a1        2    * xl.c         Ann Arbor Ambassador and XLs */
/* fine T_3a        3    * adm3a.c      Lear Siegler ADM3a */
/* fine T_31        4    * adm31.c      Lear Siegler ADM31 */
/* fine T_dy        5    * dy.c         Dave Yost terminal */
/* fine T_dm4000    6    * dm4000.c     Datamedia DM4000 */
/* fine T_h19       7    * h19.c        Heathkit H19 & H89 */
/* fine T_intext    8    * intext.c     INTERACTIVE Systems Intext */
/* fine T_adm42     9    * adm42.c      Lear Siegler ADM42 */
/* fine T_c100      10   * c100.c       Concept 100 */
/* according to /etc/termcap there are 4 flavors.  I don't know what to do
   about that for now.  --dave yost
 **/
/* fine T_2intext   11   * intext2.c    INTERACTIVE Systems Intext2 */
/* fine T_po        12   * po.c         Perkin Elmer 1251 and Owl */
#define T_vt100     13  /* vt100.c      DEC VT100 -- keyboard only */
/* fine T_sun2      14   * sun2.c       Sun -- keyboard only */
/* fine T_vt52      15   * vt52.c       DEC VT52 */
/* fine T_mbee      16   * mbee.c       Beehive Microbee */
/* fine T_tv910     17   * tv910.c      Televideo 910 - keyboard only */
/* fine T_free100   18   * free100.c    Freedom 100 -- keyboard only */
/* fine T_wy50      19   * wy50.c       Wyse 50 -- keyboard only */

#define NTERMINALS  20

S_looktbl termnames[] = {
    /* must be sorted in ascending ascii order */

#ifdef FULL_TERMINAL_LIST
#ifdef T_31
    "31",           T_31,
#endif
#ifdef T_3a
    "3a",           T_3a,
#endif
#ifdef T_aa
    "aa",           T_aa,
    "aa0",          T_aa,
    "aa1",          T_aa,
#endif
#ifdef T_a1
    "aaa",          T_a1,
    "aaa-18",       T_a1,
    "aaa-20",       T_a1,
    "aaa-22",       T_a1,
    "aaa-24",       T_a1,
    "aaa-26",       T_a1,
    "aaa-28",       T_a1,
    "aaa-30",       T_a1,
    "aaa-36",       T_a1,
    "aaa-40",       T_a1,
    "aaa-48",       T_a1,
    "aaa-60",       T_a1,
    "aaas",         T_a1,
    "aaas-18",      T_a1,
    "aaas-20",      T_a1,
    "aaas-22",      T_a1,
    "aaas-24",      T_a1,
    "aaas-26",      T_a1,
    "aaas-28",      T_a1,
    "aaas-30",      T_a1,
    "aaas-36",      T_a1,
    "aaas-40",      T_a1,
    "aaas-48",      T_a1,
    "aaas-60",      T_a1,
#endif
#ifdef T_31
    "adm31",        T_31,
#endif
#ifdef T_3a
    "adm3a",        T_3a,
#endif
#ifdef T_adm42
    "adm42",        T_adm42,
#endif
#ifdef T_a1
    "ambas",        T_a1,
    "ambassador",   T_a1,
#endif
#ifdef T_aa
    "annarbor",     T_aa,
#endif
#ifdef T_c100
    "c100",         T_c100,
#endif
#ifdef T_vt100                  /* C. Itoh 101 - one in an apparently */
    "cit101",       T_vt100,    /* long series of vt100 lookalikes    */
    "cit101w",      T_vt100,    /* No reason to treat it as otherwise */
#endif
#ifdef T_c100
    "co",           T_c100,
    "concept",      T_c100,
    "concept100",   T_c100,
#endif
    "default",      1,
#ifdef T_dm4000
    "dm4000",       T_dm4000,
#endif
#ifdef T_vt100
    "dmx14",        T_vt100,    /* Datamedia Excel 14 - another vt100. */
    "dmx14w",       T_vt100,
#endif
#ifdef T_dy
    "dy",           T_dy,
#endif
#ifdef T_free100
    "fr100",        T_free100,
    "free100",      T_free100,
#endif
#ifdef T_h19
    "h19",          T_h19,
#endif
#ifdef T_intext
    "in",           T_intext,
    "intext",       T_intext,
#endif
#ifdef T_2intext
    "intext2",      T_2intext,
#endif
#ifdef T_h19
    "k1",           T_h19,
#endif
#ifdef T_31
    "l1",           T_31,
#endif
#ifdef T_adm42
    "l4",           T_adm42,
#endif
#ifdef T_3a
    "la",           T_3a,
#endif
#ifdef T_mbee
    "microb",       T_mbee,
#endif
#ifdef T_po
    "po",           T_po,
#endif
    "standard",     T_std,
#ifdef T_sun2
    "sun",          T_sun2,
#endif
#ifdef T_vt100
    "tab132",       T_vt100,            /* yet another vt100. */
    "tab132w",      T_vt100,
#endif
#ifdef T_tv910
    "tv910",        T_tv910,
#endif
#ifdef T_vt100
    "vt100",        T_vt100,            /* Its the real thing.  */
    "vt100w",       T_vt100,
#endif
#ifdef T_vt52
    "vt52",         T_vt52,
#endif
#ifdef T_wy50
    "wyse50",       T_wy50,
    "wyse50w",      T_wy50,
#endif

#else  /* short list : - FULL_TERMINAL_LIST */

    "standard",     T_std,
#ifdef T_vt100
    "vt100",        T_vt100,
#endif

#endif /* - FULL_TERMINAL_LIST */
    0,              0,
};

/*  This file and the following files have to work on both the current and
 *  the version, so I have to fix some stuff.
 **/
#ifdef  E14
#include "term/eoldfix.h"
#endif

#include "term/standard.c"

/* I did it this way, including everything into one compile, so that
   the optimizer pass could strip out lots of duplication with its
   clever optimization of common code before jumps.
   Unfortunately, for now at least, the vax optimizer doesn't understand
   that it can optimize common code before 'ret' instructions, so
   it's not too red hot on the vax.
   I think the 11 should do better, but I haven't tried it yet.
   1/20/81 dave-yost
  */

/* clude "term/stdinlex.c"    included in standard.c */
#define KB_std &kb_std

#ifdef T_tcap
# include "term/tcap.c"
# define TM_tcap &t_tcap
# define KB_tcap &kb_std
#else
# define TM_tcap ((S_term *) 0)
# define KB_tcap ((S_kbd *) 0)
#endif

#ifdef T_aa
# include "term/annarbor.c"
# define TM_aa &t_aa
# define KB_aa &kb_aa
#else
# define TM_aa ((S_term *) 0)
# define KB_aa ((S_kbd *) 0)
#endif

#ifdef T_a1
# include "term/xl.c"
# define TM_a1 &t_a1
# define KB_a1 &kb_a1
#else
# define TM_a1 ((S_term *) 0)
# define KB_a1 ((S_kbd *) 0)
#endif

#ifdef T_3a
# include "term/adm3a.c"
# define TM_3a &t_3a
# define KB_3a &kb_std
#else
# define TM_3a ((S_term *) 0)
# define KB_3a ((S_kbd *) 0)
#endif

#ifdef T_31
# include "term/adm31.c"
# define TM_31 &t_31
# define KB_31 &kb_std
#else
# define TM_31 ((S_term *) 0)
# define KB_31 ((S_kbd *) 0)
#endif

#ifdef T_dy
# include "term/dy.c"
# define TM_dy &t_dy
# define KB_dy &kb_std
#else
# define TM_dy ((S_term *) 0)
# define KB_dy ((S_kbd *) 0)
#endif

#ifdef T_dm4000
# include "term/dm4000.c"
# define TM_dm4000 &t_dm4000
# define KB_dm4000 &kb_std
#else
# define TM_dm4000 ((S_term *) 0)
# define KB_dm4000 ((S_kbd *) 0)
#endif

#ifdef T_free100
# include "term/free100.c"
# define TM_free100 &t_tcap
# define KB_free100 &kb_free100
#else
# define TM_free100 ((S_term *) 0)
# define KB_free100 ((S_kbd *) 0)
#endif

#ifdef T_h19
# include "term/h19.c"
# define TM_h19 &t_h19
# define KB_h19 &kb_h19
#else
# define TM_h19 ((S_term *) 0)
# define KB_h19 ((S_kbd *) 0)
#endif

#ifdef T_intext
# include "term/intext.c"
# define TM_intext &t_intext
# define KB_intext &kb_intext
#else
# define TM_intext ((S_term *) 0)
# define KB_intext ((S_kbd *) 0)
#endif

#ifdef T_2intext
# include "term/intext2.c"
# define TM_2intext &t_2intext
# define KB_2intext &kb_2intext
#else
# define TM_2intext ((S_term *) 0)
# define KB_2intext ((S_kbd *) 0)
#endif

#ifdef T_adm42
# include "term/adm42.c"
# define TM_adm42 &t_adm42
# define KB_adm42 &kb_adm42
#else
# define TM_adm42 ((S_term *) 0)
# define KB_adm42 ((S_kbd *) 0)
#endif

#ifdef T_c100
# include "term/c100.c"
# define TM_c100 &t_c100
# define KB_c100 &kb_c100
#else
# define TM_c100 ((S_term *) 0)
# define KB_c100 ((S_kbd *) 0)
#endif

#ifdef T_mbee
# include "term/mbee.c"
# define TM_mbee &t_mbee
# define KB_mbee &kb_mbee
#else
# define TM_mbee ((S_term *) 0)
# define KB_mbee ((S_kbd *) 0)
#endif

#ifdef T_po
# include "term/po.c"
# define TM_po &t_po
# define KB_po &kb_std
#else
# define TM_po ((S_term *) 0)
# define KB_po ((S_kbd *) 0)
#endif

#ifdef T_vt52
# include "term/vt52.c"
# define TM_vt52 &t_vt52
# define KB_vt52 &kb_vt52
#else
# define TM_vt52 ((S_term *) 0)
# define KB_vt52 ((S_kbd *) 0)
#endif

#ifdef T_tv910
# include "term/tv910.c"
# define TM_tv910 &t_tcap
# define KB_tv910 &kb_tv910
#else
# define TM_tv910 ((S_term *) 0)
# define KB_tv910 ((S_kbd *) 0)
#endif

#ifdef T_vt100
# include "term/vt100.c"
# define TM_vt100 &t_tcap
# define KB_vt100 &kb_vt100
#else
# define TM_vt100 ((S_term *) 0)
# define KB_vt100 ((S_kbd *) 0)
#endif

#ifdef T_sun2
# include "term/sun2.c"
# define TM_sun2 &t_tcap
# define KB_sun2 &kb_sun2
#else
# define TM_sun2 ((S_term *) 0)
# define KB_sun2 ((S_kbd *) 0)
#endif

#ifdef T_wy50
# include "term/wy50.c"
# define TM_wy50 &t_tcap
# define KB_wy50 &kb_wy50
#else
# define TM_wy50 ((S_term *) 0)
# define KB_wy50 ((S_kbd *) 0)
#endif

/* the entries in this structure must be in the order of their
   defined T_ values
  */
S_term *tterm[NTERMINALS] = {
    TM_tcap     ,
    TM_aa       ,
    TM_a1       ,
    TM_3a       ,
    TM_31       ,
    TM_dy       ,
    TM_dm4000   ,
    TM_h19      ,
    TM_intext   ,
    TM_adm42    ,
    TM_c100     ,
    TM_2intext  ,
    TM_po       ,
    TM_vt100    ,
    TM_sun2     ,
    TM_vt52     ,
    TM_mbee     ,
    TM_tv910    ,
    TM_free100  ,
    TM_wy50     ,
};

/* the entries in this structure must be in the order of their
   defined T_ values
  */
S_kbd *tkbd[NTERMINALS] = {
    KB_std      ,
    KB_aa       ,
    KB_a1       ,
    KB_3a       ,
    KB_31       ,
    KB_dy       ,
    KB_dm4000   ,
    KB_h19      ,
    KB_intext   ,
    KB_adm42    ,
    KB_c100     ,
    KB_2intext  ,
    KB_po       ,
    KB_vt100    ,
    KB_sun2     ,
    KB_vt52     ,
    KB_mbee     ,
    KB_tv910    ,
    KB_free100  ,
    KB_wy50     ,
};

