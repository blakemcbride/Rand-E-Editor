
/* file e.it.h - input (keyboard) table parsing and lookup
 *
 **/

#ifdef COMMENT
	Copyright abandoned, 1983, The Rand Corporation
#endif

#ifdef COMMENT
The input table is stored as a tree, where leaves are of the form:

   +----------------------------+      +-------------------------+
   |                            |      |                         |
   | char   1   o   len   o-----+----->| next node               |
   |            |               |      |                         |
   +------------+---------------+      +-------------------------+
		|
		V
	    +--------+
	    | value  |
	    +--------+


and other nodes are of the form:

   +-------------------------+      +-------------------------+
   |                         |      |                         |
   | char   0   o      o-----+----->| next node               |
   |            |            |      |                         |
   +------------+------------+      +-------------------------+
		|
		V
	  +-------------------------+
	  |                         |
	  | child node              |
	  |                         |
	  +-------------------------+

#endif

struct itable {
    char it_c;              /* Character to be matched */
    unsigned int it_leaf;   /* This is a leaf */
    union {
	struct {
	    char *it_lval;  /* Value (for leaves only) */
	    int it_llen;    /* Length of it_lval */
	} it_un_leaf;
	struct itable *it_ulink; /* First child (for non-leaves) */
    } it_un;
    struct itable *it_next; /* Next one to try */
};

#undef OLD_H_VERSION
#ifdef OLD_H_VERSION
struct itable {
    char it_c;              /* Character to be matched */
    unsigned int it_leaf:1; /* This is a leaf (1 bit Bit-fields): not efficient */
    union {
	struct {
	    char *it_lval;  /* Value (for leaves only) */
	    int it_llen;    /* Length of it_lval */
	} it_un_leaf;
	struct itable *it_ulink; /* First child (for non-leaves) */
    } it_un;
    struct itable *it_next; /* Next one to try */
};
#endif

#define TMPSTRLEN 1024   /* Has to be long enought for init strings */

#define NULLIT (struct itable *)0

#define it_val it_un.it_un_leaf.it_lval
#define it_len it_un.it_un_leaf.it_llen
#define it_link it_un.it_ulink

extern struct itable *ithead;

/* The following will have to be changed if you want to allow space to
   be redefined or some such */
#define KBINIT  040     /* Must not conflict with CC codes */
#define KBEND   041

extern S_looktbl itsyms[];

#define IT_NOPE -1      /* Not found */
#define IT_MORE -2      /* Maybe, need more input */

extern char *kbinistr;
extern char *kbendstr;
extern int  kbinilen;
extern int  kbendlen;
