#include	<stdio.h>
#include        <c_env.h>

putlong (i, iop)
long i;
register FILE *iop;
{
    register int tmp;
    register char *cp;

    cp = (char *) &i;
    tmp = sizeof (long);
    do
	putc (*cp++, iop);
    while (--tmp);
}
