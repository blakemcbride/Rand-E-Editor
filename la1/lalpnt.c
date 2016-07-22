#ifdef COMMENT
	Copyright abandoned, 1983, The Rand Corporation
#endif

#include "la.local.h"

/*
 * la_lpnt (plas, &buf)
 *
 * gets some or all of the rest of the current line into memory and returns
 * how many characters are pointed at.  Also returns the address of the
 * buffer in &buf if return is > 0.
 *
 **/
La_linesize
la_lpnt (plas, buf)
Reg2 La_stream *plas;
char **buf;
{
    Reg1 La_linesize nchars;
    Reg3 La_fsd *cfsd;
    static char newl[] = {LA_NEWLINE};

    if ((nchars = la_lrsize (plas)) <= 0)
	return 0;
    if (   (cfsd = plas->la_cfsd)->fsdbytes[0] == 0
	&& cfsd->fsdbytes[1] == 1
       ) {
	/* special fsd - no newline */
	if (nchars == 1) {
	    *buf = newl;
	    goto gotit;
	}
	nchars--;
    }

    if ((nchars = ff_point (cfsd->fsdfile->la_ffs,
			    cfsd->fsdpos + plas->la_ffpos, buf, (long)nchars))
	< 0) {
	la_errno = LA_READERR;
	return -1;
    }

 gotit:
    (void) la_advance (plas, nchars);
    return nchars;
}
