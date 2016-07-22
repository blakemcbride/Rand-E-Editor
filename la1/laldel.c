#ifdef COMMENT
	Copyright abandoned, 1983, The Rand Corporation
#endif

#include "la.local.h"

La_linepos
la_ldelete (plas, nlines, dlas)
Reg3 La_stream *plas;
Reg4 La_linepos nlines;
La_stream *dlas;
{
    Reg2 La_linepos lpos1;
    Reg5 La_fsd    *dffsd;
    La_fsd    *nfsd;
    La_fsd    *dlfsd;
    Reg6 La_file   *nlaf;
#ifdef LA_BP
    La_bytepos bpos1;
    La_bytepos nbytes;
#endif

    if (nlines <= 0)
	return 0;

    nlaf = plas->la_file;
    if (nlaf->la_lfsd == plas->la_cfsd)
	return 0;

    if (dlas) {
	/* Don't grow dlas beyond highest int */
	lpos1 = dlas->la_file->la_nlines;
#ifdef LA_LONGFILES
	if (lpos1 + nlines < lpos1) {
#else
	if (lpos1 + nlines > la_ltrunc (lpos1 + nlines)) {
#endif
	    la_errno = LA_ERRMAXL;
	    return -1;
	}
	if (!la_zbreak (dlas))
	    return -1;
    }

    /* break fsds and make note of current position */
    Block {
	La_fsd *d1fsd;           /* dummy */
	La_linepos nlns;
	nlns = nlines;
	if (!la_break (plas, BRK_REAL, &d1fsd, &nfsd, &nlns
#ifdef LA_BP
	    , &nbytes
#endif
	    ))
	    return -1;
	nlines = nlns;
    }

    /*printf ("Break results: lines=%D, bytes=%D, nfsd=0x%04x\n",
	    nlines, nbytes, nfsd);*/
    /*la_fsddump (nlaf->la_ffsd, nfsd, YES, "");*/
    /*la_sdump (plas, "");*/

    /* unlink the fsds in the deleted area and relink around them */
    dffsd = plas->la_cfsd;
    dlfsd = nfsd->fsdback;
    if (dffsd->fsdback)
	dffsd->fsdback->fsdforw = nfsd;
    else
	nlaf->la_ffsd = nfsd;
    nfsd->fsdback = dffsd->fsdback;
    dlfsd->fsdforw = 0;         /* end of chain to be deleted */
    /*la_fsddump (nlaf->la_ffsd, nfsd, YES, "");*/

    /*  Adjust any other streams pointing at the same file
     *  Some of this readjusting has already been taken care of by la_break
     **/
    lpos1 = plas->la_lpos;
#ifdef LA_BP
    bpos1 = plas->la_bpos;
#endif
    Block {
	Reg1 La_stream *tlas;
	for (tlas = nlaf->la_fstream; tlas; tlas = tlas->la_fforw) {
	    if (tlas->la_lpos < lpos1) {
		if (tlas->la_lpos + tlas->la_rlines > lpos1)
		    tlas->la_rlines = 0;    /* reset reserved lines */
	    }
	    else {
		if (tlas->la_lpos < lpos1 + nlines) {
		    /* inside deleted area */
		    fill ((char *) tlas,
			  (unsigned int) sizeof (struct la_fpos), 0);
#ifdef LA_BP
		    tlas->la_bpos = bpos1;
#endif
		    tlas->la_lpos = lpos1;
		    tlas->la_cfsd = nfsd;
		    tlas->la_rlines = 0;    /* reset reserved lines */
		}
		else {
		    tlas->la_lpos -= nlines;
#ifdef LA_BP
		    tlas->la_bpos -= nbytes;
#endif
		}
	    }
	}
    }

    /* adjust the file size */
    nlaf->la_nlines -= nlines;
#ifdef LA_BP
    nlaf->la_nbytes -= nbytes;
#endif

    /*  if dlas is non-0, insert the deleted fsd chain into dlas at its
     *  current position else free up the deleted chain
     **/
    if (dlas)
	/* note that we did our own labreak above before unlinking the fsds
	 * so that this lalink can't fail.  If didn't do that, and the lalink
	 * should fail, we would have no recourse but to let go of the
	 * deleted lines forever.
	  */
	la_link (dlas, dffsd, dlfsd, nlines
#ifdef LA_BP
		 , nbytes
#endif
		);
    else
	la_freefsd (dffsd);

    plas->la_file->la_mode |= LA_MODIFIED;
    return nlines;
}
