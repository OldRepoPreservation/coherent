/*	fnkey.c
 *	6/10/91
 *	Usage:  fnkey [ n [ string ] ]
 *	Sets/prints IBM AT console function keys.
 *	Revised for COHERENT 3.2
 */
#include <stdio.h>
#include <sgtty.h>
#include <ctype.h>
#include <sys/kb.h>
#include <errno.h>

#define	VERSION	"2.0"			/* version number */

FNKEY	*okeys;				/* old key bindings */
FNKEY	*nkeys;				/* new key bindings */

main(argc, argv)
int argc;
char **argv;
{
	unsigned c;
	register int i;
	register unsigned char *cp, *ncp;
	int n, fd;

	fd = open("/dev/console", 2);
	if (fd == -1)
		fatal("cannot open /dev/console");
	okeys = (FNKEY *) malloc(sizeof(FNKEY) + MAX_FCHAR);
	nkeys = (FNKEY *) malloc(sizeof(FNKEY) + MAX_FCHAR);
	if (okeys == (FNKEY *)0 || nkeys == (FNKEY *)0)
		fatal("out of memory");
	cp =  &okeys->k_fnval[0];
	ncp = &nkeys->k_fnval[0];

	/* Print version number if -V. */
	if (*++argv != NULL && strcmp(*argv, "-V") == 0) {
		--argc;
		++argv;
		fprintf(stderr, "fnkey:  V%s\n", VERSION);
	}
	if (argc > 3)
		usage();

	ioctl(fd, TIOCGETF, okeys);		/* get current key bindings */
	if (errno)
		fatal("couldn't read current function key settings");

	/* Print current values if no args. */
	if (*argv == NULL ) {
		for (i=0; i<okeys->k_nfkeys && cp<&okeys->k_fnval[MAX_FCHAR]; i++)  {
			if ((c = *cp) == DELIM) {
				cp++;
				continue;
			}
			printf ("F%d:  ", i);
			while ((c = *cp++)!=DELIM && cp<&okeys->k_fnval[MAX_FCHAR]) 
				printchar(c);
			putchar('\n');
		}
		exit(0);
	}

	/* First arg must be digit. */
	if (!isdigit(**argv))
		usage();
	if ((n = atoi(*argv++)) >= MAX_FKEYS)
		usage();

	/* Set Fn to given value. */
	for (i = 0; i < MAX_FKEYS; i++) {
		if (i == n) {
			if (*argv != NULL)
				while (c = *(*argv)++)
					if (ncp < &nkeys->k_fnval[MAX_FCHAR]-1)
						*ncp++ = c;
			while ((c = *cp++)!=DELIM && cp < &okeys->k_fnval[MAX_FCHAR])
				;
		} else {
			while ((c = *cp++)!=DELIM && cp < &okeys->k_fnval[MAX_FCHAR])
				if (ncp < &nkeys->k_fnval[MAX_FCHAR]-1)
					*ncp++ = c;
		}
		*ncp++ = DELIM;
		if (ncp >= &nkeys->k_fnval[MAX_FCHAR])
			break;
	}
	nkeys->k_nfkeys = i;
	ioctl(fd, TIOCSETF, nkeys);
	if (errno)
		fatal("couldn't set function keys");

	exit(0);
}

printchar(c)
register unsigned c;
{
	if (c == '\\')
		printf("\\\\");
	else if ((c >= ' ' && c <= '~') || c >= 0200)
		putchar(c);
	else switch (c) {
	case '\n':	printf("\\n");
			break;
	case '\t':	printf("\\t");
			break;
	case '\b':	printf("\\b");
			break;
	case '\r':	printf("\\r");
			break;
	default:	printf("\\%03o", c);
			break;
	}
}

usage()
{
	fprintf(stderr, "Usage:	fnkey [ n [ string ] ]\n");
	exit(1);
}

fatal(arg)
char	*arg;
{
	fprintf(stderr, "fnkey:\t%r\n", &arg);
	exit(1);
}
/* end of fnkey.c */
