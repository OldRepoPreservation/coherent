/*
 * /usr/src/cmd/prof.c
 * 7/15/92
 * prof interprets the mon.out files produced by the runtime profiling option,
 * i.e. by programs compiled with the cc option -p (a.k.a. -VPROF).
 * This version understands both COH286 l.out and COH386 COFF executables,
 * which have different mon.out sizes in addition to different symbol formats.
 * Usage:
 *	prof [ -abcs ] [ l.out [ mon.out ] ]
 * Options:
 * 	-a	use all symbols (as opposed to only external symbols)
 *	-b	print all bin information (to detect hot spots)
 *	-c	print all call information
 *	-s	print stack depth information
 *
 * UNDONE: COFF version truncates symbols at NCPLN characters.
 */

#include <stdio.h>
#include <l.out.h>
#include <coff.h>
#include <sys/const.h>
#include <canon.h>
#include <mon.h>
#if	_I386
#include <oldmon.h>
#endif

#if	DEBUG
#define	dbprintf(args)	printf args
#else
#define	dbprintf(args)
#endif

/*
 * Note that in setting PSCALE, one must guard against overflow.
 * Examine putdata() carefully before changing this constant.
 * Also note that certain divisibility properties are assumed
 * there reguarding PSCALE and HZ.
 */
#define PSCALE	((long)100)		/* pc count scale factor	*/
#define	SWIDTH	10			/* default printf symbol width	*/
#define	TRUE	(0 == 0)
#define	FALSE	(0 != 0)

/* This should be modified to allow arbitrary length symbols for COFF... */
typedef struct	symbol {
	char		name[NCPLN];
	vaddr_t		addr;
	long		pcount;		/* pc count, scaled by PSCALE */
	long		ccount;		/* number of times routine called */
}	symbol;

/* Forward. */
char	*alloc();
void	centi();
int	cmpdata();
int	cmpsym();
symbol	**credit();
void	fatal();
void	getcdata();
void	getdata();
void	getpdata();
char	*getstring();
void	getsyms();
void	putdata();
void	readcoffsyms();
void	readsyms();
void	usage();
void	warning();

/* Globals. */
int		aflag	= FALSE;	/* iff we use all symbols	*/
int		bflag	= FALSE;	/* iff we should dump bin info	*/
int		cflag	= FALSE;	/* iff we should dump call info	*/
symbol		**dict;			/* NULL terminated list of symbols */
int		dsize;			/* number of symbols		*/
int		iscoff	= FALSE;	/* COFF executable (not l.out)	*/
char		*lout	= "l.out";	/* executable file name		*/
vaddr_t		lowpc;			/* lowest pc profiled		*/
char		*monout	= "mon.out";	/* monitor file name		*/
int		sflag	= FALSE;	/* iff we should dump low stack mark */
vaddr_t		stksz;			/* stack size			*/
long		strtable;		/* COFF string table offset	*/
long		tcalls;			/* total number of calls	*/
long		tticks;			/* total number of clock ticks	*/
unsigned int	scaler;

main(argc, argv) int argc; register char *argv[];
{
	register char *cp, ch;

	for (cp=*++argv; cp != NULL  &&  *cp++ == '-'; cp=*++argv)
		while ((ch=*cp++) != '\0')
			switch (ch) {
			case 'a':	aflag = TRUE;		break;
			case 'b':	bflag = TRUE;		break;
			case 'c':	cflag = TRUE;		break;
			case 's':	sflag = TRUE;		break;
			default:	usage();		break;
			}
	if (*argv != NULL)
		lout = *argv++;
	if (*argv != NULL)
		monout = *argv++;
	if (*argv != NULL)
		usage();
	getsyms();
	getdata();
	if (sflag)
		printf("%u bytes stack used\n", stksz);
	if (!bflag && !cflag)
		putdata();
	exit(0);
}

/*
 * alloc() is an interface to malloc() which exits if there is no room.
 */
char *
alloc(size) unsigned size;
{
	char	*result;

	result = (char *)malloc(size);
	if (result == NULL)
		fatal("out of space");
	return result;
}

/*
 * Print on standard output 'num' / 'den' to two places, with
 * at least 'width' places to the left of the decimal point.
 */
void
centi(num, den, width) long num, den; int width;
{
	long	cv;

	cv = (num*100 + den/2) / den;
	printf("%*ld.%02d", width, cv/100, (int)(cv%100));
}

/*
 * Compare the data in the dictionary entries 'sp1' and 'sp2'.
 * Return an int which reflects which entry should be listed first.
 * If the value is positive, 'sp2' should occur first.
 * If it is zero, it makes no difference. 
 * If it is negative, 'sp1' should occur first.
 */
int
cmpdata(sp1, sp2) symbol **sp1, **sp2;
{
	register symbol *adr1, *adr2;
	long rel;

	adr1 = *sp1;
	adr2 = *sp2;
	rel = adr2->pcount - adr1->pcount;
	if (rel == 0)
		rel = adr2->ccount - adr1->ccount;
	if (rel > 0)
		return 1;
	else if (rel < 0)
		return -1;
	else
		return strncmp(adr1->name, adr2->name, NCPLN);
}

/*
 * Compare the two symbols 'sp1' and 'sp2' and return an
 * int corresponding to the relative order of the address fields.
 */
int
cmpsym(sp1, sp2) symbol **sp1, **sp2;
{
	register vaddr_t adr1, adr2;

	adr1 = (*sp1)->addr;
	adr2 = (*sp2)->addr;
	if (adr1 > adr2)
		return 1;
	else if (adr1 == adr2)
		return 0;
	else
		return -1;
}

/*
 * Account for tick information.
 */
symbol	**
credit(tick, low, high, dpp) int tick; vaddr_t low, high; symbol **dpp;
{
	register unsigned	overlap;
	register symbol		*cur, *nxt;
	unsigned		binlen;

	dbprintf(("credit(%d, %x, %x, %s)\n", tick, low, high, (*dpp)->name));
	binlen = high - low;
if (binlen == 0) binlen = 1; /* ??? */
printf("binlen=%d\n", binlen);

	nxt = *dpp;
	if (nxt == NULL  ||  nxt->addr >= high) {
		if (bflag)
			printf("%3d %06o %06o\n", tick, low, high-1);
		return dpp;
	}
	do {
		cur = nxt;
		nxt = *++dpp;
	} while (nxt != NULL  &&  nxt->addr <= low);
	if (bflag)
		printf("%3d %*.*s+%-4u ", tick, SWIDTH, NCPLN, cur->name,
			low - cur->addr);
	do {
		if (nxt != NULL  &&  nxt->addr < high)
			overlap = nxt->addr;
		else
			overlap = high;
		if (cur->addr > low)
			overlap -= cur->addr;
		else
			overlap -= low;
		cur->pcount += (PSCALE*overlap*tick + binlen/2) / binlen;
		cur = nxt;
		nxt = *++dpp;
	} while (cur != NULL && cur->addr < high);
	dpp -= 2;
	if (bflag)
		printf("%*.*s+%u\n", SWIDTH, NCPLN, dpp[0]->name,
			high - 1 - dpp[0]->addr);
	return dpp;
}

/*
 * Print fatal error message and die.
 */
void
fatal(str) char *str;
{
	fprintf(stderr, "prof: %r\n", &str);
	exit(1);
}

/*
 * Read function call information from the mon.out file.
 */
void
getcdata(fp, nfnc) FILE *fp; register unsigned nfnc;
{
	register symbol	**dpp, *dp;
	struct m_func	buf;
#if	_I386
	struct	old_m_func obuf;
#endif

	dbprintf(("getcdata(): nfnc=%d\n", nfnc));
	while (nfnc-- != 0) {
#if	_I386
		if (!iscoff) {
			if (fread(&obuf, sizeof obuf, 1, fp) != 1)
				fatal("unexpected end of file on \"%s\"", monout);
			buf.m_addr = obuf.m_addr;
			buf.m_ncalls = obuf.m_ncalls;
		} else
#endif
		if (fread(&buf, sizeof buf, 1, fp) != 1)
			fatal("unexpected end of file on \"%s\"", monout);
		for (dpp=dict; (dp=*++dpp) != NULL && dp->addr <= buf.m_addr;)
			;
		dp = dpp[-1];
		if (cflag)
			printf("%4ld %*.*s+%u\n", buf.m_ncalls, SWIDTH, NCPLN,
				dp->name, buf.m_addr - dp->addr);
		tcalls += buf.m_ncalls;
		dp->ccount += buf.m_ncalls;
	}
	dbprintf((" tcalls=%ld\n", tcalls));
}

/*
 * Read the mon.out file and put the information into the dictionary.
 */
void
getdata()
{
	FILE		*fp;
	struct m_hdr	hdr;
#if	_I386
	struct	old_m_hdr ohdr;
#endif

	dbprintf(("getdata():\n"));
	fp = fopen(monout, "r");
	if (fp == NULL)
		fatal("cannot open \"%s\"", monout);
#if	_I386
	if (!iscoff) {
		/* Read COH286 mon.out and massage accordingly. */
		if (fread(&ohdr, sizeof ohdr, 1, fp) != 1)
			fatal("\"%s\" is not a 286 mon.out file", monout);
		hdr.m_nbins = ohdr.m_nbins;
		hdr.m_scale = ohdr.m_scale;
		hdr.m_nfuncs = ohdr.m_nfuncs;
		hdr.m_lowpc = ohdr.m_lowpc;
		hdr.m_lowsp = ohdr.m_lowsp;
		hdr.m_hisp = ohdr.m_hisp;
	} else
#endif
	if (fread(&hdr, sizeof hdr, 1, fp) != 1)
		fatal("\"%s\" is not a mon.out file", monout);
	dbprintf((" nbins=%d scale=%d nfuncs=%d\n", hdr.m_nbins, hdr.m_scale, hdr.m_nfuncs));
	scaler = hdr.m_scale & 0xffff;
	if ((scaler & 0xfff) == 0xfff)
		scaler++;
	lowpc = hdr.m_lowpc;
	stksz = hdr.m_hisp - hdr.m_lowsp;
	dbprintf((" scaler=%d lowpc=%x stksz=%x\n", scaler, lowpc, stksz));
	if (cflag || !bflag)
		getcdata(fp, hdr.m_nfuncs);
	else
		fseek(fp, hdr.m_nfuncs * (long)sizeof (struct m_func), SEEK_CUR);
	if (bflag || !cflag)
		getpdata(fp, hdr.m_nbins);
	fclose(fp);
}

/*
 * Reads in the profiling data and increment the corresponding
 * symbols' pcount fields.
 * N.B. the global scale must contain the mon.out scale divided by 2.
 */
void
getpdata(fp, nbins) FILE *fp; unsigned nbins;
{
	register symbol	**dpp;
	vaddr_t		high, low;
	int		highr, inc, incr;
	short		tick;
/*
 *	scale		text bytes covered per bin
 *	0x10000		2
 *	0xFFFF		2	(for historical reasons)
 *	0x8000		4
 *	0x7FFF		4	(for historical reasons)
 *	0x4000		8
 *	...		...
 *	0x0002		65536
 */

	dbprintf(("getpdata(): nbins=%d\n", nbins));
	high = lowpc;
	highr = 0;
#if 1
	inc = ((long)1<<17) / scaler;
	incr = ((long)1<<17) % scaler;
	if (incr) {
		++inc;
		incr -= scaler;
	}
#else
	inc = 131072/scaler;
#endif
	dbprintf((" inc=%d incr=%d scale=%d\n", inc, incr, scaler));
	for (dpp=dict; nbins > 0; --nbins) {
		low = high;
		high += inc;
		highr += incr;
		if (-highr >= scaler) {
			--high;
			highr += scaler;
		}
		if (fread(&tick, sizeof tick, 1, fp) != 1)
			fatal("unexpected end of file on \"%s\"", monout);
		if (tick == 0)
			continue;
		tticks += tick;
		dpp = credit(tick, low, high, dpp);
	}
	if (fgetc(fp) != EOF)
		warning("excess data in \"%s\"", monout);
}

/*
 * Read a NUL-terminated string from offset 'loc' (in COFF string table)
 * in fp into a static buffer.
 * For now, symbols longer than NCPLN are truncated!
 */
#define	NBUF	(NCPLN+1)
char *
getstring(fp, loc) FILE *fp; long loc;
{
	static char buf[NBUF];
	register long sav;
	register char *cp;
	register int c;

	sav = ftell(fp);
	if (fseek(fp, strtable+loc, SEEK_SET) == -1L)
		fatal("seek failed");
	for (cp = buf; cp < &buf[NBUF-1]; *cp++ = c)
		if ((c = fgetc(fp)) == '\0' || c == EOF)
			break;
	*cp = '\0';
	if (c != '\0' && fgetc(fp) != '\0')
		warning("symbol truncated to %s", buf);
	if (fseek(fp, sav, SEEK_SET) == -1L)
		fatal("seek failed");
	return buf;
}

/*
 * Read the symbols from an l.out file.
 * Set dict to an array of them, in sorted order.
 */
void
getsyms()
{
	FILE		*fp;
	long		skip;
	struct ldheader	hdr;
	FILEHDR		chdr;

	dbprintf(("getsyms():\n"));
	fp = fopen(lout, "r");
	if (fp == NULL)
		fatal("cannot open \"%s\"", lout);
	if (fread(&hdr, sizeof hdr, 1, fp) != 1 || hdr.l_magic != L_MAGIC) {
		/* File is not l.out, see if it is COFF. */
		rewind(fp);
		if (fread(&chdr, sizeof chdr, 1, fp) != 1
		 || !ISCOFF(chdr.f_magic))
			fatal("\"%s\" is neither l.out nor COFF executable", lout);
		iscoff = TRUE;
		dbprintf(("386 COFF executable\n"));
		if (fseek(fp, chdr.f_symptr, SEEK_SET) == -1L)
			fatal("seek to symbol table failed");
		strtable = chdr.f_symptr + chdr.f_nsyms * sizeof(SYMENT);
		readcoffsyms(chdr.f_nsyms, fp);
	} else {
		/* File is l.out. */
		dbprintf(("286 l.out executable\n"));
		cansize(hdr.l_ssize[L_SHRI]);
		cansize(hdr.l_ssize[L_PRVI]);
		cansize(hdr.l_ssize[L_SHRD]);
		cansize(hdr.l_ssize[L_PRVD]);
		cansize(hdr.l_ssize[L_DEBUG]);
		cansize(hdr.l_ssize[L_SYM]);
		skip = hdr.l_ssize[L_SHRI] + hdr.l_ssize[L_PRVI]
			+ hdr.l_ssize[L_SHRD] + hdr.l_ssize[L_PRVD]
			+ hdr.l_ssize[L_DEBUG];
		fseek(fp, skip, SEEK_CUR);
		readsyms((int)(hdr.l_ssize[L_SYM]/sizeof (struct ldsym)), fp);
	}
	qsort(dict, dsize, sizeof *dict, cmpsym);
	fclose(fp);
}

/*
 * Print out the results which have been tabulated in the dictionary.
 */
void
putdata()
{
	register symbol	**dpp, *dp;

	dbprintf(("putdata():\n"));
	qsort(dict, dsize, sizeof *dict, cmpdata);
	for (dpp=dict; (dp=*dpp++) != NULL;) {
		if (dp->pcount == 0 && dp->ccount == 0)
			continue;
		printf("%-*.*s", SWIDTH, NCPLN, dp->name);
		if (tticks != 0) {
			centi(dp->pcount, (PSCALE * tticks)/100, 2);
			putchar('%');
		}
		if (dp->ccount != 0) {
			printf(" %7ld ", dp->ccount);
			centi((1000*dp->pcount) / (PSCALE*10),
				(HZ*dp->ccount) / 10, 3);
		}
		putchar('\n');
	}
}

/*
 * Read in nsyms COFF symbols from FILE fp.
 * Set dict to an array of the resulting symbols.
 */
#define	SCNUM_TEXT	1		/* COFF .text section number */
void
readcoffsyms(nsyms, fp) register long nsyms; FILE *fp;
{
	register symbol **dpp, *dp;
	char name[NCPLN];
	SYMENT sym;

	dbprintf(("readcoffsyms(): nsyms=%ld\n", nsyms));
	dict = dpp = (symbol **)alloc((nsyms + 1) * sizeof *dpp);
	name[8] = '\0';
	while (nsyms-- > 0) {
		if (fread(&sym, sizeof sym, 1, fp) != 1)
			fatal("symbol read failed");
		if (sym.n_scnum != SCNUM_TEXT || sym.n_sclass != C_EXT)
			continue;		/* ignore all but .text */
		dp = (symbol *)alloc(sizeof *dp);
		if (sym.n_zeroes == 0L)
			strncpy(dp->name, getstring(fp, sym.n_offset), NCPLN);
		else {
			strncpy(dp->name, sym.n_name, 8);
			dp->name[8] = '\0';
		}
		dp->addr = sym.n_value;
		dp->pcount = dp->ccount = 0;
		*dpp++ = dp;
	}
	*dpp = NULL;
	dsize = dpp - dict;
	if (dsize == 0)
		fatal("no symbols found in \"%s\"", lout);
	dict = (symbol *)realloc(dict, (dpp + 1 - dict) * sizeof *dpp);
}

/*
 * Read in nsyms l.out ldsyms from the FILE fp. 
 * Set dict to an array of the resulting symbols.
 */
void
readsyms(nsyms, fp) register int nsyms; FILE *fp;
{
	register symbol	**dpp, *dp;
	struct ldsym	lsym;

	dbprintf(("readsyms(): nsyms=%d\n", nsyms));
	dict = dpp = (symbol **)alloc((nsyms + 1) * sizeof *dpp);
	while (--nsyms >= 0) {
		if (fread(&lsym, sizeof lsym, 1, fp) != 1)
			fatal("unexpected end of file on \"%s\"", lout);
		if ((lsym.ls_type & ~L_GLOBAL) > L_BSSI)
			continue;
		if ((lsym.ls_type & L_GLOBAL) == 0 && ! aflag)
			continue;
		dp = (symbol *)alloc(sizeof *dp);
		strncpy(dp->name, lsym.ls_id, NCPLN);
		dp->addr = lsym.ls_addr;
		dp->pcount = dp->ccount = 0;
		*dpp++ = dp;
	}
	*dpp = NULL;
	dsize = dpp - dict;
	if (dsize == 0)
		fatal("no symbols found in \"%s\"", lout);
	dict = (symbol *)realloc(dict, (dpp + 1 - dict) * sizeof *dpp);
}

/*
 * Print usage message and die.
 */
void
usage()
{
	fprintf(stderr, "Usage: prof [ -abcs ] [ l.out [ mon.out ] ]\n");
	exit(1);
}

/*
 * Print nonfatal warning message.
 */
void
warning(str) char *str;
{
	fprintf(stderr, "prof: Warning: %r\n", &str);
}

/* end of /usr/src/cmd/prof.c */
