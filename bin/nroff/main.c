/*
 * main.c
 * Nroff/Troff.
 * Main program and initialization.
 */

#include <ctype.h>
#include <sys/types.h>
#include <time.h>
#include <sys/stat.h>
#include <path.h>
#include "roff.h"

extern	char	*getenv();
extern	char	*path();
extern	time_t	time();
#ifdef	GEMDOS
extern	char	*tempnam();
#else
extern	char	*mktemp();
#endif

#ifdef	GEMDOS
unsigned long _stksize = 0x8000L;
#define	TMACFORMAT	"%s.tmc"
#endif

#ifdef	MSDOS
#define	TMPLATE	"nroffX.tmp"		/* Template for temp file...	*/
#define	TMACFORMAT	"%s.tmc"
#endif

#ifdef	COHERENT
#define	TMPLATE	"/tmp/rofXXXXXX"
#define	TMACFORMAT	"tmac.%s"
#endif

static	int	kflag;		/* keep tmp file for debug purposes */
static	char	*tempname;	/* temp file name */

main(argc, argv) int argc; char *argv[];
{
	register int i, fileflag, iflag;
	register char *libpath, *cp;
	register REG *rp;
	char c, name[2];

	argv0 = (ntroff == NROFF) ? "nroff" : "troff";
	cp = getenv(ntroff == NROFF ? "NROFF" : "TROFF");
	if (cp != NULL && *cp != '\0')
		addargs(cp, &argc, &argv);
	initialize(argc, argv);

	/*
	 * Process specified input files.
	 * initialize() already handled most options.
	 */
	fileflag = iflag = 0;
	for (i = 1; i < argc; i++) {
		if (argv[i][0] != '-') {
			/* Process non-option argument. */
			fileflag = 1;
			if (adsfile(argv[i]) != 0)
				process();
			continue;
		}
		c = argv[i][1];
		if (c == 'i')
			iflag = 1;		/* Process stdin when done */
		else if (c == 'm') {
			/* Process "-m" macro package argument. */
			sprintf(miscbuf, TMACFORMAT, &argv[i][2]);
			libpath = DEFLIBPATH;
			if ((libpath = path(libpath, miscbuf, AREAD)) != NULL)
				strcpy(miscbuf, libpath);
#if	(DDEBUG & DBGFILE)
			printd(DBGFILE, "tmac file = %s\n", miscbuf);
#endif
			adsfile(miscbuf);
			process();
		} else if (c == 'n') {
			/* Reset page number. */
			pno = atoi(&argv[i][2]);
			npn = 1 + pno;
		} else if (c == 'r' && argv[i][2] != '\0') {
			/* Reset register value. */
			name[0] = argv[i][2];
			name[1] = '\0';
			rp = getnreg(name);
			rp->n_reg.r_nval = atoi(&argv[i][3]);
			if (rp == nrpnreg)		/* Page # register */
				npn = pno + 1;		/* Set next page # */
		}
	}
	if (fileflag == 0 || iflag != 0) {
		/* Process standard input. */
		adsunit(stdin);
		process();
	}
	leave(0);
}

/*
 * Open temp file, set up registers and general initialization.
 */
initialize(argc, argv) int argc; char *argv[];
{
	register REG *rp;
	register REQ *qp;
	register int i;
	int tmparg;

	A_reg = ntroff==NROFF;

	/* Pass over args, process those dealing with global initialization. */
	/* main() makes another pass over the arg list to process input files. */
	tmparg = 0;
	for (i = 1; i < argc; i++) {
		if (argv[i][0] != '-')
			continue;
		switch (argv[i][1]) {
		case 'a':
			A_reg = 1;
			continue;
		case 'd':
#if	DDEBUG
			if (argv[i][2] != '\0') {
				dbglvl = atoi(&argv[i][2]);
				dbginit();
			} else
#endif
				dflag++;
			continue;
		case 'D':
			font_display();
			continue;
		case 'f':
			if (i < (argc-1))
				tmparg = ++i;
			else
				panic("-f option requires file argument");
			continue;
		case 'i':
			continue;	/* handled in main() */
		case 'K':
		case 'k':
			kflag++;
			continue;
		case 'l':
			lflag = 1;
			continue;
		case 'm':
			continue;	/* handled in main() */
		case 'n':
			continue;	/* handled in main() */
		case 'p':
			pflag = 1;
			continue;
		case 'r':
			continue;	/* handled in main() */
#ifndef GEMDOS
		case 'T':
			if (ntroff == NROFF)
				T_reg = 1;
			continue;
#endif
		case 'x':
			xflag++;
			continue;
		case 'V':
		case 'v':
			fprintf(stderr, "%s: V%s\n", argv0, VERSION);
			continue;
		default:
			panic("illegal option: %s", argv[i]);
		}
	}

	/* Device-specific initialization. */
	devparm();

	/* Initialize tempfile. */
#ifdef GEMDOS
	tempname = (tmparg ? argv[tmparg] : tempnam(0L, "nroff"));
#else
	tempname = (tmparg ? argv[tmparg] : mktemp(TMPLATE));
#endif
	dprint2(DBGFILE, "temp file name = %s\n", tempname);
	if ((tmp=fopen(tempname, "w")) == NULL)
		panic("cannot create temp file");
	else if (freopen(tempname, "rw", tmp) == NULL)
		panic("cannot reopen temp file");
	tmpseek = ENVSIZE * sizeof (ENV);
	tmpseek = (tmpseek+DBFSIZE+DBFSIZE-1) & ~(DBFSIZE-1);

#ifdef GEMDOS
	/* Under GEMDOS lseek wil not produce sparse files
	 * so it becomes necessary to write the beginning of
	 * nroff's work file.
	 */
	{
		char buff[DBFSIZE];
		register char *cp;
		register int i;

		for(cp=buff; cp < &buff[DBFSIZE];)
			*cp++ = '\0';

		for(i = 0; i < tmpseek; i += DBFSIZE) {
			dprintd(DBGFILE, "initializing tempfile\n");
			if(write(fileno(tmp), buff, DBFSIZE) != DBFSIZE)
				panic("temp file write error");
		}
	}
#endif
#ifdef	COHERENT
	/*
	 * Unlinking temp file immediately under COHERENT makes it
	 * go away if program is interrupted with <Ctrl-C>;
	 * under GEMDOS it would destroy the file immediately,
	 * so it is done under leave() below.
	 */
	if (kflag == 0)
		unlink(tempname);
#endif

	/* Initialize globals. */
	for (i = 0; i < NWIDTH; i++)
		trantab[i] = i;			/* translation table */
	for (i = 0; i < RHTSIZE; i++)
		regt[i] = NULL;			/* request hash table */
	for (qp = reqtab; qp->q_name[0]; qp++) {	/* built-in requests */
		rp = makereg(qp->q_name, RTEXT);
		rp->t_reg.r_macd.r_div.m_next = NULL;
		rp->t_reg.r_macd.r_div.m_type = MREQS;
		rp->t_reg.r_macd.r_div.m_func = qp->q_func;
	}

	/* Create built in registers. */
	nrpnreg = getnreg("%");
	nrctreg = getnreg("ct");
	nrdlreg = getnreg("dl");
	nrdnreg = getnreg("dn");
	nrdwreg = getnreg("dw");
	nrdyreg = getnreg("dy");
	nrhpreg = getnreg("hp");
	nrlnreg = getnreg("ln");
	nrmoreg = getnreg("mo");
	nrnlreg = getnreg("nl");
	nrsbreg = getnreg("sb");
	nrstreg = getnreg("st");
	nryrreg = getnreg("yr");
	setnreg();

	/* Environment initialization. */
	setenvr();
	envinit[0] = 1;

	cdivp = NULL;
	newdivn("\0\0");
	mdivp = cdivp;
#if	GEMDOS
	if (((long)mdivp) & 1L)
		panic("diversion buffer odd alignment");
#endif
	endtrap[0] = '\0';
	strp = NULL;
	pgl = (lflag) ? unit(17*SMINCH, 2*SDINCH) : unit(11*SMINCH, SDINCH);
	pno = 1;
	npn = 2;
	esc = '\\';
	enbldn	= 0;		/* Enbolden by n pts.	*/
#if	(DDEBUG & DBGCHEK)
	printd(DBGFUNC, "initialized...\n");
#endif
}

/*
 * Initialize pre-defined number registers.
 */
setnreg()
{
	time_t curtime;
	register struct tm *tmp;

	curtime = time((time_t *)0);
	tmp = localtime(&curtime);
	nryrreg->n_reg.r_nval = tmp->tm_year%100;
	nrmoreg->n_reg.r_nval = tmp->tm_mon+1;
	nrdyreg->n_reg.r_nval = tmp->tm_mday;
	nrdwreg->n_reg.r_nval = tmp->tm_wday + 1;
}

/*
 * Leave.
 */
leave(n)
{
	char name[2];
	static int depth = 0;

	if(n == 0 && depth++ == 0) {
		if (endtrap[0] != '\0') {
			name[0] = endtrap[0];
			name[1] = endtrap[1];
			endtrap[0] = '\0';
			execute(name);
		}
		setbreak();
		if (xflag == 0) {
			byeflag = 1;
			pspace(0);
		}
	}
#ifndef COHERENT
#if	(DDEBUG & DBGFILE)
	{
		struct stat statblk;

		fclose(tmp);			/* Close the temp file.	*/
		stat(tempname, &statblk);	/* so we can stat it.	*/
		printd(DBGFILE, "deleting temporary file %s, size = %ld\n",
			tempname, statblk.st_size);
	}
#endif
	/* Unlink temp file if not COHERENT. */
	if (kflag == 0)
		unlink(tempname);
#endif
	exit(n);
}

/*
 * cp contains space-separated environmental args to be added to argv.
 * Change argc/argv accordingly.
 */
addargs(cp, argcp, argvp) char *cp; int *argcp; char ***argvp;
{
	register int n;
	register char *s, **nargv, **np;

	for (s = cp, n = 1; *s != '\0'; s++)
		if (*s == ' ')
			++n;		/* number of added args */
	*argcp += n;			/* bump argc */
	np = nargv = (char **)nalloc((*argcp + 1) * sizeof (char *));	/* allocated */
	*np++ = *(*argvp)++;		/* copy old argv0 */
	for (s = cp; *s != '\0'; ) {
		*np++ = s;		/* store pointer to new arg */
		while (*s != '\0' && *s != ' ')
			s++;		/* scan to NUL or space */
		if (*s == ' ')
			*s++ = '\0';	/* NUL-terminate space-separated args */
	}
	while (**argvp != NULL)
		*np++ = *(*argvp)++;	/* copy old argv */
	*np = NULL;			/* NULL-terminate new argv */
	*argvp = nargv;			/* pass back new argv */
}

/* end of main.c */
