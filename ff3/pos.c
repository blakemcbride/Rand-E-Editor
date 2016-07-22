#ifdef COMMENT
Copyright       abandoned, 1983, The Rand Corporation
#endif


#include "ff.local.h"
#include <errno.h>

long 
ff_pos(ff)
    Reg1 Ff_stream *ff;
{
    if (FF_CHKF) {
	errno = EBADF;
	return -1;
    }
    return ff->f_offset;
}
