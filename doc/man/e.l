.de sh
.br
.ne 5
.PP
\fI\\$1\fR
.PP
..
.TH E 1l "CERN 26th July 1985"
.SH NAME
e \- Rand full screen text editor
.SH SYNOPSIS
.B e
[ options ]  [ file ] [ altfile ] [ other file ] ....
.br
.B ned
.SH DESCRIPTION
.I e
invokes the Rand editor.
The Rand editor is a full screen text editor with file editing in
windows on the screen.
Editing is done by typing directly into the window, and functions
include "cut" and "paste" operations,
simple formatting, and embedding UNIX command output in the
file.
.PP
The parameter
.I file
denotes the file to be edited and
.I altfile
is an optional alternate file.
With no file names given the editor will resume the latest
editing session in the directory, if any,
otherwise it will prompt for a file name.
.PP
.I E
is a complete rewrite of the Rand editor
.I NED
The current version (E19) takes
advantage of terminals that have the ability to
do local insert/delete line.
A reference manual
.I The Rand Editor, E, April 1984
is available on the PRIAM VAX using the
notesfile scheme and the printdoc command.
.SH CHANGES
.sp
To see the last changes in version E19 : look at the output of "help new"
command.
.TP
E19 (initial) changes include:
.br
- word processing mode (WP mode)
.br
- a delete word function
.br
- options to control the behavior at the right margin;
.br
- an alternate syntax for the "goto" command;
.br
- display of tab settings on the top border
.br
- left and right margin (width) settings on the bottom border
.br
- an option for invoking \fIE\fR with a previous state file
.br
- a new help facility;
.br
- improved support for the "track" command;
.br
- new commands for case conversion and reversal.
.TP
E18 changes include:
.br
- support for regular expression searching and replacment
.br
- implementation of the
.SM +WORD
and
.SM \-WORD
functions
.br
- a new \fBset\fR command to individually tailor
various parameters
.br
- support for symbolic links
.br
- E now runs under UNIX System III and System V
.br
- commands that specify filenames (e.g., "
.SM CMD:
e file" expand a leading "~" to the appropriate home directory;
.SH OPTIONS
.PP
The currently available command line options are:
.TP
.B \-help
.br
This causes a display of the currently available
options and their values along with the version number
of the editor.
.br
Use this option to list the updated list of options.
.TP
.BR \-bullets " or " \-nobullets
.br
Options \-bullets and \-nobullets forces border bullets on or off
regardless of line speed or terminal type.  Line speeds of 1200
baud and slower and some terminal types set border bullets off in
the absence of these options.
.TP
.B \-inplace
.br
Option \-inplace preserves file links except for singly\-linked
files.  From within the editor, link preservation can be
turned on and off, even for singly\-linked files, by the update
command.
.TP
.B \-terminal=termtype
.br
Option \-terminal=termtype identifies the terminal type explicitly.
In the absence of the \-terminal option the terminal type is retrieved
from the
.SM TERM
environment variable.
.TP
.B \-keyboard=termtype or \-dkeyboard=termtype
.br
Option \-keyboard=termtype sets the keyboard type to "termtype".  It
overrides any "\-terminal=" argument and also any
.SM EKBD
environment variable.
.TP
.B \-dtermcap
.br
Option \-dtermcap forces
.I E
to use the termcap entry for the terminal
identified by the
.SM TERM
variable or \-terminal option, disregarding
any applicable compiled-in code.
.TP
.B \-notracks
.br
Option \-notracks will allow you to edit a file without using or
disturbing the work files (keys, state, and change files) from your
previous session.
.TP
.B \-package=directory_name
.br
Option \-package=directory_name allows you to overwrite the build in
default directory in which the Rand editor is looking to find
service programs (like just, fill) and messages and information
files (like Crashdoc, helpkey).
In the absence of the \-package option the Rand package directory is
retrieved from the
.SM RAND_PACKG
environment variable.
.TP
.B \-replay=filename
.br
Option \-replay=filename allows you to replay with a specified keys
file, where "filename" represents that keys file.  Use \-silent when
the replay display is not desired.  The message "***replay
completed" or "***replay aborted" signals the end of the replay.
The
.SM <INT>
key can be used to stop the replay short of completion.
Files .ec1* and .ek1* must not be present if the replay is to
work.
.TP
.B \-norecover
.br
Option \-norecover will open the edit session as defined by the .es1
state file, ignoring any recovery processing of a prior crashed or
aborted session.
.TP
.B \-regexp
.br
Option \-regexp enables regular expression
pattern matching for searching
and replacing.  This mode is also enabled by the "re[gexp]" command.
In regular expression mode, "RE"
is displayed at the bottom of the screen.  The command "-re[gexp]"
returns to non-regular expression mode.
The regular expressions recognized are
essentially the same as that for \fIed\fR.
.TP
.B \-state=filename
.br
Option \-state=filename causes the initial state of the editor to be
read from the indicated "state" file.  The state file from a previous
\fIE\fR session (with alternate files, windows, cursor positioning,
etc.) may be saved by exiting, and renaming the state file (".es1").
.TP
.B \-dump_state_file
.br
Option \-dump_state_file display in a readable format the content of the
"state" file. This state file can be explicitely define by the
"\-state=filename" option or is the state file which will be used
when the editor is called without file name parameter.
.TP
.B \-[no]stick
.br
Option \-stick sets the behavior at the right margin to stop in response
to typing or a <RT ARROW>.  \-nostick is the default, and will automatically
shift the window right in response to text or <RT ARROW> key.
.TP
.B \-nox11
.br
Option \-nox11 disables calls to X11 library during startup to get the keyboard
mapping.
.TP
.B \-debug=[debug output file name]:[debug level]
.br
Option \-debug starts a debugging session, level = 0 : debugging suspended,
level != 0 : debugging actif. Use the command 'set debuglevel x' to
suspend, resume the debugging. When the debugging is suspended, the
debugging output file is flused, and closed. When debugging is resumed,
entering in a level != 0, the debugging output file is reopened for append.
On starting the debugging session, the sizeof the major Rand data type
(like Char, Flag, ...) is dumped on the debugging file.
.TP
.B \-version
.br
Option \-version display editor version informations. It is equivalent to
the \-help option.
.SH TERMINAL TYPES
The terminal type is defined by the option -terminal if given,
otherwise by the TERM environment variable.
.PP
The mapping of the logical keys onto physical terminal keys
is also defined by the terminal type,
unless the EKBD environment variable is set to
standard or the option -keyboard=standard is given.
This option overrides the EKBD variable, if any.
.PP
The instructions to drive the terminal may be coded in the editor
or may be stored in the termcap(5) database.
The parameter -dtermcap forces the use of termcap(5).
.PP
The following terminal types are recognised:
.PP
.RS 8
Pericom                   - pa
.br
Pericom with 132 columns  - pa132
.br
Pericom graphics terminal - vt100
.br
DEC VT100 terminal        - vt100
.br
DEC VT220 terminal        - vt200
.br
DEC VT220 dense mode      - vt200-w
.br
SUN Workstation           - sun
.br
Newbury 7001              - nb7
.br
TANDBERG 2200             - tdv
.RE
.PP
where the right hand side gives the terminalid.
.bp
.sh "Terminal tables"
The following tables show function key assignments on the
different terminals.
.br
.sp
.I The Newbury 7001 terminal:
.PP
The terminal type must be set to
.I nb7
and ned needs a keyboard file which is used
to map the newbury "function" keys.
This keyboard file is preferably set in the environment variable
EKBFILE, currently the file is called
.I /etc/e/kbfiles/newbkb.
.PP
Before the Newbury can be used it must be set up locally by the user.
He must check the terminal status page.
This is done by putting the terminal in EDIT mode, press the EDIT key.
Then press the CONTROL and HOME keys at the same time.
.PP
The terminal must be set up with the following options:
.br
In the top status line: ... DELETE=Y;NL=NO...
.br
In the status page:
.PP
.br
- All control characters allowed, no characters are underlined.
.br
- clear from home
.br
- clear on reset : yes
.br
- cursor skip : yes
.br
- attributes: protect = no, blink=no, underline=yes.
.nf
.PP
Using the above mentioned keyboard file, the ned editor works as
the following control keys table shows:


+------------------------------+-------------------------+
|   CLEAR     = ALT            |  ^F     = LINE+         |
|   DELCHAR   = DELCH          |  ^G     = PICK          |
|   INSCHAR   = INSCH          |  ^L     = PAGE+         |
|   DELLINE   = DELLN          |  ^P     = PAGE-         |
|   INSLINE   = INSLN          |  ^Q     = CTRLCHAR      |
|   DEOL      = ERASE          |  ^R     = REPL          |
|   SENDLINE  = JOIN           |  ^S     = SPLIT         |
|   SENDPAGE  = PAGE+          |  ^T     = SCH+          |
|   BREAK     = ENTER          |  ^U     = BTAB          |
|   RUBOUT    = BSP            |  ^V     = MARK          |
|   ESCAPE    = INT            |  ^W     = CHWIN         |
|   TAB       = +TAB           |  ^Z     = JOIN          |
|   ^C        = ENTER          |  ^^     = SCH+          |
|   ^D        = LIN-           |  ^\\     = SCH-         |
|   ^E-^R     = WIN RIGHT      |  ^@     = ENTER         |
|   ^E-^L     = WIN LEFT       |  ^H,<-  = LEFT          |
|   HOME,^]   = HOME           |  ^K,UP  = UP            |
|   ^X,->     = RIGHT          |  ^J,DOWN= DOWN          |
+------------------------------+-------------------------+
.fi
.bp
.I The TANDBERG 2200 terminal:
.PP
The terminal type must be set to
.I tdv
and ned needs a keyboard file which is used
to map the TANDBERG "function" keys.
This keyboard file is preferably set in the environment variable
EKBFILE, currently the file is called
.I /etc/e/kbfiles/tdvkb.
The assignments of the keys to editor functions as defined in the
keyboard file for the TANDBERG terminal is shown in the following table:
.nf


+---+----+----+----+----+----+----+----+----+----+----+----+----+----+---+
|es | p1 | p2 | p3 | p4 | p5 | p6 | p7 | p8 |MARK|PICK| pa | se | wo |lo |
+---+----+----+----+----+----+----+----+----+----+----+----+----+----+---+
|cps|  1 |  2 |  3 |  4 |  5 |  6 |  7 |  8 |  9 |  0 |  - |  ^ |-LN |DEL|
+---+----+----+----+----+----+----+----+----+----+----+----+----+----+----+
|INS| ct |  q |  w | e  | r  | t  | y  | u  | i  | o  | p  | @  | [  |+LN|
+---+----+----+----+----+----+----+----+----+----+----+----+----+----+---+
|ALT|lock| a  | s  | d  | f  | g  | h  | j  | k  | l  | ;  | :  | ]  |   |
+------+----+----+----+----+----+----+----+----+----+----+----+------+ cr|
| shift|\\   | z  | x  | c  | v  | b  | n  | m  | ,  | .  | /  | shift|   |
+------+----+----+----+----+----+----+----+----+----+----+----+------+---+

		     +--------------------------------+
		     |             space              |
		     +--------------------------------+


+------+------+------+          +------+------+------+------+
|DELCH | PICK |CHWIN |          | CMD  |      |      |      |
+------+------+------+          +------+------+------+------+
|(B)TAB|      |QUOTE |          |DELLN |INSLN | REPL |ERASE |
+------+------+------+          +------+------+------+------+
| LIN- |      | LIN+ |          |SPLIT | JOIN | SCH+ | SCH- |
+------+------+------+          +------+------+------+------+
|PAGE- | INT  |PAGE+ |          | LIN- | LIN+ |  INT |CHWIN |
+------+------+------+          +------+------+------+------+
|WIN <-|  UP  |WIN ->|          |PAGE- |PAGE+ | BTAB | ALT  |
+------+------+------+          +------+------+------+------+
| LEFT | HOME |RIGHT |          | SCH- | SCH+ | REPL |      |
+------+------+------+          +------+------+------| CMD  |
| BTAB | DOWN | TAB  |          |     MARK    | PICK |      |
+------+------+------+          +-------------+------+------+
.fi
.bp
.nf

.I Pericom terminal:

	   Numeric keypad                      Function keypad

  +-------+-------+-------+-------+       +-------+-------+-------+
  | LIN-  | LIN+  | INT   | CHWIN |       |       | WIN<- | WIN-> |
  +-------+-------+-------+-------+       +-------+-------+-------+
  | PAGE- | PAGE+ | BTAB  |  ALT  |       | INSLN |   UP  | INSCH |
  +-------+-------+-------+-------+       +-------+-------+-------+
  | SCH-  | SCH+  | REPL  |  EN-  |       | LEFT  |  HOME | RIGHT |
  +-------+-------+-------|  TER  |       +-------+-------+-------+
  |      MARK     | PICK  |       |       | DELLN |  DOWN | DELCH |
  +---------------+-------+-------+       +-------+-------+-------+


		    PF function key assignments

      pf1    pf2     pf3     pf4     pf5     pf6     pf7     pf8
  +-------+-------+-------+-------+-------+-------+-------+-------+
  |REDRAW | <-WIN | WIN-> | PAGE+ | PAGE- | LINE+ | LINE- | TAB   |
  +-------+-------+-------+-------+-------+-------+-------+-------+

     pf9     pf10    pf11    pf12    pf13    pf14    pf15    pf16
  +-------+-------+-------+-------+-------+-------+-------+-------+
  | BTAB  | WORD+ | WORD- | DWORD | HELP  | BOX   | SPLIT | JOIN  |
  +-------+-------+-------+-------+-------+-------+-------+-------+

     pf17    pf18    pf19    pf20    pf21    pf22    pf23    pf24
  +-------+-------+-------+-------+-------+-------+-------+-------+
  | CCASE | CAPS  | FILL  |JUSTIFY|CENTRE | ERASE | QUOTE | EXIT  |
  +-------+-------+-------+-------+-------+-------+-------+-------+


.bp
.I DEC VT220 terminal:

   F6     F7     F8     F9     F10        F11    F12    F13    F14
 +------+------+------+------+------+   +------+------+------+------+
 |REDRAW|CHWIN |WIN-  |WIN+  |HOME  |   |SPLIT |JOIN  |CCAP  |CCASE |
 |      |      |>WIN- |>WIN+ |>HOME |   |      |      |      |      |
 +------+------+------+------+------+   +------+------+------+------+


    Help          Do                  F17     F18      F19      F20
 +--------+-----------------+     +--------+--------+--------+--------+
 | HELP   |      CMD        |     |DELWRD  |ERASE   |DELLN   |DELCH   |
 |        |                 |     |-DELWRD |-ERASE  |-DELLN  |>DELCH  |
 +--------+-----------------+     +--------+--------+--------+--------+

 +--------+--------+--------+     +--------+--------+--------+--------+
 |SRCH+   | INSCH  | DELCH  |     | WORD-  | WORD+  | INSLN  | INSCH  |
 |SRCHW+  |        | >DELCH |     |        |        |        |        |
 +--------+--------+--------+     +--------+--------+--------+--------+
 | PICK   | PAGE-  | PAGE+  |     | LINE-  | LINE+  | INT    | CHWIN  |
 | -PICK  | >PAGE- | >PAGE+ |     | >LINE- | >LINE+ |        |        |
 +--------+--------+--------+     +--------+--------+--------+--------+
 |        | UP     |              | PAGE-  | PAGE+  | TAB-   | ALT    |
 |        | >UP    |              | >PAGE- | >PAGE+ |        |        |
 +--------+--------+--------+     +--------+--------+--------+--------+
 | LEFT   | DOWN   | RIGHT  |     | SRCH-  | SRCH+  | REPL   |        |
 | >LEFT  | >DOWN  | >RIGHT |     | SRCHW- | SRCHW+ |        |        |
 +--------+--------+--------+     +--------+--------+--------|  CMD   |
 |                                |      MARK       | PICK   |        |
 |                                |     -MARK       | -PICK  |        |
 |                                +-----------------+--------+--------+


DEL: DEL (delete previous character).   CMD DEL: >DEL.
CTRL-G: HOME.
CTRL-I (HT): TAB.
CTRL-P: PW (auto-fill mode toggle).
CTRL-R: REDRAW.
CTRL-U: CCAP.
CTRL-Z: CTRLQ (control character prefix).

.bp
.I DEC VT100 terminal:


		     +-------+-------+-------+-------+
		     | INSLN | DELLN | INSCH | DELCH |
		     +-------+-------+-------+-------+
		     | LIN-  | LIN+  | INT   | CHWIN |
		     +-------+-------+-------+-------+
		     | PAGE- | PAGE+ | BTAB  |  ALT  |
		     +-------+-------+-------+-------+
		     | SCH-  | SCH+  | REPL  |  EN-  |
		     +---------------+-------|  TER  |
		     |      MARK     | PICK  |       |
		     +---------------+-------+-------+

There are no function  keys for WIN<- and WIN-> on a VT100 terminal.
	Use 'Ctrl-X Ctrl-H' and 'Ctrl-X Ctrl-L' instead.

.fi
.bp
.I Rand editor standard keyboard
.PP
The keyboard layout below is designed for
terminals with no function keys.
"^H" means control-H, and "^X-^U" means control-X
followed by control-U.  The ALT entry gives you a choice of `^/' or
`^_' because one or the other or both will not work on some terminals.
Note that JOIN, SPLIT, and ERASE are now available as function keys.
.nf
+-------------------------------------+----------------------+
| (ext)         ^X                    |    ^@     CMD        |
| +LINE         ^F                    |    ^A     CMD        |
| +PAGE         ^R                    |    ^B     SPLIT      |
| +SRCH         ^Y                    |    ^C     DEL        |
| +TAB          ^I                    |    ^D     -LINE      |
| -LINE         ^D                    |    ^E     -PAGE      |
| -PAGE         ^E                    |    ^F     +LINE      |
| -SRCH         ^T                    |    ^G     ERASE      |
| -TAB          ^X-^U                 |    ^H     LEFT       |
| ALT           ^X-^A or ^_ or ^/     |    ^I     +TAB       |
| DEL           ^C                    |    ^J     DOWN       |
| CHG WIN       ^X-^W                 |    ^K     UP         |
| CLOSE         ^V                    |    ^L     RIGHT      |
| CMD           ^A or NULL or ^@      |    ^M     RETURN     |
| CTRLCHAR      ^X-^C                 |    ^N     JOIN       |
| DELCHAR       ^W                    |    ^O     OPEN       |
| DOWN          ^J                    |    ^P     PICK       |
| ERASE         ^X-^E or ^G           |    ^Q     BTAB       |
| HOME          ^^                    |    ^R     +PAGE      |
| INSERT        ^Z or ESC or ^[       |    ^S     CHWIN      |
| INT           ^\\                    |    ^T     -SRCH      |
| JOIN          ^N or ^X-^J           |    ^U     MARK       |
| LEFT          ^H                    |    ^V     CLOSE      |
| MARK          ^U                    |    ^W     DELCHAR    |
| OPEN          ^O                    |    ^X     (ext)      |
| PICK          ^P                    |    ^Y     +SRCH      |
| REPL          ^X-^R or ^]           |    ^Z     INSMODE    |
| RETURN        ^M                    |    ^[     INSERT     |
| RIGHT         ^L                    |    ^\\     INT        |
| SPLIT         ^B or ^X-^S           |    ^]     REPL       |
| TABS          ^X-^T                 |    ^^     HOME       |
| UP            ^K                    |    ^_     ALT        |
| WIN LEFT      ^X-^H                 |    ^X-^A  ALT        |
| WIN RIGHT     ^X-^L                 |    ^X-^C  CTRLCHAR   |
|                                     |    ^X-^E  ERASE      |
|                                     |    ^X-^H  WIN LEFT   |
|                                     |    ^X-^J  JOIN       |
|                                     |    ^X-^L  WIN RIGHT  |
|                                     |    ^X-^R  REPL       |
|                                     |    ^X-^S  SPLIT      |
|                                     |    ^X-^T  TABS       |
|                                     |    ^X-^U  BTAB       |
|                                     |    ^X-^W  CHWIN      |
+-------------------------------------+----------------------+
.fi
.bp
.SH "EDITOR FUNCTIONS"
.SH
.SH "Terminology"
.PP
Keys denoted by upper case text are logical keys.
A logical key has a unique meaning to the editor
and is mapped on a physical key of a terminal
as given in the tables above.
.PP
The following generic items are used:
.TP
filename
UNIX file name, shell metacharacters are not expanded
.TP
text
.br
string of characters, may include blanks
.TP
number
decimal number in the range 1 - 32767
.TP
cursormovement
key sequences that result in a movement of the cursor
.TP
linearea
one or more entire lines. Specified either as a decimal number
immediately followed by l meaning lines,
or as decimal number followed by p meaning paragraphs.
A paragraph is terminated by an empty line or
end of file.
.TP
area
.br
a rectangular part of the file, defined by MARK followed
by cursor movements.
.PP
[ and ] (square brackets) surround optional items.
All other character strings are to be taken literally.
.sh "Key functions"
.PP
It is better to look at the output of "help keyf" for the updated
description of the key functions.
.PP
ALT                   - swap current file and alternate file.
.PP
BACKSPACE             - move cursor one position left.
.br
ENTER BACKSPACE       - move cursor to left edge of window.
.PP
BOX                   - draw a box around the boundary of a
.RS 24
marked area
.RE
.br
MARK cursormovement BOX
.PP
BTAB                  - move cursor left to previous tabulator stop.
.PP
CAPS                  - convert lower case characters to upper case
.RS 24
in the marked area. Only characters (a-z) are affected.
An area must be defined
.RE
.PP
CCASE                 - change case (lower -> upper and vice-versa)
.RS 24
in the marked area.
An area must be defined
.RE
.PP
CENTER                - center lines beginning at the cursor
.RS 24
If an area is marked, the marked area of integer lines
is centered. Otherwise one line is centered.
.RE
.PP
CHWIN                 - make next window the current window.
.br
ENTER number CHWIN    - make window number the current window.
.PP
CONTROL X CONTROL C   - the next character entered will be entered as
.RS 24
a control character.
.RE
.PP
DELCH                 - delete the character at the cursor.
.br
ENTER DELCH           - delete all characters from the cursor to the
.RS 24
end of the line. The most recently deleted characters
are kept in the erase buffer if not in insert state,
otherwise in the close buffer.
.RE
.PP
DELETE                - if not in insert state:
.RS 24
write a blank space to the left of the cursor
and move cursor one position left,
.br
in insert state:
.br
delete the character to the left of the cursor
and move the cursor one position left.
.RE
ENTER DELETE          - if not in insert state:
.RS 24
replace the part of the line to the left of the cursor
by blanks, put the replaced text in the erase buffer,
.br
in insert state:
.br
delete the text to the left of the cursor
and put the deleted text in the close buffer.
.RE
.PP
DELLN                 - delete the line with the cursor.
.br
ENTER number DELLN    - delete number lines starting at the cursor.
.br
MARK cursormovement DELLN
.RS 22
- delete the marked area.
.RS 2
The information deleted by the most recent DELLN
is kept in the close buffer.
.RE
.RE
ENTER DELLN           - insert the contents of the close buffer before
.RS 24
the line with the cursor if it is entire line(s),
otherwise at the cursor position.
.RE
.PP
DOWN                  - move cursor one position down.
.br
ENTER DOWN            - move cursor to the bottom of window
.PP
DWORD                 - delete the current word or the next word
.RS 24
if the cursor is positioned on a space
.RE
.PP
ENTER                 - set command mode.
.br
ENTER ENTER ALT       - recall latest command.
.PP
ERASE                 - blank out (replace by spaces) lines or
.RS 24
marked area
By default, the cursor line is blanked out.
.RE
.PP
EXIT                  - exit from the editor
.pp
FILL                  - fill lines beginning at the cursor
.RS 24
Fill moves words so that each line is filled with words
up to the right margin. Fill operates on paragraphs or on
areas if one has been defined.
.RE
.PP
HELP                  - invoke the Rand editor help facility
.PP
HOME                  - move cursor to upper left corner of window.
.br
ENTER HOME            - move cursor to lower left corner of window.
.PP
INSCH                 - set and reset insert state.
.RS 24
In insert state the typed in characters are inserted
in front of the character at the cursor.
.RE
.PP
INSLN                 - insert a line in front of the line with
.RS 24
the cursor.
.RE
ENTER number INSLN    - insert number lines in front of the line with
.RS 24
the cursor.
.RE
.br
MARK cursormovement INSLN
.RS 22
- insert a rectangle of blanks, of the size
.br
.RS 2
of the area,
with the upper left
corner at the cursor.
.RE
.RE
.PP
INT                   - interrupt the current command.
.PP
JOIN                  - join the next line to the current line
.PP
JUSTIFY               - justify text beginning at the cursor
.RS 24
If an area is specifed, justify operates on this area.
Otherwise one paragraph is justified.
The line width is that specified by the most recent center
fill or justify command - default 75
.RE
.PP
LEFT                  - move cursor one position left.
.br
ENTER LEFT            - move cursor to the left edge of the window.
.PP
LINEFEED              - move cursor one position down.
.br
ENTER LINEFEED        - move cursor to the bottom of the window.
.PP
LIN-                  - move window 5 lines up.
.br
ENTER LIN-            - move window bottom to the cursor.
.br
ENTER number LIN-     - move window number lines up.
.PP
LIN+                  - move window 5 lines down.
.br
ENTER LIN+            - move window top to the cursor.
.br
ENTER number LIN+     - move window number lines down.
.PP
MARK cursormovement   - define an area.
.RS 24
The cursor positions before
and after MARK are opposite corners of the rectangular area.
Vertical cursor movement define an area of entire lines.
.RE
ENTER MARK            - cancel marking.
.PP
QUOTE                 - enter a control character into the file
.RS 24
Pressing quote followed by a <char> causes
control <char> to be entered into the file
.RE
.PP
PAGE-                 - move window one window up.
.br
ENTER PAGE-           - move window to the beginning of the file.
.br
ENTER number PAGE-    - move window number windows up.
.PP
PAGE+                 - move window one window down.
.br
ENTER PAGE+           - move window to the end of the file.
.br
ENTER number PAGE+    - move window number windows down.
.PP
PICK                  - put the line at the cursor into
.RS 24
the pick buffer.
.RE
ENTER number PICK     - put number lines, beginning at the line with
.RS 24
the cursor, into the pick buffer.
.RE
.br
MARK cursormovement PICK
.RS 22
- put the area into the pick buffer.
.RE
ENTER PICK            - insert the contents of the pick buffer before
.RS 24
the line with the cursor if it is entire line(s),
otherwise at the cursor position.
.RE
.PP
REDRAW                - redraw the current screen
.RE
.PP
REPL                  - replace the string just found by the
.RS 24
string given with the most recent replace command.
.RE
.PP
RETURN                - move cursor to the beginning of the next
.RS 24
line in the window.
.RE
.PP
RIGHT                 - move cursor one position to the right.
.br
ENTER RIGHT           - if in text move cursor to first blank after
.RS 24
the text, otherwise to right edge of window.
.RE
.PP
SCH-                  - search backwards for the most recently
.RS 24
given text.
.RE
ENTER SCH-            - search backwards for the word at the cursor.
.RS 24
Word cannot contain blanks.
.RE
ENTER text SCH-       - search backwards for text.
.RS 24
If a range is in effect (set by the range command)
the searches are limited to the range.
.RE
.PP
SCH+                  - search forwards for the most recently
.RS 24
given text.
.RE
ENTER SCH+            - search forwards for the word at the cursor.
.RS 24
Word cannot contain blanks.
.br
.RE
ENTER text SCH+       - search forwards for text.
.RS 24
If a range is in effect (set by the range command)
the searches are limited to the range.
.RE
.PP
SPLIT                 - split current line at the cursor position
.RS 24
into two lines
.RE
.PP
TAB                   - move cursor to the next tabulator stop right
.RE
.PP
WORD+                 - move cursor to the next word right
.RE
.PP
WORD-                 - move cursor to the previous word left
.RE
.PP
WIN->                 - move window 16 positions right.
.br
ENTER WIN->           - move left edge of window to the cursor.
.br
ENTER number WIN->    - move window number positions to the right.
.PP
WIN<-                 - move window 16 positions left.
.br
ENTER WIN<-           - move right edge of window to the cursor.
.br
ENTER number WIN<-    - move window number positions to the left.
.RS 24
The left edge of the window cannot move
further left than tby
the beginning of the lines.
.RE
.PP
UP                    - move cursor one position up.
.br
ENTER UP              - move cursor to the top edge of the window.
.bp
.SH "EDITOR COMMANDS"
.SH
.PP

It is better to look at the output of "help cmd" command
for the updated description of Editor Commands.
.sp 2
.br
Commands are entered after the ENTER key is pressed
and are executed when RETURN is pressed after the command
has been typed in.
In the following only the part between ENTER and RETURN is given.
The command and its shortest possible abbreviation are given.
All abbreviations down to the shortest are valid.
.PP
box       - draw a box around the boundary of a marked area
.br
bo          Simple horizontal or vertical lines can be drawn by
.RS 12
marking an area with a single character height or width.
.RE
.PP
bye       - same command as exit.
.br
b
.PP
caps <marked area>
.br
.RS 10
- change lower case characters to upper case
.RE
.RS 12
Caps only changes the "target text"  - defined with the mark command
Only characters a-z are affected
.RE
.PP
ccase <marked area>
.br
.RS 10
- convert case upper -> lower and vice-versa
.RE
.RS 12
The marked area is first defined by the \fImark\fR command.
.RE
.PP
center [ width=number ] [ number ]
.br
ce        - center lines beginning at the
.RS 12
cursor such that there is the same amount of
blanks on either side of the text.
With no number parameter one line is centered,
otherwise number lines are treated.
With no number parameter and a marked area of integer lines
the area is centered.
The lines are centered individually.
.br
A line length of width is assumed, where width is the value specified
with the most recent center, fill, or justify command (default is 75).
Width may be abbreviated w.
.br
The most recently centered lines are put in the
adjust buffer in their original form.
.RE
.PP
close [ number ] [ linearea ]
.br
clo       - delete lines or area.
.RS 12
Without parameters the cursor line is deleted.
With number parameter number lines beginning at the cursor
are deleted.
With parameter linearea
the specified number of paragraphs
or lines are deleted.
With a marked area and no parameters the area is deleted.
.br
The most recently deleted information is
kept in the close buffer.
.RE
.PP
-close    - insert the contents of the close buffer.
.br
-cl         It is inserted before the cursor line for entire lines
.RS 12
otherwise at the cursor position.
.RE
.PP
delete    - delete the current file.
.RS 12
The file is deleted from the system on exit from the editor.
The delete can be nullified by exit with a parameter.
.RE
.PP
dword     - delete word
.br
\-dword      If the cursor is "on" a word, that word is deleted
.br
.RS 12
otherwise the next word is deleted.
.br
The deleted word can be restored by \-dword.
.RE
.PP
edit [ filename ]
.br
e         - edit the file filename.
.br
.RS 12
Filename becomes the current file and the current file
becomes the alternate file. The ALT key switches between them.
.br
.B \edit -
remove the current file from the list of edited files if it was not modified,
nor referenced.
.RE
.PP
erase [ number ] [ linearea ]
.br
er        - blank out lines or area.
.RS 12
Without parameters the cursor line is blanked out (replaced by blanks).
With number parameter number lines beginning at the cursor
are blanked out.
With parameter linearea
the specified number of paragraphs
or lines are blanked out.
With a marked area and no parameters the area is blanked out.
.br
The most recently blanked out information is
kept in the erase buffer.
.RE
.PP
-erase    - insert the contents of the erase buffer.
.br
-e          It is inserted before the cursor line
.RS 12
for entire lines,
otherwise at the cursor position.
.RE
.PP
exit | ex | bye | b [ nosave | quit | abort | dump ]
.br
.RS 10
- Exit the editor.
.RE
.RS 12
The different exit options affect
whether or not the editor updates the edited files
or not, and whether the ".e" state files are changed.
The following table give the complete exit story.
.ta \w'exit nosave\ \ 'u +\w'Saves\ \ 'u +\w'Update\ \ 'u +\w'Keys\ file\ \ 'u
.sp
	Saves	Update	Remove	Remove
.br
	Files	State	Keys\ file	Change file
.br
.sp
exit	 X	 X	 X	 X
.br
exit nosave	 -	 X	 X	 X
.br
exit quit	 -	 -	 X	 X
.br
exit abort	 -	 -	 -	 X
.br
exit dump	 -	 -	 -	 -
.ta \w'underlay\ \ \ \ 'u
.sp
blot	adjust | close | erase | pick | run | box
.br
\-blot	adjust | close | erase | pick | run | box
.br
cover	adjust | close | erase | pick | run | box
.br
insert	adjust | close | erase | pick | run | box
.br
overlay	adjust | close | erase | pick | run | box
.br
underlay	adjust | close | erase | pick | run | box
.PP
All of these commands take the named buffer (one of
adjust, close, erase, pick, run, or box) and add or
merge it into the current file.
.PP
Insert takes the named buffer, moves the existing text
to make room for it, and then inserts it at the
current text location.  It is identical to the \-pick
command, except that now the other named buffers can
also be inserted.
.PP
Cover places the named buffer over top of the text without
moving anything.
.PP
Overlay is like cover, except that only the printing
characters in the buffer clobber the original text.
.PP
Underlay is the reverse of overlay, i.e. the buffer covers
only the non-printing characters of the original text.
.PP
Blot is like overlay except that positions in the file
corresponding to printing characters in the buffer are erased.
.PP
\-blot is like blot except that positions in the file
corresponding to blanks in the buffer are erased.
.RE
.PP
fill [ width=number ] [ linearea ]
.br
fi        - fill lines, beginning at the cursor,
.RS 12
such that there is the same amount of blanks on either side of the text.
.br
With parameter linearea the specified lines
or paragraphs are filled.
With no linearea parameter one paragraph is filled.
With no linearea parameter and a marked area of integer lines
the area is filled.
.br
A line length of width is assumed, where width is the value specified
with the most recent center, fill, or justify command (default 75).
Width may be abbreviated w.
.br
The most recently filled lines are put in the
adjust buffer in their original form.
.RE
.PP
feed [<range>] command-string
.RS 10
- start up a UNIX shell
.RE
.RS 12
A Unix shell is started with the command-string, and the
text in <range> is supplied to the shell as the standard
input.  Any results of the command are inserted before
the first line in the range.
.RE
.PP
goto [ b | e | number | rbeginning | rend ]
.br
g         - position cursor on line specified.
.RS 12
Parameter number specifies a sequential line number, parameter b
means begin of file, parameter e means end of file, and no parameter
means begin of file.
.br
Rbeginning and rend specify the beginning and end of a range
defined by the range command. They are valid even if the range has
been turned off. Their shortest abbreviations are rb and re,respectively.
.RE
.PP
insert [ adjust | close | erase | pick | run | box ]
.br
i         - insert contents of specified buffer at cursor position.
.br
.RS 12
.br
The buffers are filled by the following commands:
.br
adjust - center, fill, justify.
.br
close  - DELLN, close, and when in insert
.RS 9
state also:
.br
erase, ENTER DELCH, ENTER DELETE.
.br
Lost text from join.
.RE
erase  - erase, and when in insert state also:
.RS 9
ENTER DELCH and ENTER DELETE.
.RE
.br
pick   - PICK, pick.
.br
run    - run.
.br
box    - box is a buffer that always
.RS 9
contains a box around the previously
marked area.
.RE
.RE
.PP
join      - join the next line to the current.
.RS 12
The first non blank character of the next line is placed after the last
non blank character on the current line, or at the cursor if
it is farther to the right.
.br
This mode of operation may be overridden by marking the
positions on the two lines that should be joined.
Any lost text is put in the close buffer.
.RE
.PP
justify [ width=number ] [ linearea ]
.br
ju        - justify lines, beginning at the cursor such that the last
.RS 12
word on the lines ends at the right margin.
.br
With parameter linearea the specified lines
or paragraphs are justified.
With no linearea parameter one paragraph is justified.
With no linearea parameter and a marked area of integer lines
the area is justified.
.br
A line length of width is assumed, where width is the value specified
with the most recent center, fill, or justify command (default 75).
Width may be abbreviated w.
.br
The most recently justified lines are put in the
adjust buffer in their original form.
.RE
.PP
name filename
.br
.RS 10
- rename the current file
.br
.RE
.RS 12
The file being edited is renamed. The renaming takes effect when the
editor is exited normally. It may be nullified by exiting with a
parameter.
.RE
.PP
open [ number ] [ linearea ]
.br
op        - insert lines or area.
.RS 12
Without parameters an empty line is inserted before the cursor line.
With number parameter, number empty lines are inserted before the
cursor line.
With parameter linearea
the specified number of paragraphs
or lines are inserted before the cursor line.
With a marked area and no parameters
a blank filled area of that shape is
inserted at the cursor.
.RE
.PP
pick [ number ] [ linearea ]
.br
pi        - pick lines or area.
.RS 12
Without parameters the cursor line is put into the pick buffer.
With number parameter, number lines beginning at the cursor
are picked.
With parameter linearea
the specified number of paragraphs
or lines are picked.
With a marked area and no parameters the area is picked.
The most recently picked information is
kept in the pick buffer.
.RE
.PP
-pick     - insert the contents of the pick buffer.
.br
-p          It is inserted before the line with the cursor for
.RS 12
entire lines, otherwise at the cursor position.
.RE
.PP
range [ number ] [ linearea ]
.br
ra        - define a range for search and replace.
.RS 12
Searches and replaces are limited to the defined range.
Number and linearea specify the range from the
cursor. One of them must be given unless an area has been
marked, then the range is the area.
.RE
.PP
-range    - turn off previously specified range. The range no longer
.br
-ra         limits searches and replaces, but it remains defined.
.PP
?range    - show status of range.
.br
?           The status (current or dormant) and extent of the
.RS 12
range is displayed.
.RE
.PP
redraw    - redraw the screen.
.br
red         The current screen is redrawn, free of possible messages
.RS 12
not coming from the editor.
.RE
.PP
replace [ number ] [ linearea ] [ show | interactive ] /texta/textb/
.br
rep       - replace texta by textb
.RS 12
from cursor and towards the end of the file.
.br
If a number or linearea are given, or if an area has been marked,
the replacements are restricted to the area, otherwise
they are made from the cursor position to the end of the file.
If a range is in effect (set by the range command) the replacements
are limited to the range.
.br
The show parameter will show the replacements on the screen as they
take place.
.br
The interactive parameter will cause the command to
stop when a testa has been found.
The replacement by textb is then done when the REPL key is pressed,
and the search for texta is continued when the SCH+ key is pressed.
A replacement may be skipped by pressing SCH+ instead of REPL.
.br
Show and interactive may be abbreviated down to
s, and i respectively.
.br
Any character that is not a letter or a digit may be used for delimiting
texta and texta.
.RE
.PP
-replace [ number ] [ linearea ] [ show | interactive ] /texta/textb/
.br
-re       - replace texta by textb
.RS 12
from cursor and towards the beginning of the file.
.br
If a number or linearea are given, or if an area has been marked,
the replacements are restricted to the area, otherwise
they are made from the cursor position to the beginning of the file.
If a range is in effect (set by the range command) the replacements
are limited to the range.
.br
The show parameter will show the replacements on the screen as they
take place.
.br
The interactive parameter will cause the command to
stop when a testa has been found.
The replacement by textb is then done when the REPL key is pressed,
and the search for texta is continued when the SCH- key is pressed.
A replacement may be skipped by pressing SCH- instead of REPL.
.br
Show and interactive may be abbreviated down to
s, and i respectively.
.br
Any character that is not a letter or a digit may be used for delimiting
texta and texta.
.RE
.PP
regexp    - enter regular expression mode for searching & replacing;
.br
\-regexp     exits regular expression mode.
.RS 12
While in regular expression mode, "RE" is displayed at the bottom
of the screen.
.RE
.PP
run text  - execute the text as a UNIX command.
.br
r           If there is a marked area of entire lines
.RS 12
it is given to the command as standard input and is replaced by the
standard output from the command.
If there is no marked area the standard output is inserted
before the line with the cursor.
The run command is similar to the feed command, except that
the marked text is deleted and replaced by the results of running the
command-string.
.RE
.PP
save filename
.br
sa        - save a copy of the current file.
.RS 12
The current file including its modifications is copied to filename.
Filename must be different from the name of the current file.
.RE
.PP
set \fIoption\fR  used to set various options
.br
?set
.br
.RS 12
.ds t \t
.nr ,i \w'wordelim \fImode\fR'u+1P
.ta \n(,iu +1P
.nf
.sp 6p
+line \fIn\fR\*tsets the <+line> key to \fIn\fR lines
\-line \fIn\fR\*tsets the <\-line> key to \fIn\fR lines
line \fIn\fR\*tsets the <+line> and <\-line> keys to \fIn\fR lines
+page \fIn\fR\*tsets the <+page> key to \fIn\fR screens
\-page \fIn\fR\*tsets the <\-page> key to \fIn\fR screens
page \fIn\fR\*tsets the <\-page> and <+page> keys to \fIn\fR screens
?\*tdisplays the values of several options (same as the "?set" command)
bell\*tenable the bell
debuglevel \fIn\fR\*tchange the debugging level (within a debugging session),
level 0 suspend debugging, level != 0 resume.
nobell \*tdisable the bell
hy \*tenables splitting of hyphenated words by fill/just
nohy \*tdisables splitting of hyphenated words by fill/just (default)
left \fIn\fR\*tsets the <window left> key to \fIn\fR cols
lmar \fIn\fR\*tsets the left margin to \fIn\fR
preferences \*tsave the current setting in the $HOME/.Rand/preferenes file.
preferences \fI?\fR\*tdisplay preferences for the current terminal type.
right \fIn\fR\*tsets the <window right> key to \fIn\fR cols
rmar \fIn\fR\*tsets the right margin (linewidth) to \fIn\fR
window \fIn\fR\*tsets the <window left> and <window right> keys to \fIn\fR cols
width \fIn\fR\*tsets the linewidth to \fIn\fR cols (for fill, etc.)
wordelim \fImode\fR\*tsets the action of the <+word> and <\-word> keys.
.br
.fi
.in \n(.iu+\n(,iu
If mode is \fIwhitespace\fR (the default), words are delimited by blanks
and newlines, and the cursor advances to the first character following
the delimiter.
If mode is \fIalphanum\fR, words are delimited by all
special characters in addition to blanks and newlines,
and the cursor advances to the first alphanumeric character
following the delimiter.
.fi
.br
.sp 6p
.in
The "line", "page", and "window" options may be individually set in
different windows.
.RE
.PP
split     - split the cursor line in two.
.br
sp          The part of the cursor line beginning at the cursor is
.RS 12
moved to the beginning of a line inserted after the cursor line.
.RE
.PP
stop      - Use 'job control' to suspend the editor
.br
.RS 12
Returns control to the shell, resume with the 'fg' command.
Works only under Berkeley
.SM VAX UNIX
versions
.SM 4.1BSD
and
.SM 4.2BSD.
.RE
.PP
tab [number ... ]
.br
.RS 10
- set tabulators.
.RE
.RS 12
Tabulators are added at the column numbers if specified,
otherwise at the cursor position.
.RE
.PP
-tab [number ... ]
.br
.RS 10
- clear tabulators.
.RE
.RS 12
Tabulators are cleared at the column numbers if specified,
otherwise at the cursor position.
.RE
.PP
tabfile [ filename ]
.br
tabf      - add tabulators from a file.
.br
.RS 12
Tabulators are added at positions
given as integers separated by blanks in the file filename.
.RE
.PP
tabs number
.br
.RS 10
- set tabulators every number-th column.
.RE
.RS 12
The first is set in column 1 and they are added to already set tabulators.
.RE
.PP
-tabs number
.br
.RS 10
- clear tabulators every number-th column.
.RE
.RS 12
The first is cleared in column 1.
.RE
.PP
track     - turn on tracking in the current window.
.br
tr          Tracking means that the window movements
.RS 12
in the current file are reproduced in the alternate file,
such that when the alternate file is made current its window
will be positioned as if it had made the movements.
.RE
.PP
-track    - turn off tracking in the current window.
.br
-tr
.PP
update | \-update [ inplace | \-inplace ]
.br
.RS 10
- editor exit control command
.RE
.RS 12
The update command tells the editor what to do on
exit with the current file.  The update command
can be used to specify whether or not to break
links, the \-update command causes any changes to
the current file to be ignored.
.RE
.PP
window [ filename ]
.br
w         - create a window
.br
.RS 12
A new window is created as specified by the cursor position
The cursor must be at the top or left edge of the current window.
The new window is cut out of the current at the cursor.
When filename is given the file will be in the window,
otherwise the current file will be displayed.
.RE
.PP
-window   - remove the current window.
.br
-w          note that the first window cannot be removed.
.PP
wp        - enter "word processing" mode
.br
\-wp       - exit "word processing" mode
.br
.RS 12
A
.SM WP
is displayed at the bottom of the screen in word processing mode.
.SM WP
mode enables power typing or "word wrap", where any text that is
entered past the right margin (see "set rmar" above) is automatically
placed on the following line at the left margin (see "set lmar" above).
.SM WP
mode also moves the cursor to the left margin in response to a <RETURN>.
If
.SM WP
mode is entered while a marked area is in effect, the boundaries of
the marked area become the left and right margin settings.
.RE
.PP
.SH FILES
.SH
.TP
\\,filename - comma file.

The file being edited is left
unchanged until a normal exit is made from the editor.
The file is then copied onto a file with the same name
preceded by a , (comma) and thereafter
the modifications are applied.
If no modifications are made or at abnormal exits
(exit with a paramater) the comma file is not produced.
.br
For a file to be changed, it and its directory
must both have write permission.
.TP
\\.ec1      - change file.
All changed text from the file being edited is collected in
the change file.
The changed text may be accessed as alternate files
via the pseudo file names #o and #p,
and text may be picked from them.
.br
The #o (old lines) file contains all original versions
of changed lines.
.br
The #p (pick) file contains all picked text.
Text picked from the #p file is not appended to it.
.TP
\\.ek1 .ek1b  - keystroke files.
The files contain all changes made to the files being edited.
The format is such that the changes can be applied when a normal
exit is made from the editor.
.br
If an exit is made with parameter dump, or if the editor
crashes, the keystroke files can be used for recovering
the lost editing session.
Recovery is done by invoking the editor without a filename,
it will then present recovery options to the user.
The file .ek1b is used in the recovery process.
.TP
\\.es1      - state file.
The state file contains information about the last file edited in the
directory, including file name, the place in the file where the cursor
were when the editor was terminated, the width value (center, fill, justify),
the state (insert or normal), the tabulators and the last search string.
.br
When the editor is called without a file name the state file is used
to recreate the situation that existed at the previous termination.
.SH SEE ALSO
P.Villemoes, Introduction to the Rand Editor, PRIAM/VAX/3
.sp
The on line "help" command. A good entry point is the bare "help".
On Line "help" is more accurate and provide more complete information.
