char _version[] = "Version 1.1";
/*
 * Look at a file and try to
 * figure out its type. Knows about the various
 * flavours of filesystem entries, object files of various
 * types, C programs, input to one of the various flavours
 * of text formatter, etc.
 */
#include <stdio.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/uproc.h>
#include <n.out.h>
#include <canon.h>

/* UNIX archive magic numbers */
#define COFFARMAG	"!<arch>\n"
#define COFFARMAG_RAN	"!<arch>\n/"
#define ARMAG	0177535		/* n.out archives */
#define	UARMAG	0177545		/* UNIX v7 archives */
#define	OUARMAG	0177555		/* UNIX v6 and previous archives */

#define COMPRESSED 0x9D1F	/* Output of "compress(1)" command.  */

#define	EXEC	(S_IEXEC|(S_IEXEC<<3)|(S_IEXEC<<6))

/*
 * This is a tar header.
 */
struct  th_info {
	char	th_name[100],
		th_mode[8],
		th_uid[8],
		th_gid[8],
		th_size[12],
		th_mtime[12],
		th_check[8],
		th_islink,
		th_link[100],
		th_pad[255];
};


/* Cheezy hack: coff.h defines some symbols that conflict with n.out.h, so
 * we just define the structures and symbols we need here.
 */

/* COFF File header. */
typedef	struct	filehdr	{
	unsigned short	f_magic;		/* Magic number		*/
	unsigned short	f_nscns;		/* Number of sections	*/
	long		f_timdat;		/* Time and date	*/
	long		f_symptr;		/* Seek to symbol table	*/
	long		f_nsyms;		/* Number of symbols	*/
	unsigned short	f_opthdr;		/* Optional header size	*/
	unsigned short	f_flags;		/* Flags		*/
}	FILEHDR;

/* Magic number. */
#define	C_386_MAGIC	0x14C			/* Intel iAPX 80386	*/

/* Flags for f_flags field. */
#define	F_RELFLG	0x0001			/* No relocation info	*/
#define	F_EXEC		0x0002			/* Executable		*/
#define	F_LNNO		0x0004			/* No line numbers	*/
#define	F_LSYMS		0x0008			/* No local symbols	*/
#define F_MINMAL	0x0010			/* product of strip	*/
#define	F_AR32WR	0x0100			/* i80x86 byte order	*/
#define F_KER		0x0800			/* Loadable driver	*/

#define ISCOFF(x)	((x) == C_386_MAGIC)

/*
 * The first BUFSIZ bytes of the
 * file in question are read into this
 * union. It contains members for accessing
 * the possible file structures.
 */

union	iobuf
{
	char	u_buf[BUFSIZ];		/* General data */
	struct	ldheader u_lout;	/* L.out object file header */
	struct	filehdr u_coff;		/* COFF object file header */
	UPROC	u_u;			/* core file header */
	unsigned short u_armag;		/* Archive number */
	struct	th_info u_tar;		/* Tar header  */
};

char *type;

union	iobuf	iobuf;
char	*file();
char	*textclass();
char	*objclass();
char	*coffclass();
char	*dirtype();
char	*mtype();
char	*coffmtype();
char	*strcat();

main(argc, argv)
char *argv[];
{
	struct stat sb;
	register char *p;
	register int i;
	register int estat;

	if (argc < 2) {
		fprintf(stderr, "Usage: file name ...\n");
		exit(1);
	}
	estat = 0;
	for (i=1; i<argc; i++) {
		p = argv[i];
		if (stat(p, &sb) < 0) {
			fprintf(stderr, "file: %s: not accessible\n", p);
			estat = 1;
			continue;
		}
		printf("%s: %s\n", p, file(&sb, p));
	}
	exit(estat);
}

/*
 * Routine to guess filetype
 */
char *
file(sbp, fn)
struct stat *sbp;
char *fn;
{
	static char buf[50];
	unsigned short magic;
	register int nb;
	register int fd = -1;

	if ((sbp->st_mode&S_IFMT) != S_IFREG) {
		switch (sbp->st_mode & S_IFMT) {
		case S_IFDIR:
			return (dirtype(fn));

		case S_IFBLK:
			sprintf(buf, "block special file %d/%d",
			    major(sbp->st_rdev), minor(sbp->st_rdev));
			return (buf);

		case S_IFCHR:
			sprintf(buf, "character special file %d/%d",
			    major(sbp->st_rdev), minor(sbp->st_rdev));
			return (buf);

		case S_IFMPB:
			return ("block multiplexor file");

		case S_IFMPC:
			return ("character multiplexor file");

		case S_IFPIP:
			return ("named pipe");

		default:
			return ("invalid filetype");
		}
	}
	if ((fd = open(fn, 0)) < 0)
		return ("unreadable");
	if ((nb = read(fd, (char *) &iobuf, sizeof(iobuf))) < 0) {
		close(fd);
		return ("read error");
	}
	close(fd);
	if (nb == 0)
		return ("empty");
	/* l.out executable?  */
	if (nb >= sizeof(struct ldheader)) {
		magic = iobuf.u_lout.l_magic;
		canint(magic);
		if (magic == L_MAGIC) {
			canint(iobuf.u_lout.l_flag);
			canint(iobuf.u_lout.l_machine);
			return (objclass(fd, &iobuf.u_lout));
		}
	}
	/* COFF executable?  */
	if (nb >= sizeof(struct filehdr)) {
		magic = iobuf.u_coff.f_magic;
		if (ISCOFF(magic)) {
			return (coffclass(&iobuf.u_coff));
		}
	}

	/* COFF archive?  */
	if (nb >= strlen(COFFARMAG_RAN)) {
		if ( strncmp(COFFARMAG_RAN,
		     iobuf.u_buf,
		     strlen(COFFARMAG_RAN)) == 0) {
			return("COFF archive (ranlib)");
		}
		if ( strncmp(COFFARMAG, iobuf.u_buf, strlen(COFFARMAG)) == 0) {
			return("COFF archive");
		}
	}

	if (nb >= sizeof(struct th_info)) {
		if (	tar_path(iobuf.u_tar.th_name, 100) &&
			tar_oct(iobuf.u_tar.th_mode, 8) &&
			tar_dec(iobuf.u_tar.th_uid, 8) &&
			tar_dec(iobuf.u_tar.th_gid, 8) &&
			tar_dec(iobuf.u_tar.th_size, 12) &&
			tar_dec(iobuf.u_tar.th_mtime, 12) &&
			strlen(iobuf.u_tar.th_pad) < 8 ) {

			type = malloc(64);

			type[0] ='\0';
			if ( '\0' != iobuf.u_tar.th_pad[0]) {
				strcpy(type, iobuf.u_tar.th_pad);
			} else {
				strcpy(type, "v7 tar");
			}

			strcat(type, " archive");
			return(type);
		} /* if looks like a tar header.  */
	}

	/* core file?
	 * This is pure heuristic.  If the last signal number was legal
	 * (a 13/256 random chance) and if the command string is NUL
	 * terminated (about 1/5 random chance) and at least one long,
	 * and if all of the
	 * characters are printable (~ (96/256)^4), we assume it really
	 * is a core file.  (Pardon the crude statistics.)
	 * That all adds up to a less than 0.1% chance of incorrectly
	 * classifying a random binary file.
	 */
	if (nb >= sizeof(UPROC)) {
		if ((iobuf.u_u.u_signo < NSIG) &&	/* Legal signal? */
		    (iobuf.u_u.u_comm[9] == '\0') ) {  /* Terminated command?  */
			char *comm;
			int i;
			int len;
			
			/* Check to see if the command is printable.  */
			comm = iobuf.u_u.u_comm;
			len = strlen(comm);

			for (i=0; isprint(comm[i]) && (i < len); i++) {
				/* Do nothing.  */
			}
			/* If we go to the end, probably a core file.  */
			if (len == i && len > 0) {
				type = malloc(64);
				sprintf(type, "core file from \"%s\"", comm);
				return(type);
			}
					
		}
	}

	/* Archive or compress'ed file?  */
	if (nb >= sizeof (short)) {
		magic = iobuf.u_armag;
		canint(magic);
		if (magic == ARMAG)
			return ("n.out archive");
		if (magic == UARMAG)
			return ("seventh edition archive");
		if (magic == OUARMAG)
			return ("sixth edition archive");
		if (magic == COMPRESSED){
			type = malloc(64);
			sprintf(type, "%d bit compressed file",
					 (int) (0x7f & iobuf.u_buf[2]));
			return(type);
		}
	}

	if (hasnonascii((unsigned char *) &iobuf, nb))
		return ("binary data");
	if ((sbp->st_mode&EXEC) != 0) {
		if (strncmp(iobuf.u_buf, "#!", 2) == 0) {
			type = malloc(64);
			sprintf(type, "%s script",
				strtok(&(iobuf.u_buf[2]), " \t\n"));
			return(type);
		} else {
			return ("commands");
		}
	}
	return (textclass((unsigned char *) &iobuf, nb));
}

/*
 * Return the type of the directory.
 * Currently, only "s.*" is recognised as SCCS directory.
 */
char *
dirtype(dn)
register char *dn;
{
	register char *cp;

	for (cp=dn; *cp!='\0'; cp++)
		;
	while (cp > dn)
		if (*--cp == '/') {
			cp++;
			break;
		}
	if (cp[0]=='s' && cp[1]=='.')
		return ("SCCS directory");

	if (strcmp(cp, "RCS") == 0)
		return ("RCS directory");
		
	return ("directory");
}

/*
 * Return true if there ar characters
 * in the buffer that do not look like good
 * ascii characters.
 * This routine knows that ascii is a seven
 * bit code.
 */
hasnonascii(bp, nb)
register unsigned char *bp;
register nb;
{
	while (nb--) {
		if (!isascii(*bp))
			return (1);
		if (!(isprint(*bp) || isspace(*bp) || *bp=='\b' || *bp=='\a'))
			return (1);
		bp++;
	}
	return (0);
}

/*
 * Classify the first `nb'
 * bytes of a text file.
 */
char *
textclass(bp, nb)
register unsigned char *bp;
register nb;
{
	register int c;
	int nlf = 1;
	int nlbrace;
	int nrbrace;
	int nsharps;
	int nsemi;
	int xmail;
	char *sbp = bp;

	nlbrace = 0;
	nrbrace = 0;
	nsharps = 0;
	nsemi = 0;
	xmail = 0;
	while (nb--) {
		c = *bp++;
		if (c=='.' && strncmp(bp, "globl", 5)==0)
			return ("assembler");
		if (nlf) {
			if (c == '.')
				return ("nroff, tbl or eqn input");
			else if (c == ':')
				return ("commands");
			else if (c == '#')
				++nsharps;
			else if (c=='%')
				if (*bp=='%' || *bp=='{' || *bp=='}')
					return ("yacc or lex input");
		}
		if (c == '{')
			++nlbrace;
		else if (c == '}')
			++nrbrace;
		if (c == '\n') {
			nlf = 1;
			if (bp[-2] == ';')
				nsemi++;
			if (((bp-sbp) % 22) == 0) {
				if (bp[-2]==' ' || bp[-2]=='!')
					xmail++;
			} else
				xmail = 0;
		} else
			nlf = 0;
	}
	if (xmail)
		return ("xmail encoded text");
	if ((nsharps || nsemi) && (nlbrace || nrbrace))
		return ("C program");
	return ("probably text");
}

/*
 * Figure out the type of an l.out
 * object file. Tag it with the machine
 * id if not for the machine upon which the
 * command is running.
 */
char *
objclass(fd, lhp)
register struct ldheader *lhp;
{
	static char type[64];
	register char *mch;
	struct ldsym lds;
	register fsize_t stbase;
	register i;

	type[0] = '\0';
	if ((lhp->l_flag&LF_32) != 0)
		strcat(type, "32 bit ");
	if ((lhp->l_flag&LF_SLIB) != 0)
		strcat(type, "shared library ");
	if ((lhp->l_flag&LF_SLREF) != 0)
		strcat(type, "libref ");
	if ((lhp->l_flag&LF_SHR) != 0)
		strcat(type, "shared ");
	if ((lhp->l_flag&LF_SEP) != 0)
		strcat(type, "separate ");
	if ((lhp->l_flag&LF_KER) != 0) {
		register unsigned ssize;

		ssize = sizeof (lds);
		if ((lhp->l_flag & LF_32) == 0) {
			stbase = sizeof(*lhp) - 2*sizeof(int);
			ssize -= sizeof (int);
		} else {
			canshort(lhp->l_tbase);
			stbase = lhp->l_tbase;
		}
		for (i=L_SHRI; i<L_SYM; ++i)
			if (i!=L_BSSI && i!=L_BSSD) {
				cansize(lhp->l_ssize[i]);
				stbase += lhp->l_ssize[i];
			}
		lseek(fd, stbase, 0);
		cansize(lhp->l_ssize[L_SYM]);
		while (lhp->l_ssize[L_SYM] != 0) {
			if (read(fd, &lds, ssize) != ssize)
				break;
			canshort(lds.ls_type);
			if (strncmp(lds.ls_id, "conftab_", NCPLN) == 0
			&& (lds.ls_type&LR_SEG) != L_REF)
				break;
			lhp->l_ssize[L_SYM] -= ssize;
		}
		strcat(type, lhp->l_ssize[L_SYM]==0?"kernel ":"driver ");
	}
	strcat(type, "executable");
	if ((lhp->l_flag&LF_NRB) == 0)
		strcat(type, " with relocation");
	if ((mch = mtype(lhp->l_machine)) == NULL)
		mch = "Unknown machine type";
	sprintf(type, "%s (%s)", type, mch);
	return (type);
}

/*
 * Figure out the type of a coff
 * object file. Tag it with the machine
 * id if not for the machine upon which the
 * command is running.
 */
char *
coffclass(chp)
register struct filehdr *chp;
{
	static char type[64];
	register char *mch;

	sprintf(type, "COFF ");
	if ((chp->f_flags&F_MINMAL) != 0)
		strcat(type, "minimal ");
	if ((chp->f_flags&F_EXEC) != 0){
		if ((chp->f_flags&F_LSYMS) != 0)
			strcat(type, "stripped ");
		strcat(type, "executable ");
	} else {
		strcat(type, "object ");
		if ((chp->f_flags&F_RELFLG) != 0)
			strcat(type, "stripped relocation ");
		
		if ((chp->f_flags&F_LSYMS) != 0)
			strcat(type, "stripped symbols ");
	}

	if ((mch = coffmtype(chp->f_magic)) == NULL)
		mch = "Unknown machine type";
	sprintf(type, "%s(%s) ", type, mch);

	if (chp->f_nsyms > 0) {
		strcat(type, "not stripped ");
	}
	return (type);
}


/*
 * Identify a COFF executable.
 */
char *
coffmtype(magic)
	unsigned short magic;
{
	switch ((unsigned) magic) {

#ifdef IAPX16
	case IAPX16:
	case IAPX16TV:
	case IAPX20:
	case IAPX20TV:
		return("iAPX");
#endif

#ifdef B16MAGIC
	case B16MAGIC:
	case BTVMAGIC:
		return("Intel Basic-16");
#endif

#ifdef X86MAGIC
	case X86MAGIC:
	case XTVMAGIC:
		return("Intel x86");
#endif

#ifdef I286SMAGIC
	case I286SMAGIC:
		return("Intel 286");
#endif

#ifdef I386MAGIC
	case I386MAGIC:
		return("Intel 386");
#endif

#ifdef C_386_MAGIC
	case C_386_MAGIC:
		return("Intel 386");
#endif

#ifdef N3BMAGIC
	case N3BMAGIC:
	case NTVMAGIC:
		return("New 3B");
#endif

#ifdef WE32MAGIC
	case WE32MAGIC:
	case RBOMAGIC:
	case MTVMAGIC:
		return("MAC-32, 3515, 3B5");
#endif

#ifdef VAXWRMAGIC
	case VAXWRMAGIC:
	case VAXROMAGIC:
		return("VAX 11/780 or /750");
#endif

	case 0401:
	case 0405:
	case 0407:
	case 0410:
	case 0411:
	case 0437:
		return("pdp11");

#ifdef MC68MAGIC
	case MC68MAGIC:
	case MC68TVMAGIC:
	case M68MAGIC:
	case M68TVMAGIC:
		return("Motorola 680xx");
#endif

	/* case I286LMAGIC: */
#ifdef MC68KPGMAGIC
	case MC68KPGMAGIC:
		return("UNIX PC or iAPX 286");
#endif

#ifdef U370WRMAGIC
	case U370WRMAGIC:
	case U370ROMAGIC:
		return("IBM 370");
#endif

#ifdef AMDWRMAGIC
	case AMDWRMAGIC:
	case AMDROMAGIC:
		return("Amdahl 470/580");
#endif
	default:
		return(NULL);

	}
} /* coffmtype() */

/*
 * Decide if character array 'str' of length 'len' is an acceptable tar
 * octal field.  Returns TRUE if it is acceptable, FALSE otherwise.
 */
int
tar_oct(str, len)
	char *str;
	int len;
{
	for (; len > 0; --len, ++str) {
		if (*str != '\0' && *str != ' ' &&
		    !('0' <= *str && *str <= '7')) {
			break;
		}
	}

	return(0 == len);
} /* tar_oct() */

/*
 * Decide if character array 'str' of length 'len' is an acceptable tar
 * decimal field.  Returns TRUE if it is acceptable, FALSE otherwise.
 */
int
tar_dec(str, len)
	char *str;
	int len;
{
	for (; len > 0; --len, ++str) {
		if (*str != '\0' && *str != ' ' &&
		    !isdigit(*str)) {
			break;
		}
	}

	return(0 == len);
} /* tar_dec() */

/*
 * Decide if character array 'str' of length 'len' is an acceptable tar
 * path type field.  Returns TRUE if it is acceptable, FALSE otherwise.
 */
int
tar_path(str, len)
	char *str;
	int len;
{
	for (; len > 0; --len, ++str) {
		if (*str != '\0' && *str != ' ' &&
		    !isprint(*str)) {
			break;
		}
	}

	return(0 == len);
} /* tar_path() */
