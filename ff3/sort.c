#ifdef COMMENT
Copyright       abandoned, 1983, The Rand Corporation
#endif

#include "ff.local.h"

/*
 * sort the buffers associated with a file by block numbers so that when
 * flushed out, minimum seeking will take place 
 */
ff_sort(fp)
    Reg4 Ff_file   *fp;
{
    Reg2 Ff_buf    *fb;
    Reg3 char       change;

    do {
	change = 0;
	for (fb = fp->fb_qf; fb; fb = fb->fb_qf)
	    Block {
	    Reg1 Ff_buf    *fb1;
	    for (fb1 = fb->fb_qf; fb1; fb1 = fb1->fb_qf)
		if (fb->fb_bnum > fb1->fb_bnum) {
		    change = 1;
		    if (fb->fb_qf)
			fb->fb_qf->fb_qb = fb->fb_qb;
		    fb->fb_qb->fb_qf = fb->fb_qf;
		    if (fb1->fb_qf)
			fb1->fb_qf->fb_qb = fb;
		    fb->fb_qf = fb1->fb_qf;
		    fb1->fb_qf = fb;
		    fb->fb_qb = fb1;
		}
	    }
    } while (change);
    return;
}
