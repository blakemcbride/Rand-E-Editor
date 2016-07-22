#ifdef COMMENT
	Copyright abandoned, 1983, The Rand Corporation
#endif

#include "la.local.h"

#define min(a,b) ((a)<(b)?(a):(b))

La_linepos
la_blank (plas, nlines)
Reg4 La_stream *plas;
Reg5 La_linepos nlines;
{
    char blnks[LA_FSDLMAX];
    static La_stream blas;
    static La_flag opened = NO;
    static La_flag inserted = NO;

    if (nlines <= 0)
	return 0;

    /* Don't grow plas beyond highest int */
    Block {
	Reg1 La_linepos cnt;
	cnt = plas->la_file->la_nlines;
#ifdef LA_LONGFILES
	if (cnt + nlines < cnt) {
#else
	if (cnt + nlines > la_ltrunc (cnt + nlines)) {
#endif
	    la_errno = LA_ERRMAXL;
	    return 0;
	}
    }

    if (!inserted) {
	if (!opened) {
	    if (la_clone (la_chglas, &blas) == NULL)
		return 0;
	    la_stayset (&blas);
	    opened = YES;
	}
	fill (blnks, (unsigned int) LA_FSDLMAX, LA_NEWLINE);
	if (la_linsert (&blas, blnks, LA_FSDLMAX) != LA_FSDLMAX)
	    return 0;
	inserted = YES;
    }

    Block {
	Reg3 La_linepos cnt;
	cnt = 0;
	while (nlines > 0) Block {
	    Reg1 int nins;
	    Reg2 int incr;
	    incr = min (nlines, LA_FSDLMAX);
	    if ((nins = la_lcopy (&blas, plas, incr)) != incr) {
		if (nins > 0)
		    cnt += nins;
		(void) la_lseek (plas, -cnt, 1);
		(void) la_ldelete (plas, cnt, (La_stream *) 0);
		return 0;
	    }
	    cnt += nins;
	    nlines -= nins;
	}
	return cnt;
    }
}
