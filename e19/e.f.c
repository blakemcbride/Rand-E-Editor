#ifdef COMMENT
--------
file e.f.c
    File system calls involving stat, etc.
#endif

#ifdef COMMENT
	Copyright abandoned, 1983, The Rand Corporation
#endif

#include "e.h"
#include <sys/stat.h>
#include <string.h>

extern char * xdir_err;     /* previously called deffile */
extern void fwrprep ();
extern void freset ();
       void eddeffile ();


#ifdef COMMENT
int
filetype (name)
    char name[];
.
    Return the S_IFMT bits of the requested file,
    or -1 if the stat call fails.
#endif
int
filetype (name)
char name[];
{
    struct stat statbuf;

    if (stat (name[0] ? name : ".", &statbuf) == -1)
	return -1;
    return statbuf.st_mode & S_IFMT;
}

#ifdef COMMENT
int
fgetpriv (fildes)
    Fd fildes;
.
    Return the mode bits anded with 0777 for fildes.
#endif
int
fgetpriv (fildes)
Fd fildes;
{
    struct stat statbuf;

    fstat (fildes, &statbuf);
    return statbuf.st_mode & 0777;
}

#ifdef COMMENT
int
dircheck (name, dir, file, writecheck, errors)
    char  *name;
    char **dir;
    char **file;
    Flag   writecheck;
    Flag   errors;
.
    If dir != NULL, put a pointer to an alloced string containing
      the directory part into *dir.  If no '/' in name, *dir
      will be a null string, else it will be the directory name
      with a '/' as the last character.
    If file != NULL, put a pointer to the file part into *file.
    Check the directory for access, insist on writeability if
      writecheck == YES.
    Complain with calls to mesg if errors == YES.
    Truncate the file part of name to FNSTRLEN (14 characters).
    Returns file type (S_IFMT) if success or -1 if failure.
#endif
int
dircheck (name, dir, file, writecheck, errors)
char  *name;
char **dir;
char **file;
Flag   writecheck;
Flag   errors;
{
#ifdef OLD_VERSION
    Flag slashinname = NO;
    char *tdir;
    register char *cp1,
		  *cp2;

    /* determine if there is a '/' in name. if so,
       cp2 will point to the last '/' in name */
    for (cp1 = cp2 = name; *cp1; cp1++) {
	if (*cp1 == '/') {      /* get directory name, if one */
	    slashinname = YES;
	    cp2 = cp1;
	}
    }
    if (slashinname) {
	*cp2 = '\0';
	tdir = append (name, "/");
	*cp2++ = '/';
    }
    else {
	tdir = append ("", "");
	cp2 = name;
    }

#else /* new version (does not write into constant string) */

    char *tdir;
    char *cp2;
    int lname;

    cp2 = strrchr (name, '/');  /* last occurence of '/' */
    if ( cp2 ) cp2++;
    else cp2 = name;
    lname = cp2 - name;
    tdir = salloc (lname + 1, YES);
    if ( lname ) strncpy (tdir, name, lname);
    tdir[lname] = '\0';

#endif /* new version */

    if (strlen (cp2) > FNSTRLEN)
	cp2[FNSTRLEN] = '\0';
    if (file != NULL)
	*file = cp2;

    {
	register int   j;

	j = dirncheck (tdir, writecheck, errors);
	if (dir == NULL)
	    sfree (tdir);
	else
	    *dir = tdir;
	return j;
    }
}

#ifdef COMMENT
int
dirncheck (tdir, writecheck, errors)
    char *tdir;
    Flag writecheck;
    Flag errors;
.
    Does the work for dircheck above.
#endif
int
dirncheck (tdir, writecheck, errors)
register char *tdir;
Flag writecheck;
Flag errors;
{
    Flag putback = NO;
    int retval;
    register int filefmt;
    register char *cp;
    register char *dirname;

    /* temporarily remove a trailing '/' */
    if ((cp = tdir)[0] != '\0')
	for (; ; cp++)
	    if (*cp == '\0') {
		if (*--cp == '/' && cp != tdir) {
		    *cp = '\0';
		    putback = YES;
		}
		break;
	    }

    retval = -1;
    dirname = tdir[0] != '\0' ? tdir : ".";
    if ((filefmt = filetype (dirname)) == -1) {
	if (errors) {
	    mesg (ERRSTRT + 1, "Can't find");
	    goto ret;
	}
    }
    else if (filefmt != S_IFDIR) {
	if (errors)
	    mesg (ERRALL + 2, tdir, " is not a directory");
    }
    else if (access (dirname, 1) < 0) {
	if (errors) {
	    mesg (ERRSTRT + 1, "Can't work in");
	    goto ret;
	}
    }
    else if (writecheck && access (dirname, 2) < 0) {
	if (errors) {
	    mesg (ERRSTRT + 1, "Can't write in");
 ret:
	    if (tdir[0] == '\0')
		mesg (ERRDONE + 1, " current directory");
	    else
		mesg (ERRDONE + 2, " dir: ", tdir);
	}
    }
    else
	retval = filefmt;
    if (putback)
	*cp = '/';
    return retval;
}

#ifdef COMMENT
Flag
multlinks (name)    /* return (are there multiple links to the file?) */
    char name[];
.
    Return YES if number of links to name is > 1 else NO.
#endif
Flag
multlinks (name)    /* return (are there multiple links to the file?) */
char name[];
{
    struct stat statbuf;

    if (stat (name[0] ? name : ".", &statbuf) == -1)
	return 0;
    return statbuf.st_nlink;
}

#ifdef COMMENT
Flag
fmultlinks (fildes)    /* return (are there multiple links to the fildes?) */
    Fd fildes;
.
    Return YES if number of links to fildes is > 1 else NO.
#endif
Flag
fmultlinks (fildes)    /* return (are there multiple links to the fildes?) */
Fd fildes;
{
    struct stat statbuf;

    if (fstat (fildes, &statbuf) == -1)
	return 0;
    return statbuf.st_nlink;
}

#ifdef COMMENT
Fn
hvname (name)
    char *name;
.
    Return the Fn associated with name if we have it already, else -1.
#endif
Fn
hvname (name)
char *name;
{
    register Fn i;

    for (i = FIRSTFILE; i < MAXFILES; ++i)
	if (   (fileflags[i] & (INUSE | DELETED)) == INUSE
	    && strcmp (name, names[i]) == 0
	   )
	    return i;
    return -1;
}

#ifdef COMMENT
Fn
hvoldname (name)
    char *name;
.
    Return the Fn associated with name if we have it already
    as an old name of a renamed file, else -1.
#endif
Fn
hvoldname (name)
char *name;
{
    register Fn i;

    for (i = FIRSTFILE; i < MAXFILES; ++i)
	if (   (fileflags[i] & (INUSE | RENAMED)) == (INUSE | RENAMED)
	    && strcmp (name, oldnames[i]) == 0
	   )
	    return i;
    return -1;
}

#ifdef COMMENT
Fn
hvdelname (name)
    char *name;
.
    Return the Fn associated with name if we have it already
    as a deleted file, else -1.
#endif
Fn
hvdelname (name)
char *name;
{
    register Fn i;

    for (i = FIRSTFILE; i < MAXFILES; ++i)
	if (   (fileflags[i] & (INUSE | DELETED)) == (INUSE | DELETED)
	    && strcmp (name, names[i]) == 0
	   )
	    return i;
    return -1;
}

#ifdef COMMENT
void
eddeffile (puflg)
Flag puflg;
.
    Edit the default file.
#endif
void
eddeffile (puflg)
Flag puflg;
{
    if (editfile (xdir_err, (Ncols) 0, (Nlines) 0, 0, puflg) <= 0)
	mesg (ERRALL + 1, "Default file gone: notify sys admin.");
    else {
	deffn = curfile;
	fileflags[curfile] &= ~CANMODIFY;
    }
    return;
}

Flag is_eddeffile (Fn fn)
{
    int cc;

    if ( !(fileflags[fn] & INUSE) ) return NO;
    if ( (fileflags[fn] & CANMODIFY) ) return NO;
    if ( ! names[fn] ) return NO;
    cc = strcmp (names[fn], xdir_err);
    return ( cc == 0 );
}
