/*
 * ld/pass1.c
 *
 * One Pass Coff Loader.
 * By Charles Fiterman 3/31/92 for Mark Williams.
 */ 
#include "ld.h"

mod_t *sysHead;
static int  ifd;
static char *fname;

/*
 * Reverse bytes on the 80386 for archive pointers.
 */
#ifdef GEMDOS
#include <osbind.h>
#define flipbytes(x)
#else
flipbytes(c)
register char *c;
{
	register char t;

	t = c[0]; c[0] = c[3]; c[3] = t;
	t = c[1]; c[1] = c[2]; c[2] = t;
}
#endif

/*
 * Symbol name.
 */
char *
symName(sym, work)
register SYMENT *sym;
register char *work;
{
	if (1 == sym->n_zeroes) {	/* pointer to table record */
		sym_t *s;

		s = (sym_t *)sym->n_offset;
		sym = &(s->sym);
	}

	if (!sym->n_zeroes)		/* pointer to long symbol */
		return ((char *)sym->n_offset);

	/* name in record but may need terminator */
	memcpy(work, sym->n_name, SYMNMLEN);
	work[SYMNMLEN] = '\0';
	return (work);
}

/*
 * complain about redefined symbol
 */
void
symredef(sp, mp)
sym_t	*sp;
mod_t	*mp;
{
	/*
	 * Simple module.
	 */
	if (mp->mname[0] == '\0')
		spmsg(sp, "redefined in file '%s'", mp->fname);
	/* A symbol is defined in incompatable ways in different files. */

	else
		spmsg(sp, "redefined in file '%s': module '%.*s'",
			mp->fname, DIRSIZ, mp->mname );
	/* A symbol is defined in incompatable ways in different files. */
}

/*
 * Return reference to given symbol if any
 */
sym_t *
symref(name)
char *name;
{
	char work[SYMNMLEN + 1];
	register sym_t	*sp;
	register SYMENT *sym;

	/* Scan internal symbol table for undefined reference. */
	for (sp=symtable[crc16(name) % NHASH]; sp != NULL; sp=sp->next) {
		sym = &(sp->sym);
		if (undefined(sym) && !strcmp(symName(sym, work), name))
			break;
	}
	return(sp); /* Return reference, or NULL. */
}

/*
 * Read input files.
 */
readFile(fn, loadsw)
char *fn;	/* file name */
int  loadsw;	/* 1 = load this file, 0 = use its symbol table */
{
	register char *p;
	struct stat st;

	fname = fn;
	/*
	 * Look for rename entrys.
	 * con=atcon
	 * is not a filename with an =
	 * it means rename con to atcon.
	 * This is a drvld requirment.
	 */
	if (NULL != (p = strchr(fname, '='))) {
		ren_t	*new;

		new = alloc(sizeof(*new));
		new->next = rhead;
		rhead = new;
		new->from = fname;
		*p = '\0';
		new->to = p + 1;
		return;
	}

	/* all names must be *.[ao] */
	if (NULL == (p = strrchr(fname, '.')) || p[2])
		p = ".?";
	if (!loadsw)	/* drvld's read of the system */
		p = "_s.o" + 2;
	/* shared libs have names like libc_s.a */
	if ((p[-2] == '_') && (p[-1] == 's'))
		loadsw = 0;

#ifdef GEMDOS
	stat(fname, &st);
	ifd = qopen(fname, 0);
#else
	ifd = qopen(fname, 0);
	fstat(ifd, &st);
#endif
	switch(p[1]) {
	case 'a':
		archive(loadsw);
		break;
	case 'o':
		object("", st.st_size, loadsw);
		break;
	default:
		message("unlikely input file name '%s'", fname);
		/* Input file names must end .o for object or .a
		 * for archive. */
	}
	close(ifd);
}

/*
 * Add item to correct common
 */
static void
addComm(v)
long v;
{
	switch ((int)v & 3) {
	case 0:
		comnl += v;	/* Long aligned commons */
		break;
	case 2:
		comns += v;	/* Short aligned commons */
		break;
	default:
		comnb += v;	/* Byte aligned commons */
	}
}

/*
 * add a symbol to the symbol table.
 */
addsym(s, mp)
register SYMENT *s;
mod_t *mp;
{
	register sym_t *sp;
	ren_t *ren;
	enum state {
		local, gdef, gref, comm
	} new, old;
	int h, sec;
	char *name;
	char w1[SYMNMLEN + 1], w2[SYMNMLEN + 1];

	sec = s->n_scnum;
	switch (s->n_sclass) {
	case C_EXTDEF:
		s->n_sclass = C_EXT;
		sec = s->n_value = s->n_scnum = 0;
	case C_EXT:
		if (sec)
			new = gdef;
		else if (s->n_value)
			new = comm;
		else
			new = gref;
		break;
	case C_STAT:
		new = local;
		break;
	default:
		return;
	}

	name = symName(s, w1);
	/* check rename entrys */
	for (ren = rhead; NULL != ren; ren = ren->next) {
		if (!strcmp(ren->from, name)) {
			s->n_zeroes = 0;
			name = ren->to;
			s->n_offset = (long)name;
			break;
		}
	}
	h = crc16(name) % NHASH;

	/* Make symbols segment relative, if mp == NULL than sec == 0 */
	if (sec > 0)
		s->n_value += secth[sec - 1].s_size - mp->s[sec - 1].s_vaddr;
		
	if (local == new && 
	    (nolcl ||
	     (noilcl && (name[0] == '.') && (name[1] == 'L'))))
		return;

	for (sp = ((local == new) ? NULL : symtable[h]);
	     sp != NULL;
	     sp = sp->next) {
		if ((sp->sym.n_sclass != C_EXT) ||
		    (strcmp(symName(&(sp->sym), w2), name)))
			continue;
	
		if (sp->sym.n_scnum)
			old = gdef;
		else if (sp->sym.n_value)
			old = comm;
		else
			old = gref;

		switch (new) {
		/* case local: can't get here */

		case gref:
			s->n_offset = (long)sp;
			s->n_zeroes = 1;
			return;

		case gdef:
			switch (old) {
			case comm:
				spwarn(sp,
			"symbol defined as a common and a global");
			/* A symbol was defined as a common for example
			 * int x; and a global for example int x = 5;
			 * There is no good way to fix this without reading
			 * the code and thinking about the variable usage.
			 * The linker turned the global into an external.
			 * That is it turned int x; into extern int x;
			 * This matches the INIX linker. */
				memcpy(&(sp->sym), s, sizeof(*s));
				sp->mod = mp;
				s->n_offset = (long)sp;
				s->n_zeroes = 1;
				return;

			case gref:
				nundef--;
				memcpy(&(sp->sym), s, sizeof(*s));
				sp->mod = mp;
				s->n_offset = (long)sp;
				s->n_zeroes = 1;
				return;

			default:
				symredef(sp, mp);
				return;
			}

		case comm:
			switch (old) {
			case comm:
				s->n_offset = (long)sp;
				s->n_zeroes = 1;

				if (sp->sym.n_value == s->n_value)
					return;

				spwarn(sp,  "defined with lengths %ld and %ld",
					sp->sym.n_value,
					s->n_value);
				/* A common was defined with different lengths,
				 * while this is legal it is very unusual in
				 * C programs. This warning may be turned off
				 * with the -c flag */

				addComm(- sp->sym.n_value);
				if (sp->sym.n_value < s->n_value) {
					sp->sym.n_value = s->n_value;
					sp->mod = mp;
				}
				sp->sym.n_value += 3;
				sp->sym.n_value &= ~3L;
				addComm(sp->sym.n_value);
				return;

			case gref:
				addComm(s->n_value);
				nundef--;
				memcpy(&(sp->sym), s, sizeof(*s));
				sp->mod = mp;
				s->n_offset = (long)sp;
				s->n_zeroes = 1;
				return;

			case gdef:
				addComm(- sp->sym.n_value);
				spwarn(sp, "Defined as a common and a global");
					/* NODOC */
				sp->mod = mp;
				s->n_offset = (long)sp;
				s->n_zeroes = 1;
				return;
			}
		}
	}

	/* symbol local or not found */
	sp = alloc(sizeof(*sp));
	memcpy(&(sp->sym), s, sizeof(*s));
	sp->next = symtable[h];
	symtable[h] = sp;

	switch (new) {
	case comm:
		addComm(s->n_value);
	case local:
		break;
	case gref:
		nundef++;
	}
	sp->mod = mp;
	s->n_offset = (long)sp;
	s->n_zeroes = 1;
}

/*
 * Read input file.
 */
static void
xread(to, len)
char *to;
int len;
{
	int got;

	if (len != (got = read(ifd, to, len)))
		fatal("error reading '%s' expected %d bytes got %d",
			fname, len, got); /**/
}

/*
 * Inhale object file.
 */
object(mname, size, loadsw)
char *mname;
long size;
{
	register SYMENT *sym;
	register SCNHDR *s;
	SYMENT *endSym;
	mod_t *mp;
	long i, j, k;

	if (watch) {
		errCount--;
		modmsg(fname, mname, "adding");	/* NODOC */
	}
	mp    = alloc(sizeof(*mp));	/* allocate our header */
	mp->f = alloc((int)size);	/* allocate space for file */
	xread(mp->f, (int)size);	/* inhale file */

	if (mp->f->f_magic != C_386_MAGIC) {
		modmsg(fname, mname, "bad header");
		free(mp->f);
		free(mp);
		return;
	}

	mp->fname = newcpy(fname);
	if (*mname)
		mp->mname = newcpy(mname);
	else
		mp->mname = "";

	if (loadsw) {	/* put modules on load list */
		if (head == NULL)
			head = mp;
		else
			tail->next = mp;
		tail = mp;
		fileh.f_magic = C_386_MAGIC;
	}
	else {		/* put modules on linkto list */
		if (xhead == NULL)
			xhead = mp;
		else
			xtail->next = mp;
		xtail = mp;
	}

	/*
	 * Turn disk pointers into ram pointers.
	 */
	j = ((long)(mp->f));
	mp->s = (SCNHDR *)(sizeof(FILEHDR) + j + mp->f->f_opthdr);
	mp->f->f_symptr += j;
	mp->l = (char *)(mp->f->f_symptr + (mp->f->f_nsyms * sizeof(SYMENT)));

	/* Setup all sections */
	for (i = 0; i < mp->f->f_nscns; i++) {
		s = mp->s + i;
		s->s_scnptr += j;
		s->s_relptr += j;
		s->s_lnnoptr += j;

		for (k = 0; k < osegs; k++)
			if (!strcmp(secth[k].s_name, s->s_name))
				break;

		if ((k == osegs) && loadsw) {	/* New segment */
			w_message("adding segment '%s'", s->s_name);
			if (NULL == (secth = 
			    realloc(secth, ++osegs * sizeof(*secth))))
				fatal("out of space"); /* NODOC */
			memcpy(secth + k, s, sizeof(*s));
			secth[k].s_size = secth[k].s_nreloc = 0;
		}
	}

	/* Do all symbols */
	sym = (SYMENT *)mp->f->f_symptr;
	endSym =  sym + mp->f->f_nsyms;
	for (; sym < endSym; sym += sym->n_numaux + 1) {
		if (!sym->n_zeroes)
			sym->n_offset += (long)(mp->l);
		addsym(sym, mp);
	}

	if (loadsw) {
		/* Add to all sections */
		for (i = 0; i < mp->f->f_nscns; i++) {
			s = mp->s + i;

			for (k = 0; k < osegs; k++)
				if (!strcmp(secth[k].s_name, s->s_name))
					break;

			secth[k].s_size += s->s_size;
			if (reloc)
				secth[k].s_nreloc += s->s_nreloc;
		}
	}
}

/*
 * Read archive.
 */
archive(loadsw)
{
	struct old_hdr {
		char	ar_name[DIRSIZ];	/* Member name */
		time_t	ar_date;		/* Time inserted */
		short	ar_gid;			/* Group id */
		short	ar_uid;			/* User id */
		short	ar_mode;		/* Mode */
		fsize_t	ar_size;		/* File size */
	} arh;

	struct  ar_hdr in_arh;
	fsize_t	count, size, *ptrs;
	char	magic[SARMAG], *p;
	int	found;
	unsigned i;
	char 	*names, *name;

	xread(magic, sizeof(magic));	/* read archive magic string */

	if (memcmp(ARMAG, magic, SARMAG))
		fatal("'%s' is not a COFF archive", fname);
		/* All files ending .a should be COFF archives. */

	xread(&in_arh, sizeof(in_arh));	/* read archive header */

	memset(&arh, '\0', sizeof(arh));
	memcpy(arh.ar_name, in_arh.ar_name, DIRSIZ);
	if (NULL != (p = strchr(arh.ar_name, '/')))
		*p = '\0';

	sscanf(in_arh.ar_date, "%ld %d %d %o %ld",
		&arh.ar_date, &arh.ar_uid,
		&arh.ar_gid, &arh.ar_mode, &arh.ar_size);

	if (arh.ar_name[0])
		fatal("Library must be created with ar -s option");
		/* The \fBar \-s\fR option gives librarys a symbol table
		 * for the use of \fBld\fR. */

	/*
	 * read random libraries symbol table.
	 */
	xread(&count, sizeof(count));	/* read pointer count */
	flipbytes(&count);

	/* read file pointers */
	i = size = count * sizeof(count);
	if (i != size)
		fatal("archive '%s' is corrupt", fname);
		/* This file makes no sense as a COFF archive. */
	ptrs = alloc(i);
	xread(ptrs, i);

	/* read symbol names corresponding to pointers */
	i = size = arh.ar_size - size - sizeof(count);
	if (i != size)
		fatal("archive '%s' is corrupt", fname); /* NODOC */
	names = alloc(i);
	xread(names, i);

	/* search symbol table unitl nothing found */
	do {
		for (found = i = 0, name = names;
		     (i < count) && nundef;
		     i++, name = strchr(name, '\0') + 1) {
			if(!ptrs[i] || symref(name, 0) == NULL)
				continue;

			found = 1;	/* found something this pass */
			flipbytes(ptrs + i);
			lseek(ifd, ptrs[i], 0);
			xread(&in_arh, sizeof(in_arh));

			sscanf(in_arh.ar_date,
				"%ld %d %d %o %ld",
				&arh.ar_date, &arh.ar_uid,
				&arh.ar_gid, &arh.ar_mode,
				&arh.ar_size);

			in_arh.ar_date[0] = '\0';
			if (NULL != (p = strchr(in_arh.ar_name,'/')))
				*p = '\0';

			object(in_arh.ar_name, arh.ar_size, loadsw);

			ptrs[i] = 0;	/* don't find this again */
		}
	} while (found);

	free(ptrs);
	free(names);
}
