#ifdef COMMENT
Copyright       abandoned, 1983, The Rand Corporation
#endif


#include <c_env.h>
#include <localenv.h>

#ifdef UNIXV7
#include <sys/types.h>
#endif

#define FF_CHKF (ff < &ff_streams[0] || ff >= &ff_streams[NOFFFDS] || ff->f_file == 0)

#define CHKBLK  (!(fb = fp->fb_qf) || fb->fb_bnum != block || (ff_flist.fb_forw != fb && ff_flist.fb_forw->fb_forw != fb))

/*
 * If there were a way to do this with only a single division, I'd use it.
 * Used to be on V6 there was a ldiv routine that left the remainder in a
 * global 'ldivr', but that went away with V7.  --dave yost 
 *
 */
#define ldiv(a,b,c) ((*(c) = (a) % (b)), ((long)(a)) / (b))

/* the following include must appear in the right order */
#include "ff.h"

#ifndef NULL
#define NULL ((char *)0)
#endif

#define Block
