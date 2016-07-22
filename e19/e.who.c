/* e.who.c : This is an agregate from who.c and canon-host.c
   from GNU shell utility distribution
   Fabien PERRIOLLAT, March 2000, CERN
*/

/* Host name canonicalization

   Copyright (C) 1995 Free Software Foundation, Inc.

   Written by Miles Bader <miles@gnu.ai.mit.edu>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2, or (at
   your option) any later version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  */

# define HAVE_UT_HOST
# define HAVE_GETHOSTBYNAME
# define HAVE_GETHOSTBYADDR
# define HAVE_INET_NTOA
# define HAVE_UNISTD_H
# define HAVE_STRING_H
# define HAVE_NETDB_H
# define HAVE_SYS_SOCKET_H
# define HAVE_NETINET_IN_H
# define HAVE_ARPA_INET_H

#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>

#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif
#ifdef HAVE_STRING_H
# include <string.h>
#endif
#ifdef HAVE_NETDB_H
# include <netdb.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
# include <sys/socket.h>
#endif

#ifdef HAVE_NETINET_IN_H
# include <netinet/in.h>
#endif
#ifdef HAVE_ARPA_INET_H
# include <arpa/inet.h>
#endif

#include <string.h>

#ifdef HAVE_UTMPX_H
# include <utmpx.h>
# define STRUCT_UTMP struct utmpx
# define UT_TIME_MEMBER(UT_PTR) ((UT_PTR)->ut_tv.tv_sec)
#else
# include <utmp.h>
# define STRUCT_UTMP struct utmp
# define UT_TIME_MEMBER(UT_PTR) ((UT_PTR)->ut_time)
#endif

#include <time.h>
#ifdef HAVE_SYS_PARAM_H
# include <sys/param.h>
#endif

#include <sys/stat.h>
#include <errno.h>

#if !defined (UTMP_FILE) && defined (_PATH_UTMP)
# define UTMP_FILE _PATH_UTMP
#endif

#if !defined (WTMP_FILE) && defined (_PATH_WTMP)
# define WTMP_FILE _PATH_WTMP
#endif

#ifdef UTMPX_FILE /* Solaris, SysVr4 */
# undef UTMP_FILE
# define UTMP_FILE UTMPX_FILE
#endif

#ifdef WTMPX_FILE /* Solaris, SysVr4 */
# undef WTMP_FILE
# define WTMP_FILE WTMPX_FILE
#endif

#ifndef UTMP_FILE
# define UTMP_FILE "/etc/utmp"
#endif

#ifndef WTMP_FILE
# define WTMP_FILE "/etc/wtmp"
#endif

#ifndef MAXHOSTNAMELEN
# define MAXHOSTNAMELEN 64
#endif

#ifndef S_IWGRP
# define S_IWGRP 020
#endif

char * ttyname ();
static char hostname [MAXHOSTNAMELEN + 1];


/* Returns the canonical hostname associated with HOST (allocated in a static
   buffer), or 0 if it can't be determined.  */
char *
canon_host (char *host, char *loopback, char *** aliases_pt)
{
#ifdef HAVE_GETHOSTBYNAME
  struct hostent *he, *lohe;
  int he_addr, lohe_addr;

  lohe = ( loopback ) ? gethostbyname (loopback) : NULL;  /* normaly loopback */
  if ( lohe ) lohe_addr = ( lohe ) ? *(int *) lohe->h_addr : 0;

  he   = gethostbyname (host);
  if ( he ) he_addr = ( he ) ? *(int *) he->h_addr : 0;

  if (he)
    {
# ifdef HAVE_GETHOSTBYADDR
      char *addr = 0;

      /* Try and get an ascii version of the numeric host address.  */
      switch (he->h_addrtype)
	{
#  ifdef HAVE_INET_NTOA
	case AF_INET:
	  if ( lohe ) {
	    if ( he_addr == lohe_addr )
		/* using loopback */
		return hostname;
	  }
	  addr = inet_ntoa (*(struct in_addr *) he->h_addr);
	  break;
#  endif /* HAVE_INET_NTOA */
	}

      if (addr && strcmp (he->h_name, addr) == 0)
	/* gethostbyname() cheated!  Lookup the host name via the address
	   this time to get the actual host name.  */
	he = gethostbyaddr (he->h_addr, he->h_length, he->h_addrtype);
# endif /* HAVE_GETHOSTBYADDR */

      if (he) {
	if ( aliases_pt ) *aliases_pt = he->h_aliases;
	return (char *) (he->h_name);
      }
    }
#endif /* HAVE_GETHOSTBYNAME */
  return 0;
}

static STRUCT_UTMP *utmp_contents;

/* Search `utmp_contents', which should have N entries, for
   an entry with a `ut_line' field identical to LINE.
   Return the first matching entry found, or NULL if there
   is no matching entry. */

static STRUCT_UTMP *
search_entries (int n, char *line)
{
  register STRUCT_UTMP *this = utmp_contents;

  while (n--)
    {
      if (this->ut_name[0]
# ifdef USER_PROCESS
	  && this->ut_type == USER_PROCESS
# endif
	  && !strncmp (line, this->ut_line, sizeof (this->ut_line)))
	return this;
      this++;
    }
  return NULL;
}

static char * get_this_host (STRUCT_UTMP *this, char **display_pt,
			     char *loopback, char *** aliases_pt)
{
  char *host, *display;

  host = display = NULL;

# ifdef HAVE_UT_HOST
  if (this->ut_host[0])
    {
      static char ut_host[sizeof (this->ut_host) + 1];

      /* Copy the host name into UT_HOST, and ensure it's nul terminated. */
      strncpy (ut_host, this->ut_host, (int) sizeof (this->ut_host));
      ut_host[sizeof (this->ut_host)] = '\0';

      /* Look for an X display.  */
      display = strrchr (ut_host, ':');
      if (display)
	*display++ = '\0';

      if (*ut_host)
	/* See if we can canonicalize it.  */
	host = canon_host (ut_host, loopback, aliases_pt);
      if (! host)
	host = ut_host;
    }
# endif
    if ( display_pt ) *display_pt = display;
    return host;
}

/* Display a line of information about entry THIS. */

static void
print_entry (STRUCT_UTMP *this)
{
  int i;
  struct stat stats;
  time_t last_change;
  char mesg;
  char *host, *display, **aliases;

# define DEV_DIR_WITH_TRAILING_SLASH "/dev/"
# define DEV_DIR_LEN (sizeof (DEV_DIR_WITH_TRAILING_SLASH) - 1)

  char line[sizeof (this->ut_line) + DEV_DIR_LEN + 1];
  time_t tm;

  /* Copy ut_line into LINE, prepending `/dev/' if ut_line is not
     already an absolute pathname.  Some system may put the full,
     absolute pathname in ut_line.  */
  if (this->ut_line[0] == '/')
    {
      strncpy (line, this->ut_line, sizeof (this->ut_line));
      line[sizeof (this->ut_line)] = '\0';
    }
  else
    {
      strcpy (line, DEV_DIR_WITH_TRAILING_SLASH);
      strncpy (line + DEV_DIR_LEN, this->ut_line, sizeof (this->ut_line));
      line[DEV_DIR_LEN + sizeof (this->ut_line)] = '\0';
    }

  if (stat (line, &stats) == 0)
    {
      mesg = (stats.st_mode & S_IWGRP) ? '+' : '-';
      last_change = stats.st_atime;
    }
  else
    {
      mesg = '?';
      last_change = 0;
    }

  printf ("%-8.*s", (int) sizeof (this->ut_name), this->ut_name);
  printf ("  %c  ", mesg);
  printf (" %-8.*s", (int) sizeof (this->ut_line), this->ut_line);

  /* Don't take the address of UT_TIME_MEMBER directly.
     Ulrich Drepper wrote:
     ``... GNU libc (and perhaps other libcs as well) have extended
     utmp file formats which do not use a simple time_t ut_time field.
     In glibc, ut_time is a macro which selects for backward compatibility
     the tv_sec member of a struct timeval value.''  */
  tm = UT_TIME_MEMBER (this);
  printf (" %-12.12s", ctime (&tm) + 4);

  host = display = NULL;
  host = get_this_host (this, &display, NULL, &aliases);
  if ( host ) {
    if (display)
      printf (" (%s:%s)", host, display);
    else
      printf (" (%s)", host);
    if ( aliases ) {
	for ( i = 0 ; aliases [i] ; i++ ) ;
	if ( i > 0 ) printf ("\n  Alias : \"%s\"", aliases [i -1]);
    }
  }
  putchar ('\n');
}

/* Read the utmp file FILENAME into UTMP_CONTENTS and return the
   number of entries it contains. */

static int
read_utmp (char *filename)
{
  FILE *utmp;
  struct stat file_stats;
  size_t n_read;
  size_t size;

  utmp = fopen (filename, "r");
  if (utmp == NULL)
    return 0;   /* error on openening utmp file */

  fstat (fileno (utmp), &file_stats);
  size = file_stats.st_size;
  if (size > 0)
    utmp_contents = (STRUCT_UTMP *) malloc (size);
  else
    {
      fclose (utmp);
      return 0;
    }

  /* Use < instead of != in case the utmp just grew.  */
  n_read = fread (utmp_contents, 1, size, utmp);
  if (ferror (utmp) || fclose (utmp) == EOF
      || n_read < size)
    return 0;   /* error on access to utmp file */

  return size / sizeof (STRUCT_UTMP);
}


/* from_host : returns the remote computer (or NULL if running local)
 *             the host (hostname) on which you are running,
 *             and the display (of the remote computer) if any.
 *      If loopback (IP address in . notation) is provided,
 *          return the host (hostname) itself if remote computer is
 *          the loopback.
 */

char * from_host (char **hostname_pt, char **display_pt,
		  char *loopback, char *** aliases_pt)
{
  register STRUCT_UTMP *utmp_entry;
  char *tty, *host;
  char *filename;

  filename = UTMP_FILE;

  memset (hostname, 0, sizeof (hostname));
  if (gethostname (hostname, sizeof (hostname)))
    *hostname = 0;

  if ( hostname_pt ) *hostname_pt = hostname;

  tty = ttyname (0);
  if (tty == NULL)
    return NULL;
  tty += 5;			/* Remove "/dev/".  */

  utmp_entry = search_entries (read_utmp (filename), tty);
  if (utmp_entry == NULL)
    return NULL;

  host = NULL;
  host = get_this_host (utmp_entry, display_pt, loopback, aliases_pt);
  return host;
}



/* Display the entry in utmp file FILENAME for this tty on standard input,
   or nothing if there is no entry for it. */

static void
who_am_i (char *filename)
{
  register STRUCT_UTMP *utmp_entry;
  char hostname[MAXHOSTNAMELEN + 1];
  char *tty;

  if (gethostname (hostname, MAXHOSTNAMELEN + 1))
    *hostname = 0;

  tty = ttyname (0);
  if (tty == NULL)
    return;
  tty += 5;			/* Remove "/dev/".  */

  utmp_entry = search_entries (read_utmp (filename), tty);
  if (utmp_entry == NULL)
    return;

  printf ("%s!", hostname);
  print_entry (utmp_entry);
}

#ifdef TEST_WHO
int main (int argc, char **argv)
{
    static char loopback [] = "127.0.0.1";

    int i, sz;
    char *hostname, *host, *display, **aliases;
    char *name, *str, *str1;

    host = from_host (&hostname, &display, loopback, &aliases);
    printf ("local hostname \"%s\", remote host \"%s\", display \"%s\"\n",
	    hostname, host, display);
    if ( aliases ) {
	for ( i = 0 ; aliases [i] ; i++ ) ;
	if ( i > 0 ) printf ("  Alias : \"%s\"\n", aliases [i -1]);
    }

    who_am_i (UTMP_FILE);
    fputc ('\n', stdout);

    name = NULL;
    if ( (argc > 1) && argv [1] ) str = argv [1];
    else {
	str = getenv ("SSH_CLIENT");
	if ( str ) printf ("ssh session : %s\n", str);
	}
    if ( str ) {
	str1 = strchr (str, ' ');
	if ( !str1 ) str1 = &str[strlen (str)];
	if ( str1 ) {
	    sz = str1 - str;
	    name = malloc (sz +1);
	    if ( name ) {
		strncpy (name, str, sz);
		name [sz] = '\0';
	    }
	}
    }
    if ( name ) {
	printf ("Look for the name of : %s\n", name);
	host = canon_host (name, NULL, &aliases);
	if ( host ) {
	    printf ("Canonical name for \"%s\" = \"%s\"\n", name,  host);
	    if ( aliases ) {
		for ( i = 0 ; aliases [i] ; i++ ) ;
		if ( i > 0 ) printf ("  Alias : \"%s\"\n", aliases [i -1]);
	    }
	}
	else printf ("Nothing for \"%s\"\n", name);
    }
    exit (0);
}
#endif /* TEST_WHO */
