/*
 * Strip symbols, lines, and relocation from executable.
 */
#include <misc.h>
#include <errno.h>
#include <coff.h>
#include <setjmp.h>
#include <sys/stat.h>

static jmp_buf env;	/* setjmp longjmp buffer */
static char *filen;	/* current file in process */
static int errCt;

/*
 * Put message and longjmp to next file.
 */
fatal(s)
char *s;
{
	fprintf(stderr, "strip: %s %r.\n", filen, &s);
	errCt++;
	longjmp(env, 1);
}

/*
 * Strip a file
 */
strip()
{
	register SCNHDR *sh;
	static FILEHDR *fh = NULL;	
	static FILE *fp = NULL;
	long i, top, hi;
	struct stat sb;

	if (stat(filen, &sb))
		fatal("Can't locate");

	/* inhale input file */
	if (NULL != fp)
		fclose(fp);
	fp = xopen(filen, "rb");

	if (NULL != fh)
		free(fh);
	fh = alloc(sb.st_size);

	if (1 != fread(fh, sb.st_size, 1, fp))
		fatal("Error in read");

	fclose(fp);
	fp = NULL;

	if ((fh->f_magic != C_386_MAGIC) ||
	    !fh->f_opthdr ||
	    !(fh->f_flags & F_EXEC))
		fatal("Not COFF executable");

	fh->f_symptr = fh->f_nsyms = 0;
	fh->f_flags |= F_RELFLG | F_LNNO | F_LSYMS;

	/* pass segments and find top address */
	sh = ((char *)fh) + fh->f_opthdr + sizeof(*fh);
	top = (long)(sh + fh->f_nscns);
	for (top = i = 0; i < fh->f_nscns; i++, sh++) {
		/* find top of sector data */
		if (sh->s_scnptr && (sh->s_flags != STYP_BSS)) {
			hi = sh->s_size + sh->s_scnptr;
			if (top < hi)
				top = hi;
		}
		sh->s_relptr = sh->s_lnnoptr = sh->s_nreloc = sh->s_nlnno = 0;
	}

	if (top > sb.st_size)
		fatal("Corrupt file");

	if (top < sb.st_size) {
		/* exhale stripped file */
		fp = xopen(filen, "wb");
		if (1 != fwrite(fh, top, 1, fp))
			fatal("Error in write");
	}
}

main(argc, argv)
char *argv[];
{
	register int i;

	for (i = 1; i < argc; i++) {
		filen = argv[i];
		if (!setjmp(env))
			strip();
	}

	if(!errCt)
		return (0);

	fprintf(stderr, "%d error(s) flagged\n", errCt);
	return (1);
}
