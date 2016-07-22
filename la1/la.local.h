#ifdef COMMENT
	Copyright abandoned, 1983, The Rand Corporation
#endif

#include <c_env.h>
#include <localenv.h>
#if defined(NOREGCHAR) || defined(NOREGSHORT)
#undef Reg1
#undef Reg2
#undef Reg3
#undef Reg4
#undef Reg5
#undef Reg6
#define Reg1
#define Reg2
#define Reg3
#define Reg4
#define Reg5
#define Reg6
#endif

#ifdef UNIXV7
#include <sys/types.h>
#endif

#define REALLOC         /* use realloc() -- more efficient & more code */
			/* comment out if you can't use realloc - XENIX? */
#define Block

#define LA_FSDSIZE (sizeof (La_fsd) - 1)

#ifdef LA_DEBUG
#define LA_FSDLMAX  5       /* max for fsdnlines */
#else
#define LA_FSDLMAX  127     /* max for fsdnlines */
#endif
#define LA_FSDBMAX  255     /* max for fsdbytes */
#define LA_FSDNBMAX 32767   /* max for fsdnbytes */

#include "la.h"

#define BRK_REAL  0
#define BRK_COPY  1
#define BRK_AGAIN 2

extern La_linepos   la_parse ();
extern La_flag      la_zbreak (), la_break ();
