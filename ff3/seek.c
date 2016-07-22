#ifdef COMMENT
Copyright       abandoned, 1983, The Rand Corporation
#endif

#include "ff.local.h"
#include <errno.h>

long
ff_seek(ff, pos, whence)
    Reg1 Ff_stream *ff;
    Reg2 long       pos;
    int             whence;
{
    if (FF_CHKF) {
	errno = EBADF;
	return -1;
    }
    ff_stats.fs_ffseek++;

    switch (whence) {
    default:
    case 0:
	return ff->f_offset = pos;

    case 1:
	return ff->f_offset += pos;

    case 2:
	return ff->f_offset = ff->f_file->fn_size + pos;
    }
}
