#ifdef COMMENT
	Copyright abandoned, 1983, The Rand Corporation
#endif

#include "la.local.h"

static FILE * dmp_stream = NULL;

la_dump_stream (FILE *stream)
{
    dmp_stream = stream ? stream : stdout;
}

la_fsddump (ffsd, lfsd, fsdbyteflg, txt)
Reg3 La_fsd *ffsd;
Reg4 La_fsd *lfsd;
Reg5 La_flag fsdbyteflg;
char *txt;
{
    fprintf (dmp_stream, "FSD dump: form La_fsd= 0x%x, to La_fsd= 0x%x, %s", ffsd, lfsd, txt);
    putc ('\n', dmp_stream);
    for ( ; ffsd ; ) {
	fprintf (dmp_stream, "La_fsd 0x%x ", ffsd);
	if ( ffsd->fsdfile && ffsd->fsdfile->la_ffs && ffsd->fsdfile->la_ffs->f_file )
	    fprintf (dmp_stream, " ->fsdfile= 0x%x, ->la_ffs= 0x%x, ->f_file= 0x%x\n    = \"%s\"\n",
		    ffsd->fsdfile,
		    ffsd->fsdfile->la_ffs,
		    ffsd->fsdfile->la_ffs->f_file,
		    ffsd->fsdfile->la_ffs->f_file->fl_path);
	fprintf (dmp_stream, " back= 0x%04x, forw= 0x%04x, ",
		ffsd->fsdback,  ffsd->fsdforw);
#ifdef LA_BP
	fprintf (dmp_stream, "fsdnbytes =%5d, ", ffsd->fsdnbytes);
#endif
	fprintf (dmp_stream, "fsdnlines= %3d, fsdfile= 0x%04x, ", ffsd->fsdnlines,
		ffsd->fsdfile);
	fprintf (dmp_stream, "fsdpos= %ld\n", ffsd->fsdpos);
	if (fsdbyteflg) {
	    if (   ffsd->fsdnlines == 1
		&& ffsd->fsdbytes[0] == 0
	       ) Block {
		La_linesize speclength;
		move (&ffsd->fsdbytes[2], (char *) &speclength,
		      sizeof speclength);
#ifdef LA_LONGLINES
		fprintf (dmp_stream, "   Special: %d, %D\n",
#else
		fprintf (dmp_stream, "   Special: %d, %d\n",
#endif
			ffsd->fsdbytes[1], speclength);
	    }
	    else Block {
		Reg1 int j;
		Reg2 int k;
		fprintf (dmp_stream, "   ");
		k = ffsd->fsdnlines;
		for (j = 0; j < k; j++) {
		    fprintf (dmp_stream, "%5d, ", ffsd->fsdbytes[j]);
		    if (ffsd->fsdbytes[j] < 0)
			k++;
		}
		putc ('\n', dmp_stream);
	    }
	}
	if (ffsd == lfsd)
	    break;
	ffsd = ffsd->fsdforw;
    }
    return;
}

la_sdump (plas, txt)
Reg1 La_stream *plas;
char *txt;
{
    fprintf (dmp_stream, "La_stream= 0x%x dump: %s", plas,txt);
    putc ('\n', dmp_stream);
    fprintf (dmp_stream, "  file= 0x%x, cfsd= 0x%x, fsline= %d, ",
	    plas->la_file, plas->la_cfsd, plas->la_fsline);
    fprintf (dmp_stream, "fsbyte= %d, lbyte= %d, ffpos= %d\n",
	    plas->la_fsbyte, plas->la_lbyte, plas->la_ffpos);
    fprintf (dmp_stream, "  lpos= %d, bpos= %d, back= 0x%x, forw= 0x%x, sflags= %d\n",
	    plas->la_lpos,
#ifdef LA_BP
	    plas->la_bpos,
#else
	    0,
#endif
	    plas->la_sback, plas->la_sforw, plas->la_sflags);
    return;
}

la_fdump (plaf, txt)
Reg1 La_file *plaf;
char *txt;
{
    fprintf (dmp_stream, "La_file= 0x%x dump: %s", plaf, txt);
    putc ('\n', dmp_stream);
    if ( plaf->la_ffs && plaf->la_ffs->f_file )
	fprintf (dmp_stream, "  la_ffs->f_file: %s\n", plaf->la_ffs->f_file->fl_path);
    else putc ('\n', dmp_stream);
    fprintf (dmp_stream, "  ffsd= 0x%04x, lfsd= 0x%04x, ffs= 0x%04x, ",
	    plaf->la_ffsd, plaf->la_lfsd, plaf->la_ffs);
    fprintf (dmp_stream, "fsrefs= %d, refs= %d\n",
	    plaf->la_fsrefs, plaf->la_refs);
    fprintf (dmp_stream, "  fstream= 0x%04x, lstream= 0x%04x\n",
	    plaf->la_fstream, plaf->la_lstream);
    fprintf (dmp_stream, "  mode=%d, maxline= %d, ", plaf->la_mode, plaf->la_maxline);
    fprintf (dmp_stream, "nlines= %d", plaf->la_nlines);
#ifdef LA_BP
    fprintf (dmp_stream, ", nbytes= %D\n", plaf->la_nbytes);
#else
    fprintf (dmp_stream, "\n");
#endif
    return;
}

la_schaindump (txt)
char *txt;
{
    Reg1 La_stream *tlas;
    Ff_file *ffile;

    fprintf (dmp_stream, "La_stream chain dump: %s\n", txt);
    for (tlas = la_firststream; tlas; tlas = tlas->la_sforw) {
	if ( tlas && tlas->la_file && tlas->la_file->la_ffs && tlas->la_file->la_ffs->f_file ) {
	    ffile = tlas->la_file->la_ffs->f_file;
	    fprintf (dmp_stream, "  La_stream 0x%x ->la_file 0x%x -> la_ffs 0x%x ->f_file 0x%x :\n",
		     tlas, tlas->la_file, tlas->la_file->la_ffs, ffile);
/*
	    fprintf (dmp_stream, "    fn_refs= %d, fn_fd= %d, (dev %d, ino %d) : \"%s\"\n",
		     ffile->fn_refs, ffile->fn_fd, ffile->fn_dev, ffile->fn_ino, ffile->fl_path);
*/
	    fprintf (dmp_stream, "    fn_refs= %d, fn_fd= %d, (dev %d, ino %d) : \n    \"%s\"\n",
		     ffile->fn_refs, ffile->fn_fd, (int) ffile->fn_dev, ffile->fn_ino, ffile->fl_path);
	} else fprintf (dmp_stream, "  La_stream 0x%x\n", tlas);
    }
    return;
}

la_fschaindump (plas, txt)
La_stream *plas;
char *txt;
{
    Reg1 La_stream *tlas;

    fprintf (dmp_stream, "La_stream file chain dump:        %r", &txt);
    putc ('\n', dmp_stream);
    fprintf (dmp_stream, "La_stream 0x%x\n", plas);
    for (tlas = plas->la_file->la_fstream; ; tlas = tlas->la_fforw) {
	fprintf (dmp_stream, "  La_stream 0x%x\n", tlas);
	if ( ! tlas ) break;
    }
    return;
}

la_sforfdump (La_file *plaf)
{
    La_stream *tlas;

    fprintf (dmp_stream, "La_stream for La_file 0x%x dump:\n", plaf);
    for (tlas = la_firststream; tlas; tlas = tlas->la_sforw) {
	if ( tlas->la_file == plaf )
	    fprintf (dmp_stream, "  La_stream 0x%x\n", tlas);
    }
    return;
}

