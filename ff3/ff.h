#ifdef COMMENT
file            ff.h
    Modified version for MSDOS / F. PERRIOLLAT - CERN - Feb 1996
    Dec 1997 (for the file style : MicroSoft or UNIX)
    and Dec 1998 for Linux version
#endif

#ifdef COMMENT
                Copyright abandoned, 1983, The Rand Corporation
#endif

#include <stdio.h>
#include <limits.h>

extern int      errno;

#define FF_BSIZE   BUFSIZ	/* Size of a FF block */

#ifndef NOFILE
#define NOFILE _NFILE		/* Number of system opens allowed */
#endif
#if NOFILE > 64                 /* must be small enough for the 'char' storage */
#undef NOFILE
#define NOFILE 64
#endif

#define NOFFFDS (NOFILE+NOFILE)	/* # of active FF files */


/* file description structure.  one for each distinct file open */

#define FL_PATH_SIZE PATH_MAX

typedef
struct ff_file {
    struct ff_buf  *fb_qf;	/* Queue of blks associated with this file */
    char            fn_fd,	/* File Descriptor	 */
                    fn_mode,	/* Open mode for file	 */
                    fn_refs;	/* # of references	 */
#ifdef UNIXV7
    dev_t           fn_dev;	/* Device file is on    */
    ino_t           fn_ino;	/* Inode of file        */
#endif
#ifdef UNIXV6
    char            fn_minor,	/* Device file is on    */
                    fn_major;
    int             fn_ino;     /* Inode of file        */
#endif
    long            fn_realblk;	/* Image of sys fil pos */
    long            fn_size;	/* Current file size    */
#ifdef  EUNICE
    char           *fn_memaddr;	/* address of in-core file */
#endif				/* EUNICE */
    char            style;      /* Current file style   */
    char            user_style; /* User defined file style */
    char            *fl_path;   /* file name */
}               Ff_file;
extern Ff_file  ff_files[];

/* File style (which define the end of line mark
 *  MicroSoft : CR,LF
 *  UNIX      : LF
 *      style 0 : not define, will use the default
 */
#define MS_FILE 1       /* MicroSoft file style */
#define UNIX_FILE 2     /* UNIX file style */

/* stream structure */
typedef
struct ff_stream {
    char            f_mode,	/* Open mode for handle */
                    f_count;	/* Reference count	 */
    Ff_file        *f_file;	/* Fnode pointer	 */
    long            f_offset;	/* Current file position */
}               Ff_stream;      /* or buffered amount    */
/* f_flag bits */
#define F_READ	01		/* File opened for read */
#define F_WRITE 02		/* File opened for write */
extern Ff_stream ff_streams[];

/* Buffer structure.  one for each buffer in cache */
typedef struct ff_buf {
    struct ff_buf  *fb_qf,	/* Q of blks associated */
                   *fb_qb,	/* with this file      */
                   *fb_forw,	/* forw ptr */
                   *fb_back;	/* back ptr */
    Ff_file        *fb_file;	/* Fnode blk is q'd on	 */
    long            fb_bnum;	/* Block # of this blk  */
    int             fb_count,   /* Byte count of block  */
                    fb_wflg;	/* Block modified flag  */
    char           *fb_buf;	/* Actual data buffer [FF_BSIZE]  */
}               Ff_buf;

/* list of all buffers. there is only one of these */
typedef struct ff_rbuf {
    struct ff_buf  *fb_qf,	/* not used */
                   *fb_qb,	/* not used */
                   *fb_forw,	/* first buf in chain */
                   *fb_back;	/* last buf in chain */
    int             fr_count;   /* total number of buffers */
}               Ff_rbuf;
extern Ff_rbuf  ff_flist;

typedef struct ff_st {
    int             fs_seek,	/* Total seek sys calls */
                    fs_read,	/* read  "    "	 */
                    fs_write;	/* write "    "	 */
    int             fs_ffseek,	/* Total seek calls */
                    fs_ffread,	/* read   "   */
                    fs_ffwrite;	/* write  "   */
}               Ff_stats;
extern Ff_stats ff_stats;

extern long     ff_size();
extern long     ff_grow();
extern long     ff_pos();
extern long     ff_seek();
extern Ff_buf  *ff_getblk();
extern Ff_buf  *ff_gblk();
extern Ff_buf  *ff_haveblk();
extern Ff_buf  *ff_putblk();
extern Ff_stream *ff_open();
extern Ff_stream *ff_fdopen();
