/*
 * file e.cm.h - return values for functions called by command ()
 **/

#ifdef COMMENT
	Copyright abandoned, 1983, The Rand Corporation
#endif

/* all of the following are of type Cmdret */
#define CROK            ((Cmdret) 0 )
#define CRUNRECARG      ((Cmdret) -1)   /* matches lookup return */
#define CRAMBIGARG      ((Cmdret) -2)   /* matches lookup return */
#define CRTOOMANYARGS   ((Cmdret) -3)
#define CRNEEDARG       ((Cmdret) -4)
#define CRBADARG        ((Cmdret) -5)
#define CRNOVALUE       ((Cmdret) -6)
#define CRMULTARG       ((Cmdret) -7)
#define CRMARKCNFL      ((Cmdret) -8)

#define NOTSTRERR       ((Cmdret) 1)
#define NOWRITERR       ((Cmdret) 2)
#define NOARGERR        ((Cmdret) 3)
#define NOPIPERR        ((Cmdret) 4)
#define NOMARKERR       ((Cmdret) 5)
#define NORECTERR       ((Cmdret) 6)
#define NOBUFERR        ((Cmdret) 7)
#define CONTIN          ((Cmdret) 8)
#define MARGERR         ((Cmdret) 9)
#define NOMEMERR        ((Cmdret) 10)
#define NOTINTERR       ((Cmdret) 11)
#define NOTPOSERR       ((Cmdret) 12)
#define NOTRECTERR      ((Cmdret) 13)
#define NOFORKERR       ((Cmdret) 14)
#define NOSPCERR        ((Cmdret) 15)
#define TOOLNGERR       ((Cmdret) 16)
#define NDMARKERR       ((Cmdret) 17)
#define NORANGERR       ((Cmdret) 18)
#define NOTIMPERR       ((Cmdret) 19)

#define RE_CMD          ((Cmdret) 99)


/* Command class value : returned by command_class () */

#define NOT_CMD_CLASS  0    /* not a command */
#define FILE_CMD_CLASS 1    /* file parameter command */
#define DIR_CMD_CLASS  2    /* directory parameter command */
#define HELP_CMD_CLASS 3    /* help command */
#define ARG_CMD_CLASS  4    /* command with predifined argument(s) */

