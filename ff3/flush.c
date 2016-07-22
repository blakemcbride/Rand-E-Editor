#ifdef COMMENT
Copyright       abandoned, 1983, The Rand Corporation
#endif


#include "ff.local.h"
#include <errno.h>

ff_flush(ff)
    Reg3 Ff_stream *ff;
{
    if (FF_CHKF) {
	errno = EBADF;
	return -1;
    }
    if (ff->f_mode & F_WRITE)
	Block {
	Reg1 Ff_file   *fp;
	Reg2 Ff_buf    *fb;
	fp = ff->f_file;
	ff_sort(fp);
	if (fb = fp->fb_qf)
	    do
		if (ff_putblk(fb, 0) == (Ff_buf *) 0)
		    return -1;
	    while (fb = fb->fb_qf);
	}
    return 0;
}
