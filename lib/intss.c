
intss()
{   int buf[3];

#ifdef UNIXV7
    return(gtty(0,buf) != -1);
#else
    isatty (0);
#endif
}
