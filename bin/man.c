/*
 * maninx.c
 * 5/2/90
 * Usage: man [ -w ] [ topic ... ]
 * Quick and dirty man hack.
 * Reads manual index, scats manual section.
 */

#include <stdio.h>

#define	BBBIGBUF	32000			/* manual index buffer */
#define	NBUF		80			/* other buffers */
#define	MANHELP	"/usr/man/man.help"
#define	MANINX	"/usr/man/man.index"

extern	char	*index();

/* Globals. */
char	buf[NBUF];
char	cmd[NBUF];
char	*indexp;
char	manindex[BBBIGBUF];
int	wflag;

/* Forward. */
void	fatal();
char	*lookup();
void	nonfatal();

main(argc, argv) int argc; char *argv[];
{
	int	i, fd, status;
	char	*lp;
	int	found;

	status = 0;
	if (argc > 1 && strcmp(argv[1], "-w") == 0) {
		++wflag;
		--argc;
		++argv;
	}	
	if (argc == 1) {
		/* No args: print manual help information. */
		if (wflag)
			printf("%s\n", MANHELP);
		else {
			sprintf(cmd, "scat %s", MANHELP);
			system(cmd);
		}
		exit(status);
	}

	/* Args given, look up each.  First read the index. */
	if ((fd = open(MANINX, 0)) == -1)
		fatal("cannot open manual index %s", MANINX);
	else if ((i = read(fd, manindex, BBBIGBUF)) == -1)
		fatal("cannot read index buffer");
	else if (i >= BBBIGBUF)
		fatal("index file too large");
#if	DEBUG
	nonfatal("%s=%d bytes", MANHELP, i);
#endif]

	/* Process each arg. */
	for (i = 1; i < argc; i++) {
#if	DEBUG
		nonfatal("argv[%d]=%s", i, argv[i]);
#endif
		indexp = manindex;
		found = 0;
		/* Look up arg in index.  May find multiple hits. */
		while ((lp = lookup(argv[i])) != NULL) {
			found++;
#if	DEBUG
			nonfatal("index line=%s", lp);
#endif
			if (wflag)
				printf("/usr/man/%s\n", lp);
			else {
				sprintf(cmd, "scat -s '/usr/man/%s'", lp);
#if	DEBUG
				nonfatal("command=%s", cmd);
#endif
				system(cmd);
			}
		}
		if (found == 0) {
			nonfatal("%s not found in manual", argv[i]);
			status = 1;
		}
	}
	exit(status);
}

/* Cry and die. */
void
fatal(s) char *s;
{
	fprintf(stderr, "man: %r\n", &s);
	exit(1);
}

/*
 * Look up string s in helpfile index starting at indexp.
 * Return name of file on match, else NULL.
 */
char *
lookup(s) char *s;
{
	register char *namep, *next;

	while ((next = index(indexp, '\n')) != NULL) {
		strncpy(buf, indexp, next - indexp);
		buf[next-indexp] = '\0';
#if	DEBUG
		nonfatal("[%s]", buf);
#endif
		namep = index(buf, ' ');
		indexp = next + 1;		/* bump to next index line */
		if (namep == NULL)
			continue;
		else
			*namep++ = '\0';
#if	DEBUG
		nonfatal("buf=%s namep=%s", buf, namep);
#endif
		if (strcmp(namep, s) == 0)
			return buf;		/* gotcha */
	}
	return NULL;				/* no match */
}

void
nonfatal(s) char *s;
{
	fprintf(stderr, "man: %r\n", &s);
}
	
/* end of maninx.c */
