#ifdef COMMENT
--------
file e.put.c
    put qbuffers into the file
#endif

#ifdef COMMENT
	Copyright abandoned, 1983, The Rand Corporation
#endif

#include "e.h"
#include "e.m.h"
#include "e.cm.h"
#include "e.e.h"

S_looktbl qbuftable [] = {
    "adjust"  , QADJUST ,
    "box"     , QBOX    ,
    "close"   , QCLOSE  ,
    "erase"   , QERASE  ,
    "pick"    , QPICK   ,
    "run"     , QRUN    ,
    0         , 0
};

Small insqbuf;
Small nbufargs;

#ifdef COMMENT
Cmdret
insert (cmd)
    int cmd;
.
    Do the cover, overlay, underlay, blot, and insert commands
    'cmd':
    0 COVER
    1 OVERLAY
    2 UNDERLAY
    3 BLOT
    4 -BLOT
    5 INSERT
#endif
Cmdret
insert (cmd)
int cmd;
{
    char *cp;
    Cmdret tmp;
    Cmdret qbufopts ();

    if (opstr[0] == '\0')
	return CRNEEDARG;       /* opstr not alloced */

    nbufargs = 0;
    cp = cmdopstr;
    if ((tmp = scanopts (&cp, NO, qbuftable, qbufopts)) < 0)
	return tmp;
/*       3 marked area        \
 *       2 rectangle           \
 *       1 number of lines      > may have stopped on an unknown option
 *       0 no area spec        /
 *      -2 ambiguous option     ) see parmlines, parmcols for type of area
 *   <= -3 other error
 **/
    if (*cp != '\0')
	return CRBADARG;
    if (nbufargs < 1)
	return CRNEEDARG;
    if (nbufargs > 1)
	return CRTOOMANYARGS;

    switch (tmp) {
    case 0:
	return ed (OPCOVER << cmd,
		   insqbuf,
		   curwksp->wlin + cursorline,
		   curwksp->wcol + cursorcol, 0, (Ncols) 0, YES);

    case 1:
    case 2:
	if (insqbuf != QBOX)
	    return CRBADARG;
	return ed (OPCOVER << cmd,
		   insqbuf,
		   curwksp->wlin + cursorline,
		   curwksp->wcol + cursorcol, parmlines, parmcols, YES);

    case 3:
	if (insqbuf != QBOX)
	    return NOMARKERR;
	return edmark (OPCOVER << cmd, insqbuf);
    }
    return tmp;
}

#ifdef COMMENT
qbufopts (cp, str, tmp, equals)
register char *cp;
    char **str;
    Small tmp;
    Flag equals;
.
    Get the buffer name for the insert command.
    Called only by insert().
#endif
Cmdret
qbufopts (cp, str, tmp, equals)
register char *cp;
char **str;
Small tmp;
Flag equals;
{
    nbufargs++;
    if (equals)
	return CRBADARG;
    insqbuf = qbuftable[tmp].val;
    for (; *cp && *cp == ' '; cp++)
	continue;
    *str = cp;
    return 1;
}


#ifdef COMMENT
Cmdret
insbuf (buf)
    Small buf;
.
    Do the -pick, -close, and -erase commands.
    'buf':
    0 -close
    1 -pick
    2 -erase
#endif
Cmdret
insbuf (buf)
Small buf;
{
    if (curmark)
	return NOMARKERR;
    if (*cmdopstr)
	return CRTOOMANYARGS;

    return ed (OPINSERT,
	       buf,
	       curwksp->wlin + cursorline,
	       curwksp->wcol + cursorcol, 0, (Ncols) 0, YES);
}
