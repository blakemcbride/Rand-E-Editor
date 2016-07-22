#include	<stdio.h>

short
getshort (iop)
register FILE *iop;
{
    short i;
    register int tmp;
    register char *cp;

    cp = (char *) &i;
    tmp = sizeof (short);
    *cp++ = getc (iop);
    /* XXXXXXXXXXXXXXXXXXXXXXXXXXXX
    if (iop->_flag&_IOEOF)
       XXXXXXXXXXXXXXXXXXXXXXXXXXXX */
    if (feof(iop))
	return -1;
    --tmp;
    do
	*cp++ = getc (iop);
    while (--tmp);

    return i;
}
