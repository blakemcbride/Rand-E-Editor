#ifdef COMMENT
--------
file e.main.c
    main() only
#endif

#ifdef COMMENT
	Copyright abandoned, 1983, The Rand Corporation
#endif

#include "e.h"

extern void main1 ();
extern void mainloop ();

#ifdef COMMENT
void
main (argc, argv)
    int     argc;
    char   *argv[];
.
    The main routine.
    First it does all of the startup stuff by calling main1, then it calls
    mainloop.  This is structured this way so that if overlays are to be
    implemented, all of the main1 stuff can be in one startup overlay that
    is discarded when it is time to call mainloop.
#endif
int
main (argc, argv)
int     argc;
char   *argv[];
{
    main1 (argc, argv);

    mainloop ();
    /* NOTREACHED */
    return 0;
}

