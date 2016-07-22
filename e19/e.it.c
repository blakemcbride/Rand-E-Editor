#ifdef COMMENT
--------
file e.it.c
    input (keyboard) table parsing and lookup
#endif

#ifdef COMMENT
	Copyright abandoned, 1983, The Rand Corporation
#endif


#include "e.h"
#ifdef  KBFILE
#include "e.it.h"

/* input char size is 7 bit */
#ifdef CHAR7BITS
Flag inputcharis7bits = YES;
#else
Flag inputcharis7bits = NO;
#endif

struct itable *ithead;

/****************************************/
/* file-driven input (no associated terminal number) */

int
in_file (lexp, count)
Uchar *lexp;
int *count;
{
    extern int CtrlC_flg;
    int code;
    Uchar *inp, *outp, chr;
    int i;

    if ( inputcharis7bits ) {
	/* Mask off high bit of all chars */
	for (inp = lexp, i = *count; i-- > 0;)
	    *inp++ &= 0177;
    }

    /* outp should be different so a string can be replaced by a longer one */
    inp = outp = lexp;  /* !!!! what append if output is longer than input ! */
    while (*count > 0) {
	chr = *inp;
	CtrlC_flg = (chr == ('C' & '\037'));
	if ( ! ISCTRLCHAR(chr) ) {
	    *outp++ = *inp++;
	    --*count;
	    continue;
	}

	code = itget (&inp, count, ithead, outp);
	if (code >= 0) {    /* Number of characters resulting */
	    outp += code;
	    continue;
	}
	if (code == IT_MORE)    /* Stop here if in the middle of a seq */
	    break;
				/* Otherwise not in table */
#if 0
	mesg (ERRALL + 1, "Bad control key");
#endif
	inp++;       /* Skip over this key */
	--*count;
    }
    move (inp, outp, *count);
    return outp - lexp;
}


static int itget_addr (cpp, countp, head, it_pt)
char **cpp;
int *countp;
struct itable *head;
struct itable **it_pt;

{
    register struct itable *it;
    register char *cp;
    int count;

    if ( it_pt) *it_pt = NULL;
    cp = *cpp;
    count = *countp;
next:
    for (it = head; it != NULLIT; it = it->it_next) {
	if (count <= 0)
	    return IT_MORE;             /* Need more input */
	if (it->it_c == *cp) {          /* Does character match? */
	    cp++;
	    --count;
	    if (it->it_leaf) {
		    if ( it_pt) *it_pt = it;
		    *cpp = cp;
		    *countp = count;
		    return it->it_len;
	    }
	    else {
		head = it->it_link;
		goto next;
	    }
	}
    }
    return IT_NOPE;
}

#endif

/* storage for the last matched input string */
static char matched_strg [128];

char * it_strg ()
{
    return matched_strg;
}

#ifdef COMMENT
itget (..) matches input (at *cpp) against input table
If some prefix of the input matches the table, returns the number of
   characters in the value corresponding to the matched input, stores
   the value in valp, points cpp past the matching input,
   and decrements *countp by the number of characters matched.

If no match, returns IT_NOPE.

If the input matches some proper previx of an entry in the input table,
   returns IT_MORE.

cpp and countp are not changed in the last two cases.
#endif

int
itget (cpp, countp, head, valp)
char **cpp;
int *countp;
struct itable *head;
char *valp;
{
    int cc, nb;
    char *icpp;
    struct itable *it_pt;

    icpp = *cpp;
    cc = itget_addr (cpp, countp, head, &it_pt);
    if ( cc > 0 ) {
	memset (matched_strg, 0, sizeof (matched_strg));
	nb = (int) (*cpp - icpp);
	if ( nb >= sizeof (matched_strg) ) nb = sizeof (matched_strg) -1;
	memcpy (matched_strg, icpp, nb);
	if ( valp ) memcpy (valp, it_pt->it_val, it_pt->it_len);    /* can overwrite input */
    }
    return cc;
}


/* itswapdeldchar : swap between del and dchar for string fstrg
 *      if the new_val = 0 : flip the value
 *      if new_val == CCBACKSPACE or valstr == CCDELCH : set this value
 *      return the (new) value or 0 on error;
 */
int itswapdeldchar (char * fstrg, int new_val, int *init_val_pt)
{
    static int init_val = 0;
    int cc;
    char *cp, cmd;
    int count;
    struct itable *its;

    cp = fstrg;
    count = strlen (cp);
    cc = itget_addr (&cp, &count, ithead, &its);
    if ( !its || (cc < 0) ) return 0;
    if ( its->it_len != 1 ) return 0;

    cmd = *(its->it_val);
    if ( init_val == 0 ) init_val = cmd;    /* save the initial value */
    if ( init_val_pt ) *init_val_pt = init_val;
    if ( new_val == -1 )
	return cmd;

    if ( (new_val == CCBACKSPACE) || (new_val == CCDELCH) ) {
	if ( new_val == cmd ) return cmd;
    } else if ( new_val != 0 ) return cmd;

    /* flip the value */
    *(its->it_val) = ( cmd == CCBACKSPACE )  ? CCDELCH : CCBACKSPACE;
    return cmd;
}

/* itoverwrite : overwrite the value of the leave define by dsetstrg
 *               by value of the leave defined by srcstrg
 * -----------------------------------------------------------------
 */
int itoverwrite (srcstrg, deststrg, head)
char *srcstrg, *deststrg;
struct itable *head;

{
    int cc;
    char *cp;
    int count;
    struct itable *its, *itd;

    cp = srcstrg;
    count = strlen (cp);
    cc = itget_addr (&cp, &count, head, &its);
    if ( !its || (cc < 0) ) return cc;

    cp = deststrg;
    count = strlen (cp);
    cc = itget_addr (&cp, &count, head, &itd);
    if ( !itd || (cc < 0) ) return cc;

    itd->it_len = its->it_len;
    itd->it_val = its->it_val;
    return (cc);
}


/* itgetleave : get the leave for the given string
 * -----------------------------------------------
 */
int itgetleave (strg, it_pt, head)
char *strg;
struct itable **it_pt;
struct itable *head;

{
    int cc;
    char *cp;
    int count;

    cp = strg;
    count = strlen (cp);
    cc = itget_addr (&cp, &count, head, it_pt);
    return (cc);
}

/* Get the address of the value for a given string */
char * itgetvalue (char * strg)
{
    int cc;
    struct itable *it_pt;

    cc = itgetleave (strg, &it_pt, ithead);
    if ( !it_pt || (cc < 0) ) return NULL;
    if ( it_pt->it_len != 1 ) return NULL;

    return (it_pt->it_val);
}
