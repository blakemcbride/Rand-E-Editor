#ifdef COMMENT
	Copyright abandoned, 1983, The Rand Corporation
#endif

#include "la.local.h"

la_error ()
{
    Reg1 int error;

    error = la_errno;
    la_errno = 0;
    return error;
}

