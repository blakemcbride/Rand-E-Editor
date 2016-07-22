#ifdef COMMENT
	Copyright abandoned, 1983, The Rand Corporation
#endif

#include "la.local.h"

#ifdef COMMENT
  /*
  Link the fsd chain into plas at the current position.  It is assumed
  that the current position points to the beginning of an fsd.
  Also, the caller should verify that the number of lines to be linked
  in will not grow the file large enough that its size can't be represented
  within an object of type La_linepos.
  */
#endif
la_link (plas, ffsd, lfsd, nl
#ifdef LA_BP
	     , nchars
#endif
	)
Reg6 La_stream *plas;
Reg5 La_fsd *ffsd;              /* first fsd in the chain */
La_fsd *lfsd;                   /* last  fsd in the chain */
La_linepos nl;                  /* number of lines in the chain */
#ifdef LA_BP
La_bytepos nchars;              /* number of chars in the chain */
#endif
{
    Reg3 La_file *nlaf;
    Reg4 La_fsd *cfsd;

    /* link in the new chain */
/*  if ((cfsd = plas->la_cfsd) == (nlaf = plas->la_file)->la_ffsd)      */ /* Some compilers stumble here */
    nlaf = plas->la_file;
    if ((cfsd = plas->la_cfsd) == nlaf->la_ffsd)
	nlaf->la_ffsd = ffsd;
    else
	cfsd->fsdback->fsdforw = ffsd;
    ffsd->fsdback = cfsd->fsdback;
    lfsd->fsdforw = cfsd;
    cfsd->fsdback = lfsd;

    /* adjust the size of the file */
    nlaf->la_nlines += nl;
#ifdef LA_BP
    nlaf->la_nbytes += nchars;
#endif

    nlaf->la_mode |= LA_MODIFIED;

    /* adjust other streams into same file */
    Block {
	Reg1 La_stream *tlas;
	Reg2 La_linepos plaslpos;
	plaslpos = plas->la_lpos;
	for (tlas = nlaf->la_fstream; tlas; tlas = tlas->la_fforw) {
	    if (tlas->la_lpos == plaslpos) {
		if (!(tlas->la_sflags & LA_STAY))
		    goto moveit;
		if (tlas->la_lbyte) {
		    /* make sure we're at the beginning of the line */
#ifdef LA_BP
		    tlas->la_bpos -= tlas->la_lbyte;
#endif
		    tlas->la_ffpos = 0;
		    tlas->la_lbyte = 0;
		}
		tlas->la_cfsd = ffsd;
		tlas->la_rlines = 0;    /* reset reserved lines */
	    }
	    else if (tlas->la_lpos < plaslpos) {
		if (tlas->la_lpos + tlas->la_rlines > plaslpos)
		    tlas->la_rlines = 0;    /* reset reserved lines */
	    }
	    else {
		if (tlas->la_lpos > plaslpos) {
 moveit:            tlas->la_lpos += nl;
#ifdef LA_BP
		    tlas->la_bpos += nchars;
#endif
		}
	    }
	}
    }

    return;
}
