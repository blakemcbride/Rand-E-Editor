#ifdef COMMENT

  file run.c - source for /etc/e/run

  When the editor "runs" an arbitrary filter program, it sends the text to
  be filtered to the program through a pipe and wants the program to write
  onto the end of the changes file.  To prevent the program from seeking
  backwards and destroying the changes file, its output is piped through
  this "run" program.

#endif

#include <stdio.h>

#define NBUF (BUFSIZ * 8)

main ()
{
    char buf[NBUF]; /* static only to avoid 11 kernel bug */
    register int nread, naccum;

    for (;;) {
	for (naccum = 0; naccum < NBUF; naccum += nread) {
	    nread = NBUF - naccum;
	    if ((nread = read (0, &buf[naccum], nread)) <= 0)
		break;
	}
	if (naccum > 0)
	    write (1, buf, naccum);
	if (nread <= 0)
	    break;
    }
    exit (0);
}
