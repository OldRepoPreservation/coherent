#include <stdio.h>
#include <ctype.h>
#include <sgtty.h>
#include <errno.h>
#include <signal.h>

/*
 *	HP --
 *	filter for the hewlet packhard printer.
 *
 *		This program translates nroff font specifications into the 
 *	correct escape sequences for the Hewlet Packhard Laser printer.
 *	It also allows the user to set indentation, page length, landscape
 *	mode, et cetera. Because the HP printer stacks pages in the reverse
 *	order as they come out of the printer, this filter puts them out in the
 *	reverse order, unless the -f (forward) flag is set.
 *
 *	History:
 *		3/21/85	(jtk) -- Changed so that pages were printed in the
 *			order, and fixed bug where trailing lines with only
 *			non-blank character were deleted.
 *		4/09/85 (ella) -- Changed so that pages were printed in
 *			the landscape mode with the pitch 16.66, and 
 *			fixed bug where underlining in the landscape mode
 *			and/or !cartin wasn't ever terminated.
 *			Incorporated rma's fix (-a option) to reflect the
 *			manual page and properly process accentgrav:
 *			default: straight (apostrophe), with -a: slanted
 *			(single quote).
 *			Included all available options into Usage message.
 *
 */

#define ROMAN_F		0
#define BOLD_F		1
#define ITALIC_F	2
#define LINESZ		512
#define	MAXPAGE		1000
#define PRIMARY		"\033(s0T"
#define PITCH		"\033(s16.66H"
#define LANDSCAPE	"\033&l%dO"
#define INDENT		"\033&a%dL"
#define UNDEROFF	"\033&d@"
#define	UNDERON		"\033&dD"
#define STROKEWEIGHT	"\033(s%dB"
#define ADVANCE		"\033&a+%dC"
#define RESET		"\033E"
#define SETLINES	"\033&l%dP"
#define TEXTLENGTH	"\033&l%dF"
#define	TOPMARGIN	"\033&l%dE"
#define BACKSPACE	'\010'
#define FORMFEED	'\014'
#define ACCENTACUT	0xA9
#define ILINES		"\033&l%dD"
#define ITALIC		"\033(s1s-1B"
#define BIGBOLD		"\033(s0s1B"
#define REGULAR		"\033(s0s0B"

FILE *pfp;			/* The printer device */
char pdev[64] = "/dev/hp";	/* Default printer device */
char *umess = "Usage: hp [-a] [-c] [-f] [-l] [-imarg] [-ttop] [-plines] file ...\n";
char *argv0;
char accentgrav = '\'';
int  quit = 0;			/* Signal to interrupt program. */
int curfont = ROMAN_F;
int ilines = 6;			/* Lines per inch */
int pitch = 0;			/* primary pitch 10 */
int nlines = 66;		/* Lines per page */
int indent = 0;			/* Left margin */
int land = 0;			/* 0 for portrait mode, 1 for landscape */
int cartin = 1;			/* 1 if font cartridge available */
int nreverse = 0;		/* No reverse, pages printed in order. */
int tlength = 66;		/* Text length -- must satisfy */
				/* tlength <= nlines - topmarg */
int topmarg = 0;		/* Top margin */
int nfiles = 0;			/* Number of files to print */
char page[66][LINESZ];		/* Current page buffer */
long pageaddr[MAXPAGE];
int pgcount = 0;		/* Number of pages marked on input */
FILE *output = NULL;		/* File containing pages in unreversed order */
char tempfile[20];		/* Name of temporary file. */

main(argc, argv)
char **argv;
{
	register char **files;
	FILE *setup();

	argv0 = argv[0];
	argv++; argc--;
	*files = NULL;
	while (argc > 0)  {
		switch(**argv)  {
			case '-':
				switch(*++*argv)  {
					case 'f':
						nreverse = 1;
						argv++;
						argc--;
						break;
					case 'l':
						land = 1;
						pitch = 1;
						ilines = 8;

						argv++;
						argc--;
						break;
					case 'i':
						indent = atoi(++*argv);
						argv++;
						argc--;
						break;
					case 'p':
						tlength = atoi(++*argv);
						argv++;
						argc--;
						break;
					case 't':
						topmarg = atoi(++*argv);
						argv++;
						argc--;
						break;
					case 'c':
						cartin = !cartin;
						pitch = 0;
						argv++;
						argc--;
						break;
					case 'a':
						accentgrav = 0xA8;
						argv++;
						argc--;
						break;
					default:
						usage();
				}
				break;
			default:
				if (nfiles == 0) {
					output = setup(output);
					if (nreverse)
						init();
				}
				printfile(*argv);
				nfiles++;
				argv++;
				argc--;
		}
	}
	if (nfiles == 0) {
		output = setup(output);
		if (nreverse)
			init();
		printfile(NULL);
	}
	if (!nreverse)
		readbkwd(output);
	printf(RESET);
	wrapup(0);
}

/*
 * Open temporary file unless nreverse is specified, in which case
 * output is set to stdout.
 * Set interrupt trap.
 *
 */

FILE *
setup(fp)
FILE	*fp;
{
	int   trap();

	signal(SIGINT, trap);
	if (fp == NULL)
		if (nreverse)
			return(stdout);
		else {
			sprintf(tempfile, "/tmp/hptmp.%d", getpid());
			if ((fp = fopen(tempfile, "wr")) == NULL)
				fatal("Cannot open temporary file %s",
					 tempfile);
			return(fp);
		}
}

/*
 * Set quit flag when interrupt signal is sent.
 *
 */

trap()
{
	quit = 1;
}

/*
 * Send character sequences to printer to initialize it for another file.
 * Check for interrupt.
 *
 */

init()
{
	if (quit)
		wrapup(0);
	printf(RESET);
	if (pitch == 1)
		printf(PITCH, pitch);
	printf(LANDSCAPE, land);
	printf(SETLINES, nlines);
	printf(ILINES, ilines);
	printf(TOPMARGIN, topmarg);
	printf(TEXTLENGTH, tlength);
	printf(INDENT, indent);
	if (quit) {
		wrapup(0);
	}
}

printfile(file)
char *file;
{
	FILE *fp;
	register int i;
	int lnbl;		/* Last non-blank line */
	char *skipws();
	int end = 0;
	int c;

	if (file == NULL)
		if (nfiles == 0)
			fp = stdin;
		else
			return;
	else if ((fp = fopen(file, "r")) == NULL)
		fatal("cannot open %s\n", file);

	while (end != 1) {
		markpage(output);
		for (i=0; i<nlines; i++)  
			if (fgets(&page[i][0], LINESZ, fp) == NULL) {
				end = 1;
				/* If last page is blank, decrement pgcount */
				if (i == 0) {
					pgcount--;
					return;
				}
				break;
			}
		for (lnbl = --i; lnbl > 0; --i)
			if ((c = *skipws(page[i])) == '\0' || c == '\n')
				lnbl--;
			else
				break;
		for (i = 0; i <= lnbl; i++)
			printline(page[i]);
		fprintf(output, "%c", '\r');
		fprintf(output, "%c", FORMFEED);
		if (end)
			break;
	}
}

/*
 * Mark the top of pages in output.
 * Read current disk address, save it in pageaddr, and increment pgcount.
 * There is only space in array to save MAXPAGE number of addresses.
 *
 */

markpage(fp)
FILE	*fp;
{
	if (pgcount >= MAXPAGE)
		fatal("More than %d pages read in.\n", MAXPAGE);
	pageaddr[pgcount++] = ftell(fp);
}

/*
 * Skip white space, ie spaces and tabs.
 *
 */

char *
skipws(s)
char *s;
{
	register int c;

	while ((c = *s) == ' ' || c == '\t')
		s++;	/* Increment s if c is white space. */
	return (s);
}

/*
 * Print one line of input file, substituting special characters with the hp
 * character set.
 *
 */

printline(cp)
char *cp;
{
	register int c1, c2;
	char *cp_x;

	cp_x = cp;

	/*
	 *	pass over line replacing
	 *	all accent marks found with there
	 *	equivalents from the extended char-set
	 */

	while(*cp_x != '\0')
	{
		if(*cp_x == '\'')
			*cp_x = accentgrav;
		else if (*cp_x == '`')
			*cp_x = ACCENTACUT;
		cp_x++;
	}
	while((c1 = *cp++) != '\0') {
		if (c1 == '_')
			if ((c2 = *cp++) == BACKSPACE) {
				font(ITALIC_F);
				putc(*cp++, output);
				continue;
			} else {
				putc(c1, output);
				cp--;
			}
		else if ((c2 = *cp++) == BACKSPACE) {
			if ((c2 = *cp++) == c1) {
				/* Change an overstrike to a true bold face
				 * character. */
				if (!cartin || land)
					boldprint(c2);
				else {
					font(BOLD_F);
					putc(c2, output);
				}
				continue;
			} else {
				putc(BACKSPACE, output);
				cp--;
			}
		} else {
			font(ROMAN_F);
			putc(c1, output);
			cp--;
		}
		/* Check for the interrupt signal after each character. */
		if (quit)
			wrapup(0);
	}
}

/* 
 * Change the current font to the specified type.
 *
 */

font(type)
int type;
{
	if (curfont != type)
		switch(type)  {
			case ITALIC_F:
				if (cartin && !land)
				{
					fprintf(output, ITALIC);
					curfont = ITALIC_F;
				} else {
					fprintf(output, STROKEWEIGHT, 0);
					fprintf(output, UNDERON);
					curfont = ITALIC_F;
				}
				break;
			case BOLD_F:
				fprintf(output, UNDEROFF);
				fprintf(output, STROKEWEIGHT, 4);

				if (cartin)
					fprintf(output, BIGBOLD);

				curfont = BOLD_F;
				break;
			case ROMAN_F:
				if (cartin) {
					fprintf(output, REGULAR);
					if (land) 
						fprintf(output, UNDEROFF);
				}
				else {
					fprintf(output, STROKEWEIGHT, 0);
					fprintf(output, UNDEROFF);
				}
				curfont = ROMAN_F;
		}
}

/*
 * Print given character in boldface.
 *
 */

boldprint(c)
int c;
{
	putc(c, output);
	fprintf(output, "\033&a-1C");
	if (land)
	{
		fprintf(output, "\033&a+7H");	/* In landscape mode */
		putc(c, output);
		fprintf(output, "\033&a-7H");
	} else {
		fprintf(output, "\033&a+8H");	/* In portrait mode */
		putc(c, output);
		fprintf(output, "\033&a-8H");
	}
}

fatal(s)
char *s;
{
	fprintf(stderr, "%s: ", argv0);
	fprintf(stderr, "%r", &s);
	wrapup(1);
}

usage()
{
	fprintf(stderr, umess);
	exit(1);
}

debug (str)
char *str;
{
	FILE *fp;

	if ((fp = fopen("hp.db", "ar")) == NULL)
		return;
	fprintf(fp, "hp: ");
	fprintf(fp, "%r", &str);
	fclose (fp);
	return;
}

/*
 * Read the temporary file to stdout with the pages in reverse order.
 * Starting with the last address in pageaddr, seek to that address, read a
 * page, and then do the same with the previous address.
 *
 */

readbkwd(input)
FILE	*input;
{
	char c;

	if (quit)
		wrapup(0);
	init();
	while (--pgcount >= 0) {
		fseek(input, pageaddr[pgcount], 0);
		while ((c = getc(input)) != '\f')
			/* after each character, check for interrupt signal. */
			if (quit)
				wrapup(0);
			else
				putchar(c);
		putchar(c);
	}
}

/*
 * Get rid of temporary file if it has been opened, and exit.
 *
 */

wrapup(status)
int	status;
{
	fclose(output);
	if ((output != stdout) && (output != NULL))
		unlink(tempfile);
	exit(status);
}

