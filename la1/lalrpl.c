#ifdef COMMENT
	Copyright abandoned, 1983, The Rand Corporation
#endif

#include "la.local.h"

La_linepos
la_lreplace (plas, buf, nchars, nlines, dlas)
Reg2 La_stream *plas;
char *buf;
int nchars;
Reg3 La_linepos *nlines;
La_stream *dlas;
{
    Reg1 La_linepos nins;

    if (nchars != 0) {
	if ((nins = la_linsert (plas, buf, nchars)) < 0)
	    return -1;
    }
    else
	nins = 0;

    if (*nlines == 0)
	return 0;

    Block {
	Reg4 La_linepos ndel;
	if ((ndel = la_ldelete (plas, *nlines, dlas)) == -1) {
	    if (nins > 0) {
		if (!la_stay (plas))
		    (void) la_lseek (plas, -nins, 1);
		(void) la_ldelete (plas, nins, (La_stream *) 0);
	    }
	    return -1;
	}
	*nlines = ndel;
	return ndel;
    }
}
