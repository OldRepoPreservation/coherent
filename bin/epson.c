/*
 *	epson.c
 *	11/4/88
 *	Epson MX-80 printer driver
 *	Usage:  epson [ - cdfrw8 ] [ -b banner ] [ -in ] [ -o ofile ] [ -sn ] [ file ... ]
 */

#include <stdio.h>

/* version number */
#define	VERSION	"1.7"

/* Epson special characters */
#define	COMPON	'\017'
#define COMPOFF	'\222'
#define FORMFD	'\014'
#define	SELECT	'\021'
#define	WIDEON	'\016'
#define	WIDEOFF	'\224'

/* default output device,  /dev/lp for COHERENT or prn: for MSDOS */
#ifdef COHERENT
#define	OUTFILE	"/dev/lp"
#else
#define	OUTFILE	"prn:"
#endif

FILE *ifp, *ofp;
int indentn = 0;		/* indent */
int wide = 0;			/* double width */

main (argc, argv)
int argc;
char **argv;
{
	char *version = VERSION;
	int this, next;
	int i;

	/* default option settings */
	char *banner = NULL;		/* banner */
	int compressed = 0;		/* compressed */
	int dsbold = 0;			/* double struck (default: emphasized) */
	int formfeed = 1;		/* formfeed */
	char *ofile = OUTFILE;		/* output file */
	int roman = 0;			/* Roman (default: italic) */
	int spaces = 1;			/* vertical spaces */
	int vspace8 = 0;		/* eight lines per inch */

	/* process option flags */
	while (--argc > 0 && **++argv == '-') {
		switch (*++*argv) {
			case 'b':	if (--argc == 0) usage ();
					banner = *++argv;
					break;
			case 'c':	compressed = 1;
					break;
			case 'd':	dsbold = 1;
					break;
			case 'f':	formfeed = 0;
					break;
			case 'i':	indentn = atoi (++*argv);
					if (indentn < 0)
						fatal ("bad -i arg", *argv);
					break;
			case 'o':	if (--argc == 0) usage ();
					ofile = *++argv;
					break;
			case 'r':	roman = 1;
					break;
			case 's':	spaces = atoi (++*argv);
					if (spaces <1 || spaces > 3)
						fatal ("bad -s arg", *argv);
					break;
			case 'w':	wide = 1;
					break;
			case 'V':	fprintf (stderr, "epson:  %s\n", version);
					break;
			case '8':	vspace8 = 1;
					break;
			default:	usage();
					break;
		}
	}

	/* initialize Epson */
	if ((ofp = fopen (ofile, "w")) == NULL)
		fatal ("cannot open", ofile);
	fputs ("\033@", ofp);

	/* print banner if given */
	if (banner != NULL) {
		i = (wide ? indentn * 2 : compressed ? indentn / 2 : indentn);
		while (i-- > 0) fputc (' ', ofp);
		fputc (WIDEON, ofp);
		fputs (banner, ofp);
		fputc (WIDEOFF, ofp);
		fputc ('\n', ofp);
	}
	if (compressed) fputc (COMPON, ofp);
	if (vspace8) fputs ("\0330\033CX", ofp);
	doindent ();

	/* process input files */
	while (argc >= 0) {

		/* set ifp to next input file */
		if (argc-- == 0) ifp = stdin;
		else {
			if (argc == 0) --argc;
			if ((ifp = fopen (*argv, "r")) == NULL)
				fatal ("cannot open", *argv);
			argv++;
		}

		/* process a file */
		while ((this = fgetc(ifp)) != EOF) {
			if (this == '\n') {
				for (i=spaces; i-- > 0; ) fputc (this, ofp);
				doindent ();
			}
			else if (this == FORMFD) {
				fputc (this, ofp);
				doindent ();
			}
			else if ((next = fgetc (ifp)) != '\b') {
				fputc (this, ofp);
				ungetc (next, ifp);
			}
			else {	/* next char is backspace */
				if (this == '_' && !roman) {
					fputc (fgetc (ifp) + 0200, ofp);
				}
				else if (!dsbold) {
					if ((next = fgetc (ifp)) == this) {
						fputs ("\033E", ofp);
						fputc (this, ofp);
						fputs ("\033F", ofp);
					}
					else {
						fputc (this, ofp);
						fputc ('\b', ofp);
						ungetc (next, ifp);
					}
				}
				else {
					fputc (this, ofp);
					ungetc (next, ifp);
				}
			}
		}
		if (ifp != stdin) fclose(ifp);
		if (formfeed) {
			fputc (FORMFD, ofp);
			doindent ();
		}
	}

	/* cleanup */
	if (compressed) fputc (COMPOFF, ofp);
	if (vspace8) fputs ("\0332\033CB", ofp);
	if (wide) fputc (WIDEOFF, ofp);
	fputs ("\033@", ofp);
	if (fclose (ofp) == EOF)
		fatal ("cannot close", ofile);
	exit (0);
}

/* perform indentation */
doindent ()
{
	int i;
	if (wide) fputc (WIDEON, ofp);
	for (i=indentn; i-- > 0; ) fputc (' ', ofp);
}

/* print usage message and exit */
usage ()
{
	fprintf (stderr, "Usage:  epson [ -cdfrw8 ] [ -b banner] [ -in ] [ -o ofile ] [ -sn ] [ file ... ]\n");
	exit (1);
}

/* print fatal error message and exit */
fatal (s1, s2)
char *s1, *s2;
{
	fprintf (stderr, "epson:  %s %s\n", s1, s2);
	exit (1);
}
