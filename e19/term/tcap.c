#ifdef COMMENT
Copyright abandoned, 1983, The Rand Corporation
#endif

/****************************************/
/**** 0 = terminal from termcap ****/

extern char *tgoto ();
extern Cmdret help_std ();

#ifdef LMCLDC
char *ldtab[NSPCHR];
/*
   **      Line Drawing Definitions used in Tcap.c to draw boxes
   **              (thanks to Eric Negaard@aerospace)
   **
   **      Gs - Introduce Line Drawing     Bj - Bottom join
   **      Ge - End Line Drawing           Tj - Top join
   **      Tl - Top left corner            Lj - Left join
   **      Tr - Top right corner           Rj - Right join
   **      Bl - Bottom left corner         Vl - Vertical line
   **      Br - Bottom right corner        Hl - Horizontal line
   **      Cj - Center join (cross)        Xc - Other unique character
 */
#endif

#undef UP
#undef HO

char *BC;			/* bc - cursor left                     */
char *ND;			/* nd - cursor right                    */
char *DO;			/* do - cursor down                     */
char *UP;			/* up - cursor up                       */
char *CR;			/* cr - carriage return                 */
char *NL;			/* nl - new line                        */
char *CL;			/* cl - clear screen                    */
char *CD;			/* cd - clear to end of display         */
char *HO;			/* ho - home cursor                     */
char *CM;			/* cm - general cursor movement         */
char *TI;			/* ti - allow cm (?)                    */
char *TE;			/* te - return terminal to normal       */
char *KS;			/* ks - start keypad-xmit mode          */
char *KE;			/* ke - end keypad-xmit mode            */
char *VS;			/* vs - start visual/open mode          */
char *VE;			/* ve - end visual/open mode            */
char PC;			/* pc - pad character                   */
char *AL;			/* al - add (insert) line               */
char *DL;			/* dl - delete line                     */
char *IC;			/* ic - insert character                */
char *DC;			/* dc - delete character                */
char *CE;			/* ce - clear to eol                    */
char *CS;			/* cs - set scrolling region            */
char *SR;			/* sr - scroll up (reverse)             */
#ifdef LMCLDC
char *GS;			/* Gs - start Graphics                  */
char *GE;			/* Ge - end Graphics                    */
#endif
#ifdef LMCHELP
char *SO;			/* so - begin standout mode             */
char *SE;			/* se - end standout mode               */
#endif
#ifdef LMCVBELL
char *VB;			/* vb - visual bell                     */
#endif
char *MR;                       /* mr - enter reverse video mode        */
char *ME;                       /* me - reset all attribute             */

pch (ch)
{
    putchar (ch);
}

lt_tcap ()
{
    tputs (BC, 1, pch);
}

rt_tcap ()
{
    tputs (ND, 1, pch);
}

dn_tcap ()
{
    tputs (DO, 1, pch);
}

up_tcap ()
{
    tputs (UP, 1, pch);
}

cr_tcap ()
{
    tputs (CR, 1, pch);
}

nl_tcap ()
{
    tputs (CR, 1, pch);
    tputs (DO, 1, pch);
}

clr_tcap ()
{
    tputs (CL, 1, pch);
}

clr1_tcap ()
{
    punt_tcap ();
    tputs (CD, 1, pch);
}

hm_tcap ()
{
    tputs (HO, 1, pch);
}

bsp_tcap ()
{
    tputs (BC, 1, pch);
    putchar (' ');
    tputs (BC, 1, pch);
}

addr_tcap (lin, col)
{
    tputs (tgoto (CM, col, lin), 1, pch);
}

/* ARGSUSED */
il_tcap (num)
{
    tputs (AL, 1, pch);
    return 1;
}
/* ARGSUSED */
dl_tcap (num)
{
    tputs (DL, 1, pch);
    return 1;
}
/* ARGSUSED */
ic_tcap (num)
{
    tputs (IC, 1, pch);
    return 1;
}
/* ARGSUSED */
dc_tcap (num)
{
    tputs (DC, 1, pch);
    return 1;
}
cle_tcap ()
{
    tputs (CE, 1, pch);
    return 1;
}
#ifdef LMCHELP
so_tcap ()
{
    tputs (SO, 1, pch);
}
soe_tcap ()
{
    tputs (SE, 1, pch);
}
#endif
#ifdef LMCVBELL
vb_tcap ()
{
    tputs (VB, 1, pch);
}
#endif
vsc_tcap (top, bottom, num)
{
    tputs (tgoto (CS, bottom, top), 1, pch);
    if (num > 0) {
	tputs (tgoto (CM, 0, top), 1, pch);
	do {
	    tputs (SR, 1, pch);
	} while (--num);
    }
    else {
	tputs (tgoto (CM, 0, bottom), 1, pch);
	do {
	    tputs (DO, 1, pch);
	} while (++num);
    }
    tputs (tgoto (CS, term.tt_height - 1, 0), 1, pch);
    return NO;			/* i.e. can't be sure where the cursor is */
}

ini1_tcap ()
{
    tputs (TI, 1, pch);		/* initialize cm */
    tputs (KS, 1, pch);		/* set keypad transmit mode */
    tputs (VS, 1, pch);		/* set visual mode */
}

end_tcap ()
{
    if (CS && SR) {
	tputs (tgoto (CS, term.tt_height - 1, 0), 1, pch);
	tputs (tgoto (CM, icol, ilin), 1, pch);
    }
    tputs (VE, 1, pch);
    tputs (KE, 1, pch);
    tputs (TE, 1, pch);
}
punt_tcap ()
{
    tputs (tgoto (CM, icol, ilin), 1, pch);
}

#ifdef LMCLDC
tcap_xl (chr)
#ifdef UNSCHAR
     Uchar chr;
#else
     int chr;
#endif
{
    if (chr >= FIRSTSPCL && chr <= LASTGRAF) {
	if (!line_draw) {
	    line_draw = YES;
	    tputs (GS, 1, pch);
	}
	P (*ldtab[chr - FIRSTSPCL]);
    }
    else {
	if (line_draw) {
	    line_draw = NO;
	    tputs (GE, 1, pch);
	}
	if (chr >= FIRSTSPCL)
	    P (*ldtab[chr - FIRSTSPCL]);
	else if (chr)
	    P (chr);
    }
}
#endif

/* Set the terminal video mode (inverted or normal) */
static int video (Flag inverted)
{
    char * str;
    str = inverted ? MR : ME;
    if ( !str ) return 1;

    tputs (str, 1, pch);
    return 0;
}

S_term t_tcap =
{
/* tt_ini0    */ nop,
/* tt_ini1    */ ini1_tcap,
/* tt_end     */ end_tcap,
/* tt_left    */ lt_tcap,
/* tt_right   */ rt_tcap,
/* tt_dn      */ dn_tcap,
/* tt_up      */ up_tcap,
/* tt_cret    */ cr_tcap,
/* tt_nl      */ nl_tcap,
/* tt_clear   */ clr_tcap,
/* tt_home    */ hm_tcap,
/* tt_bsp     */ bsp_tcap,
/* tt_addr    */ addr_tcap,
/* tt_lad     */ bad,
/* tt_cad     */ bad,
/* tt_xlate   */ xlate1,
/* tt_insline */ il_tcap,
/* tt_delline */ dl_tcap,
/* tt_inschar */ ic_tcap,
/* tt_delchar */ dc_tcap,
/* tt_clreol  */ cle_tcap,
/* tt_vscroll */ vsc_tcap,
/* tt_deflwin */ (int (*)()) 0,
/* tt_erase   */ (int (*)()) 0,
#ifdef LMCHELP
/* tt_so      */ so_tcap,
/* tt_soe     */ soe_tcap,
/* tt_help    */ help_std,
#endif
#ifdef LMCVBELL
/* tt_vbell   */ vb_tcap,
#endif
/* tt_nleft   */ 0,
/* tt_nright  */ 0,
/* tt_ndn     */ 0,
/* tt_nup     */ 0,
/* tt_nnl     */ 0,
/* tt_nbsp    */ 0,
/* tt_naddr   */ 0,
/* tt_nlad    */ 0,
/* tt_ncad    */ 0,
/* tt_wl      */ 1,
/* tt_cwr     */ 1,
/* tt_pwr     */ 1,
/* tt_axis    */ 0,
#ifndef LMCLDC
/* tt_bullets */ NO,
#else
/* tt_bullets */ YES,
#endif
/* tt_prtok   */ YES,
/* tt_width   */ 80,
/* tt_height  */ 24,

/* tt_video   */ video,
};

#define NG  -2
#define UNKNOWN -1
#define OK  0

static void check_tt_size (int *width, int *height)
{
    if ( width && (*width >= MAXWIDTH) ) *width = MAXWIDTH -1;
    /* do not over run the max size allowed by poscursor :
     * see : poscursor () routine
     */
    if ( width  && (*width  > (255 - 041)) ) *width  = (255 - 041);
    if ( height && (*height > (255 - 041)) ) *height = (255 - 041);
    /* ensure enough room for the editing window */
    if ( width  && (*width  < 3) ) *width = 3;
    if ( height && (*height < (NPARAMLINES +3)) ) *height = NPARAMLINES +3;
}

/* Get the terminal size by ioctl call */
void get_term_size (int * width, int * height)
{
#ifdef TIOCGWINSZ
    struct winsize windowsz;
#endif

    if ( !width || !height ) return;

#ifdef TIOCGWINSZ
    if ( ioctl (0, TIOCGWINSZ, &windowsz) == 0 ) {
	if ( windowsz.ws_col ) *width  = windowsz.ws_col;
	if ( windowsz.ws_row ) *height = windowsz.ws_row;
    }
#endif
}

/* Get the terminal size by ioctl call */
void get_tt_size (int * width, int * height)
{
    get_term_size (width, height);
    check_tt_size (width, height);
}

/* Set the terminal size into the terminal descriptor */
void set_tt_size (S_term *term_pt, int width, int height)
{
    int wid, hei;

    wid = width;
    hei = height;
    check_tt_size (&wid, &hei);
    term_pt->tt_width  = wid;
    term_pt->tt_height = hei;

    /* build the "blanks line" (not used any more ?) */
    memset (blanks, '\0', MAXWIDTH+1);
    memset (blanks, ' ', term_pt->tt_width);
}


Small
getcap (term, alt_term, str)
     char *term, *alt_term, **str;
{
    char tcbuf[2048];
    char *cp;
    extern char *tgetstr ();
#ifdef LMCLDC
    static char dot[] = ".";
    static char dit[] = ";";
#endif
    int cc;
    int width, height;

    if ( str ) *str = term;
    cc = tgetent (tcbuf, term);
    if ( (cc <= 0) && alt_term ) {
	if ( str ) *str = alt_term;
	cc = tgetent (tcbuf, alt_term);
    }
    if ( cc <= 0 ) {
	if ( str ) *str = "This terminal is unknown in terminfo data base";
	return UNKNOWN;
    }

    cp = salloc (2048, YES);

    /* get the screen size */
    width  = tgetnum ("co");
    if ( tgetflag ("xv") > 0 ) width--;     /* vt100 brain damage ?!? */
    height = tgetnum ("li");
    get_tt_size (&width, &height);
    set_tt_size (&t_tcap, width, height);   /* overwrite by ioctl value */

/* if no home command, fake it with cursor movement. */
    if ((HO = tgetstr ("ho", &cp)) == NULL)
	t_tcap.tt_home = punt_tcap;
/* can't do without either clear or clear to end of display */
    if ((CL = tgetstr ("cl", &cp)) == NULL) {
	if ((CD = tgetstr ("cd", &cp)) == NULL) {
	    if ( str ) *str = "termcap clear (cl) or clear to end of display (cd) is mandatory";
	    return NG;
	}
/* use home/clear-to-end if there is no straight clear */
	t_tcap.tt_clear = clr1_tcap;
    }
/* got to have cursor addressing */
    if ((CM = tgetstr ("cm", &cp)) == NULL) {
	if ( str ) *str = "termcap cursor addressing (cm) is mandatory";
	return NG;
    }
    t_tcap.tt_naddr = strlen (tgoto (CM, 10, 10));
/* set up backspace; if none, fake with cm */
    if ((BC = tgetstr ("bc", &cp)) == NULL)
	if (tgetflag ("bs") > 0)
	    BC = "\b";
	else {
	    t_tcap.tt_left = punt_tcap;
	    t_tcap.tt_nleft = t_tcap.tt_naddr;
	    t_tcap.tt_nbsp = 2 * t_tcap.tt_naddr + 1;
	    goto endbc;
	}
    t_tcap.tt_nleft = strlen (BC);
    t_tcap.tt_nbsp = 2 * strlen (BC) + 1;
  endbc:
/* what's this? a null string for right cursor? */
    if ((ND = tgetstr ("nd", &cp)) == NULL) {
	t_tcap.tt_right = punt_tcap;
	t_tcap.tt_nright = t_tcap.tt_naddr;
/* I overrule it:
   ND = "";       */
    }
    else
	t_tcap.tt_nright = strlen (ND);
/* fake up with cm if needed */
    if ((UP = tgetstr ("up", &cp)) == NULL) {
	t_tcap.tt_up = punt_tcap;
	t_tcap.tt_nup = t_tcap.tt_naddr;
    }
    else
	t_tcap.tt_nup = strlen (UP);
/* with strange flavors of c/r and n/l, fake them too. */
    if (tgetflag ("nc") > 0) {
	t_tcap.tt_cret = punt_tcap;
	t_tcap.tt_nl = punt_tcap;
	t_tcap.tt_nnl = t_tcap.tt_naddr;
    }
    else if ((CR = tgetstr ("cr", &cp)) == NULL)
	CR = "\r";
/* ditto down */
    if ((DO = tgetstr ("do", &cp)) == NULL
	&& (DO = tgetstr ("nl", &cp)) == NULL
	) {
	t_tcap.tt_dn = punt_tcap;
	t_tcap.tt_ndn = t_tcap.tt_naddr;
	t_tcap.tt_nl = punt_tcap;
	t_tcap.tt_nnl = t_tcap.tt_naddr;
    }
    else {
	t_tcap.tt_ndn = strlen (DO);
	t_tcap.tt_nnl = strlen (CR) + strlen (DO);
    }
/* unless both il and dl work, remove them */
    AL = tgetstr ("al", &cp);
    DL = tgetstr ("dl", &cp);
    if ((AL = tgetstr ("al", &cp)) == NULL
	|| (DL = tgetstr ("dl", &cp)) == NULL
	) {
	t_tcap.tt_insline = (int (*)()) 0;
	t_tcap.tt_delline = (int (*)()) 0;
    }
/* zot insert and delete char if they aren't there. */
    IC = tgetstr ("ic", &cp);
    DC = tgetstr ("dc", &cp);
    if ((IC = tgetstr ("ic", &cp)) == NULL
	|| (DC = tgetstr ("dc", &cp)) == NULL
	) {
	t_tcap.tt_inschar = (int (*)()) 0;
	t_tcap.tt_delchar = (int (*)()) 0;
    }
/* clear clreol unless it does exist */
    if ((CE = tgetstr ("ce", &cp)) == NULL)
	t_tcap.tt_clreol = (int (*)()) 0;
/* we can vscroll if we have set-region and reverse scroll!! */
    if ((CS = tgetstr ("cs", &cp)) == NULL
	|| (SR = tgetstr ("sr", &cp)) == NULL
	)
	t_tcap.tt_vscroll = (int (*)()) 0;
/* set/reset for cm. Set pad char if there is one */
    TI = tgetstr ("ti", &cp);
    TE = tgetstr ("te", &cp);
    KS = tgetstr ("ks", &cp);
    KE = tgetstr ("ke", &cp);
    VS = tgetstr ("vs", &cp);
    VE = tgetstr ("ve", &cp);
    Block {
	Reg1 char *pc;
	if (pc = tgetstr ("pc", &cp))
	    PC = *pc;
    }
#ifdef LMCLDC
/* set up the line-drawing for windows. */
    if ((GE = tgetstr ("Ge", &cp)) != NULL
	&& (GS = tgetstr ("Gs", &cp)) != NULL
	) {
	ldtab[BULCHAR - FIRSTSPCL] = tgetstr ("Cj", &cp);
	ldtab[LMCH - FIRSTSPCL] =
	    ldtab[RMCH - FIRSTSPCL] = tgetstr ("Vl", &cp);
	ldtab[MLMCH - FIRSTSPCL] = tgetstr ("Rj", &cp);
	ldtab[MRMCH - FIRSTSPCL] = tgetstr ("Lj", &cp);
	ldtab[TMCH - FIRSTSPCL] =
	    ldtab[BMCH - FIRSTSPCL] = tgetstr ("Hl", &cp);
	ldtab[TLCMCH - FIRSTSPCL] = tgetstr ("Tl", &cp);
	ldtab[TRCMCH - FIRSTSPCL] = tgetstr ("Tr", &cp);
	ldtab[BLCMCH - FIRSTSPCL] = tgetstr ("Bl", &cp);
	ldtab[BRCMCH - FIRSTSPCL] = tgetstr ("Br", &cp);
	ldtab[TTMCH - FIRSTSPCL] = tgetstr ("Bj", &cp);
	ldtab[BTMCH - FIRSTSPCL] = tgetstr ("Tj", &cp);
	ldtab[INMCH - FIRSTSPCL] = dot;
	ldtab[ELMCH - FIRSTSPCL] = dit;
	if ((ldtab[ESCCHAR - FIRSTSPCL] = tgetstr ("Xc", &cp)) == NULL)
	    ldtab[ESCCHAR - FIRSTSPCL] = ldtab[BULCHAR - FIRSTSPCL];
	t_tcap.tt_prtok = NO;
	t_tcap.tt_xlate = tcap_xl;
    }
#endif
#ifdef LMCHELP
/* standout mode, if you will */
/* no standout if terminal has magic cookies... */
    if ((SO = tgetstr ("so", &cp)) != NULL && tgetnum ("sg") == 0)
	SE = tgetstr ("se", &cp);
    else
	t_tcap.tt_so = t_tcap.tt_soe = (int (*)()) 0;
#endif
#ifdef LMCVBELL
/* visual bell */
    if ((VB = tgetstr ("vb", &cp)) == NULL)
	t_tcap.tt_vbell = (int (*)()) 0;
#endif
/* set wrap variables. */
    t_tcap.tt_wl = 0;		/* termcap doesn't guarantee what it will do */
    /* t_tcap.tt_pwr = (tgetflag("am") > 0) ? ((tgetflag("xn") > 0 ?) 4 : 1) : 3;   */
    t_tcap.tt_pwr = 0;		/* might need delay, so we punt */
    t_tcap.tt_cwr = 0;		/* termcap doesn't guarantee what it will do */
    t_tcap.tt_axis = 0;
    t_tcap.tt_nlad = 0;
    t_tcap.tt_ncad = 0;

/* get video mode */
    ME = tgetstr ("me", &cp);
    MR = tgetstr ("mr", &cp);

    return OK;
}
