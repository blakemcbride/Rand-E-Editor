#ifdef COMMENT
Copyright       abandoned, 1983, The Rand Corporation
#endif


#include "ff.local.h"

ff_free(nbuf, arena)
    Reg3 int        nbuf;
    int             arena;
{
    extern          end;

    if (arena != 0)
	goto done;
    if (nbuf >= ff_flist.fr_count)	/* leave at least one buf */
	nbuf = ff_flist.fr_count - 1;
    while (nbuf--)
	Block {
	Reg1 Ff_buf    *fb;
	fb = ff_flist.fb_back;
	/* skip over bufs that weren't alloced */
	while ((int *) fb < &end)
	    if ((fb = fb->fb_back) == ff_flist.fb_back)
		goto done;
	if ((ff_putblk(fb, 1)) == (Ff_buf *) 0)
	    return -1;
	fb->fb_forw->fb_back = fb->fb_back;
	fb->fb_back->fb_forw = fb->fb_forw;
	free((char *) fb);
	ff_flist.fr_count--;
	}
done:
    return ff_flist.fr_count;
}
