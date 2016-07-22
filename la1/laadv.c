#ifdef COMMENT
	Copyright abandoned, 1983, The Rand Corporation
#endif

#include "la.local.h"

La_linesize
la_advance (plas, nchars)
Reg1 La_stream *plas;
Reg2 La_linesize nchars;
{
    Reg4 La_linesize j;

    if (nchars >= (j = la_lrsize (plas)) || nchars < 0) Block {
	/* must advance pointer to next line */
	Reg3 La_fsd *cfsd;
	nchars = j;
	cfsd = plas->la_cfsd;
	plas->la_lpos++;
	plas->la_lbyte = 0;
	if (++plas->la_fsline >= cfsd->fsdnlines) {  /* end of this fsd */
	    plas->la_fsline = 0;
	    plas->la_cfsd = cfsd->fsdforw;
	    plas->la_fsbyte = 0;
	    plas->la_ffpos = 0;
	}
	else {
	    /* can't be a special fsd here */
#ifndef NOSIGNEDCHAR
	    if (cfsd->fsdbytes[plas->la_fsbyte++] < 0)
#else
	    if (cfsd->fsdbytes[plas->la_fsbyte++] & LA_LLINE)
#endif
		plas->la_fsbyte++;
	    plas->la_ffpos += nchars;
	}
    }
    else {
	/* not going past this line */
	plas->la_lbyte += nchars;
	plas->la_ffpos += nchars;
    }
#ifdef LA_BP
    plas->la_bpos += nchars;
#endif
    return nchars;
}
