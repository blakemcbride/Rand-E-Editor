
#include <localenv.h>

#ifdef NOIOCTL_H
#include <sys/tty.h>
#else
#include <sys/ioctl.h>
#endif

#ifdef FIONREAD
empty (fd)
{
    int  count;

    if (ioctl (fd, FIONREAD, &count) < 0)
	return 1;
    return count <= 0;
}
#endif /* FIONREAD */
