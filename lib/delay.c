

/* Thanks to Berkeley termcap */

/*
 * The following array gives the number of tens of milliseconds per
 * character for each speed as returned by gtty.  Thus since 300
 * baud returns a 7, there are 33.3 milliseconds per char at 300 baud.
 * Assumes that EXTA and EXTB speeds are 19200 and 38400 baud.
 */
static
short	tmspc10[] = {
	0, 2000, 1333, 909, 743, 666, 500, 333, 166, 83, 55, 41, 20, 10, 5, 3
};
extern char ospeed;

delay (ms)
register int ms;
{
    register int mspc10;

    /*
     * If no delay needed, or output speed is
     * not comprehensible, then don't try to delay.
     */
    if (ms == 0)
	    return;
    if (ospeed <= 0 || ospeed >= (sizeof tmspc10 / sizeof tmspc10[0]))
	    return;

    /*
     * Round up by a half a character frame,
     * and then do the delay.
     * Too bad there are no user program accessible programmed delays.
     * Transmitting pad characters slows many
     * terminals down and also loads the system.
     */
    mspc10 = tmspc10[ospeed];
    ms += mspc10 / 2;
    for (ms /= mspc10; ms > 0; ms--)
	    putchar ('\0');
    return;
}
