#ifdef COMMENT
	Copyright abandoned, 1983, The Rand Corporation
#endif

/* parameters for drawing borders */

#define WIN_INACTIVE  0 /* inactive */
#define WIN_ACTIVE    1 /* active   */
#define WIN_DRAWSIDES 2 /* draw sides */

#define PLINESET 02     /* winflgs: a "set +line" was issued in this window */
#define MLINESET 04     /* winflgs: a "set -line" was issued in this window */

#ifdef LMCMARG
#define TOPMAR  1
#define BOTMAR  2
#endif
