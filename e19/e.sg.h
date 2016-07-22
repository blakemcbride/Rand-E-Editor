/*
 * file e.sg.h: sgtty stuff
 **/

#ifdef COMMENT
	Copyright abandoned, 1983, The Rand Corporation
#endif

#ifndef SYSIII
#include SGTT_INCL
#endif

/* termio.h must be include before sys/termio.h in Lynx os :
 *          problem with mutual exclusion between termio.h ioctl.h and sys/ioctl.h
 */
#ifdef SYSIII
#include <termio.h>
#endif

#ifndef NOIOCTL_H
#include <sys/ioctl.h>
#endif

#ifdef SYSIII
#include <termio.h>
#endif

#ifndef SYSIII
extern struct sgttyb instty, outstty;
#endif

#ifdef SYSIII
extern struct termio in_termio, out_termio;
extern int fcntlsave;
#endif

#ifdef  CBREAK
#ifdef  TIOCGETC
extern struct tchars spchars;
#endif
#ifdef  TIOCGLTC
extern struct ltchars lspchars;
#endif
extern Flag cbreakflg;
#endif  /* CBREAK */

extern Flag istyflg, ostyflg;
extern unsigned Short oldttmode;
