#ifdef COMMENT
--------
file e.p.c
    name and delete commands
#endif

#ifdef COMMENT
	Copyright abandoned, 1983, The Rand Corporation
#endif

#include "e.h"
#include "e.inf.h"
#include "e.m.h"
#include "e.cm.h"
#include <sys/stat.h>
#include <unistd.h>

extern void dlfile ();

#ifdef COMMENT
Cmdret
name ()
.
    Do the name command.
#endif
Cmdret
name ()
{
    extern char *cwdfiledir [];
    Flag renamed,
	 new;
    char *name;
    char *cwd, cwdbuf [PATH_MAX];

    if (opstr[0] == '\0')
	return CRNEEDARG;       /* opstr not alloced */
    if (*nxtop)
	return CRTOOMANYARGS;

#ifdef SYMLINKS
    if (fileflags[curfile] & SYMLINK) {
	mesg (ERRALL + 1, "Can't rename file pointed to by a symbolic link");
	return CROK;
    }
#endif

    name = names[curfile];

    /* are we naming it back to its original name? */
    if (   (renamed = fileflags[curfile] & RENAMED ? YES : NO)
	&& strcmp (oldnames[curfile], opstr) == 0
       ) {
	sfree (oldnames[curfile]);
	oldnames[curfile] = NULL;
	fileflags[curfile] &= ~RENAMED;
	goto ret;
    }

    /* are we trying to rename ".", ".." or a TMPFILE? */
    if (   dotdot (name)
	|| curfile < FIRSTFILE + NTMPFILES
       ) {
	mesg (ERRALL + 2, "Can't rename ", name);
	return CROK;
    }

    /* do we have write permission in the file's directory? */
    if ((fileflags[curfile] & DWRITEABLE) == 0) {
	dircheck (name, (char **) 0, (char **) 0, YES, YES);
	return CROK;
    }

    /* do we have that name already internally? */
    /* note that hvname will pass over DELETED names */
    if (hvname (opstr) != -1)
	goto exists;

    /* is the proposed directory writeable? */
    if (dircheck (opstr, (char **) 0, (char **) 0, YES, YES) == -1)
	return CROK;

    /* does that name exist already? */
    /* don't look on disk if we hold it internally as deleted or as the
       old name before renaming */
    if (   hvdelname (opstr) == -1
	&& hvoldname (opstr) == -1
	&& access (opstr, 0) >= 0
       ) {
 exists:
	mesg (ERRALL + 1, "That name exists already");
	return CROK;
    }

    if (!(new = (fileflags[curfile] & NEW))) Block {
	/* can we rename, or will it take a copy operation? */
	char *d1, *d2;
	struct stat s1, s2;

	dircheck ((renamed ? oldnames : names)[curfile],
		  &d1, (char **) 0, NO, NO);
	dircheck (opstr, &d2, (char **) 0, NO, NO);

	stat (d1, &s1);
	stat (d2, &s2);

#ifdef UNIXV7
	if (s1.st_dev != s2.st_dev) {
#else
#ifdef UNIXV6
	if (   s1.st_minor != s2.st_minor
	    || s1.st_major != s2.st_major
	   ) {
#else
ERROR:  must define one or the other
#endif
#endif
	    mesg (ERRALL + 1, "Can't rename to there, have to copy");
	    return CROK;
	}
    }
    if (new || renamed)
	sfree (name);
    else {
	oldnames[curfile] = name;
	fileflags[curfile] |= RENAMED;
    }
 ret:
    rand_info (inf_file, strlen (names[infofile]), opstr);
    names[curfile] = append (opstr, "");
	cwdfiledir [curfile] = NULL;
	cwd = getcwd (cwdbuf, sizeof (cwdbuf));
	if ( cwd ) cwdfiledir [curfile] = append (cwdbuf, "");
    return CROK;
}

#ifdef COMMENT
Cmdret
delete ()
.
    Do the delete command.
#endif
Cmdret
delete ()
{
    register Short flags;
    register char *name;
    int j;
    Small len;

    if (opstr[0] != '\0')
	return CRTOOMANYARGS;

    name = names[curfile];
    flags = fileflags[curfile];

    if (curfile < FIRSTFILE + NTMPFILES) {
	mesg (ERRALL + 2, "Can't delete ", name);
	return CROK;
    }

    /* do we have permission to delete this file? */
    if ((flags & DWRITEABLE) == 0) {
	dircheck (name, (char **) 0, (char **) 0, YES, YES);
	return CROK;
    }

    /* can't delete directories */
    if (   !(flags & NEW)
	&& (j = filetype (name)) != -1    /* file already exists         */
	&& j == S_IFDIR /* can't modify directories */
       ) {
	mesg (ERRALL + 1, "Can't delete directories");
	return CROK;
    }

    if (flags & (NEW | RENAMED)) {
	len = strlen (name);
	sfree (name);
    }
    if (flags & NEW)
	/* throw the file away entirely */
	flags = 0;
    else {
	if (flags & RENAMED) {
	    names[curfile] = oldnames[curfile];
	    oldnames[curfile] = NULL;
	}
	else
	    len = strlen (name);
	flags &= ~(SAVED | RENAMED | CANMODIFY);
	la_unmodify (curlas);
	flags |= DELETED;
    }
    (void) la_close (&fnlas[curfile]);
    fileflags[curfile] = flags;
    curwin->winflgs &= ~TRACKSET;
    infotrack (NO);
    dlfile (curfile);
    rand_info (inf_file, len, names[curfile]);
    return CROK;
}

#ifdef COMMENT
Flag
dotdot (name)
    char *name;
.
    Return YES if any element of the pathname 'name' is "..".
#endif
Flag
dotdot (name)
char *name;
{
    register char *cp;

    for (cp = name; *cp; ) {
	if (   (cp == name || *cp++ == '/')
	    && (*cp++ == '.')
	    && (   *cp == '\0'
		|| (*cp++ == '.' && *cp == '\0')
	       )
	   )
	    return YES;
    }
    return NO;
}

#ifdef COMMENT
void
dlfile (fn)
    Fn fn;
.
    Remove the file from any workspaces in any windows where it occurs.
#endif
void
dlfile (fn)
Fn fn;
{
    register S_wksp *wk, *awk;
    register Small  ind;
    S_window *oldwin;

    for (ind = Z; ind < nwinlist; ind++) {
	if ((awk = winlist[ind]->altwksp)->wfile == fn)
	    releasewk (awk);
	if ((wk = winlist[ind]->wksp)->wfile == fn) Block {
	    Flag docurs;
	    releasewk (wk);
	    oldwin = curwin;
	    if (docurs = (oldwin != curwin))
		savecurs ();
	    switchwindow (winlist[ind]);
	    if (docurs)
		chgborders = 0;
	    if (!swfile ()) {
		if (docurs)
		    chgborders = 0;
		edscrfile (YES);
	    }
	    switchwindow (oldwin);
	    if (docurs) {
		chgborders = 1;
		restcurs ();
	    }
	}
    }
    return;
}
