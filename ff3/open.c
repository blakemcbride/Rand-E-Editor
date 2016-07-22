/*
 * This file contains all data and routines which are required as an absolute
 * minimum to use the FF Package. One exception: div.c is required, but it
 * should be customized for the CPU used. 
 */

#ifdef COMMENT
Copyright       abandoned, 1983, The Rand Corporation
#endif

#include "ff.local.h"
#include <errno.h>
#include <sys/stat.h>

extern long     lseek();
extern char    *malloc();
extern char    *append();

Ff_stream      *ff_gfil();
Ff_stream      *_doffopen();

Ff_stream       ff_streams[NOFFFDS];
Ff_file         ff_files[NOFILE];
Ff_rbuf         ff_flist =
{
 (Ff_buf *) 0,
 (Ff_buf *) 0,
 (Ff_buf *) & ff_flist,
 (Ff_buf *) & ff_flist,
 0,
};
Ff_stats        ff_stats;

Ff_stream      *
ff_open(path, mode, arena)
    Reg1 char      *path;
    int             mode;
    int             arena;
{
    if (!path || *path == '\0')
	return 0;
    return _doffopen(path, 0, mode, arena);
}

Ff_stream      *
ff_fdopen(chan, mode, arena)
    int             chan;
    int             mode;
    int             arena;
{
    return _doffopen((char *) NULL, chan, mode, arena);
}

Ff_stream      *
_doffopen(path, chan, mode, arena)
    Reg5 char      *path;
    Reg6 int        chan;
    int             mode;
    int             arena;
{
    Reg3 Ff_file   *fp1;
    struct stat     fstbf;
    Reg4 int        md;
    char flpath [FL_PATH_SIZE];

    if (arena != 0)		/* for now */
	return 0;
    md = (mode + 1) | F_READ;	/* Always need this     */
    mode = md - 1;
    if (!ff_flist.fr_count && !ff_alloc(1, 0))
	return 0;

    memset (flpath, 0, sizeof (flpath));
    if (path) {
	/* open by file name */
	if (stat(path, &fstbf) < 0)
	    return 0;
	strncpy (flpath, path, sizeof (flpath) -1);
    } else {
	/* duplicate : open by file descriptor nb */
	Ff_file   *fp;
	if ( fstat (chan, &fstbf) < 0 )
	    return 0;
	for ( fp = ff_files ; fp < &ff_files[NOFILE] ; fp++ ) {
	    if ( fp->fn_refs <= 0 ) continue;
	    if ( fp->fn_fd == chan ) {
		strncpy (flpath, fp->fl_path, sizeof (flpath) -1);
		break;
	    }
	}
    }
    flpath [sizeof (flpath) -1] = '\0';

    fp1 = 0;
    Block {
	/* look for an Ff_file */
	Reg2 Ff_file   *fp;
	for (fp = ff_files; fp < &ff_files[NOFILE]; fp++) {
	    if (fp->fn_refs) {
		/* in use Ff_file, look for same file : {dev, ino} */
		if (
#ifdef UNIXV7
		    fp->fn_dev == fstbf.st_dev
		    && fp->fn_ino == fstbf.st_ino
#endif
#ifdef UNIXV6
		    fp->fn_minor == fstbf.st_minor
		    && fp->fn_major == fstbf.st_major
		    && fp->fn_ino == fstbf.st_inumber
#endif
		    ) {
		    /* found */
		    Reg1 Ff_stream *ffi;
		    if (ffi = ff_gfil (fp, md)) {
			/* Ff_file already connected to an Ff_stream */
			fp->fn_refs++;
			fp->fn_mode |= md;
			return ffi;
		    } else
			return 0;
		}
	    } else if (!fp1)
		/* not used, remember */
		fp1 = fp;
	}
    }

    if (!fp1) {
	/* nothing available */
	errno = EMFILE;
	return 0;
    }

    /* Ff_file found : fp1 */
    Block {
	Reg1 Ff_stream *ffp;
	Reg2 long       seekpos;

	if ( !(ffp = ff_gfil (fp1, md)) )
	    return 0;   /* already attached to Ff_stream */

	if (path) {
	    /* open the file by name, get the filedesc */
	    if ((chan = open (path, mode)) < 0) {
		ffp->f_count = 0;
		return 0;
	    }
	    seekpos = 0;
	} else
	    /* duplication : goto begining of file */
	    seekpos = lseek (chan, 0L, 1);
#ifdef UNIXV7
	fp1->fn_dev = fstbf.st_dev;
	fp1->fn_ino = fstbf.st_ino;
	/* fp1->fn_size = fstbf.st_size; */
#endif
#ifdef UNIXV6
	fp1->fn_minor = fstbf.st_minor;
	fp1->fn_major = fstbf.st_major;
	fp1->fn_ino = fstbf.st_inumber;
	/*
	 * fp1->fn_size = ((long)fstbf.st_size0) << 16; fp1->fn_size +=
	 * (long) (unsigned) fstbf.st_size1; 
	 */
#endif
	/* save the file name and size */
	fp1->fl_path  = append (flpath, "");
	fp1->fn_size = lseek (chan, (long) 0, 2);

#ifdef  EUNICE
	/* if (FD_FAB_Pointer[chan]->type == VAR_FILE) { */
	if (1) {
	    if ((fp1->fn_memaddr = malloc((unsigned int) fp1->fn_size))
		== NULL) {
	bad1:
		if (path)
		    (void) close(chan);
		else
		    (void) lseek(chan, seekpos, 0);
		/* something wrong */
		ffp->f_count = 0;
		return 0;
	    }
	    (void) lseek(chan, (long) 0, 0);
	    if ((fp1->fn_size = read(chan, fp1->fn_memaddr, fp1->fn_size))
		< 0) {
		free(fp1->fn_memaddr);
		goto bad1;
	    }
	    (void) realloc(fp1->fn_memaddr, fp1->fn_size);
	} else
	    fp1->fn_memaddr = (char *) 0;
#endif				/* EUNICE */
	(void) lseek(chan, seekpos, 0);
	fp1->fn_fd = chan;
	fp1->fn_mode = md;
	fp1->fn_refs = 1;
	fp1->fn_realblk = 0;
	fp1->fb_qf = (Ff_buf *) 0;
	fp1->style = fp1->user_style = 0;

	/* Done, return Ff_stream */
	return ffp;
    }
}

/* Found an un-used Ff_stream for the given Ff_file */
Ff_stream      *
ff_gfil (fnp, mode)
    Ff_file        *fnp;
    int             mode;
{
    Reg1 Ff_stream *fp;

    for (fp = ff_streams; fp->f_count;)
	if (++fp >= &ff_streams[NOFFFDS]) {
	    errno = EMFILE;
	    return 0;
	}
    fp->f_mode = mode;
    fp->f_file = fnp;
    fp->f_count = 1;
    fp->f_offset = 0;
    return fp;
}

ff_alloc (nbuf, arena)
    Reg2 int        nbuf;
    int             arena;
{
    if (arena != 0)
	return 0;
    while (nbuf--) {
	Reg1 Ff_buf    *fb;
	if ((fb = (Ff_buf *) malloc(sizeof(Ff_buf))) == NULL)
	    break;
	if ((fb->fb_buf = malloc((unsigned int) FF_BSIZE)) == NULL) {
	    free((char *) fb);
	    break;
	}
	(void) ff_use ((char *) fb, 0);
    }
    return ff_flist.fr_count;
}

ff_use (cp, arena)
    Reg1 char      *cp;
    int             arena;
{
#define fb ((Ff_buf *) cp)

    if (arena == 0) {
	ff_flist.fb_forw->fb_back = fb;
	fb->fb_forw = ff_flist.fb_forw;
	fb->fb_back = (Ff_buf *) & ff_flist,
	    ff_flist.fb_forw = fb;

	fb->fb_file = (Ff_file *) 0;
	fb->fb_bnum = -1;
	fb->fb_count = -1;
	fb->fb_qf = (Ff_buf *) 0;
	fb->fb_qb = (Ff_buf *) 0;
	fb->fb_wflg = 0;
	ff_flist.fr_count++;
    }
    return ff_flist.fr_count;
#undef fb
}

Ff_buf         *
ff_getblk (afp, blk)
    Ff_file        *afp;
    long            blk;
{
    return ff_gblk(afp, blk, 1);
}

Ff_buf         *
ff_gblk (fp, blk, rdflg)
    Reg3 Ff_file   *fp;
    Reg5 long       blk;
    Reg4 int        rdflg;
{
    Reg2 Ff_buf    *fb;

#ifdef  EUNICE
    if (fp->fn_memaddr) {
	static Ff_buf  *memfb;
	fb = memfb;
	goto ismem;
    } else
#endif				/* EUNICE */
    if (!(fb = ff_haveblk(fp, blk))) {
	Reg1 long       addr;
	fb = ff_flist.fb_back;
	if (!ff_putblk(fb, 1))
	    return 0;
#ifdef  EUNICE
ismem:
#endif				/* EUNICE */
	if ((fp->fn_mode & F_READ)
	    && ((addr = blk * FF_BSIZE) < fp->fn_size
		|| !(fp->fn_mode & F_WRITE)
		)
	    ) {
#ifdef  EUNICE
	    if (fp->fn_memaddr) {
		fb->fb_buf = &fp->fn_memaddr[addr];
		if ((fb->fb_count = fp->fn_size - addr) < 0)
		    return 0;
		return fb;
	    } else
#endif				/* EUNICE */
	    {
		if (fp->fn_realblk != blk) {
		    ff_stats.fs_seek++;
		    (void) lseek(fp->fn_fd, addr, 0);
		}
		if (rdflg) {
		    ff_stats.fs_read++;
		    if ((fb->fb_count = read(fp->fn_fd, fb->fb_buf, FF_BSIZE))
			== -1)
			return 0;
		} else
		    fb->fb_count = FF_BSIZE;
	    }
	    if (fb->fb_count == FF_BSIZE)
		fp->fn_realblk = blk + 1;
	    else if (fb->fb_count)
		fp->fn_realblk = -1;
	    else
		fp->fn_realblk = blk;
	} else
	    fb->fb_count = 0;
	fb->fb_bnum = blk;
	fb->fb_file = fp;
    }
    /*
     * If the block is not already the first or second block in the free
     * list, move it to head of free list to mark it as a recent reference 
     */
    if (fb != ff_flist.fb_forw && fb != ff_flist.fb_forw->fb_forw) {
	fb->fb_back->fb_forw = fb->fb_forw;
	fb->fb_forw->fb_back = fb->fb_back;
	ff_flist.fb_forw->fb_back = fb;
	fb->fb_forw = ff_flist.fb_forw;
	ff_flist.fb_forw = fb;
	fb->fb_back = (Ff_buf *) & ff_flist;
    }
    /*
     * If not one of first two queue entries, then move block to head of the
     * queue 
     */
    Block {
	Reg1 Ff_buf    *fb1;
	if (!fp->fb_qf
	    || (fp->fb_qf != fb
		&& (!fp->fb_qf->fb_qf
		    || fp->fb_qf->fb_qf != fb
		    )
		)
	    ) {
	    if (fb1 = fb->fb_qf)
		fb1->fb_qb = fb->fb_qb;
	    if (fb1 = fb->fb_qb)
		fb1->fb_qf = fb->fb_qf;
	    fb1 = fp->fb_qf;
	    fb->fb_qf = fb1;
	    if (fb1)
		fb1->fb_qb = fb;
	    fp->fb_qf = fb;
	    fb->fb_qb = (Ff_buf *) fp;
	}
    }
    return fb;
}

Ff_buf         *
ff_haveblk (fp, blk)
    Reg2 Ff_file   *fp;
    Reg3 long       blk;
{
    Reg1 Ff_buf    *fb;

    for (fb = fp->fb_qf; fb; fb = fb->fb_qf)
	if (fb->fb_bnum == blk)
	    return fb;
    return (Ff_buf *) 0;
}

Ff_buf         *
ff_putblk (fb, release)
    Reg3 Ff_buf    *fb;
    int             release;	/* if non-0, then release the block from the
				 * active chain */
{
    Reg2 Ff_file   *fp;
    Reg4 int        try;
    Reg5 int        cnt;

    if ((fp = fb->fb_file) == (Ff_file *) 0)
	return fb;
    if (fb->fb_wflg) {
	if (fp->fn_realblk != fb->fb_bnum) {
	    ff_stats.fs_seek++;
	    (void) lseek(fp->fn_fd, (long) (fb->fb_bnum * FF_BSIZE), 0);
	}
	ff_stats.fs_write++;
#ifdef DEBUG
	if (fb->fb_count > FF_BSIZE) {
	    write(2, "ff:putblk write > %d\n", 22, FF_BSIZE);
	    abort();
	}
#endif
	for (try = 1
	     ; (cnt = write(fp->fn_fd, fb->fb_buf, fb->fb_count))
	     != fb->fb_count
	     ; try++
	    ) {
	    if (try > 5 || cnt != -1 || errno != EINTR) {
		fp->fn_realblk = -1;
		return 0;
	    }
	}
	if (fb->fb_count == FF_BSIZE)
	    fp->fn_realblk = fb->fb_bnum + 1;
	else
	    fp->fn_realblk = -1;
	fb->fb_wflg = 0;
	fb->fb_count = 0;
    }
    /* take off the active list?  */
    if (release)
	Block {
	Reg1 Ff_buf    *fb1;
	fb->fb_qb->fb_qf = fb->fb_qf;	/* ! should check against nblks? */
	if (fb1 = fb->fb_qf)
	    fb1->fb_qb = fb->fb_qb;
	fb->fb_file = (Ff_file *) 0;
	fb->fb_qf = (Ff_buf *) 0;
	fb->fb_qb = (Ff_buf *) 0;
	}
    return fb;
}
