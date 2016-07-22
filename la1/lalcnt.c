#ifdef COMMENT
	Copyright abandoned, 1983, The Rand Corporation
#endif

#include "la.local.h"

#define min(a,b) ((a)<(b)?(a):(b))
#define abs(a) ((a)>=0?(a):(-a))

/* la_lcount (plas, start, number, mode)
 *      Mode: -2 and 2 - paragraphs backward and forward
 *      Mode: -1 and 1 - lines backward and forward
 *      Return the actual number of lines.  End of file can cause
 *      the number to be less than nl.
 *      A paragraph is a block of non-blank lines.
 *      A line with a form-feed in the first column is considered blank.
 */
La_linepos
la_lcount (plas, start, number, mode)
Reg6 La_stream *plas;
La_linepos start;
La_linepos number;
int mode;
{
    switch (abs (mode)) {
    case 1:
	Block {
	    Reg1 La_linepos nlines;
	    if (mode > 0) {
		if ((nlines = la_lsize (plas) - start) <= 0)
		    return 0;
		return min (nlines, number);
	    }
	    else
		return min (start, number);
	}

    case 2:
	if (mode > 0) Block {
	    Reg1 La_stream *tlas;
	    Reg2 La_linepos nlines;
	    Reg5 La_linepos ngap;
	    La_flag gap;
	    La_stream llas;
	    if (   number <= 0
		|| la_lsize (plas) - start <= 0
	       )
		return 0;
	    if (la_clone (plas, tlas = &llas) == NULL)
		return -1;
	    (void) la_lseek (tlas, start, 0);
	    nlines = 0;
	    ngap = 0;
	    gap = YES;
	    for (;;) Block {
		char buf[1];
		La_linesize size;
		*buf = '\0';
		if ((size = (la_lwsize (tlas))) == 0)
		    break;
		if (gap) {
		    /* inside blank lines */
		    if (size == 1)
			ngap++;
		    else {
			if (la_lget (tlas, buf, 1) < 1) {
			    nlines = -1;
			    break;
			}
			if (   *buf == '\f'
			    && size == 2
			   )
			    ngap++;
			else {
			    nlines += ngap + 1;
			    ngap = 0;
			    gap = NO;
			}
		    }
		}
		else {
		    /* in text lines */
		    if (size == 1)
			goto newgap;
		    if (la_lget (tlas, buf, 1) < 1) {
			nlines = -1;
			break;
		    }
		    if (*buf == '\f') {
 newgap:                if (--number <= 0)
			    break;
			if (size <= 2) {
			    gap = YES;
			    ngap = 1;
			}
			else
			    nlines++;
		    }
		    else
			nlines++;
		}
		(void) la_advance (tlas, (La_linesize) -1);
	    }
	    (void) la_close (tlas);
	    return nlines;
	}
	else
	    /* backwards paragraph counting not implemented yet */
	    return 0;
    }
    la_errno = LA_INVMODE;
    return -1;
}
