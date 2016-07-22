#ifdef COMMENT
Copyright       abandoned, 1983, The Rand Corporation
#endif


#include "ff.local.h"
#include <errno.h>

long
ff_grow(ff)
    Reg3 Ff_stream *ff;
{
    if (FF_CHKF) {
	errno = EBADF;
	return -1;
    }
    /*
     * If the last block of the file according to the old notion of its size
     * is in the cache, we have to see that it is written out if necessary
     * and released from the cache so that it will have to be read in again
     * with the correct new size 
     */
    Block {
	Reg1 Ff_buf    *fb;
	if ((fb = ff_haveblk(ff->f_file, ff->f_file->fn_size / FF_BSIZE))
	    && !ff_putblk(fb, 1)
	    )
	    return -1;
    }

    Block {
	Reg1 int        fd;
	Reg2 long       newsize;
	Reg4 long       oldseek;
	extern long     lseek();

	oldseek = lseek(fd = ff->f_file->fn_fd, (long) 0, 1);
	newsize = lseek(fd, (long) 0, 2);
	(void) lseek(fd, oldseek, 0);
	if (newsize > ff->f_file->fn_size)
	    return ff->f_file->fn_size = newsize;
	else
	    return ff->f_file->fn_size;
    }
}
