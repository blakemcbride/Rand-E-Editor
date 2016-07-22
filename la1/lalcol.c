#ifdef COMMENT
	Copyright abandoned, 1983, The Rand Corporation
#endif

#include "la.local.h"

#define DORMANT    0
#define COLLECTING 1
static La_flag prevstate = DORMANT;
static La_bytepos chgpos;
static La_bytepos nxchgpos;

La_linepos
la_lcollect (state, buf, nchars)
int        state;     /* 1=start, 0=continue */
Reg3 char *buf;
Reg1 int   nchars;
{
    Reg2 La_linepos errval;

    if (state) {
	nxchgpos = chgpos = ff_grow (la_chgffs);
	goto write;
    }
    else {
	if (prevstate == DORMANT) {
	    errval = LA_BADCOLL;
	    goto err;
	}
 write:
	prevstate = COLLECTING;
	if (   nchars < 0
	    || (   nchars > 0
		&& buf[nchars-1] != LA_NEWLINE
	       )
	   ) {
	    errval = LA_NONEWL;
	    goto err;
	}
	if (nxchgpos != ff_size (la_chgffs)) {
	    errval = LA_BRKCOLL;
	    goto err;
	}
	if (nchars > 0) {
	    (void) ff_seek (la_chgffs, nxchgpos, 0);
	    if (ff_write (la_chgffs, buf, nchars) != nchars) {
		errval = LA_WRTERR;
		goto err;
	    }
	    nxchgpos += nchars;
	}
	return 0;
    }

 err:
    prevstate = DORMANT;
    la_errno = errval;
    return -1;
}

int
la_tcollect (pos)
long pos;
{
    if ((chgpos = pos) > (nxchgpos = ff_grow (la_chgffs))) {
	prevstate = DORMANT;
	return 0;
    }
    prevstate = COLLECTING;
    return 1;
}

/* VARARGS2 */
La_linepos
la_lrcollect (plas, nlines, dlas)
Reg2 La_stream  *plas;
Reg6 La_linepos *nlines;
La_stream  *dlas;
{
    Reg3 La_linepos nl;
    Reg4 La_linepos errval;
    Reg5 La_bytepos totchars;
    La_fsd *ffsd;               /* first fsd to be made for buf */
    La_fsd *lfsd;               /* last  fsd to be made for buf */

    if (prevstate == DORMANT) {
	errval = LA_BADCOLL;
	goto err;
    }
    if (!la_zbreak (plas))
	goto err1;
    /*printf ("about to parse: chgpos=%d, nchars=%d\n", chgpos, nchars);*/
    if ((totchars = nxchgpos - chgpos) < 0) {
	errval = LA_NEGCOUNT;
	prevstate = DORMANT;
	goto err;
    }
    if ((nl = la_parse (la_chgffs, chgpos, &ffsd, &lfsd,
			la_chglas->la_file, totchars, ""))
	< 0)
	goto err1;

    if (nl > 0) Block {
	Reg1 La_linepos nlsize;
	/* Don't grow plas beyond highest int */
	nlsize = plas->la_file->la_nlines;
#ifdef LA_LONGFILES
	if (nlsize + nl < nlsize) {
#else
	if (nlsize + nl > la_ltrunc (nlsize + nl)) {
#endif
	    la_errno = LA_ERRMAXL;
	    la_freefsd (ffsd);
	    prevstate = DORMANT;
	    return -1;
	}

	if (*nlines > 0) Block {
	    La_linepos ndel;
	    if ((ndel = la_ldelete (plas, *nlines, dlas)) == -1) {
		la_freefsd (ffsd);
		goto err1;
	    }
	    *nlines = ndel;
	}

	la_link (plas, ffsd, lfsd, nl
#ifdef LA_BP
		 , (La_bytepos) totchars
#endif
		);
    }
    prevstate = DORMANT;
    return nl;

 err:
    la_errno = errval;
 err1:
    return -1;
}
