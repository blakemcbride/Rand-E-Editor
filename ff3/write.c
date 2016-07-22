#ifdef COMMENT
Copyright       abandoned, 1983, The Rand Corporation
#endif

#include "ff.local.h"
#include <errno.h>

extern char    *move();

ff_write(ff, buf, count)
    Reg2 Ff_stream *ff;
    char           *buf;
    int             count;
{
    Reg4 int        cnt;
    Reg5 char      *from;

    if (FF_CHKF || !(ff->f_mode & F_WRITE)) {
	errno = EBADF;
	return -1;
    }
    ff_stats.fs_ffwrite++;

    cnt = count;
    from = buf;
    do
	Block {
	Reg1 int        num;
	int             offset;
	Reg3 Ff_buf    *fb;
	Block {
	    Reg6 long       block;
	    block = ldiv(ff->f_offset, FF_BSIZE, &offset);
	    num = FF_BSIZE - offset;
	    if (num > cnt)
		num = cnt;
	    if (!(fb = ff_gblk(ff->f_file, block, num < FF_BSIZE)))
		return count - cnt;	/* Return written count */
	}
	Block {
	    Reg6 char      *to;
	    to = &fb->fb_buf[offset];
	    ff->f_offset += num;
	    cnt -= num;
	    if ((offset += num) >= fb->fb_count)
		fb->fb_count = offset;
#ifdef DEBUG
	    if (fb->fb_count > FF_BSIZE) {
		write(2, "ff:write cnt past blk\n", 22);
		abort();
	    }
#endif
	    to = move(from, to, num);
	    from += num;
#ifdef DEBUG
	    if (to > &fb->fb_buf[FF_BSIZE]) {
		write(2, "ff:write ptr past blk\n", 22);
		abort();
	    }
#endif
	}
	fb->fb_wflg = 1;
    } while (cnt);

    if (ff->f_offset > ff->f_file->fn_size)
	ff->f_file->fn_size = ff->f_offset;

    return count;
}
