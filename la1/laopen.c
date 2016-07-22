#ifdef COMMENT
	Copyright abandoned, 1983, The Rand Corporation
#endif

#include "la.local.h"

#include <sys/stat.h>
#include <sys/resource.h>
#include <limits.h>

/* these are the actual global definitions */
int         la_maxchans = _NFILE;/* maximum system opens allowed to la */
La_linesize la_maxline = LA_MAXLINE;/* maximum line length allowed */
				/* affects la_open and la_lins, la_lrpl */
La_stream  *la_chglas;
Ff_stream  *la_chgffs;
La_flag     la_chgopen;         /* change file is open */
int         la_chans;           /* how many channels used by la package */
int         la_nbufs = 10;      /* how many cache buffers to use */
La_stream  *la_firststream;     /* first stream is always the changes file */
La_stream  *la_laststream;
int         la_errno;           /* last non-La_stream error that occurred */

static long max_lines = 0;      /* max number of line in a file */

extern La_stream  *la_newstream ();
extern La_stream  *la_ffopen ();
extern char *malloc ();

static max_lines_nb ()
{
    int cc, nb;
    struct rlimit rlim;

    max_lines = (sizeof (La_linepos) > sizeof (short)) ? INT_MAX : SHRT_MAX;
    cc = getrlimit (RLIMIT_DATA, &rlim);
    if ( cc == 0 ) {
	nb = rlim.rlim_cur / (2 * (LA_FSDSIZE + LA_FSDBMAX));
	if ( nb < max_lines ) max_lines = nb;
    }
}

char * la_max_size ()
{
    static char buf [128];
    long sz;

    if ( ! max_lines ) max_lines_nb ();
    sprintf (buf, "max line size : %d, max number of lines : %d",
	    la_maxline, max_lines);
    return buf;
}


/* VARARGS3 */
La_stream *
la_open (filename, modestr, plas, offset, pffs, chan)
Reg4 char *filename;
char *modestr;
long offset;
Reg5 La_stream *plas;
Ff_stream *pffs;
int chan;
{
    Reg2 char mode;
    Reg3 char cflag;

    mode = 0;
    cflag = 0;
    Block {
	Reg1 char *cp;
	for (cp = modestr; *cp; cp++) {
	    switch (*cp) {
	    case 'n':
		mode |= LA_NEW;
		break;

	    case 't':
		mode |= LA_TMP | LA_NEW;
		break;

	    case 'c':
		cflag = YES;
		break;

	    default:
		la_errno = LA_INVMODE;
		return NULL;
	    }
	}
    }

    if (cflag && (mode & (LA_TMP | LA_NEW))) {
	la_errno = LA_INVMODE;
	return NULL;
    }

    if (mode & LA_NEW)
	return la_ffopen ((Ff_stream *) 0, plas, (long) 0);

    Block {
	Reg1 Ff_stream *ffl;
	if (!filename || *filename == '\0') {
	    if (pffs)
		return la_ffopen (pffs, plas, offset);
	    if (ffl = ff_fdopen (chan, 0, 0)) Block {
		La_stream *nlas;
     openit:    if ((nlas = la_ffopen (ffl, plas, offset)) == NULL) {
		    (void) ff_close (ffl);
		    return NULL;
		}
		return nlas;
	    }
	}
	else Block {
	    struct stat st;
	    if (ffl = ff_open (filename, 0, 0))
		goto openit;
	    if (cflag && stat (filename, &st) == -1)
		return la_ffopen ((Ff_stream *) 0, plas, (long) 0);
	}
    }
    la_errno = LA_NOOPN;
    return NULL;
}


static La_file *la_build_fsd ( Ff_stream *pffs, long pos, La_file *tlaf)
{
    La_fsd    *ffsd;
    La_fsd    *lfsd;
    long totlft;
    char nonewl;    /* must be char */

    if ( pffs ) {
	totlft = pffs->f_file->fn_size - pos;
	/* parse the file */
	if ( (tlaf->la_nlines =
	     la_parse (pffs, pos, &ffsd, &lfsd, tlaf, totlft, &nonewl))
	     < 0 )
	    return NULL;

	tlaf->la_ffsd = ffsd;
#ifdef LA_BP
	tlaf->la_nbytes = totlft + nonewl;
#endif
	/* if there were any lines in the file, make an fsd */
	/* with no lines in it and link it to the end of the chain */
	if (tlaf->la_nlines > 0) {
	    if (la_parse ((Ff_stream *) 0, (La_bytepos) 0, &ffsd, &ffsd,
			  la_chglas->la_file, (La_bytepos) 0, "") < 0)
		return NULL;

	    lfsd->fsdforw = ffsd;
	    ffsd->fsdback = lfsd;
	}
	la_chans++;
    }
    else {
	if (la_parse ((Ff_stream *) 0, (La_bytepos) 0, &ffsd, &ffsd,
		      la_chglas->la_file, (long) 0, "") < 0)
	    return NULL;

	tlaf->la_ffsd = ffsd;
#ifdef LA_BP
	tlaf->la_nbytes = 0;
#endif
    }
    tlaf->la_lfsd = ffsd;
    return tlaf;
}

La_stream *
la_ffopen (pffs, plas, pos)
La_stream *plas;
Reg2 Ff_stream *pffs;
long pos;
{
    Reg4 La_stream  *nlas;     /* the new La_stream struct we are making */
    Reg3 La_file    *tlaf;
    La_fsd          *ffsd;
    La_fsd          *lfsd;

    if (!la_chgopen) Block {
	Reg1 int regi;
	static La_stream chglas;
	la_chgopen = YES;   /* must be here because of recursion */
	if ((regi = la_nbufs - ff_flist.fr_count) > 0)
	    (void) ff_alloc (regi, 0);
	(void) unlink (la_cfile);
	if (   (regi = creat (la_cfile, 0600)) == -1
	    || close (regi) == -1
	    || (la_chgffs = ff_open (la_cfile, 2, 0)) == (Ff_stream *) 0
	    || (la_chglas = la_open ((char *) 0, "", &chglas, (La_bytepos) 0,
				     la_chgffs)
	       ) == NULL
	   ) {
	    la_chgopen = NO;
	    la_errno = LA_NOCHG;
	    return NULL;
	}
    }

    if (!(nlas = la_newstream (plas)))
	return NULL;

    /* see if this is another stream into one we have already */
    if (pffs) {
	if (pffs->f_file->fn_refs > 1) Block {
	    La_stream *tlas;
	    /* let's see if we have this file already */
	    for (tlas = la_firststream; tlas; tlas = tlas->la_sforw) {
		if (ff_fd (tlas->la_file->la_ffs) == ff_fd (pffs)) {
		    /* yes, it is */
		    if ( ! tlas->la_file->la_ffsd ) {
			/* closed but referenced */
			/* Not fully debuged, but currently not called */
			if ( ! la_build_fsd (pffs, pos, tlas->la_file) )
			    return NULL;
		    }
		    return la_clone (tlas, nlas);
		}
	    }
	}
    }

    if (pffs && la_chans >= la_maxchans) {
	la_errno = LA_NOCHANS;
	goto bad;
    }

    /* We have to make a new la_file for this stream */
    if ((tlaf = (La_file *) malloc ((unsigned int) sizeof *tlaf)) == NULL) {
	la_errno = LA_NOMEM;
	    goto bad;
    }
    fill ((char *) tlaf, (unsigned int) sizeof *tlaf, 0);
    if (!pffs)
	tlaf->la_mode = LA_NEW;
    tlaf->la_ffs = pffs ? pffs : la_chglas->la_file->la_ffs;
    tlaf->la_refs = 1;

    if ( ! la_build_fsd (pffs, pos, tlaf) ) {
	free ((char *) tlaf);
 bad:   if (nlas->la_sflags & LA_ALLOCED)
	    free ((char *) nlas);
	return NULL;
    }

#if 0 /* ============================= */
    if (pffs) Block {
	long totlft;
	char nonewl;    /* must be char */
	totlft = pffs->f_file->fn_size - pos;
	/* parse the file */
	if ((tlaf->la_nlines =
	    la_parse (pffs, pos, &ffsd, &lfsd, tlaf, totlft, &nonewl))
	    < 0) {
 bad1:      free ((char *) tlaf);
 bad:       if (nlas->la_sflags & LA_ALLOCED)
		free ((char *) nlas);
	    return NULL;
	}
	tlaf->la_ffsd = ffsd;
#ifdef LA_BP
	tlaf->la_nbytes = totlft + nonewl;
#endif
	/* if there were any lines in the file, make an fsd */
	/* with no lines in it and link it to the end of the chain */
	if (tlaf->la_nlines > 0) {
	    if (la_parse ((Ff_stream *) 0, (La_bytepos) 0, &ffsd, &ffsd,
			  la_chglas->la_file, (La_bytepos) 0, "") < 0)
		goto bad1;
	    lfsd->fsdforw = ffsd;
	    ffsd->fsdback = lfsd;
	}
	la_chans++;
    }
    else {
	if (la_parse ((Ff_stream *) 0, (La_bytepos) 0, &ffsd, &ffsd,
		      la_chglas->la_file, (long) 0, "") < 0)
	    goto bad1;
	tlaf->la_ffsd = ffsd;
#ifdef LA_BP
	tlaf->la_nbytes = 0;
#endif
    }

    tlaf->la_lfsd = ffsd;
#endif /* ============================= */

    Block {
	Reg1 char sflags;
	sflags = nlas->la_sflags;
	fill ((char *) nlas, (unsigned int) sizeof *nlas, 0);
	nlas->la_sflags = sflags;
    }

    nlas->la_cfsd = tlaf->la_ffsd;
    nlas->la_file = tlaf;
    la_makestream (nlas, tlaf);
    return nlas;
}

La_stream *
la_clone (oldlas, newlas)
Reg3 La_stream *oldlas;
La_stream *newlas;
{
    Reg1 La_stream *nlas;
    Reg2 La_file   *tlaf;

    if (!la_verify (oldlas))
	return NULL;

    if (!(nlas = la_newstream (newlas)))
	return NULL;

    tlaf = oldlas->la_file;
    tlaf->la_refs++;

    Block {
	Reg4 char sflags;
	sflags = nlas->la_sflags;
	/* the following move is really: *nlas = *oldlas; */
	move ((char *) oldlas, (char *) nlas, (unsigned int) sizeof *nlas);
	nlas->la_sflags = sflags;
    }
    la_makestream (nlas, tlaf);
    return nlas;
}

la_verify (plas)
Reg2 La_stream *plas;
{
    Reg1 La_stream *tlas;

    for (tlas = la_firststream; tlas; tlas = tlas->la_sforw)
	if (tlas == plas)
	    return YES;
    la_errno = LA_BADSTREAM;
    return NO;
}

La_stream *
la_newstream (plas)
Reg1 La_stream *plas;
{
    if (!plas) {
	if ((plas = (La_stream *) malloc ((unsigned int) sizeof *plas))
	    == NULL) {
	    la_errno = LA_NOMEM;
	    return NULL;
	}
	plas->la_sflags = LA_ALLOCED;
    }
    fill ((char *) plas, (unsigned int) sizeof *plas, 0);
    return plas;
}

la_makestream (plas, plaf)
Reg1 La_stream *plas;
Reg2 La_file *plaf;
{
    plas->la_sforw = 0;
    plas->la_sback = la_laststream;
    if (!la_firststream)
	la_firststream = plas;
    if (la_laststream)
	la_laststream->la_sforw = plas;
    la_laststream = plas;

    plas->la_fforw = 0;
    plas->la_fback = plaf->la_lstream;
    if (!plaf->la_fstream)
	plaf->la_fstream = plas;
    if (plaf->la_lstream)
	plaf->la_lstream->la_fforw = plas;
    plaf->la_lstream = plas;
    return;
}

La_linepos
la_parse (pffs, seekpos, ffsd, lfsd, plaf, nchars, buf)
Ff_stream     *pffs;     /* NULL means parse memory buffer */
long           seekpos;
La_fsd       **ffsd;
La_fsd       **lfsd;
La_file       *plaf;
La_bytepos     nchars;
char          *buf;
{
    /*  The following union is a trick to get a character array 'b'
     *  aligned so that b[2] falls on a long boundary.
      */
    union fsb {
	struct {
	    char        _bchr[6];
	    char        b[LA_FSDBMAX];
	} bytes;
	struct {
	    char        _schr[8];
	    La_linesize len; /* len must be on a long boundary AND be */
	} spcl;
    };
    union fsb       fsb;        /* a tmp array for fsdbytes */
    Reg1 char      *cp;
    Reg2 int        nlft;       /* number of chars left in current block */
    Reg3 La_fsd    *cfsd;       /* current fsd */
    Reg5 La_bytepos totlft;     /* total characters left to parse */
    La_bytepos      ototlft;    /* old totlft */
    Reg6 La_flag    nonewline;
    int             nfsb;       /* number of fsdbytes */
    La_bytepos      totchcnt;   /* total character count */
    long            fsdchcnt;   /* fsd character count (long is correct) */
    long            lcnt;       /* fsd line count */
    long            totlcnt;    /* total # lines in this new chain */
    long            curpos;     /* current pos for ff_point */
    La_linesize     maxline;    /* maximum chars in any line we have parsed */
#ifdef LA_LONGLINES
    La_flag         longline;
#endif
    La_flag         toobigfsd;

    *ffsd = 0;
    *lfsd = 0;
    cfsd = 0;
    totlcnt = 0;
    nfsb = 0;
    totchcnt = 0;
    fsdchcnt = 0;
#ifdef LA_LONGLINES
    longline = NO;
#endif
    toobigfsd = NO;
    nonewline = 0;
    maxline = plaf->la_maxline;

    if ( ! max_lines ) max_lines_nb ();

    if (nchars < 0) {
	la_errno = LA_NEGCOUNT;
	goto err;
    }
    if (pffs) {
	if ( *buf ) *buf = 0;   /* do not write into a constant string ! */
	nlft = 0;
	totlft = nchars;
	curpos = seekpos;
    }
    else {
	cp = buf;
	if (   nchars > 0
	    && cp[nchars - 1] != LA_NEWLINE
	   ) {
	    la_errno = LA_NONEWL;
	    goto err;
	}
    }

    /* do this loop once per line */
    for (lcnt = 0; ;lcnt++) Block {
	Reg4 long lnchcnt;     /* character count in current line */
	La_linesize savelength;
#ifdef lint
	savelength = 0;
#endif
	/* if it's time to make an fsd, do so */
	if (   lcnt >= LA_FSDLMAX
	    || totchcnt >= nchars
	    || nfsb >= LA_FSDBMAX - 2
	   ) {
 makefsd:   if (totlcnt + lcnt >= max_lines) {
		/* can only happen if La_linepos is typedefed to short */
		la_errno = LA_ERRMAXL;
		goto err;
	    }
	    if (la_int ()) {
		la_errno = LA_INT;
		goto err;
	    }
	    if ((cfsd = (La_fsd *) malloc ((unsigned int) (LA_FSDSIZE + nfsb)))
		== NULL) {
		la_errno = LA_NOMEM;
		goto err;
	    }
	    cfsd->fsdforw = 0;
	    cfsd->fsdback = *lfsd;
	    cfsd->fsdpos = seekpos;
	    seekpos += fsdchcnt;
	    cfsd->fsdfile = plaf;
	    cfsd->fsdnbytes = fsdchcnt;
	    if (cfsd->fsdnlines = lcnt)
		plaf->la_fsrefs++;
	    move (fsb.bytes.b, cfsd->fsdbytes, (unsigned int) nfsb);
	    if (*ffsd)
		(*lfsd)->fsdforw = cfsd;
	    else
		*ffsd = cfsd;
	    *lfsd = cfsd;
	    totlcnt += lcnt;
	    if (totchcnt >= nchars)
		break;
 makefirst:
#ifdef LA_LONGLINES
	    if (nonewline || longline) {
#else
	    if (nonewline) {
#endif
		lcnt = 1;
		fsb.bytes.b[0] = 0;
		fsb.bytes.b[1] = nonewline;
		fsb.spcl.len = savelength;
		fsdchcnt = savelength;
		totchcnt += savelength + nonewline;
		nfsb = 6;
		nonewline = 0;
#ifdef LA_LONGLINES
		longline = NO;
#endif
		goto makefsd;
	    }
	    else {
		lcnt = 0;
		nfsb = 0;
		fsdchcnt = 0;
		if (toobigfsd) {
		    toobigfsd = NO;
		    lnchcnt = savelength;
		    goto makebytes;
		}
	    }
	}

	/* count the characters in the current line */
	if (pffs) {     /* parsing from a file */
	    ototlft = totlft + nlft;
	    /* parse a line.  */
	    do {
		if (--nlft < 0) Block {
		    /* need a block to work on */
		    char *newcp;
		    if (la_int ()) {
			la_errno = LA_INT;
			goto err;
		    }
		    if ((nlft = ff_point (pffs, curpos, &newcp, (long) totlft))
			< 0) {
			la_errno = LA_FFERR;
			goto err;
		    }
		    curpos += nlft;
		    if (nlft == 0) {
			/* end of file */
			nchars -= totlft; /* truncate nchars to file length */
			ototlft -= totlft;
			totlft = 0;
			if (ototlft == 0) {
			    /* end of file before any chars in line */
			    goto makefsd;
			}
			/*
			 *  We get to this point when we have been asked
			 *  to parse more than there is in the file
			 */
		    }
		    if (totlft <= 0) {
			/* don't want to parse any more */
			nlft = 0;
			nonewline = 1;  /* must be 1 */
			*buf = 1;
			break;
		    }
		    cp = newcp;
		    totlft -= nlft--;
		}
	    } while (*cp++ != LA_NEWLINE);
	    lnchcnt = ototlft - (totlft + nlft);
	    if (maxline < lnchcnt + nonewline)
		maxline = lnchcnt + nonewline;
	}
	else Block {    /* parsing an array */
	    char *ocp;
	    ocp = cp;
	    while (*cp++ != LA_NEWLINE)
		continue;
	    lnchcnt = cp - ocp;
	    if (maxline < lnchcnt)
		maxline = lnchcnt;
	}
	if (   maxline > la_maxline
	    || lnchcnt < 0
	   ) {
	    la_errno = LA_TOOLONG;
	    goto err;
	}

	/* make the fsdbytes for the line */
	if (lnchcnt <= LA_MAX_NON_SPECIAL_FSD) {
	    if (nonewline)
		goto save;
	    if (fsdchcnt + lnchcnt > (long) LA_FSDNBMAX) {
		toobigfsd = YES;
 save:          savelength = lnchcnt;
		if (lcnt)
		    goto makefsd;
		else
		    goto makefirst;
	    }
 makebytes:
	    Block {
		int ttmp;
		fsdchcnt += ttmp = lnchcnt;
		totchcnt += ttmp;
		if (ttmp > ~LA_LLINE) {
		    fsb.bytes.b[nfsb++] = -(ttmp >> LA_NLLINE);
		    ttmp &= ~LA_LLINE;
		}
		fsb.bytes.b[nfsb++] = ttmp;
	    }
	}
	else {
#ifdef LA_LONGLINES
	    longline = YES;
	    goto save;
#else
	    la_errno = LA_TOOLONG;
	    goto err;
#endif
	}
    }

    plaf->la_maxline = maxline;
    return totlcnt;

 err:
    la_freefsd (*ffsd);
    return -1;
}

la_freefsd (tfsd)
Reg1 La_fsd *tfsd;
{
    Reg2 La_fsd *nfsd;

    while (tfsd) {
	nfsd = tfsd->fsdforw;
	if (tfsd->fsdnlines)
	    tfsd->fsdfile->la_fsrefs--;
	free ((char *) tfsd);
	tfsd = nfsd;
    }
    return;
}

int la_stream_is_allocated (La_stream *plas)
{
    if ( !plas ) return NO;
    return ((plas->la_sflags & LA_ALLOCED) != 0);
}
