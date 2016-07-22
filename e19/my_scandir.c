/* Derivated from the glibc file scandir.c by F. Perriollat January 2001
 * Fabien.Perriollat@cern.ch
 * To be used on system which does not provide scandir function in
 * libc (like Solaris).
 */

/* Copyright (C) 1992, 93, 94, 95, 96, 97, 98 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#ifdef __linux__
#define _FILE_OFFSET_BITS 64
#endif

#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

void set_errno (err)
    int err;
{
    errno = err;
}

int
alphasort (const void *a, const void *b)
{
  return strcoll ((*(const struct dirent **) a)->d_name,
		  (*(const struct dirent **) b)->d_name);
}


int
scandir (dir, namelist, select, cmp)
     const char *dir;
     struct dirent ***namelist;
     int (*select) (const struct dirent *);
     int (*cmp) (const void *, const void *);
{
  DIR *dp = opendir (dir);
  struct dirent **v = NULL;
  size_t vsize = 0, i;
  struct dirent *d;
  int save;

  if (dp == NULL)
    return -1;

  save = errno;
  set_errno (0);

  i = 0;
  while ((d = readdir (dp)) != NULL)
    if (select == NULL || (*select) (d))
      {
	struct dirent *vnew;
	size_t dsize;

	/* Ignore errors from select or readdir */
	set_errno (0);

	if (i == vsize)
	  {
	    struct dirent **new;
	    if (vsize == 0)
	      vsize = 10;
	    else
	      vsize *= 2;
	    new = (struct dirent **) realloc (v, vsize * sizeof (*v));
	    if (new == NULL)
	      break;
	    v = new;
	  }

/*
	dsize = &d->d_name[_D_ALLOC_NAMLEN (d)] - (char *) d;
*/
#ifdef _DIRENT_HAVE_D_RECLEN
	dsize = d->d_reclen;
#else
	dsize = (&d->d_name[0] - (char *) d) + strlen (d->d_name) + 1;
#endif
	vnew = (struct dirent *) malloc (dsize);
	if (vnew == NULL)
	  break;

	v[i++] = (struct dirent *) memcpy (vnew, d, dsize);
      }

  if (errno != 0)
    {
      save = errno;
      (void) closedir (dp);
      while (i > 0)
	free (v[--i]);
      free (v);
      set_errno (save);
      return -1;
    }

  (void) closedir (dp);
  set_errno (save);

  /* Sort the list if we have a comparison function to sort with.  */
  if (cmp != NULL)
    qsort (v, i, sizeof (*v), cmp);
  *namelist = v;
  return i;
}


#ifdef SCANDIR_TEST

main ()
{
    int nb, i;
    char *dir;
    struct dirent **namelist = NULL;
    struct dirent *d;

    dir = "./";
    nb = scandir (dir, &namelist, NULL, alphasort);

    printf ("nb of entry : %d\n", nb);

    if ( nb > 0 ) {
	for ( i = 0 ; i < nb ; i++ ) {
	    d = namelist[i];
	    printf (" %3d (%d) ino = %d %s\n", i, d->d_reclen, d->d_ino, d->d_name);
	}
    }
}

#endif /* SCANDIR_TEST */
