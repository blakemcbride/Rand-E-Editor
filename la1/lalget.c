#ifdef COMMENT
	Copyright abandoned, 1983, The Rand Corporation
#endif

#include "la.local.h"

/*
 * la_lget (plas, buf, nchars)
 *
 * gets up to nchars characters from the La_stream plas and puts them
 * into the buffer pointed to by buf.  Input is terminated by a newline.
 *
 * returns the number of characters read.
 *
 **/
La_linesize
la_lget (plas, buf, nchars)
Reg2 La_stream *plas;
char *buf;
Reg5 int nchars;
{
    Reg3 La_fsd    *cfsd;
    Reg1 int        j;
    Reg4 La_flag    nonewl;

    if (nchars <= 0) {
	la_errno = LA_NEGCOUNT;
	return -1;
    }
    if ((cfsd = plas->la_cfsd) == plas->la_file->la_lfsd)
	return 0;

    nonewl = NO;
    if (nchars < (j = la_lrsize (plas)) )
	j = nchars;
    else if (   cfsd->fsdbytes[0] == 0
	     && cfsd->fsdbytes[1] == 1
	    ) {
	/* special fsd - no newline */
	buf[--j] = LA_NEWLINE;
	nonewl = YES;
    }
    if (j > 0) Block {
	Reg6 Ff_stream *ffn;
	(void) ff_seek (ffn = cfsd->fsdfile->la_ffs,
		 (long) (cfsd->fsdpos + plas->la_ffpos), 0);
	if (ff_read (ffn, buf, j, 0) != j) {    /* yes, read directly */
	    la_errno = LA_READERR;
	    return -1;
	}
    }
    if (nonewl)
	j++;
    (void) la_advance (plas, (La_linesize) j);
    return j;       /* return # bytes read */
}
