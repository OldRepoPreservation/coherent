/*
 * kbdinstall.c
 * 6/4/91
 * Keyboard support installation.
 *
 * cc kbdinstall.c -lcurses -lterm
 * Usage: kbdinstall [ -b ]
 * Option -b means kbdinstall was invoked from build.
 *
 * Steve's first curses program, screen handling might be more efficient
 * if I had time to figure out how curses works.
 */

#include <stdio.h>
#include <string.h>
#include <curses.h>

extern	char	*fgets();
extern	char	*malloc();

#define	VERSION	"1.2"			/* Version number.	*/
#define	KBDDIR	"/conf/kbd"		/* Keyboard directory.	*/
#define	KBDLIST	"/conf/kbd/.list"	/* List of keyboards.	*/
#define	KBDY	8			/* y-coordinate of keyboard list. */
#define	NLINE	512			/* Line length.		*/
#define	NKBDS	10			/* Number of keyboards.	*/
					/* N.B. must be <=10 for now! */

/* Keyboard list file entries. */
typedef	struct	kline	{
	char	*k_file;		/* File name.		*/
	char	*k_desc;		/* Description.		*/
}	KLINE;

/* Forward. */
char	*copystr();
void	display_line();
void	display_rev();
void	fatal();
void	nonfatal();
void	read_klist();

/* Globals. */
int	bflag;				/* Invoked from build.	*/
char	buf[NLINE];			/* Input buffer.	*/
KLINE	klist[NKBDS];			/* Keyboard list.	*/
int	nkbds;				/* Number of keyboards.	*/
int	initflag;			/* Curses initialized.	*/

main(argc, argv) int argc; char *argv[];
{
	register int n, c, status;

	/* Process command line options. */
	if (argc > 1 && strcmp(argv[1], "-b") == 0) {
		++bflag;
		--argc;
		++argv;
	}
	if (argc > 1 && strcmp(argv[1], "-V") == 0)
		nonfatal("V%s", VERSION);

	/* Read the keyboard list file. */
	read_klist();

	/* Initialize screen and terminal modes. */
	initscr();
	initflag = 1;
	noecho();
	raw();
	clear();

	/* Display instructions at top of screen. */
	mvaddstr(0, 0, "Select the entry below which indentifies your keyboard type.");
	mvaddstr(1, 0, "Hit <Enter> to select the highlighted entry.");
	mvaddstr(2, 0, "Hit <space> to move down to the next entry.");
	mvaddstr(3, 0, "Hit the desired number on the numeric keypad");
	mvaddstr(4, 0, "if the NumLock light is on.");
	mvaddstr(5, 0, "Do not use the arrow keys or number keys.");

	/* Display choices. */
	for (n = 0; n < nkbds; n++)
		display_line(n);

	/* Interactive input loop: display choice n highlighted. */
	for (n = 0; ; ) {
		display_rev(n);		/* display default choice in reverse */
		refresh();
		switch(c = getch()) {
		case ' ':		/* space: try next choice */
			if (++n == nkbds)
				n = 0;
			continue;
		case '\n':
		case '\r':		/* enter: take default value */
			break;
		default:
			if (c < '0' || c > '9')
				continue;	/* nondigit: ignore */
			if (c - '0' >= nkbds)
				continue;	/* out of range: ignore */
			/* Gotcha. Must look for more digits if NKBDS > 10. */
			n = c - '0';		/* use given value */
			break;
		}
		break;			/* done, choice is in n */
	}
	endwin();

	/* Execute the keyboard support file. */
	sprintf(buf, "%s/%s", KBDDIR, klist[n].k_file);
	if ((status = system(buf)) != 0)
		nonfatal("command \"%s\" failed", buf);

	/* Echo appropriate line to /tmp/drvld.all if invoked from build. */
	if (status == 0 && bflag) {
		sprintf(buf,
			"/bin/echo %s/%s >>/tmp/drvld.all",
			KBDDIR, klist[n].k_file);
		if (system(buf) != 0)
			nonfatal("command \"%s\" failed", buf);
	}

	/* Done. */
	exit(status);
}

/*
 * Allocate a copy of a string and return a pointer to it.
 */
char *
copystr(s) register char *s;
{
	register char *cp;

	if ((cp = malloc(strlen(s) + 1)) == NULL)
		fatal("no space for string \"%s\"");
	strcpy(cp, s);
	return cp;
}

/*
 * Display choice n.
 */
void
display_line(n) register int n;
{
	char val[6];

	sprintf(val, "%d", n);
	mvaddstr(KBDY+n, 8, val);
	mvaddstr(KBDY+n, 12, klist[n].k_desc);
}

/*
 * Display choice n in reverse video.
 */
void
display_rev(n) int n;
{
	static int last = -1;

	if (last != -1)
		display_line(last);	/* redisplay last in normal */
	standout();
	display_line(n);		/* display n in reverse */
	standend();
	last = n;
}

/*
 * Cry and die.
 */
/* VARARGS */
void
fatal(s) char *s;
{
	if (initflag)
		endwin();
	fprintf(stderr, "kbdinstall: %r\n", &s);
	exit(1);
}

/*
 * Issue nonfatal informative message.
 */
/* VARARGS */
void
nonfatal(s) char *s;
{
	fprintf(stderr, "kbdinstall: %r\n", &s);
}

/*
 * Read a file containing keyboard file names and descriptions,
 * build a keyboard list.
 */
void
read_klist()
{
	register FILE *fp;
	register char *s, *s1;

	if ((fp = fopen(KBDLIST, "r")) == NULL)
		fatal("cannot open \"%s\"", KBDLIST);
	while (fgets(buf, sizeof buf, fp) != NULL) {
		if (nkbds == NKBDS) {
			nonfatal("more than %d keyboard entries", NKBDS-1);
			continue;
		}

		/* Look for '\t' separating filename and description. */
		if ((s = strchr(buf, '\t')) == NULL) {
			nonfatal("no tab in \"%s\"", buf);
			continue;
		}
		*s++ = '\0';
		klist[nkbds].k_file = copystr(buf);

		/* Look for '\n' ending description. */
		if ((s1 = strchr(s, '\n')) == NULL) {
			nonfatal("no newline in \"%s\"", s);
			continue;
		}
		*s1 = '\0';
		klist[nkbds].k_desc = copystr(s);

		nkbds++;
	}
	fclose(fp);
}

/* end of kbdinstall.c */
