/*
 * One pass Coff Loader.
 * Acts as ld and drvld. Will be mkshrlb when the specs are done.
 *
 * By Charles Fiterman for Mark Williams 3/30/92.
 */
#include "ld.h"
#include <signal.h>
#include <ctype.h>

flag_t	reloc,	/* Combine input into a new .o not an executable */
	nosym,	/* No symbol table out. */
	watch,	/* Produce a trace */
	noilcl, /* Discard C local symbols beginning .L */
	nolcl,	/* Discard all local symbols */
	qflag, /* No warn on commons of different length */
	auxflg,	/* Pass through aux symbols */
	drvld,	/* Called as drvld */
	fource = -1;	/* halt on error */

int	errCount;
int	nundef;
mod_t	*head,			/* head of list of modules to load */
	*tail,			/* tail or list of modules */
	*xhead,			/* head of noload modules */
	*xtail;			/* tail of noload modules */
ren_t	*rhead;			/* rename list */

char *ofname = "a.out";		/* output file name */
long comnb, comns, comnl;	/* common lengths */
char *entrys;			/* entry string */
sym_t	*symtable[NHASH];	/* hashed symbol table */

int osegs = NLSEG;			/* the number of output segments */
FILEHDR fileh;
AOUTHDR aouth;
SCNHDR  *secth;		/* output segments */

static long str_length;
static FILE *ofd;
char *argv0;

/*
 * Write or die.
 */
xwrite(loc, size)
char *loc;
{
	if (size != write(ofd, loc, size))
		fatal("write error"); /**/
}

/*
 * Init segment descriptors.
 */
void
initSegs()
{
	secth = alloc(sizeof(*secth) * NLSEG);
	strcpy(secth[S_TEXT].s_name, _TEXT);
	secth[S_TEXT].s_flags = STYP_TEXT;
	strcpy(secth[S_DATA].s_name, _DATA);
	secth[S_DATA].s_flags = STYP_DATA;
	strcpy(secth[S_BSSD].s_name, _BSS);
	secth[S_BSSD].s_flags = STYP_BSS;
}

/*
 * Come here on interrupt.
 */
byebye()
{
	driver_fail(&aouth);
	exit(1);
}

/*
 * We are drvld not ld. Ask the system where to put the stuff.
 */
getsys()
{
	register int i;
	long bsz, daddr;

	time(&fileh.f_timdat);
	fileh.f_nscns = osegs = NLSEG;

	daddr = sizeof(fileh) + (sizeof(* secth) * osegs) +
		(fileh.f_opthdr = sizeof(aouth));
	
	for (i = 0; i < S_BSSD; i++) {	
		secth[i].s_scnptr = daddr;
		daddr += secth[i].s_size;
	}
	bsz = secth[S_BSSD].s_size;
	aouth.tsize = secth[S_TEXT].s_size;
	aouth.dsize = secth[S_DATA].s_size + bsz;
	signal(SIGKILL, byebye);
	if (!driver_alloc(&aouth))
		fatal("kernel interface failed");
		/* This will become more elaborite when the kernel is done */
	secth[S_TEXT].s_vaddr = secth[S_TEXT].s_paddr = aouth.text_start;
	secth[S_DATA].s_vaddr = secth[S_DATA].s_paddr = aouth.data_start;	
	secth[S_BSSD].s_vaddr = secth[S_BSSD].s_paddr = aouth.data_start + bsz;
	aouth.dsize -= aouth.bsize = bsz;

	if (!nosym)
		fileh.f_symptr = daddr;
}

/*
 * Set virtual bases in array of segments.
 */
baseall()
{
	register int i;
	long daddr, vaddr, size, inc;

	time(&fileh.f_timdat);
	fileh.f_nscns = osegs;

	size = 0;
	daddr = sizeof(fileh) + (sizeof(* secth) * osegs);
	if (reloc) {
		inc = 3;		/* object file */
		vaddr = 0;
	}
	else {
		inc = 15;
		daddr += fileh.f_opthdr = sizeof(aouth);
		if (fileh.f_flags & F_KER)	/* kernel */
			vaddr = KERBASE;
		else				/* executable */
			vaddr = daddr;
	}

	/* set s_vaddr, s_paddr, s_size, and s_scnptr for all segments */
	for (i = 0; i < osegs; i++) {
		switch (i) {
		case S_TEXT:
			aouth.text_start = secth[i].s_vaddr = vaddr;
			aouth.tsize = size = secth[i].s_size;		
			break;

		case S_DATA:	/* data adjustment */
			if (!reloc) {
				if (fileh.f_flags & F_KER)
					vaddr = (vaddr + 0x0fff) & ~0x0fffL;
				else
					vaddr = (vaddr & 0x0fff) + DATABASE;
			}

			aouth.data_start = secth[i].s_vaddr = vaddr;
			aouth.dsize = size = secth[i].s_size;		
			break;

		case S_BSSD: /* round up bssd */
			vaddr = (vaddr + inc) & ~inc;

			secth[i].s_paddr = secth[i].s_vaddr = vaddr;
			vaddr += aouth.bsize = secth[i].s_size;
			secth[i].s_scnptr = 0;
			continue;

		default:
			secth[i].s_vaddr = vaddr;
			size = secth[i].s_size;
		}

		secth[i].s_paddr = secth[i].s_vaddr;
		secth[i].s_scnptr = daddr;
		daddr += size;
		vaddr += size;
	}

	/* set relocations for all segments */
	for (i = 0; i < osegs; i++) {
		if (!reloc || (S_BSSD == i) || !secth[i].s_nreloc) {
			secth[i].s_relptr = 0;
			continue;
		}
		secth[i].s_relptr = daddr;
		daddr += secth[i].s_nreloc * RELSZ;
	}

	if (!nosym)
		fileh.f_symptr = daddr;
}

/*
 * return entry point
 */
static long
lentry(str)
char	*str;
{
	if (NULL == str)	/* not user specified */
		return(aouth.text_start);

	/* If an octal number use that */
	{
		register unsigned char	c, *s;
		long	oaddr = 0;

		/* Try to scan whole string as octal. */
		for (s = str; c = *s++;) {
			if (('0' > c) || (c > '7'))
				break;
			oaddr = (oaddr * 8) + (c - '0');
		}

		if (!c)
			return (oaddr);
	}

	/* Search for string in symbol table. */
	{
		register sym_t	*sp;
		char work[SYMNMLEN + 1];

		for (sp = symtable[crc16(str) % NHASH];
		     sp != NULL;
		     sp = sp->next) {
			if ((sp->sym.n_sclass == C_EXT) &&
			    !strcmp(symName(&(sp->sym), work), str)) {
				if (sp->sym.n_scnum != (S_TEXT + 1))
				   message("entry point '%s' not in .text");
				return(sp->sym.n_value);
			}
		}

		message("entry point '%s' undefined", str); /**/
		return (aouth.text_start);
	}
}

/*
 * Define referenced special symbols.
 */
void
symBind(sn, ldrv, name, loc)
int	sn, ldrv;
char	*name;
long	loc;
{
	register SYMENT *sym;
	register sym_t *sp;
	char  work[SYMNMLEN + 1];

	for (sp = symtable[crc16(name) % NHASH]; sp != NULL; sp = sp->next) {
		sym = &(sp->sym);
		if ((sym->n_sclass != C_EXT) ||
		     strcmp(symName(sym, work), name))
			continue;

		if (undefined(sym)) {
			nundef--;
			sym->n_scnum = sn + 1;
			sym->n_value = loc;
			sp->mod = NULL;	/* not defined in any mod */
			return;
		}

		if (ldrv)
			return;
		else
			spwarn(sp, "redefines builtin symbol");
			/* Some symbols such as __end and __end_text
			 * are special to the linker. In general symbols
			 * beginning __ are reserved to implementors and
			 * should be avoided by users.
			 * Your definition has been used. */
	}
}

/*
 * Add reference to symbol table
 */
void
undef(s)
char *s;
{
	SYMENT	lds;

	memset(&lds, '\0', sizeof(lds));
	lds.n_offset = (long)s;		/* point symbol at our stuff */
	lds.n_sclass = C_EXTDEF;	/* mark undefined */
	addsym(&lds, NULL);		/* connect to no module */
}

/*
 * Show undefined symbols.
 */
showUndef(sp, sym)
register sym_t *sp;
register SYMENT *sym;
{
	if (undefined(sym))
		spmsg(sp, ""); /* NODOC */
}

/*
 * Pass all symbols through a function.
 */
void
allSym(fun)
int (*fun)();
{
	register SYMENT *sym;
	register sym_t *sp;
	register mod_t *mp;
	register i;
	SYMENT *symEnd;

	/*
	 * Do symbols connected to modules in module order.
	 * Only process a global symbol for it's owner.
	 */
	for (mp = head; mp != NULL; mp = mp->next) {
		sym = ((SYMENT *)(mp->f->f_symptr));
		symEnd = sym + mp->f->f_nsyms;
		for (; sym < symEnd; sym += sym->n_numaux + 1) {
			if (1 == sym->n_zeroes) { /* pointer to original */
				sp = (sym_t *)sym->n_offset;
				if (sp->mod == mp)
					(*fun)(sp, &(sp->sym));
			}
			else
				(*fun)(NULL, sym);
		}
	}

	/*
	 * Do symbols not connected to a module.
	 * These are end symbols and symbols defined with -u option
	 */
	for (i = 0; i < NHASH; i++)
		for (sp = symtable[i]; NULL != sp; sp = sp->next)
			if (NULL == sp->mod)
				(*fun)(sp, &(sp->sym));
}

/*
 * Fixup a symbol between passes.
 */
symFix(sp, sym)
sym_t *sp;
register SYMENT *sym;
{
	int segn, len;

	segn = sym->n_scnum;
	if (!reloc && common(sym)) {
		switch ((len = sym->n_value) & 3) {
		case 2:	/* 2 byte aligned */
			sym->n_value = comns;
			comns += len;
			break;
		case 0:	/* 4 byte aligned */
			sym->n_value = comnl;
			comnl += len;
			break;
		default: /* unaligned */
			sym->n_value = comnb;
			comnb += len;
		}
		sym->n_scnum  = S_BSSD + 1;
	}
	else if (segn > 0)
		sym->n_value += secth[segn - 1].s_vaddr;

	if (NULL != sp && !nosym)
		sp->symno = fileh.f_nsyms++;
}

/*
 * Do work between passes.
 */
void
betweenPass()
{
	if (reloc)
		fileh.f_flags |= F_LNNO | F_AR32WR;
	else {
		fileh.f_flags |= F_RELFLG | F_LNNO | F_EXEC | F_AR32WR;
		comnb += (4 - ((comnb + comns) & 3)) & 3;
		secth[S_BSSD].s_size += comnb + comns + comnl;
	}
	if (nosym)
		fileh.f_flags |= F_LSYMS;

	if (drvld)
		getsys();	/* get segment base information from system */
	else
		baseall();	/* compute segment base information */

	if (!reloc) {
		int i;

		/* define referenced end of segment symbols */
		for (i = 0; i < osegs; i++) {
			char end_name[20], c, *p;

			sprintf(end_name, "__end%.8s", secth[i].s_name);
			for (p = end_name; c = *p; p++)
				if (!isalnum(c))
					*p = '_';

			symBind(i, drvld, end_name, secth[i].s_size);
		}
		/* define absolute end symbol */
		symBind(S_BSSD, drvld, "__end", secth[S_BSSD].s_size);

		/* get starting addresses for 1, 2 and 4 alligned commons */
		comnb = secth[S_BSSD].s_vaddr + secth[S_BSSD].s_size - comnb;
		comns = comnb - comns;
		comnl = comns - comnl;
	}

	if (nundef && !reloc) {
		message("the following symbols are undefined");
		errCount--;
		allSym(showUndef);
	}

	switch (errCount & fource) {
	case 0:
		break;
	case 1:
		fatal("pass 1, 1 error"); /* NODOC */
	default:
		fatal("pass 1, %d errors", errCount);
		/* At the end of pass 1 there were \fIn\fB errors detected.
		 * The link stopped here. */
	}

	/* Run through symbol table doing fixups */
	fileh.f_nsyms = 0;
	allSym(symFix);

	aouth.entry = lentry(entrys);
}

/*
 * output symbol table.
 */
static
outputSym(s, sm)
register sym_t *s;
register SYMENT *sm;
{
	int i;
	char *name, work[SYMNMLEN + 1];
	SYMENT sym;

	if (NULL == s)
		return;

	memcpy(&sym, sm, sizeof(sym));
	name = symName(sm, work);
	if (SYMNMLEN < (i = strlen(name))) {
		sym.n_offset = str_length;
		str_length += i + 1;
	}
	else
		memcpy(sym.n_name, name, SYMNMLEN);

	if (auxflg && sym.n_numaux) {
		SYMENT *aux;

		xwrite(&sym, SYMESZ, 1);
		aux = (SYMENT *)sym.n_offset;
		xwrite(aux + 1, SYMESZ, 1);
	}
	else {
		sym.n_numaux = 0;
		xwrite(&sym, SYMESZ, 1);
	}
}

/*
 * output long symbols.
 */
static
longSym(s, sm)
register sym_t *s;
register SYMENT *sm;
{
	int i;
	char *name, work[SYMNMLEN + 1];

	if (NULL == s)
		return;

	name = symName(sm, work);
	if (SYMNMLEN < (i = strlen(name)))
		xwrite(name, i + 1, 1);
}

/*
 * Do relocations
 */
relocations(mp, segn)
mod_t *  mp;
{
	register RELOC *rel;
	char	*t;	/* actual text */
	SCNHDR	*isgp, *orsp;
	unsigned i;
	long	size, told, fixr;

	isgp = mp->s + segn;
	if (!(size = isgp->s_size))
		return;
	orsp = secth + segn;
	fixr = isgp->s_vaddr - orsp->s_vaddr;
	t = (char *)isgp->s_scnptr;

	if (watch) {
		errCount--;
		mpmsg(mp, "relocating seg#%d[%06lx]@%06lx to %06lx r %ld",
			segn,
			size,
			isgp->s_vaddr,
			orsp->s_vaddr,
			isgp->s_nreloc); /* NODOC */
		told = lseek(ofd, 0, 2);
	}
	
	for (i = 0; i < isgp->s_nreloc; i++) {
		char *ptr;
		char *mtype;
		sym_t *sp;
		SYMENT *s, *sym;
		long relf, w, at;
		long	workl;
		int   undef;
		short	works;
		char work[SYMNMLEN + 1], *name;
		static char *pcrel = "pcrel";

		/* get reloc record */
		rel = ((RELOC *)isgp->s_relptr) + i;

		w = rel->r_vaddr - isgp->s_vaddr;
		if ((w < 0) || (w > size)) {
			mpmsg(mp, "relocation out of range %lx", w);
			/* A relocation record points outside the
			 * range of its segment. */
			continue;
		}
		ptr = t + w;

		s = (SYMENT *)mp->f->f_symptr + (int)rel->r_symndx;
		if (1 == s->n_zeroes) {	/* fixed elsewhere */ 
			sp = (sym_t *)s->n_offset;
			sym = &(sp->sym);
		} else {
			sp = NULL;
			sym = s;
		}
		relf = sym->n_value;
		mtype = "rel";
		undef = undefined(sym);				

		if (watch) {
			at = told + w;
			name = symName(sym, work);
		}
		/*
		 * This wierdness is to deal with a coff wierdness.
		 * The address of a common is incremented by the
		 * length of the common as seen in that module.
		 */
		if (common(s))
			relf -= s->n_value;
		/*
		 * If the symbol is native to this module
		 * the reference already has this modules
		 * segment address. Subtract it.
		 */
		else if (!undef && ((NULL == sp) || (sp->mod == mp)))
			relf -= mp->s[s->n_scnum - 1].s_vaddr;

		if (reloc) {
			rel->r_vaddr = w + orsp->s_vaddr;
			rel->r_symndx = sp->symno;

			switch (rel->r_type) {
			case R_PCRBYTE:
				mtype = pcrel;
				relf = fixr;

			case R_RELBYTE:
				w = *ptr;
				*ptr = w + relf;
				break;

			case R_PCRWORD:
				mtype = pcrel;
				relf = fixr;

			case R_DIR16:
			case R_RELWORD:
				memcpy(&works, ptr, 2);
				w = works;
				works += relf;
				memcpy(ptr, &works, 2);
				break;

			case R_PCRLONG:
				mtype = pcrel;
				relf = fixr;

			case R_DIR32:
			case R_RELLONG:
				memcpy(&workl, ptr, 4);
				w = workl;
				workl += relf;
				memcpy(ptr, &workl, 4);
				break;

			case R_NONREL:
				mtype = "nonrel";
				memcpy(&workl, ptr, 4);
				w = workl;
				relf = 0;
				break;

			default:
			mpmsg(mp, "unknown r_type %d in segment %d record %d",
				rel->r_type, segn, i);
			/* Unknown type on COFF relocation record. */
			}

			if ((sym->n_scnum > 0) && (mtype != pcrel))
				rel->r_symndx = sym->n_scnum - 1;

			if (!undef && (sym->n_scnum < 0))
				rel->r_type = R_NONREL;

			w_message("%lx '%s'(%d %lx %lx) %lx = '%s'(%lx) + %lx",
				at,
				mtype,
				rel->r_type,
				rel->r_vaddr,
				rel->r_symndx,
				relf + w,
				name,
				sym->n_value,
				w);
		}
		else {
			switch (rel->r_type) {
			case R_PCRBYTE:
				mtype = pcrel;
				relf -= orsp->s_vaddr;
			case R_RELBYTE:
				w = *ptr;
				*ptr = w + relf;
				break;
			case R_PCRWORD:
				mtype = pcrel;
				relf -= orsp->s_vaddr;
			case R_RELWORD:
			case R_DIR16:
				memcpy(&works, ptr, 2);
				w = works;
				works += relf;
				memcpy(ptr, &works, 2);
				break;
			case R_PCRLONG:
				mtype = pcrel;
				relf -= orsp->s_vaddr;
			case R_RELLONG:
			case R_DIR32:
				memcpy(&workl, ptr, 4);
				w = workl;
				workl += relf;
				memcpy(ptr, &workl, 4);
				break;
			case R_NONREL:
				mtype = "nonrel";
				memcpy(&workl, ptr, 4);
				w = workl;
				relf = 0;
				break;
			default:
			mpmsg(mp, "unknown r_type %d in segment %d record %d",
				rel->r_type, segn, i);
			/* NODOC */
			}
			w_message("%lx '%s'(%d) %lx = '%s'(%lx) + %lx",
				at,
				mtype,
				rel->r_type,
				relf + w,
				name,
				sym->n_value,
				w);
		}
	}
	orsp->s_vaddr += size;
	xwrite(t, (int)size, 1);
}

/*
 * Output all data.
 */
outputAll()
{
	register mod_t *mp;
	register SCNHDR *scn;
	int i;
	struct stat statbuf;

	/* Open output file */
	ofd = qopen(ofname, 1);

	/* write header */
	if (watch)
		w_message("before header %lx", lseek(ofd, 0, 2));
	xwrite(&fileh, sizeof(fileh), 1);

	if (!reloc) {	/* if executable set bits and write opt header */
		stat(ofname, &statbuf);
		chmod(ofname,
			statbuf.st_mode | S_IEXEC|(S_IEXEC>>3)|(S_IEXEC>>6));

		aouth.magic = Z_MAGIC;
		xwrite(&aouth, sizeof(aouth));
	}

	/* write sector headers */
	if (watch)
		w_message("before sect headers %lx", lseek(ofd, 0, 2));
	xwrite(secth, sizeof(* secth) * osegs);

	/* write corrected text segments */
	for (i = 0; i < osegs; i++)
		for (mp = head; mp != NULL; mp = mp->next)
			if ((S_BSSD != i) && (mp->f->f_nscns > i))
				relocations(mp, i);

	/* write relocation if required */
	for (i = (reloc ? 0 : osegs); i < osegs; i++) {
		for (mp = head; mp != NULL; mp = mp->next) {
			if ((S_BSSD == i) || (mp->f->f_nscns <= i))
				continue;
			scn = mp->s + i;
			if (!scn->s_nreloc)
				continue;
			xwrite(scn->s_relptr, RELSZ * (int)scn->s_nreloc);
		}
	}

	if (!nosym) {
		str_length = 4;

		if (watch)
			w_message("before symbols %ld", lseek(ofd, 0, 2));
		allSym(outputSym);
		if (4 != str_length) {
			if (watch)
				w_message("before long syms %ld", 
					lseek(ofd, 0, 2));
			xwrite(&str_length, sizeof(str_length));
			allSym(longSym);
		}
	}

	close(ofd);
}

/*
 * Process arguements, and call other work.
 */
main(argc, argv)
char *argv[];
{
	int	c;
	char	*specialList = NULL;
	char	*argString = "?ae:finKl:L:o:rsu:wXxZ:q";

	/* We use a lot of storage, this makes malloc() faster */
	free(alloc(1024 * 256));

	/* find program name */
	if (NULL == (argv0 = strrchr(argv[0], '/')))
		argv0 = argv[0];
	else
		argv0++;

	/*
	 * drvld is an alternative name for ld.
	 * In this mode ld will load the kernel's symbol table, so
	 * that the loadable driver can link directly to kernel services.
	 * After pass1 we call driver_alloc() to tell the kernel the
	 * sizes of the driver segments, the kernel replys with locations
	 * for them. Then we output the driver to a tmp file and execl()
	 */
	if (!strcmp(argv0, "drvld")) {
#if 0
		ofname = tmpnam(NULL);
#endif
		argString = "?ae:l:L:u:wd";
		fileh.f_flags |= F_KER;
		drvld = 1;
		/* read kernel for symbol table but don't load */
		readFile(kernelName(), 0);
		_addargs("DRVLD", &argc, &argv);	
	}
	else
		_addargs("LD", &argc, &argv);	

	initSegs();

	while (EOF != (c = getargs(argc, argv, argString))) {
		switch (c) {
		case 0:		/* Not an option, read a file for load */
			readFile(optarg, 1);
			continue;

		case 'f':	/* attempt link even if errors */
			fource = 0;
			continue;

		case 'a':	/* save extra segments and aux symbols */
			auxflg ^= 1;
			continue;

		case 'e':
			entrys = optarg;

		case 'Z':	/* use and erase after */
		case 'i':	/* obselete options */
		case 'n':
			continue;

		case 'K':	/* recompile of Kernel */
			fileh.f_flags |= F_KER;
			continue;

		case 'L':
			/*
			 * Special filelist for lookup.
			 */
			{
				char *new;

				if (NULL == specialList) {
					new = alloc(strlen(optarg) + 2);
					sprintf(new, "%c%s", LISTSEP, optarg);
				}
				else {
					new = alloc(
					 strlen(specialList)+strlen(optarg)+2);
					sprintf(new, "%s%c%s",
					 specialList, LISTSEP, optarg);
					free(specialList);
				}
				specialList = new;
				continue;
			}

		case 'l':
			/* -l<lib>: use standard lib */
			{
				char *xp, *lp;

				xp = alloc(strlen(optarg) + 6);
				sprintf(xp, "lib%s.a", optarg);
				lp = NULL;
				if (NULL != specialList)
					lp = pathn(xp, NULL, specialList, "r");
				if (NULL == lp)
					lp = pathn(xp, "LIBPATH",
						DEFLIBPATH, "r");
				if (NULL == lp)
				   fatal("can't find '%s'", xp);
				   /* Can't locate requested library */

				readFile(lp, 1);
				free(xp);
				continue;
			}

		case 'o':
			ofname = optarg;
			continue;

		case 'r':
			reloc ^= 1;	/* retain relocation information */
			continue;

		case 's':
			nosym ^= 1;
			continue;

		case 'u':
			undef(optarg);
			continue;

		case 'q':
			qflag = 1;
			continue;

		case 'w':
			if (watch ^= 1)
				w_message("Watch messages on");
			else
				w_message("Watch messages off");
			continue;

		case 'X':
			noilcl ^= 1;
			continue;

		case 'x':
			nolcl ^= 1;
			continue;

		case '?':
			help();		/* non returning */

		default:
			usage();	/* non returning */
		}
	}
	if (nosym)
		nolcl = 1;

	if (reloc)
		nosym = nolcl = noilcl = 0;

	if (!fileh.f_magic)
		fatal("No work");
	/* There were no object files loaded. */

	betweenPass();	/* between passes */
	outputAll();	/* output all data */

	/* repass argument list to erase any -Z files */
	for (optix = 1; EOF != (c = getargs(argc, argv, argString));)
		if ('Z' == c)
			unlink(optarg);

#if 0
	if (drvld) {
		execl(ofname, ofname, NULL); /* should never return */
		fatal("cannot execute loadable driver '%s'", ofname); /**/
	}
#endif
	return (0);
}
