/*
 * Strip symbols, lines, and relocation from executable.
 */
#include <misc.h>
#include <errno.h>
#include <coff.h>
#include <setjmp.h>
#include <sys/stat.h>

static FILE *ifp = NULL, *ofp = NULL;
static jmp_buf env;
static char *tfile = NULL, *filen;
static struct stat sb;

#define xread(x, m) if(1 != fread(&x, sizeof(x), 1, ifp)) fatal(rmsg, m);
#define xwrite(x, m) if(1 != fwrite(&x, sizeof(x), 1, ofp)) fatal(wmsg, m);
static char rmsg[] = "Error reading %s";
static char wmsg[] = "Error writing %s";

#define SLASH '/'
extern char *strrchr();

/*
 * Put message and longjmp to next file.
 */
fatal(s)
char *s;
{
	int save = errno;

	fprintf(stderr, "strip: %s %r\n", filen, &s);
	if (0 != (errno = save))
		perror("errno reports");
	longjmp(env, 1);
}

/*
 * Copy ifp to ofp
 */
fcopy(len)
long len;
{
	char buf[BUFSIZ];
	int i;

	/* align on BUFSIZ boundary then copy buffers */
	for (i = ftell(ifp) % BUFSIZ; len; (len -= i), (i = 0)) {
		if ((i = BUFSIZ - i) > len)
			i = len;

		if (1 != fread(buf, i, 1, ifp))
			fatal(rmsg, "text");
		if (1 != fwrite(buf, i, 1, ofp))
			fatal(wmsg, "text");
	}
}

/*
 * Strip a file
 */
strip()
{
	FILEHDR fh;
	long i, top, hi;

	xread(fh, "file header");
	if (fh.f_magic != C_386_MAGIC)
		fatal("Wrong magic number %x", fh.f_magic);
	if (!fh.f_opthdr || !(fh.f_flags & F_EXEC))
		fatal("Not executable");

	fh.f_symptr = fh.f_nsyms = 0;
	fh.f_flags |= F_RELFLG | F_LNNO | F_LSYMS;
	xwrite(fh, "file header");
	fcopy((long)fh.f_opthdr); /* copy to section headers */

	for (top = i = 0; i < fh.f_nscns; i++) {
		SCNHDR sh;

		xread(sh, "sector");

		/* find top of sector data */
		if (sh.s_scnptr && (sh.s_flags != STYP_BSS)) {
			hi = sh.s_size + sh.s_scnptr;
			if (top < hi)
				top = hi;
		}

		sh.s_relptr = sh.s_lnnoptr = sh.s_nreloc = sh.s_nlnno = 0;
		xwrite(sh, "sector");
	}

	fcopy(top - ftell(ifp));
}

main(argc, argv)
char *argv[];
{
	int i;
	char *p, *cmd;
	extern char *tempnam();

	for (i = 1; i < argc; i++) {
		/* fatal errors longjmp to here for next file */
		if (setjmp(env)) {
			if (NULL != ifp) {
				fclose(ifp);
				ifp = NULL;
			}
			if (NULL != ofp) {
				fclose(ofp);
				ofp = NULL;
			}
			if (NULL != tfile) {
				unlink(tfile);
				free(tfile);
				tfile = NULL;
			}
			continue;
		}

		if (stat(filen = argv[i], &sb))
			fatal("Can't find %s", filen);

		/* open input file */
		ifp = xopen(filen, "rb");

		if (NULL != (p = strrchr(filen, SLASH))) {
			*p = '\0';
			tfile = tempnam(filen, NULL);
			*p = SLASH;
		}
		else
			tfile = tempnam(".", NULL);

		ofp = xopen(tfile, "wb");

		strip();

		if (NULL != ifp) {
			fclose(ifp);
			ifp = NULL;
		}
		if (NULL != ofp) {
			fclose(ofp);
			ofp = NULL;
		}

		cmd = alloc(strlen(filen) + strlen(tfile) + 6);
		sprintf(cmd, "mv %s %s", tfile, filen);
		system(cmd);
		chmod(filen, sb.st_mode);
		free(cmd);
		free(tfile);
		tfile = NULL;
	}
	exit(0);
}
