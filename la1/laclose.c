#ifdef COMMENT
	Copyright abandoned, 1983, The Rand Corporation
#endif

#include "la.local.h"

int la_can_be_closed (La_stream *plas)
/*  return the number of reference (la_fsrefs) outside this la_file
 *      if 0 : the f_stream and file will be close by la_close
 *        -1 : error
 */
{
    int nb;
    La_file *nlaf;
    La_fsd *tfsd;

    if ( !la_verify (plas) )
	return -1;

    nlaf = plas->la_file;
    if ( (plas == la_chglas) || (nlaf->la_refs < 1) ) {
	la_errno = LA_BADSTREAM;
	return -1;
    }

    nb = nlaf->la_fsrefs;
    tfsd = nlaf->la_ffsd;
    for (tfsd = nlaf->la_ffsd ; tfsd ; tfsd = tfsd->fsdforw ) {
	if ( tfsd->fsdnlines ) nb--;
    }
    return nb;
}

/* la_close : close a La_stream
 *  return -1 error, see la_errno
 *          0 the La_stream is close and removed
 *         >0 the value of la_fsrefs in the associated La_file
 */

La_linepos
la_close (plas)
La_stream *plas;
{
    La_file *nlaf;
    La_flag freelaf;

    if (!la_verify (plas))
	return -1;

    nlaf = plas->la_file;
    if ( (plas == la_chglas) || (nlaf->la_refs < 1) ) {
	la_errno = LA_BADSTREAM;
	return -1;
    }

    freelaf = NO;
    if (nlaf->la_refs > 1)
	nlaf->la_refs--;
    else {
	if (nlaf->la_fsrefs > 0) {
	    la_freefsd (nlaf->la_ffsd);
	    nlaf->la_ffsd = nlaf->la_lfsd = NULL;
	    plas->la_cfsd = NULL;
	}
	if (nlaf->la_fsrefs > 0)
	    /* still some references to this file */
	    return nlaf->la_fsrefs;

	else if (nlaf->la_fsrefs < 0)
	    la_abort ("nlaf->la_fsrefs < 0 in la_close");
	else {
	    (void) ff_close (nlaf->la_ffs);
	    freelaf = YES;
	    la_chans--;
	}
    }

    /* unlink the stream from the chain of all streams */
    if (plas->la_sback)
	plas->la_sback->la_sforw = plas->la_sforw;
    if (plas->la_sforw)
	plas->la_sforw->la_sback = plas->la_sback;
    else
	la_laststream = plas->la_sback;

    /* unlink the stream from the chain streams into this file */
    if (plas->la_fback)
	plas->la_fback->la_fforw = plas->la_fforw;
    else
	nlaf->la_fstream = plas->la_fforw;
    if (plas->la_fforw)
	plas->la_fforw->la_fback = plas->la_fback;
    else
	nlaf->la_lstream = plas->la_fback;

    if (freelaf) {
	free ((char *) nlaf);
	plas->la_file = NULL;
    }
    if (plas->la_sflags & LA_ALLOCED)
	free ((char *) plas);
    return 0;
}
