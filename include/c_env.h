/*   C Standard defines.
 *   This file containes machine- and compiler-specific #defines
 */

/*
 *  In the #define's that follow, define the first n to 'register'
 *   and the rest to nothing, where 'n' is the number of registers
 *   supported by your compiler.
 *   For an explanation of this, see ../man/man5/c_env.5
 */
#define Reg1
#define Reg2
#define Reg3
#define Reg4
#define Reg5
#define Reg6
#define Reg7
#define Reg8
#define Reg9
#define Reg10
#define Reg11
#define Reg12

#define CHARMASK   0x0FF
#define CHARNBITS  8
#ifdef __alpha
#define MAXCHAR    0x7F
#else
#define MAXCHAR    0x0FF
#endif 

#define SHORTMASK  0x0FFFF
#define SHORTNBITS 16
#define MAXSHORT   0x07FFF

#ifdef __alpha
#define LONGMASK  0xFFFFFFFFFFFFFFFF
#define LONGNBITS 64
#define MAXLONG   0x7FFFFFFFFFFFFFFF
#else
#define LONGMASK  0xFFFFFFFF
#define LONGNBITS 32
#define MAXLONG   0x7FFFFFFF
#endif

#define INTMASK  0xFFFFFFFF
#define INTNBITS 32
#define MAXINT   0x7FFFFFFF

#define BIGADDR         /* text address space > 64K */
/* fine ADDR64K            text and data share 64K of memory (no split I&D */

#define INT4            /* sizeof (int) == 4 */
/* fine INT2               sizeof (int) == 2 */

			/* unsigned types supported by the compiler: */
#ifdef _AIX
#define UNSCHAR         /* unsigned char  */
#endif
#define UNSSHORT        /* unsigned short */
#define UNSLONG         /* unsigned long  */

#ifdef _AIX
#define NOSIGNEDCHAR    /* to avoid degenerate unsigned comparisons in la1 */
#endif

#define STRUCTASSIGN	/* Compiler does struct assignments */

#define UNIONS_IN_REGISTERS     /* compiler allows unions in registers */

/* fine void int           Fake the new 'void' type to an int */

/* defend against POSIX routines with same name */

#define putshort rand_put_short
#define putlong  rand_put_long
#define move     rand_move
