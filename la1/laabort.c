#ifdef COMMENT
	Copyright abandoned, 1983, The Rand Corporation
#endif

#include "la.local.h"

/* VARARGS1 */
void
la_abort (str)
char *str;
{
    fprintf (stderr, "LA ABORT: %r", &str);
    fprintf (stderr, "\n");
    abort ();
    /* NOTREACHED */
}
