/*
 * Object and archive file services.
 * This section is common to nm and size.
 * And should be common to ndis.
 */

#include <stdio.h>
#include <ar.h>
#include <canon.h>

#if COHERENT
#include <n.out.h>
#define DEFFILE	"l.out"
#endif
#if MSDOS
#include <nout.h>
#define DEFFILE	"l.exe"
#define	DOEXE	1
#endif
#if GEMDOS
#include <nout.h>
#define DEFFILE	"l.prg"
#define	DOPRG	1
#endif

char	*argv0;			/* Command name */
char	*fn;			/* File name */
FILE	*fp;			/* Input file */
struct	ldheader	ldh;	/* The l.out.h header */
struct	ldheader	skh;	/* Seek positions for segments */
struct	ldsym		lds;	/* Symbol buffer */
struct	ar_hdr		ahb;	/* Archive header buffer */
int	title;			/* Title each line */
int	amemb;			/* Archive member */
long	offset;			/* Into archive for start of l.out */

/*
 * Open and close files.
 * If an archive arrange that (*ffp)()
 * is called on each member.
 * Otherwise call (*ffp)() on the file itself.
 */
ar(ffp)	int (*ffp)();
{
	int magic;

	amemb = 0;
	offset = 0;
	if ((fp = fopen(fn, "rb")) == NULL) {
		error("cannot open");
		return;
	}
	magic = getw(fp);
	canshort(magic);
	if (magic == ARMAG) {
		title = amemb = 1;
		while (fread(&ahb, sizeof(ahb), 1, fp) == 1) {
			offset = ftell(fp);
			if (strncmp(ahb.ar_name, HDRNAME, DIRSIZ) != 0)
				(*ffp)();
			cansize(ahb.ar_size);
			fseek(fp, offset+ahb.ar_size, 0);
		}
	} else {
		fseek(fp, 0L, 0);
		(*ffp)();
	}
	if (ferror(fp))
		error("read error");
	fclose(fp);
}

/*
 *	Read in the l.out header.
 */
gethdr()
{
	register int i;
	fread((char *)&ldh, 1, sizeof(ldh), fp);
	canshort(ldh.l_magic);
	canshort(ldh.l_flag);
	canshort(ldh.l_machine);
	if (ldh.l_magic != L_MAGIC)
		return 1;
	if ((ldh.l_flag&LF_32) == 0) {
		canshort(ldh.l_tbase);
		ldh.l_entry = ldh.l_tbase;
		ldh.l_tbase = sizeof(ldh) - 2*sizeof(short);
	} else {
		canshort(ldh.l_tbase);
		canlong(ldh.l_entry);
	}
	skh.l_ssize[L_SYM] = ldh.l_tbase;
	for (i=0; i<NLSEG; i++) {
		cansize(ldh.l_ssize[i]);
		if (i < L_SYM && i != L_BSSI && i != L_BSSD)
			skh.l_ssize[L_SYM] += ldh.l_ssize[i];
	}
	return (0);
}

/*
 * Read the exe header.
 */
#ifndef DOEXE
getexe() { return 1; }
#else
#include "exe.h"

execani(ip)	/* convert from MSDOS exe header byte order to host */
register short *ip;
{
	/* First convert to standard pdp-11 byte order */
	/* Notice, nothing to do to accomplish this    */

	/* Now convert from pdp-11 byte order to host */

	canshort(*ip);
}

getexe()
{
	long daddr;
	exehdr_t exehdr;

	fseek(fp, offset, 0);
	fread((char *)&exehdr, 1, sizeof(exehdr), fp);
	execani(&exehdr.x_magic);
	if (exehdr.x_magic != EXEMAGIC)
		return (1);
	execani(&exehdr.x_sectors);
	execani(&exehdr.x_bytes);
	daddr = (long) exehdr.x_sectors * 512;
	if ( exehdr.x_bytes != 0 )
		daddr += exehdr.x_bytes - 512;
	fseek(fp, offset+daddr, 0);
	if (gethdr() != 0)
		return 1;
	daddr += ldh.l_tbase + ldh.l_ssize[L_DEBUG];
	skh.l_ssize[L_SYM] = daddr;
	return 0;
}
#endif

/*
 *	Read a gemdos .prg header.
 */
#ifndef DOPRG
getprg() { return 1; }
#else
#include "gemout.h"
/* convert 68000 byte order to pdp11 byte order and vice versa */
#if PDP11
gcani(ip) register unsigned char *ip;
{
	register t;
	t = ip[0]; ip[0] = ip[1]; ip[1] = t;
}
gcanl(lp) register unsigned char *lp;
{
	register t;
	t = lp[0]; lp[0] = lp[1]; lp[1] = t;
	t = lp[2]; lp[2] = lp[3]; lp[3] = t;
}
#endif
#if M68000
#define gcani(i)	/* Nil */
#define gcanl(l)	/* Nil */
#endif

getprg()
{
	struct gemohdr ghd;
	register long daddr;
	register int c;
	fseek(fp, offset, 0);
	fread((char *)&ghd, 1, sizeof(ghd), fp);
	gcani(&ghd.g_magic);
	if (ghd.g_magic != GEMOMAGIC)
		return 1;
	gcanl(&ghd.g_ssize[0]);
	gcanl(&ghd.g_ssize[1]);
	gcanl(&ghd.g_ssize[2]);
	gcanl(&ghd.g_ssize[3]);
	daddr = sizeof(ghd) + ghd.g_ssize[0] + ghd.g_ssize[1] + ghd.g_ssize[3];
	fseek(fp, daddr+offset, 0);
	if (getw(fp) | getw(fp)) {
		while (c = getc(fp))
			if (c == EOF)
				return 1;
	}
	daddr = ftell(fp) - offset;
	if (gethdr() != 0)
		return 1;
	daddr += ldh.l_tbase + ldh.l_ssize[L_DEBUG];
	skh.l_ssize[L_SYM] = daddr;
	return 0;
}
#endif

/*
 * Print a title.
 */
dotitle(fp, tail) FILE *fp; char *tail;
{
	if (amemb)
		fprintf(fp, "%s(%.*s)", fn, DIRSIZ, ahb.ar_name);
	else
		fprintf(fp, "%s", fn);
	if (tail)
		fprintf(fp, "%s", tail);
}
/*
 * Give up.
 * Tag the line with the file
 * name.
 */
error(a)
{
	fprintf(stderr, "%s: ", argv0);
	dotitle(stderr, ": ");
	fprintf(stderr, "%r\n", &a);
}

/*
 * Name list specific code.
 */
int	aflag;
int	dflag;
int	gflag;
int	nflag;
int	oflag;
int	pflag;
int	rflag;
int	uflag;

char	*fmt1, *fmt2;
int	stuff;
char	usage[]	= "Usage: %s [-adgnopru] [file ...]\n";

char	*gn[]	= {
	"SI",	"PI",	"BI",
	"SD",	"PD",	"BD",
	" D",	"  ",	"  ",
	" A",   " C",   "??"
};

char	*ln[]	= {
	"si",	"pi",	"bi",
	"sd",	"pd",	"bd",
	" d",	"  ",	"  ",
	" a",   " c",   " ?"
};

struct	fmtab
{
	char	*f_fmt1;
	char	*f_fmt2;
};

char	octb[]	= "       ";
char	octf[]	= "%06lo ";
char	hexb[]	= "     ";
char	hexf[]	= "%04lx ";
char	lngb[]	= "         ";
char	lngf[]	= "%08lx ";
char	segb[]	= "       ";
char	segf[]	= "%06lx ";

struct	fmtab	fmtab[]	= {
	octb,	octf,			/* Unused or unknown */
	octb,	octf,			/* 11 */
	lngb,	lngf,			/* VAX */
	segb,	segf,			/* 360 */
	lngb,	lngf,			/* Z-8001 */
	hexb,	hexf,			/* Z-8002 */
	hexb,	hexf,			/* 8086 */
	hexb,	hexf,			/* 8080 and 8085 */
	hexb,	hexf,			/* 6800 */
	hexb,	hexf,			/* 6809 */
	lngb,	lngf,			/* 68000 */
	lngb,	lngf,			/* NS16000 */
	lngb,	lngf			/* Large Model 8086 */
};

#define	MTYPES	(sizeof(fmtab)/sizeof(struct fmtab))

main(argc, argv)
char *argv[];
{
	register char *cp;
	register c, i;
	int nf;
	extern nm();

	argv0 = argv[0];
	nf = argc-1;
	for (i=1; i<argc; ++i) {
		cp = argv[i];
		if (cp[0] == '-') {
			argv[i] = NULL;
			--nf;
			while ((c = *++cp) != '\0') {
				switch (c) {

				case 'a':
					++aflag;
					break;

				case 'd':
					++dflag;
					break;

				case 'g':
					++gflag;
					break;

				case 'n':
					++nflag;
					break;

				case 'o':
					++oflag;
					++title;
					break;

				case 'p':
					++pflag;
					break;

				case 'r':
					++rflag;
					break;

				case 'u':
					++uflag;
					break;

				default:
					fprintf(stderr, usage, argv0);
					exit(1);
				}
			}
		}
	}
	if (nf == 0) {
		fn = DEFFILE;
		ar(nm);
	} else {
		if (nf > 1)
			title = 1;
		for (i=1; i<argc; ++i)
			if ((fn = argv[i]) != NULL)
				ar(nm);
	}
	exit(0);
}

/*
 * Namelist.
 * You are seeked to the start of
 * a file which is not an archive.
 * Read in symbols, sort if required
 * and print them out.
 */
nm()
{
	register struct ldsym *wstp;
	register int i, t;
	register struct ldsym *stp, *estp;
	int qscmp();

	if (gethdr() != 0
	 && getexe() != 0
	 && getprg() != 0) {
		error("not an object file");
		return;
	}

	if (ldh.l_ssize[L_SYM] == 0) {
		error("no symbol table");
		return;
	}
	if (ferror(fp)) {
		error("read error");
		return;
	}
	if ((t = ldh.l_machine) >= MTYPES)
		t = 0;
	fmt1 = fmtab[t].f_fmt1;
	fmt2 = fmtab[t].f_fmt2;
	fseek(fp, skh.l_ssize[L_SYM]+offset, 0);

	if (! pflag) {
#if ! GEMDOS
		i = ldh.l_ssize[L_SYM];
		if (i != ldh.l_ssize[L_SYM]) {
			error("symbol table too large, use -p option");
			return;
		}
		if ((ldh.l_flag & LF_32) == 0)
			i = i/(sizeof(lds)-2*sizeof(short)) * sizeof(lds);
		if ((stp = malloc(i)) == NULL) {
			error("too many symbols to sort");
			return;
		}
#else
		register long i;
		extern char *lmalloc();
		i = ldh.l_ssize[L_SYM];
		if ((ldh.l_flag & LF_32) == 0)
			i = i/(sizeof(lds)-2*sizeof(short)) * sizeof(lds);
		if ((stp = lmalloc(i)) == NULL) {
			error("too many symbols to sort");
			return;
		}
#endif
		wstp = stp;
	} else {
		wstp = stp = &lds;
	}
	if (title && !oflag) {
		if (stuff)
			printf("\n");
		++stuff;
		if (amemb)
			printf("%.*s:\n", DIRSIZ, ahb.ar_name);
		else
			printf("%s:\n", fn);
	}
	while (ldh.l_ssize[L_SYM] > 0) {
		if (readsym(wstp) != 0)
			break;
		if (gflag && (wstp->ls_type&L_GLOBAL)==0)
			continue;
		if ((wstp->ls_type&~L_GLOBAL) == L_REF) {
			if (dflag)
				continue;
			if (uflag && wstp->ls_addr!=0)
				continue;
		} else {
			if (uflag)
				continue;
		}
		if (!aflag && !csymbol(wstp))
			continue;
		if (pflag)
			writesym(wstp);
		else
			++wstp;
	}
	if (pflag)
		return;
	if ((estp = wstp) == stp) {
		free((char *) stp);
		return;
	}
	shellsort(stp, estp-stp, sizeof(*stp), qscmp);
	wstp = stp;
	while (wstp < estp)
		writesym(wstp++);
	free((char *) stp);
}

/*
 * Read in one entry from the symbol table.
 * Adjusts for 32-bit or 16-bit l.out formats.
 */
readsym(lsp)
register struct ldsym *lsp;
{
	unsigned short vaddr;

	fread(lsp->ls_id, NCPLN, 1, fp);
	fread((char *)&lsp->ls_type, sizeof(int), 1, fp);
	if ((ldh.l_flag & LF_32) != 0) {
		fread((char *)&lsp->ls_addr, sizeof(long), 1, fp);
		canlong(lsp->ls_addr);
		ldh.l_ssize[L_SYM] -= sizeof(lds);
	} else {
		fread((char *)&vaddr, sizeof vaddr, 1, fp);
		canshort(vaddr);
		lsp->ls_addr = vaddr;
		ldh.l_ssize[L_SYM] -= sizeof(lds)-sizeof(short);
	}
	if (feof(fp)) {
		error("read error");
		return (1);
	}
	canint(lsp->ls_type);
	return (0);
}

/*
 * Write out a symbol.
 */

writesym(lsp) register struct ldsym *lsp;
{
	register int t;

	if (title && oflag) {
		if (amemb)
			printf("%.*s ", DIRSIZ, ahb.ar_name);
		else
			printf("%s ", fn);
	}
	t = lsp->ls_type & ~L_GLOBAL;
	if (t<L_SHRI || t>L_REF)
		t = L_REF+1;
	if (t==L_REF && lsp->ls_addr==0) {
		printf(fmt1);
		putchar(' ');
		putchar((lsp->ls_type&L_GLOBAL)!=0 ? 'U' : 'u');
	} else {
		printf(fmt2, lsp->ls_addr);
		printf((lsp->ls_type&L_GLOBAL)!=0 ? gn[t] : ln[t]);
	}
	printf(" %.*s\n", NCPLN, lsp->ls_id);
}

/*
 * This routine gets called if we
 * are not in '-a' mode. It determines if
 * the symbol pointed to by 'sp' is a C
 * style symbol (trailing '_' or longer than
 * (NCPLN-1) characters). If it is it eats the '_'
 * and returns true.
 */
csymbol(sp)
register struct ldsym *sp;
{
	register char *cp1, *cp2;

	cp1 = &sp->ls_id[0];
	cp2 = &sp->ls_id[NCPLN];
	while (cp2!=cp1 && *--cp2==0)
		;
	if (*cp2 != 0) {
		if (*cp2 == '_') {
			*cp2 = 0;
			return (1);
		}
		if (cp2-cp1 >= (NCPLN-1))
			return (1);
#if PDP11	/* Special hack for orphan compiler */
		if (ldh.l_machine == M_PDP11 && (cp2-cp1) >= 7)
			return (1);
#endif
	}
	return (0);
}

/*
 * Compare routine for 'qsort'.
 * Actually for 'shellsort', since 'qsort'
 * eats so much stack.
 * Handles the '-n' and '-r' command
 * line options.
 */
qscmp(sp1, sp2)
register struct ldsym *sp1, *sp2;
{
	register v = 0;
	register t1 = sp1->ls_type & ~L_GLOBAL;
	register t2 = sp2->ls_type & ~L_GLOBAL;

	if (nflag) {
		if( ldh.l_flag & LF_SEP) {
			if( t1 < t2)
				v = -1;
			else if( t2 < t1)
				v = 1;
		}
		if( !v) {
			if ((unsigned long)sp1->ls_addr < sp2->ls_addr)
				v = -1;
			else if ((unsigned long)sp1->ls_addr > sp2->ls_addr)
				v = 1;
			else if( t1 == L_REF  &&  t2 != L_REF)
				v = -1;
			else if( t2 == L_REF  &&  t1 != L_REF)
				v = 1;
		}
	}
	if( !v)
		v = strncmp(sp1->ls_id, sp2->ls_id, NCPLN);
	if (rflag)
		return (-v);
	return (v);
}

