#ifdef COMMENT
Copyright       abandoned, 1983, The Rand Corporation
#endif


long
ldiv(a, b, remainder)
    long            a;
    int             b;
    int            *remainder;
{
    *remainder = a % b;
    return a / b;
}
