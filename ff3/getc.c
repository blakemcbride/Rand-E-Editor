#ifdef COMMENT
Copyright       abandoned, 1983, The Rand Corporation
#endif


#include "ff.local.h"
#include <errno.h>

ff_getc(ff)
    Reg3 Ff_stream *ff;
{
    Reg4 long       block;
    int             offset;

    if (FF_CHKF || !(ff->f_mode & F_READ)) {
	errno = EBADF;
	return EOF;
    }
    if (ff->f_offset >= ff->f_file->fn_size) {
	errno = 0;
	return EOF;
    }
    block = ldiv(ff->f_offset, FF_BSIZE, &offset);

    Block {
	Reg1 Ff_buf    *fb;
	Reg2 Ff_file   *fp;
	fp = ff->f_file;
	if (CHKBLK)
	    if (!(fb = ff_getblk(fp, block)))
		return EOF;	/* errno will have been set by getblk */
#ifdef DEBUG
	if (offset >= fb->fb_count) {
	    write(2, "ff:getc ptr past count\n", 23);
	    abort();
	}
#endif
	ff->f_offset++;
	return fb->fb_buf[offset] & 0377;
    }
}
