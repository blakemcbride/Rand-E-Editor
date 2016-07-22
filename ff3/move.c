/*
 * file move.c   memory move 
 *
 */
#include <c_env.h>
char           *
move(source, dest, count)
    char           *source, *dest;
    unsigned int    count;
{
    memmove (dest, source, count);
    return &dest[count];
}

