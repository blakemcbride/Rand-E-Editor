/*
 *  file la.h - include stuff for the la package
 */

#ifdef COMMENT
	Copyright abandoned, 1983, The Rand Corporation
#endif

#include <ff.h>

#ifndef YES
#define YES 1
#define NO  0
#endif

/* #define LA_LONGLINES            very esoteric */

/* You may still want this, even if your machine is not INT4 */
/* but you will probably run out of address space on large files. */
#ifdef INT4
# define LA_LONGFILES           /* not completely debugged */
#endif

#define LA_NLLINE  (CHARNBITS - 1)
#define LA_LLINE   (~(CHARMASK >> 1))
#define LA_MAX_NON_SPECIAL_FSD  ((1 << (LA_NLLINE + LA_NLLINE)) + ~LA_LLINE)

#ifdef LA_LONGLINES
# define LA_MAXLINE MAXLONG
  typedef int   La_linesize;    /* must not be unsigned */
#else
# define LA_MAXLINE LA_MAX_NON_SPECIAL_FSD
  typedef short La_linesize;    /* must not be unsigned */
#endif

#ifdef LA_LONGFILES
# define LA_MAXNLINES MAXLONG
typedef int  La_linepos;        /* must not be unsigned */
#else
# define LA_MAXNLINES MAXSHORT
typedef short La_linepos;       /* must not be unsigned */
#endif

typedef long  La_bytepos;       /* must not be unsigned */
typedef int La_flag;

/*  fsd - file segment descriptor.  Describes 1 to 127 contiguous lines
 *      in file.
 */
typedef struct lafsd
{
    struct lafsd *fsdback;  /* previous fsd in chain                       */
    struct lafsd *fsdforw;  /* next fsd in chain; 0 if last fsd            */
    long        fsdpos;     /* location of first character                 */
    struct lafile
	       *fsdfile;    /* lafile pointer of file containing lines.    */
			    /*  0 if this is the last fsd in chain.        */
    int         fsdnbytes;  /* number of bytes in this fsd if regular fsd  */
    char        fsdnlines;  /* number of lines in this fsd. 0 if this is   */
			    /*  the last fsd in chain.                     */
    char        fsdbytes[1];/* bytes describing the linelengths. See doc   */
 } La_fsd;
				/* byte size of fsd exclusive of fsdbytes */
typedef struct lafile {
    La_fsd          *la_ffsd;   /* first fsd in the chain */
    La_fsd          *la_lfsd;   /* last fsd in the chain */
    Ff_stream       *la_ffs;    /* pointer to an ff stream for this file */
    struct lastream *la_fstream;/* first in chain for this lafile */
    struct lastream *la_lstream;/* last  in chain for this lafile */
    La_linepos       la_fsrefs; /* number of refs by fsds */
    int              la_refs;   /* number of refs by La_streams */
    La_linesize      la_maxline;/* max line size in the file */
    La_linepos       la_nlines; /* number of lines in file */
#ifdef LA_BP
    La_bytepos       la_nbytes; /* number of bytes in file */
#endif
    char             la_mode;   /* mode bits (see below) */
 } La_file;
/* la_mode element of La_file: */
#define LA_NEW        01        /* no associated disk file */
#define LA_TMP        02        /* tmp file */
#define LA_MODIFIED   04        /* file has been modified */

typedef struct lastream {
    int              la_fsline; /* which line of current fsd starting at 0 */
    int              la_fsbyte; /* index into fsdbytes in current fsd */
    int              la_lbyte;  /* offset from beginning of current line */
    int              la_ffpos;  /* fsd fsdpos + this = offset in la_ffs */
    La_bytepos       la_bpos;   /* current byte position in file */
    La_linepos       la_lpos;   /* current line number */
    La_fsd          *la_cfsd;   /* current fsd */
    La_linepos       la_rlines; /* reserved lines from cur position */
    La_file         *la_file;   /* pointer to stream-independent stuff */
    struct lastream *la_sback;  /* chain of all streams */
    struct lastream *la_sforw;  /* chain of all streams */
    struct lastream *la_fback;  /* chain of streams into la_file */
    struct lastream *la_fforw;  /* chain of streams into la_file */
    char             la_sflags; /* stream flags */
 } La_stream;
/* la_sflags element of La_stream: */
#define LA_ALLOCED   001        /* open call alloced la_stream struc */
#define LA_STAY      002        /* stay at same line after insert there */
struct la_spos {                /* used by la_align */
    int              la_fsline;
    int              la_fsbyte;
    int              la_lbyte;
    int              la_ffpos;
    La_bytepos       la_bpos;
    La_linepos       la_lpos;
    La_fsd          *la_cfsd;
 };
struct la_fpos {                /* used by la_ldelete */
    int              la_fsline;
    int              la_fsbyte;
    int              la_lbyte;
    int              la_ffpos;
 };

#define la_modified(plas)   (((plas)->la_file->la_mode & LA_MODIFIED)? YES : NO)
#define la_domodify(plas)   ((plas)->la_file->la_mode |= LA_MODIFIED)
#define la_unmodify(plas)   ((plas)->la_file->la_mode &= ~LA_MODIFIED)

#define la_reserved(plas)   ((plas)->la_rlines)
#define la_setrlines(plas,l) ((plas)->la_rlines = l)

#define la_stay(plas)       (((plas)->la_sflags & LA_STAY) ? YES : NO)
#define la_stayset(plas)    ((plas)->la_sflags |= LA_STAY)
#define la_stayclr(plas)    ((plas)->la_sflags &= ~LA_STAY)

#define la_chan(plas)       (ff_fd ((plas)->la_file->la_ffs))
#define la_nstreams(plas)   ((plas)->la_file->la_refs)
#define la_linepos(plas)    ((plas)->la_lpos)
#define la_bytepos(plas)    ((plas)->la_bpos)
#define la_eof(plas)        ((plas)->la_cfsd == (plas)->la_file->la_lfsd)
#define la_lsize(plas)      ((plas)->la_file->la_nlines)
#define la_lmax(plas)       ((plas)->la_file->la_maxline)
#define la_sync(flg)        ff_sync (flg)
#ifdef LA_LONGFILES
#define la_ltrunc(i)        (i)
#else
#define la_ltrunc(i)        (((i)>32767)?32767:(i))
#endif
#ifdef LA_BP
#define la_bsize(plas)      ((plas)->la_file->la_nbytes)
#else
extern La_bytepos la_bsize ();
#endif

extern La_linepos   la_align (), la_blank (), la_close (), la_finsert ();
extern La_linepos   la_lcollect (), la_lcopy (), la_lcount (), la_ldelete ();
extern La_linepos   la_lflush (), la_linsert (), la_lrcollect ();
extern La_linepos   la_lreplace (), la_lseek ();
extern La_linesize  la_advance (), la_lget (), la_lpnt ();
extern La_linesize  la_lrsize (), la_lwsize ();
extern La_stream   *la_clone (), *la_open (), *la_other ();
extern int          la_tcollect ();

#define LA_NEWLINE ('\n')

/*  If any of these is not supplied by the user,
 *  a default one will be loaded from the library.
 */
extern void   la_abort();       /* called in some bug situations */
extern int    la_int();         /* is called periodically during some long  */
				/* operations, and if it returns non-0, the */
				/* operation is aborted */

/*  Either of these may be stored into by the user. */
extern int         la_maxchans; /* maximum system opens allowed to la */
extern char       *la_cfile;    /* the name to use for the changes file */
extern La_linesize la_maxline;  /* maximum line length allowed */
				/* affects la_open and la_lins, la_lrpl */

/* These are only to be modified by the library */
extern La_stream *la_chglas;    /* change file */
extern Ff_stream *la_chgffs;    /* change file */
extern La_flag    la_chgopen;   /* YES means change file has been la_opened */
extern int        la_chans;     /* number of system opens in use by la */
extern int        la_nbufs;     /* how many cache buffers to use */
extern La_stream *la_firststream;/* first in chain of open La_streams */
extern La_stream *la_laststream;/* last in chain of open La_streams */
extern int        la_errno;     /* last non-La_stream error that occurred */
extern La_flag    la_colstate;
/* la_errno values: */
#define LA_BADBAD     2 /* bad stream ptr to la_error call */
#define LA_INT        3 /* operation was interrupted */
#define LA_EOF        4 /* attempt to read past EOF */
#define LA_ERRMAXL    5 /* a file had too many lines to process */
#define LA_READERR    6 /* a read error was received from ffio */
#define LA_NONEWL     7 /* last character not a newline */
#define LA_WRTERR     8 /* a write error was received from ffio */
#define LA_NOCHG      9 /* can't open change file */
#define LA_NOMEM     10 /* no more alloc space */
#define LA_NOCHANS   11 /* no more system open channels left */
#define LA_INVMODE   12 /* invalid mode to open, lcount, lseek, or bseek */
#define LA_NOOPN     13 /* can't open file */
#define LA_BADSTREAM 14 /* bad lastream */
#define LA_FFERR     15 /* error in ff package */
#define LA_GETERR    16 /* la_getlin returned fewer than asked */
#define LA_NEGCOUNT  17 /* negative count arg to function */
#define LA_TOOLONG   18 /* line to long */
#define LA_NOTSAME   19 /* la_align of 2 streams into different files */
#define LA_BADCOLL   20 /* la_lcollects interleaved */
#define LA_BRKCOLL   21 /* la_lcollect broken by other insert */
