#include	<stdio.h>
#include        <c_env.h>

putshort (i, iop)
short i;
register FILE *iop;
{
    register int tmp;
    register char *cp;

    cp = (char *) &i;
    tmp = sizeof (short);
    do
	putc (*cp++, iop);
    while (--tmp);
}
