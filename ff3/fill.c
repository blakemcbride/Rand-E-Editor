/*
 * file fill.c   memory fill 
 *
 */
#include <c_env.h>

char           * fill(dest, count, fillchar)
    char           *dest;
    unsigned int    count;
    char            fillchar;
{
    memset(dest, fillchar, count);
    return &dest[count];
}

