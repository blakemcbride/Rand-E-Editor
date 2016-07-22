#ifdef COMMENT
Copyright       abandoned, 1983, The Rand Corporation
#endif

#include "ff.local.h"
#include <errno.h>

int
ff_close(ff)
    Reg3 Ff_stream *ff;
{
    if (FF_CHKF) {
	errno = EBADF;
	return -1;
    }
    if (--ff->f_count == 0)
	Block {
	Reg2 Ff_file   *fp;
	ff->f_mode = 0;
	fp = ff->f_file;
	ff->f_file = 0;
	if (--fp->fn_refs == 0)
	    Block {
	    Reg1 Ff_buf    *fb;
	    if (fp->fn_mode & 02)
		ff_sort(fp);
	    while (fb = fp->fb_qf)
		if (ff_putblk(fb, 1) == (Ff_buf *) 0)
		    return -1;
	    (void) close(fp->fn_fd);
	    fp->fn_fd = -1;
#ifdef  EUNICE
	    fp->fn_memaddr = (char *) 0;
#endif				/* EUNICE */
	    }
	}
    return 0;
}
