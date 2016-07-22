/*
**      header file for help stuff.
*/

#define MXLATE(c) (*term.tt_xlate) (c)
#define MHOME     (*term.tt_home) ()
#define MCLEAR    (*term.tt_clear) ()
#define MSSO      (*term.tt_so) ()
#define MSSE      (*term.tt_soe) ()

#define HELP_GS         14      /* ctrl-n */
#define HELP_GE         15      /* ctrl-o */
#define HELP_SS         16      /* ctrl-p */
#define HELP_SE         17      /* ctrl-q */

#define HELP_CH         'E'     /* first graphics character */
#define HELP_CX         'O'     /* last graphics character */
