/*
 * tee.c
 * 4/30/91
 * Pipe redirection.
 * Usage: tee [ -a ] [ -i ] [ file ] ...
 * Rec'd from Lauren Weinstein, 7-16-84.
 * Hacked by rec to use stdio so that everything doesn't come out buffered.
 * Slightly rewritten by steve 4/30/91 for clarity; added "isatty" check.
 */

#include	<stdio.h>
#include	<errno.h>
#include	<signal.h>

#define	NUFILE	_NFILE			/* max # open files */
#define	USAGE	"Usage: tee [ -a ] [ -i ] [ file ] ...\n"

/* External. */
extern	int	exit();

/* Forward. */
FILE	*openf();
void	fatal();

main(argc, argv) int argc; register char *argv[];
{
	register int c;
	register FILE **fpp;
	FILE *fp[NUFILE];
	int aflag;

	if (signal(SIGINT, SIG_IGN) != SIG_IGN)
		signal(SIGINT, exit);

	/* Process option arguments. */
	aflag = 0;
	while (*++argv && argv[0][0]=='-') {
		switch (argv[0][1]) {
		case 'a':
			++aflag;
			break;
		case 'i':
			signal(SIGINT, SIG_IGN);
			break;
		default:
			fprintf(stderr, USAGE);
			exit(1);
		}
	}

	/* Open file arguments. */
	for (fpp = fp; *argv; ) {
		if (fpp >= &fp[NUFILE])
			fatal("too many files");
		*fpp++ = openf(*argv++, aflag);
	}
	*fpp = NULL;

	/* Make i/o unbuffered if interactive. */
	if (isatty(fileno(stdin)))
		setbuf(stdout, NULL);
	if (isatty(fileno(stdout)))
		setbuf(stdin, NULL);

	/* Copy stdin to stdout, duplicate to each specified file. */
	while ((c = getchar()) != EOF) {
		putchar(c);
		for (fpp = fp; *fpp != NULL; fpp++)
			putc(c, *fpp);
	}

	/* Done. */
	exit(0);
}

/*
 * Cry and die.
 */
/* VARARGS */
void
fatal(s) char *s;
{	fprintf(stderr, "tee: %r\n", &s);
	exit(1);
}

/*
 * Open a file.
 */
FILE *
openf(file, aflag) char *file; int aflag;
{
	register FILE *fp;

	if ((fp = fopen(file, (aflag) ? "a" : "w")) != NULL) {
		if (aflag)
			fseek(fp, 0L, SEEK_END);
		return fp;
	}
	switch (errno) {
	case EMFILE:
	case ENFILE:
		fatal("too many files");
		break;
	default:
		fatal("cannot create %s", file);
	}
}

/* end of tee.c */
