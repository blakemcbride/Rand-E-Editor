
#include "ff.local.h"

#define MIN(a,b) ((a)<(b)?(a):(b))

/*
 * We want nchars starting at addr in the file mapped into memory. Return the
 * number of characters mapped, and store the memory address in *buf. 
 */
int
ff_point(ff, addr, buf, nchars)
    Ff_stream      *ff;
    Reg1 long       addr;
    char          **buf;
    long            nchars;
{
    Reg2 int        offset;
    Ff_buf         *ffbuf;

    if ((ffbuf = ff_getblk(ff->f_file, addr / FF_BSIZE)) == NULL)
	return -1;
    *buf = &ffbuf->fb_buf[offset = addr % FF_BSIZE];
    return MIN(nchars, FF_BSIZE - offset);
}
