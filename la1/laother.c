#ifdef COMMENT
	Copyright abandoned, 1983, The Rand Corporation
#endif

#include "la.local.h"

La_stream *
la_other (plas)
Reg2 La_stream *plas;
{
    Reg1 La_stream *tlas;
    Reg3 La_file *nlaf;

    if (plas->la_file->la_refs > 1) {
	nlaf = plas->la_file;
	for (tlas = nlaf->la_fstream; tlas; tlas = tlas->la_fforw)
	    if (tlas != plas)
		return tlas;
#ifdef LA_DEBUG
	la_abort ("la_other can't find stream. Refs = %d",
		  plas->la_file->la_refs);
#endif
    }
    return (La_stream *) 0;
}
