#ifdef COMMENT
	Copyright abandoned, 1983, The Rand Corporation
#endif

#define OPPICK       1
#define OPCLOSE      2
#define OPERASE      4
#define OPCOVER    010  /***********************************/
#define OPOVERLAY  020  /*  these six must be consecutive  */
#define OPUNDERLAY 040  /*                                 */
#define OPBLOT    0100  /*                                 */
#define OP_BLOT   0200  /*                                 */
#define OPINSERT  0400  /***********************************/
#define OPOPEN   01000
#define OPBOX    02000
#ifdef LMCCASE
#define OPCAPS   04000
#define OPCCASE 010000
#endif

#define OPQTO       (OPCLOSE | OPPICK | OPERASE)
#define OPQFROM     (OPCOVER|OPOVERLAY|OPUNDERLAY|OPBLOT|OP_BLOT|OPINSERT)
#ifdef LMCCASE
#define OPINPLC     (OPCAPS | OPCCASE)
#endif
#define OPLENGTHEN  (OPOPEN | OPINSERT)


/* if you want to change the order of the numbers that folow, you may have
 * to also rearrange the array initialization for
 * qtmpfn[NQBUFS] in e.x.c
 **/
#define QNONE   -1
#define QADJUST  0
#define QPICK    1      /************************************/
#define QCLOSE   2      /*  these three must be consecutive */
#define QERASE   3      /************************************/
#define QRUN     4
#define QBOX     5  /* this is a pseudo-buffer */
#define NQBUFS   5  /* not counting pseudo-buffers */
extern
S_svbuf qbuf[];
extern
AFn qtmpfn[];
