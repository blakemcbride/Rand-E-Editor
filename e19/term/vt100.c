#ifdef COMMENT
	Copyright abandoned, 1983, The Rand Corporation
#endif

/***********************************************************************

This vt100 termcap is known to work with E:

vt100common|vt100 common characteristics:\
	:ti=\E=:te=\E>:do=2^J:bc=\E[D:cr=^M:nl=30\ED:ho=5\E[0;0H:\
	:co#80:li#24:am:cl=50\E[;H\E[2J:nd=\E[C:up=\E[A:cm=5\E[%i%d;%dH:\
	:ce=3\E[K:cd=50\E[J:so=2\E[7m:se=2\E[m:us=2\E[4m:ue=2\E[m:\
	:is=\E>\E[?1l\E[?3l\E[?7h\E[?8h\E(B\E)B:ks=\E=:ke=\E>:\
	:if=/usr/lib/tabset/vt100:ku=\EOA:kd=\EOB:kr=\EOC:kl=\EOD:\
	:kh=\E[H:k1=\EOP:k2=\EOQ:k3=\EOR:k4=\EOS:pt:sr=30\EM:xn:
d0|vt100n|vt100 w/no init:is@:if@:tc=vt100:
d1|vt100|vt-100|pt100|pt-100|dec vt100:\
	:tc=vt100common:
d3|vt132|vt-132:\
	:al=99\E[L:dl=99\E[M:ip=7:dc=7\E[P:ei=\E[4l:im=\E[4h:xn:dN#30:tc=vt100common:
ds|vt100s|vt-100s|pt100s|pt-100s|dec vt100 132 cols without -AB option:\
	:li#14:tc=vt100w:
dt|vt100w|vt-100w|pt100w|pt-100w|dec vt100 132 cols with -AB option:\
	:co#132:li#24:is=\E>\E[?1l\E[?7h\E[?8h\E(B\E)B\E[?3h:tc=vt100common:

------------------------------
This is the latest official one from Mark Horton, keeper of termcap:

17-Jan-82 12:57:59-EST (Sun)

From: harpo!cbosg!cbosgd!mark
To:   cbosg!harpo!randvax!day

Subject: Re:  po termcap type

Here is our complete DEC piece of termcap.  See you at USENIX!

	Mark
#
# Note that xn glitch in vt100 is not quite the same as concept, since
# the cursor is left in a different position while in the weird state
# (concept at beginning of next line, vt100 at end of this line) so
# all versions of vi before 3.7 don't handle xn right on vt100.
# I assume you have smooth scroll off or are at a slow enough baud
# rate that it doesn't matter (1200? or less).  Also this assumes
# that you set auto-nl to "on", if you set it off use vt100-nam below.
#
# Since there are two things here called vt100, the installer can make
# a local decision to make either one standard "vt100" by including
# it in the list of terminals in reorder, since the first vt100 in
# /etc/termcap is the one that it will find.  The choice is between
# nam (no automatic margins) and am (automatic margins), as determined
# by the wrapline switch (group 3 #2).  I presonally recommend turning
# on the bit and using vt100-am, since having stuff hammer on the right
# margin is sort of hard to read.  However, the xn glitch does not occur
# if you turn the bit off.
#
# I am unsure about the padding requirements listed here.  I have heard
# a claim that the vt100 needs no padding.  It's possible that it needs
# padding only if the xon/xoff switch is off.  For UNIX, this switch
# should probably be on.
#
# The vt100 uses rs and rf rather than is/ct/st because the tab settings
# are in non-volatile memory and don't need to be reset upon login.
# You can type "reset" to get them set.
d0|vt100|vt100-am|vt100|dec vt100:\
	:cr=^M:do=^J:nl=^J:bl=^G:co#80:li#24:cl=50\E[;H\E[2J:\
	:le=^H:bs:am:cm=5\E[%i%d;%dH:nd=2\E[C:up=2\E[A:\
	:ce=3\E[K:cd=50\E[J:so=2\E[7m:se=2\E[m:us=2\E[4m:ue=2\E[m:\
	:md=2\E[1m:mr=2\E[7m:mb=2\E[5m:me=2\E[m:is=\E[1;24r\E[24;1H:\
	:rs=\E>\E[?3l\E[?4l\E[?5l\E[?7h\E[?8h:ks=\E[?1h\E=:ke=\E[?1l\E>:\
	:rf=/usr/lib/tabset/vt100:ku=\EOA:kd=\EOB:kr=\EOC:kl=\EOD:kb=^H:\
	:ho=\E[H:k1=\EOP:k2=\EOQ:k3=\EOR:k4=\EOS:ta=^I:pt:sr=5\EM:vt#3:xn:\
	:sc=\E7:rc=\E8:cs=\E[%i%d;%dr:
d1|vt100|vt100-nam|vt100 w/no am:\
	:am@:xn@:tc=vt100-am:
d2|gt42|dec gt42:\
	:cr=^M:do=^J:bl=^G:\
	:le=^H:bs:co#72:ns:li#40:os:
d3|vt132|vt132:\
	:al=99\E[L:dl=99\E[M:ip=7:dc=7\E[P:ei=\E[4l:im=\E[4h:xn:dN#30:tc=vt100:
d4|gt40|dec gt40:\
	:cr=^M:do=^J:bl=^G:\
	:le=^H:bs:co#72:ns:li#30:os:
d5|vt50|dec vt50:\
	:cr=^M:do=^J:nl=^J:bl=^G:\
	:le=^H:bs:cd=\EJ:ce=\EK:cl=\EH\EJ:co#80:li#12:nd=\EC:ta=^I:pt:up=\EA:
dI|dw1|decwriter I:\
	:cr=^M:do=^J:nl=^J:bl=^G:\
	:le=^H:bs:co#72:hc:os:
dh|vt50h|dec vt50h:\
	:cr=^M:do=^J:nl=^J:bl=^G:\
	:le=^H:bs:cd=\EJ:ce=\EK:cl=\EH\EJ:cm=\EY%+ %+ :co#80:li#12:nd=\EC:\
	:ta=^I:pt:sr=\EI:up=\EA:
di|vt100-23|vt100 for use with vt100sys:\
	:li#23:is=\E[1;23r\E[23;1H:tc=vt100-am:
# This version works but you might find the initialization sequence annoying.
do|ovt100|old dec vt100:\
	:cr=^M:do=^J:nl=^J:bl=^G:co#80:li#24:am:cl=50\E[;H\E[2J:\
	:le=^H:bs:cm=5\E[%i%2;%2H:nd=2\E[C:up=2\E[A:\
	:ce=3\E[K:cd=50\E[J:so=2\E[7m:se=2\E[m:us=2\E[4m:ue=2\E[m:\
	:is=\E>\E[?3l\E[?4l\E[?5l\E[?7h\E[?8h:ks=\E[?1h\E=:ke=\E[?1l\E>:\
	:if=/usr/lib/tabset/vt100:ku=\EOA:kd=\EOB:kr=\EOC:kl=\EOD:\
	:kh=\E[H:k1=\EOP:k2=\EOQ:k3=\EOR:k4=\EOS:ta=^I:pt:sr=5\EM:xn:
ds|vt100-s|dec vt100 132 cols 14 lines (w/o advanced video option):\
	:li#14:tc=vt100-w:
dt|vt100-w|dec vt100 132 cols (w/advanced video):\
	:co#132:li#24:rs=\E>\E[?3h\E[?4l\E[?5l\E[?8h:tc=vt100:
dv|vt52|dec vt52:\
	:cr=^M:do=^J:nl=^J:bl=^G:\
	:le=^H:bs:cd=\EJ:ce=\EK:cl=\EH\EJ:cm=\EY%+ %+ :co#80:li#24:nd=\EC:\
	:ta=^I:pt:sr=\EI:up=\EA:ku=\EA:kd=\EB:kr=\EC:kl=\ED:kb=^H:
dw|dw2|dw3|decwriter II:\
	:cr=^M:do=^J:nl=^J:bl=^G:\
	:kb=^h:le=^H:bs:co#132:hc:os:
# From cbosg!ucbvax!G:tut Thu Sep 24 22:10:46 1981
df|dw4|decwriter IV:\
	:cr=^M:do=^J:nl=^J:bl=^G:\
	:le=^H:bs:co#132:hc:os:am:ta=^I:pt:is=\Ec:k0=\EOP:k1=\EOQ:k2=\EOR:k3=\EOS:kb=^H:

***************************************************************/

#define MY_CTRL(x) ((x) & 31)
#define _BAD_ CCUNAS1


in_vt100 (lexp, count)
char *lexp;
int *count;
{
    Reg1 int chr;
    Reg2 int nr;
    Reg3 char *icp;
    Reg4 char *ocp;

    icp = ocp = lexp;
    nr = *count;
    for (; nr > 0; nr--) {
	if ((chr = *icp++ & 0177) >= 040) {
	    if (chr == 0177)         /* DEL KEY KLUDGE FOR NOW */
		*ocp++ = CCBACKSPACE;
	    else
		*ocp++ = chr;
	}
	else if (chr == 033) {
	    if (nr < 2) {
		icp--;
		goto nomore;
	    }
	    nr--;
	    chr = *icp++ & 0177;
	    if (chr == '[') {
		if (nr < 2) {
		    icp -= 2;
		    nr++;
		    goto nomore;
		}
		nr--;
		switch (chr = *icp++ & 0177) {
		case 'A':
		    *ocp++ = CCMOVEUP;
		    break;

		case 'B':
		    *ocp++ = CCMOVEDOWN;
		    break;

		case 'C':
		    *ocp++ = CCMOVERIGHT;
		    break;

		case 'D':
		    *ocp++ = CCMOVELEFT;
		    break;

		default:
		    *ocp++ = CCUNAS1;
		    break;
		}
	    }
	    else if (chr == 'O') {
		if (nr < 2) {
		    icp -= 2;
		    nr++;
		    goto nomore;
		}
		nr--;
		chr = *icp++ & 0177;
		if ('l' <= chr && chr <= 'y') Block {
		    static unsigned char xlt[] = {
			CCSETFILE      , /* l  , */
			CCCHWINDOW     , /* m  - */
			CCPICK         , /* n  . */
			CCUNAS1        , /* o    */
			CCMARK         , /* p  0 */
			CCMISRCH       , /* q  1 */
			CCPLSRCH       , /* r  2 */
			CCREPLACE      , /* s  3 */
			CCMIPAGE       , /* t  4 */
			CCPLPAGE       , /* u  5 */
			CCBACKTAB      , /* v  6 */
			CCMILINE       , /* w  7 */
			CCPLLINE       , /* x  8 */
			CCINT          , /* y  9 */
		    };
		    *ocp++ = xlt[chr - 'l'];
		}
		else if ('A' <= chr && chr <= 'D') Block {
		    static char xlt[] = {
			CCMOVEUP       , /* UPARROW  */
			CCMOVEDOWN     , /* DOWNARROW*/
			CCMOVERIGHT    , /* RIGHT -> */
			CCMOVELEFT     , /* LEFT <-  */
		    };
		    *ocp++ = xlt[chr - 'A'];
		}
		else if ('M' <= chr && chr <= 'S') Block {
		    static unsigned char xlt[] = {
			CCCMD          , /* M  ENTER */
			CCUNAS1        , /* N        */
			CCUNAS1        , /* O        */
			CCOPEN         , /* P  PF1   */
			CCCLOSE        , /* Q  PF2   */
			CCINSMODE      , /* R  PF3   */
			CCDELCH        , /* S  PF4   */
		    };
		    *ocp++ = xlt[chr - 'M'];
		}
		else
		    *ocp++ = CCUNAS1;
	    }
	    else
		*ocp++ = CCUNAS1;
	}
	else if (chr == ('X' & 31)) {
	    if (nr < 2) {
		icp--;
		goto nomore;
	    }
	    nr--;
	    chr = *icp++ & 0177;
	    switch (chr) {
	    case MY_CTRL ('a'):
		*ocp++ = CCSETFILE;
		break;
	    case MY_CTRL ('b'):
		*ocp++ = CCSPLIT;
		break;
	    case MY_CTRL ('c'):
		*ocp++ = CCCTRLQUOTE;
		break;
	    case MY_CTRL ('e'):
		*ocp++ = CCERASE;
		break;
	    case MY_CTRL ('h'):
		*ocp++ = CCLWINDOW;
		break;
	    case MY_CTRL ('j'):
		*ocp++ = CCJOIN;
		break;
	    case MY_CTRL ('l'):
		*ocp++ = CCRWINDOW;
		break;
	    case MY_CTRL ('r'):
		*ocp++ = CCREPLACE;
		break;
	    case MY_CTRL ('t'):
		*ocp++ = CCTABS;
		break;
	    case MY_CTRL ('u'):
		*ocp++ = CCBACKTAB;
		break;
	    case MY_CTRL ('w'):
		*ocp++ = CCCHWINDOW;
		break;
	    default:
		*ocp++ = CCUNAS1;
		break;
	    }
	}
	else
	    *ocp++ = lexstd[chr];
    }
 nomore:
    Block {
	int conv;
	*count = nr;     /* number left over - still raw */
	conv = ocp - lexp;
	while (nr-- > 0)
	    *ocp++ = *icp++;
	return conv;
    }
}

kini_vt100 ()
{
    static char ini[] = {033,'='};
    fwrite (ini, sizeof ini, 1, stdout);
}
kend_vt100 ()
{
    static char end[] = {033,'>'};
    fwrite (end, sizeof end, 1, stdout);
}


S_kbd kb_vt100 = {
/* kb_inlex */  in_vt100,
/* kb_init  */  kini_vt100,
/* kb_end   */  kend_vt100,
};
