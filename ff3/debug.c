/* some ff3 package debuging routine */
/*  Fabien PERRIOLLAT cern Feb 2002 */


#include "ff.local.h"

static FILE * dmp_stream = NULL;

ff_dump_stream (FILE *stream)
{
    dmp_stream = stream ? stream : stdout;
}


ff_sdump (Ff_stream *pffs, char *txt)
{
    fprintf (dmp_stream, "Ff_stream 0x%x dump: %s\n", pffs, txt);
    fprintf (dmp_stream, "  f_file= 0x%x, f_mode= 0x%x, f_count= %d, f_offset=%d",
	    pffs->f_file, pffs->f_mode, pffs->f_count, pffs->f_offset);
    return;
}

ff_fdump (Ff_file *pfff, char *txt)
{
    fprintf (dmp_stream, "Ff_file 0x%x dump: %s\n", pfff, txt);
    fprintf (dmp_stream, "  fl_path= %s, fn_fd= %d, fb_qf= 0x%x, fn_refs= %d",
	    pfff->fl_path, pfff->fn_fd, pfff->fb_qf, pfff->fn_refs);
    return;
}

