/*
 * qfind.c
 * 3/21/91
 * Find files with given name in filesystem using file database.
 * Usage: qfind [ -adp ] name ...
 * 	  qfind -b[v]
 * Options:
 *	-a	All: search for files or directories.
 *	-b	Build file database.
 *	-d	Search for directories only.
 *	-p	Partial name matching.
 *	-v	Verbose information.
 * Run as root when using -b to find everything.
 * Uses find, sed, sort.
 * Does not ignore SIG_INT, so "qfind -b&" aborts if <Ctrl-C> typed.
 */

#include <stdio.h>
#include <string.h>

extern	char	*mktemp();

#define	VERSION	"1.6"
#define	USAGE	"Usage:\tqfind [ -adp ] name ...\n\tqfind -b[v]\n"
#define	MINSEEK	512			/* binary search threshold */
#define	NBUF	512			/* buffer size		*/
#define	NCHARS	128			/* first characters	*/
#define	QFFILES	"/usr/adm/qffiles"	/* database filename	*/
#define	QFNEW	"/usr/adm/qffiles.new"	/* new database filename */
#define	QFTMP	"/tmp/qfXXXXXX"		/* tmpname prototype	*/

/* Forward. */
int	build();
void	fatal();
void	fpseek();
int	qfind();
int	qseek();
void	sys();
void	usage();

/* Globals. */
int	aflag;				/* look for all		*/
int	bflag;				/* build QFFILES	*/
char	buf[NBUF];			/* command buffer	*/
int	dflag;				/* look for directories	*/
FILE	*ifp;				/* input FILE		*/
int	pflag;				/* partial match	*/
long	seektab[NCHARS];		/* seek table		*/
char	*tmpname;			/* temporary filename	*/
int	vflag;				/* verbose information	*/

main(argc, argv) int argc; char *argv[];
{
	register char *s;
	register int status;

	/* Process options. */
	while (argc > 1 && argv[1][0] == '-') {
		for (s = &argv[1][1]; *s; s++) {
			switch(*s) {
			case 'a':	++aflag;	break;
			case 'b':	++bflag;	break;
			case 'd':	++dflag;	break;
			case 'p':	++pflag;	break;
			case 'v':	++vflag;	break;
			case 'V':
				fprintf(stderr, "qfind: V%s\n", VERSION);
				break;
			default:	usage();
			}
		}
		--argc;
		++argv;
	}
	if ((bflag && argc != 1) || (!bflag && argc == 1))
		usage();

	/* Build new database. */
	if (bflag)
		exit(build());

	/* Find given names in existing database. */
	if ((ifp = fopen(QFFILES, "r")) == NULL)
		fatal("cannot open \"%s\"", QFFILES);
	else if (fread(seektab, sizeof(seektab), 1, ifp) != 1)
		fatal("seek buffer read error");
	for (status = 0; *++argv != NULL; )
		status |= qfind(*argv);
	if (fclose(ifp) == EOF)
		fatal("cannot close \"%s\"", QFFILES);
	exit(status);
}

/*
 * Build the file and directory database.
 * The database consists of a seek pointer table
 * followed by a sorted list of files and directories.
 * seektab[c] gives the seek to the first line in the file starting with c.
 * The sorted list contains "file /dir1/dir2" for each file /dir1/dir2/file
 * and "dir3/ /dir1/dir2" for each directory /dir1/dir2/dir3.
 */
int
build()
{
	register FILE *fp;
	register int nfiles;
	int last;
	long lastseek;

	if ((tmpname = mktemp(QFTMP)) == NULL)
		fatal("cannot make temporary file name");

	/* Generate "file /dir1/dir2" for each file /dir1/dir2/file. */
	sys("find / ! -type d | sed -e 's/\\(.*\\)\\/\\(.*\\)/\\2 \\1/' >%s",
		tmpname);

	/* Append "dir3/ /dir1/dir2" for each directory /dir1/dir2/dir3. */
	sys("find / -type d | sed -e 's/\\(.*\\)\\/\\(.*\\)/\\2\\/ \\1/' >>%s",
		tmpname);

	/* Create data file containing an empty seek table. */
	if ((fp = fopen(QFNEW, "w")) == NULL)
		fatal("cannot open \"%s\"", QFNEW);
	else if (fwrite(seektab, sizeof(seektab), 1, fp) != 1)
		fatal("write error on \"%s\"", QFNEW);
	else if (fclose(fp) == EOF)
		fatal("cannot close \"%s\"", QFNEW);

	/* Sort tempfile and append to new data file. */
	sys("sort %s >>%s", tmpname, QFNEW);
	if (unlink(tmpname) == -1)
		fatal("cannot unlink temp file");

	/* Initialize the seek table. */
	lastseek = (long)sizeof(seektab);
	last = -1;
	if ((fp = fopen(QFNEW, "rw")) == NULL)
		fatal("cannot open \"%s\"", QFNEW);
	fpseek(fp, lastseek, SEEK_SET);
	for (nfiles = 0; fgets(buf, sizeof(buf)-1, fp) != NULL; ++nfiles) {
		if (*buf != last) {
			last = *buf;
			seektab[last] = lastseek;
		}
		lastseek += (long)strlen(buf);
	}
	if (vflag)
		printf("%d files\n%ld bytes\n", nfiles, lastseek);

	/* Rewrite the seek table in the data file. */
	fpseek(fp, 0L, SEEK_SET);
	if (fwrite(seektab, sizeof(seektab), 1, fp) != 1)
		fatal("write error on \"%s\"", QFNEW);
	else if (fclose(fp) == EOF)
		fatal("cannot close \"%s\"", QFNEW);

	/* Remove old if it exists, rename new accordingly. */
	unlink(QFFILES);		/* may not exist */
	if (link(QFNEW, QFFILES) == -1)
		fatal("cannot link \"%s\" to \"%s\"", QFNEW, QFFILES);
	else if (unlink(QFNEW))
		fatal("cannoot unlink \"%s\"", QFNEW);
	return 0;
}

/*
 * Cry and die.
 */
/* VARARGS */
void
fatal(s) char *s;
{
	fprintf(stderr, "qfind: %r\n", &s);
	if (tmpname != NULL)
		unlink(tmpname);
	if (bflag)
		unlink(QFNEW);
	exit(1);
}

/*
 * Seek on fp, die on failure.
 */
void
fpseek(fp, where, how) FILE *fp; long where; int how;
{
	if (fseek(fp, where, how) == -1)
		fatal("seek failed");
}

/*
 * Find s.
 */
int
qfind(s) char *s;
{
	register int val;
	register char *cp;
	int len, notfound, isdir;

	/* Seek to appropriate place in data file to begin linear search. */
	notfound = 1;
	if (!qseek(s)) {
		fprintf(stderr, "qfind: %s: not found\n", s);
		return notfound;
	}

	/* Read lines and look for matches. */
	len = strlen(s);
	while (fgets(buf, sizeof(buf)-1, ifp) != NULL) {

		if ((val = strncmp(buf, s, len)) < 0)
			continue;		/* not there yet */
		else if (val > 0)
			break;			/* past it */

		/* Possible match. */
		if ((cp = strchr(buf, ' ')) == NULL)
			fatal("strchr botch, buf=%s", buf);
		isdir = *(cp - 1) == '/';	/* iff buf contains dir name */
		if ((isdir && !dflag && !aflag)	/* directories not wanted */
		 || (!isdir && dflag))		/* files not wanted */
			continue;
		if (!pflag && (buf[len] != ((isdir) ? '/' : ' ')))
			continue;		/* not exact match */

		/* Match, print it out. */
		notfound = 0;
		buf[strlen(buf)-1] = '\0';	/* zap trailing newline */
		*cp++ = '\0';			/* NUL-terminate filename */
		printf("%s/%s\n", cp, buf);	/* print the match */
	}

	/* Return appropriate status. */
	if (notfound)
		fprintf(stderr, "qfind: %s: not found\n", s);
	return notfound;
}

/*
 * Seek in data file to someplace preceding the desired key.
 * Use binary search to get close, for efficiency.
 * Return 0 on failure.
 */
int
qseek(key) char *key;
{
	register int i, len;
	long new, min, max;

	i = *key;
	if ((min = seektab[i]) == 0L)		/* lower bound for search */
		return 0;		/* no entries with right first char */

#if	1
	/* Binary search. */
	for (++i; i < NCHARS; ++i) {
		if (seektab[i] != 0L) {
			max = seektab[i];	/* upper bound for search */
			break;
		}
	}
	if (i == NCHARS) {
		fpseek(ifp, 0L, SEEK_END);
		max = ftell(ifp);
	}
	len = strlen(key);
	while (max - min > MINSEEK) {
		new = (min + max) / 2;
		fpseek(ifp, new, SEEK_SET);	/* seek to midpoint of range */
		while ((i = getc(ifp)) != EOF) {
			++new;
			if (i == '\n')
				break;		/* scan to next newline */
		}
		if (new >= max
		 || fgets(buf, sizeof(buf) - 1, ifp) == NULL)
			break;			/* should not happen */
		if ((i = strncmp(key, buf, len)) <= 0)
			max = new;
		else
			min = new;
	}
#endif
	fpseek(ifp, min, SEEK_SET);
	return 1;
}

/*
 * Execute a system command, die on failure.
 */
/* VARARGS */
void
sys(s) char *s;
{
	sprintf(buf, "%r", &s);
#if	0
	fprintf(stderr, "%s\n", buf);		/* for debugging */
#endif
	if (system(buf) != 0)
		fatal("command \"%s\" failed", buf);
}

/*
 * Print a usage message and die.
 */
void
usage()
{
	fprintf(stderr, USAGE);
	exit(1);
}

/* end of qfind.c */
