#ifdef COMMENT
Copyright       abandoned, 1983, The Rand Corporation
#endif

#include "ff.local.h"
#include <errno.h>

extern char    *move();

/* VARARGS4 */
ff_read(ff, buf, count, brkcnt, brklst)
    Reg3 Ff_stream *ff;
    char           *buf;
    int             brkcnt;
    char           *brklst;
{
    Reg5 int        cnt;
    Reg6 char      *to;

    if (FF_CHKF || !(ff->f_mode & F_READ)) {
	errno = EBADF;
	return -1;
    }
    ff_stats.fs_ffread++;

    cnt = count;
    to = buf;
    do
	Block {
	/* once per block */
	Reg2 int        rn;
	long            block;
	Ff_buf         *fb;
	int             offset;
	block = ldiv(ff->f_offset, FF_BSIZE, &offset);
	rn = FF_BSIZE - offset;
	if (rn > cnt)
	    rn = cnt;
	if (!(fb = ff_getblk(ff->f_file, block)))
	    return -1;
	Block {
	    Reg1 long       dn;
	    dn = ff->f_file->fn_size - ff->f_offset;
	    if (dn <= 0)
		break;
	    if (dn < rn)
		rn = dn;
	}
	Block {
	    Reg1 char      *from;
	    Reg4 int        tmp;
	    from = &fb->fb_buf[offset];
	    if (brkcnt)
		rn = lenbrk(from, tmp = rn, brkcnt, brklst);
	    if (to)
		to = move(from, to, rn);
#ifdef DEBUG
	    if (from + rn > &fb->fb_buf[FF_BSIZE]) {
		write(2, "ff:read ptr past blk\n", 21);
		abort();
	    }
#endif
	    ff->f_offset += rn;
	    cnt -= rn;
	    if (brkcnt && rn <= tmp)
		break;
	}
    } while (cnt);
    return count - cnt;
}

lenbrk(buf, rn, brkcnt, brklst)
    char           *buf;
    Reg5 int        rn;
    Reg6 int        brkcnt;	/* must be > 0 */
    char           *brklst;
{
    Reg4 char      *from;

    for (from = buf; rn--;)
	Block {
	Reg1 char      *sp;
	Reg2 int        chr;
	Reg3 int        nbrk;
	chr = *from++;
	sp = brklst;
	nbrk = brkcnt;
	do {
	    if (chr == *sp++)
		goto ret;
	} while (--nbrk);
	}
ret:
    return from - buf;
}
