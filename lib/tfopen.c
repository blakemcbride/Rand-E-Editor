#include <stdio.h>
#ifdef UNIXV7
#  include <sys/types.h>
#endif
#include <sys/stat.h>

FILE *
tfopen (file, mode)
char *file;
register char *mode;
{
    register int f;
    register FILE *iop;
    int read = 0;
    int write = 0;
    struct stat scratch;

#ifdef _AIX
    for (iop = _iob; iop->_flag & (_IOREAD | _IOWRT); iop++)
	if (iop >= _iob + _NFILE)
#else
    for (iop = __iob; iop->_flag & (_IOREAD | _IOWRT); iop++)
	if (iop >= __iob + _NFILE)
#endif
	    return NULL;
    for (; *mode; mode++) {
	switch (*mode) {
	case 'w':
	case 'a':
	    write = 1;
	    break;

	case 'r':
	    read = 1;
	    break;
	}
    }
    if (!read && !write)
	return NULL;
    if (stat (file,&scratch) == -1) { /* doesn't exist */
	if (write) {
	    f = creat (file, 0644);
	    if (f != -1)
		unlink (file);
	}
	else
	    f = -1;
    }
    else {                          /* does exist */
	if (write) {
	    if (read)
		f = open (file, 2);
	    else
		f = open (file, 1);
	}
	else
	    f = open (file, 0);
    }
    if (f < 0)
	return NULL;
    else
	close (f);
    return iop;
}
