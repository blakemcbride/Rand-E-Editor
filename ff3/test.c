#ifdef COMMENT
Copyright       abandoned, 1983, The Rand Corporation
#endif

#include <stdio.h>
#include "ff.local.h"

main(argc, argv)
    char          **argv;
{
    Ff_stream      *fi;
    Ff_stream      *fo;
    int             chr;
    char            buf[100];
    register int    j;

    fprintf(stderr, "Starting\n");
    fo = ff_open("/dev/tty", 1, 0);
    fprintf(stderr, "fo is open\n");
    ff_alloc(2, 0);
    fprintf(stderr, "ff_alloc done\n");

    while (--argc >= 1) {
	++argv;
	if ((fi = ff_open(*argv, 0, 0)) == NULL) {
	    fprintf(stderr, "cat: can't open %s\n", *argv);
	    continue;
	}
	for (;;) {
	    if ((j = ff_read(fi, buf, sizeof buf, 0)) <= 0)
		break;
	    if (ff_write(fo, buf, j) != j)
		break;
	}
	/*
	 * while ((chr = ff_getc(fi)) != EOF) ff_putc(chr, fo); 
	 */
	ff_close(fi);
    }
    exit(0);
}

#ifdef CAT
/*
 * Concatenate files. 
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>

char            stdbuf[BUFSIZ];

main(argc, argv)
    char          **argv;
{
    int             fflg = 0;
    register Ff_stream *fi;
    register        c;
    int             dev, ino = -1;
    struct stat     statb;
    Ff_stream      *ffstdin;
    Ff_stream      *ffstdout;

    ffstdin = ff_open("/dev/tty", 0, 0);
    ffstdout = ff_open("/dev/tty", 1, 0);
    /*
     * ffstdin  = ff_fdopen (0, 0, 0); ffstdout = ff_fdopen (1, 1, 0); 
     */
    for (; argc > 1 && argv[1][0] == '-'; argc--, argv++) {
	switch (argv[1][1]) {
	case 0:
	    break;
	}
	break;
    }
    fstat(ff_fd(ffstdout), &statb);
    statb.st_mode &= S_IFMT;
    if (statb.st_mode != S_IFCHR && statb.st_mode != S_IFBLK) {
	dev = statb.st_dev;
	ino = statb.st_ino;
    }
    if (argc < 2) {
	argc = 2;
	fflg++;
    }
    while (--argc > 0) {
	if (fflg || (*++argv)[0] == '-' && (*argv)[1] == '\0')
	    fi = ffstdin;
	else {
	    if ((fi = ff_open(*argv, 0, 0)) == NULL) {
		fprintf(stderr, "cat: can't open %s\n", *argv);
		continue;
	    }
	}
	fstat(ff_fd(fi), &statb);
	if (statb.st_dev == dev && statb.st_ino == ino) {
	    fprintf(stderr, "cat: input %s is output\n",
		    fflg ? "-" : *argv);
	    ff_close(fi);
	    continue;
	}
	while ((c = ff_getc(fi)) != EOF)
	    ff_putc(c, ffstdout);
	if (ff_fd(fi) != 0)
	    ff_close(fi);
    }
    return (0);
}
#endif
