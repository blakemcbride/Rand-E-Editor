#ifdef COMMENT
	Copyright abandoned, 1983, The Rand Corporation
#endif

#ifdef COMMENT

 la_lseek - seek by line number

 type = 0  absolute line number
	1  relative line number
	2  relative from end of file

 It is currently not allowed to seek beyond end-of-file.  If you try to
 do so, you will simply be left at the end-of-file.

 Returns the current line number after the seek.

#endif

#include "la.local.h"

La_linepos
la_lseek (plas, nlines, type)
La_stream *plas;
Reg6 La_linepos nlines;
int type;
{
    Reg2 char *cp;
    Reg3 La_fsd *cfsd;
    unsigned int   fsoff;
#ifdef LA_BP
    La_linesize speclength;
    La_linesize lbyte;
    Reg5 La_bytepos bp;
#endif
#   define rlas ((La_stream *) cp)

    cp = (char *) plas;
    switch (type) {
    case 0:     /* relative to beginning of file */
	break;

    case 1:     /* relative to current position */
	if (nlines == 0)
	    goto cur;
	nlines += rlas->la_lpos;
	break;

    case 2:     /* relative to end of file */
	nlines += rlas->la_file->la_nlines;
	break;

    default:
	la_errno = LA_INVMODE;
	return -1;
    }

    /* bound nlines to be within the file */
    Block {
	La_linepos ldelta;
	if (nlines <= 0) {
	    nlines = 0;
	    goto begin;
	}
	else if (nlines >= rlas->la_file->la_nlines) {
	    nlines = 0;
	    goto end;
	}

	/* decide which is easier: relative to beginning, current, or end */
	else if
	    (   (ldelta = nlines - rlas->la_lpos) < 0
	     && -ldelta > rlas->la_fsline  /* won't be within this fsd */
	     && nlines * 2 <= rlas->la_lpos
	    ) {
	    /* go with relative to beginning */
 begin:     /*printf ("end %D\n", nlines);*/
	    rlas->la_lpos = 0;
#ifdef LA_BP
	    rlas->la_bpos = 0;
	    bp = 0;
#endif
	    rlas->la_cfsd = rlas->la_file->la_ffsd;
	    goto abs1;
	} else if
	   (   ldelta > 0
	    && ldelta >= rlas->la_cfsd->fsdnlines - rlas->la_fsline
		     /* won't be within this fsd */
	    && ldelta * 2 > rlas->la_file->la_nlines - rlas->la_lpos
	   ) {
	    /* convert to relative to end */
	    nlines -= rlas->la_file->la_nlines;
 end:       /*printf ("end %D\n", nlines);*/
	    rlas->la_lpos = rlas->la_file->la_nlines;
#ifdef LA_BP
	    rlas->la_bpos = bp = rlas->la_file->la_nbytes;
#endif
	    rlas->la_cfsd = rlas->la_file->la_lfsd;
 abs1:
	    rlas->la_fsline = 0;
	    rlas->la_fsbyte = 0;
	    rlas->la_ffpos  = 0;
	}
	else {
	    /* convert to relative to current */
	    nlines -= rlas->la_lpos;
 cur:       /*printf ("cur %D\n", nlines);*/
#ifdef LA_BP
	    lbyte = rlas->la_lbyte;
	    rlas->la_bpos = bp = rlas->la_bpos - lbyte;
	    rlas->la_ffpos -= lbyte;
#else
	    rlas->la_ffpos -= rlas->la_lbyte;
#endif
	}
    }

    rlas->la_lbyte = 0;
    if (nlines == 0)
	return rlas->la_lpos;

    rlas->la_lpos += nlines;
    cfsd = rlas->la_cfsd;
    if (nlines > 0) Block {
	Reg1 int   j;
	j = cfsd->fsdnlines - rlas->la_fsline;
	cp = &cfsd->fsdbytes[rlas->la_fsbyte];
	if (nlines >= j) {
	    for (;nlines >= j;) {
		/* we're going to have to go past this fsd */
		nlines -= j;
#ifdef LA_BP
		if (*cp) {
		    if (j == cfsd->fsdnlines)
			bp += cfsd->fsdnbytes;
		    else
			for (;j--;) {
#ifndef NOSIGNEDCHAR
			    if (*cp < 0)
				bp += -(*cp++) << LA_NLLINE;
#else
			    if (*cp & LA_LLINE)
				bp += - (*cp++ | LA_LLINE) << LA_NLLINE;
#endif
			    bp += *cp++;
			}
		}
		else {
		    cp++;
		    if (*cp++)
			bp++;
		    move (cp, (char *) &speclength, sizeof speclength);
		    bp += speclength;
		}
#endif
		if (!(cfsd = cfsd->fsdforw))
		    la_abort ("fsdforw problem in la_lseek");
		j = cfsd->fsdnlines;
		cp = cfsd->fsdbytes;
	    }
	    plas->la_fsline = 0;
	    plas->la_cfsd = cfsd;
	    fsoff = 0;
	}
	else
	    fsoff = plas->la_ffpos;
	/* If the fsd at this point is a special fsd, then nlines == 0 */
	plas->la_fsline += j = nlines;
	Block {
	    Reg4 int   ffpos;
	    ffpos = 0;
	    for (;j--;) {
#ifndef NOSIGNEDCHAR
		if (*cp < 0)
		    ffpos += -(*cp++) << LA_NLLINE;
#else
		if (*cp & LA_LLINE)
		    ffpos += - (*cp++ | LA_LLINE) << LA_NLLINE;
#endif
		ffpos += *cp++;
	    }
	    fsoff += ffpos;
#ifdef LA_BP
	    bp += ffpos;
#endif
	}
	plas->la_fsbyte = cp - cfsd->fsdbytes;
    }

    else Block { /* (nlines < 0) */
	Reg1 int   j;
	nlines = -nlines;
	j = rlas->la_fsline;
	if (nlines > j) {
	    cp = cfsd->fsdbytes;
	    /* got to go back past this fsd */
	    for (;nlines > j;) {
		/* we're going to have to go back past this fsd */
		nlines -= j;
#ifdef LA_BP
		if (j > 0 && *cp) {
		    if (j == cfsd->fsdnlines)
			bp -= cfsd->fsdnbytes;
		    else
			for (;j--;) {
#ifndef NOSIGNEDCHAR
			    if (*cp < 0)
				bp -= -(*cp++) << LA_NLLINE;
#else
			    if (*cp & LA_LLINE)
				bp -= - (*cp++ | LA_LLINE) << LA_NLLINE;
#endif
			    bp -= *cp++;
			}
		}
		else if (j > 0) {
		    cp++;
		    if (*cp++)
			bp--;
		    move (cp, (char *) &speclength, sizeof speclength);
		    bp -= speclength;
		}
#endif
		if (!(cfsd = cfsd->fsdback))
		    la_abort ("fsdback problem in la_lseek");
		j = cfsd->fsdnlines;
		cp = cfsd->fsdbytes;
	    }
	    plas->la_cfsd = cfsd;
	    plas->la_fsline = j -= nlines;
	    Block {
		Reg4 int   ffpos;
		ffpos = 0;
		for (;j--;) {
#ifndef NOSIGNEDCHAR
		    if (*cp < 0)
			ffpos += -(*cp++) << LA_NLLINE;
#else
		    if (*cp & LA_LLINE)
			ffpos += - (*cp++ | LA_LLINE) << LA_NLLINE;
#endif
		    ffpos += *cp++;
		}
		fsoff = ffpos;
	    }
	    plas->la_fsbyte = cp - cfsd->fsdbytes;
#ifdef LA_BP
	    if (*cp)
		for (j = nlines; j--; ) {
#ifndef NOSIGNEDCHAR
		    if (*cp < 0)
			bp -= -(*cp++) << LA_NLLINE;
#else
		    if (*cp & LA_LLINE)
			bp -= - (*cp++ | LA_LLINE) << LA_NLLINE;
#endif
		    bp -= *cp++;
		}
	    else {
		cp++;
		if (*cp++)
		    bp--;
		move (cp, (char *) &speclength, sizeof speclength);
		bp -= speclength;
	    }
#endif
	} else Block {
	    /*printf ("within %D\n", nlines);*/
	    /* walk back within this fsd */
	    /* can't be a spceial fsd */
	    Reg4 int   ffpos;
	    cp = &cfsd->fsdbytes[rlas->la_fsbyte - 1];
	    plas->la_fsline -= j = nlines;
	    ffpos = 0;
	    for (;j--;) {
		ffpos += *cp;
#ifndef NOSIGNEDCHAR
		if (*--cp < 0)
		    ffpos += -(*cp--) << LA_NLLINE;
#else
		if (*--cp & LA_LLINE)
		    ffpos += - (*cp-- | LA_LLINE) << LA_NLLINE;
#endif
	    }
	    fsoff = plas->la_ffpos - ffpos;
#ifdef LA_BP
	    bp -= ffpos;
#endif
	    plas->la_fsbyte = cp + 1 - cfsd->fsdbytes;
	}
    }
    plas->la_ffpos = fsoff;
#ifdef LA_BP
    plas->la_bpos = bp;
#endif

    return plas->la_lpos;
}
