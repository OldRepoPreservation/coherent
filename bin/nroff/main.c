/*
 * Nroff/Troff.
 * Main programme and initialisation.
 */
#include <stdio.h>
#include <ctype.h>
#include <sys/types.h>
#include <time.h>
#include <sys/stat.h>
#include <path.h>
#include "roff.h"
#include "code.h"
#include "env.h"
#include "esc.h"
#include "div.h"
#include "reg.h"
#include "str.h"
#include "codebug.h"

#define	VERSION	"Nroff: 2.7 (c) 1982-1987 by Mark Williams Company, Chicago\n"

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

static char nodel=0;	/* keep tmp file for debug purposes */
static char *tempname;	/* temp file name */

main(argc, argv)
char *argv[];
{
	char name[2];
	int filflag, tinflag;
	register REG *rp;
	register int i;

#ifdef MSDOS
	msdoscvt("nroff", &argc, &argv);
#endif
	initialise(argc, argv);
	filflag = 0;
	tinflag = 0;
	for (i=1; i<argc; i++) {
		if (argv[i][0] != '-') {
			filflag = 1;
			if (adsfile(argv[i]) != 0)
				process();
			continue;
		}
		switch (argv[i][1]) {
		case 'f':
			i++;	/* done in initialize */
			continue;
		case 'a':
			antflag = 1;
			continue;
		case 'd':
#if	DDEBUG
			if (argv[i][2] != '\0') {
				dbglvl = atoi(&argv[i][2]);
				dbginit();
			} else
#endif
				debflag++;
			continue;
		case 'i':
			tinflag++;
			continue;
		case 'm':
			sprintf(miscbuf, TMACFORMAT, &argv[i][2]);
			{
				char *getenv(), *path();
				char *libpath;

				if ((libpath = getenv("TMACPATH")) == NULL)
					if ((libpath = getenv("LIBPATH")) == NULL)
						libpath = DEFLIBPATH;
				if ((libpath = path(libpath, miscbuf, AREAD)) != NULL)
					strcpy(miscbuf, libpath);
			}
#if	(DDEBUG & DBGFILE)
			printd(DBGFILE, "tmac file = %s\n", miscbuf);
#endif
			adsfile(miscbuf);
			process();
			continue;
		case 'n':
			pno = atoi(&argv[i][2]);
			npn = 1 + pno;
			continue;
		case 'r':
			if ((name[0]=argv[i][2]) == '\0')
				continue;
			name[1] = '\0';
			rp = getnreg(name);
			rp->r_nval = atoi(&argv[i][3]);
			if (rp == nrpnreg)	/* If the page # register... */
				npn = pno + 1;	/* Next page # gets set... */
			continue;
#ifndef GEMDOS
		case 'T':
			if (ntroff == NROFF)
				tntflag = 1;
			continue;
#endif
		case 'x':
			ntrflag++;
			continue;
		case 'K':
		case 'k':
			nodel++;
			continue;
		case 'V':
		case 'v':
			fprintf(stderr, VERSION);
			continue;
		default:
			fprintf(stderr, "Bad option: %s\n", argv[i]);
			leave(1);
		}
	}

	if (filflag==0 || tinflag!=0) {
		adsunit(stdin);
		process();
	}
	leave(0);
}

/*
 * Open temp file, set up registers and general initialisation.
 */
initialise(argc, argv)
char *argv[];
{
	register REG *rp;
	register REQ *qp;
	register int i, j;
#ifdef	GEMDOS
	char	*tempnam();
#else
	char	*mktemp();
#endif
	j=0;
	for(i=1; i < argc; i++) {
		if(streq("-D", argv[i])) {
			font_display();
			exit(0);
		}			
		if(!strcmp("-f", argv[i])) {
			if (i < (argc-1))
				j = ++i;
			break;
		}
	}
	devparm();		/* Initialize this device.	*/
#if RSX
	if ((tmp=fopen(tempname=(j ? argv[i] : "rofftmp"), "r+w")) == NULL)
		panic("Cannot open temp file");
	fmkdl(tmp);
#else
#ifdef GEMDOS
	tempname = (j ? argv[i] : tempnam(0L, "nroff"));
#else
	tempname = (j ? argv[i] : mktemp(TMPLATE));
#endif

	dprint2(DBGFILE, "temp file name = %s\n", tempname);
	if ((tmp=fopen(tempname, "w")) == NULL) {
		panic("Cannot create temp file");
	} else if (freopen(tempname, "rw", tmp) == NULL) {
		panic("Cannot reopen temp file");
	}
#endif
	tmpseek = ENVSIZE * sizeof (ENV);
	tmpseek = (tmpseek+DBFSIZE+DBFSIZE-1) & ~(DBFSIZE-1);

#ifdef GEMDOS
	/* Under GEMDOS lseek wil not produce sparse files
	 * so it becomes necessary to write the beginning of
	 * nroff's work file. Also unlinking files under
	 * GEMDOS has the effect of immediate destruction.
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
				panic("Temp file write error");
		}
	}
#endif
#ifdef COHERENT
	if(nodel == 0)
		unlink(tempname);
#endif
	for (i=0; i<RHTSIZE; i++)
		regt[i] = NULL;
	for (qp=reqtab; qp->q_name[0]; qp++) {
		rp = makereg(qp->q_name, RTEXT);
		rp->r_macd.m_next = NULL;
		rp->r_macd.m_type = MREQS;
		rp->r_macd.m_func = qp->q_func;
	}
	/* Create built in registers		*/
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

	for (i=0; i<ENVSIZE; i++)
		envinit[i] = 0;
	envs = 0;
	envstak[envs] = 0;
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
	outflag = 0;
	strp = NULL;
	pgl = unit(11*SMINCH, SDINCH);
	pct = 0;
	pno = 1;
	nrpnreg->r_nval = 1;
	npn = 2;
	esc = '\\';
	svs = 0;
	nbrflag = 0;
	byeflag = 0;
	ifeflag = 0;
	ntrflag = 0;
	debflag = 0;
	antflag = ntroff==NROFF;
	tntflag = 0;
	nrorval = 0;
	enbldn	= 0;		/* Enbolden by n pts.	*/
#if	(DDEBUG & DBGCHEK)
	printd(DBGFUNC, "initialized...\n");
#endif
}

/*
 * Initialise pre-defined number registers.
 */
setnreg()
{
#if RSX
	int timebuf[8];

	time(timebuf);
	nryrreg->r_nval = timebuf[0];
	nrmoreg->r_nval = timebuf[1];
	nrdyreg->r_nval = timebuf[2];
#else
	time_t time();
	time_t curtime;
	register struct tm *tmp;

	curtime = time((time_t *)0);
	tmp = localtime(&curtime);
	nryrreg->r_nval = tmp->tm_year%100;
	nrmoreg->r_nval = tmp->tm_mon+1;
	nrdyreg->r_nval = tmp->tm_mday;
	nrdwreg->r_nval = tmp->tm_wday + 1;
#endif
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
		if (ntrflag == 0) {
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
	if(nodel == 0)
		unlink(tempname);
#endif
	exit(n);
}
