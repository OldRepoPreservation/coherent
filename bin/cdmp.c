/*
 * Reads and list COFF files.
 */
#include <misc.h>	/* misc usefull stuff */
#include <coff.h>
#include <errno.h>

#define VHSZ 48	/* line size in vertical hex dump */

/* some shortcut display stuff */
#define show(flag, msg) if (fh.f_flags & flag) printf("\t" msg "\n");
#define cs(x) case x: printf(#x); break;
#define cx(x) case x: printf(#x " = 0x%lx ", se->n_value); break;

long	num_sections;		/* number of section */
long	section_seek;		/* used to seek first section */
long	symptr;			/* File pointer to symbol table entrys */
long	num_symbols;		/* Number of symbols */
char	*str_tab;		/* String char array */
unsigned long str_length;	/* length in bytes of string array. */
FILE *fd;			/* COFF file descriptor */
static char dswitch, rswitch, lswitch, sswitch, aswitch, uswitch;
extern char *optarg;
extern int optind;

/*
 * Print message to stderr and stdout and die.
 */
fatal(s)
char *s;
{
	int save = errno;

	fprintf(stderr, "fatal: %r\n", &s);
	if (0 != (errno = save))
		perror("errno reports");
	exit(1);
}

/*
 * Check String Data.
 */
char *
checkStr(s)
unsigned char *s;
{
	static *work = NULL;
	register unsigned char *p, c;
	int badct = 0, ct = 1;

	for (p = s; c = *p++; ct++)
		if ((c <= ' ') || (c > '~'))
			badct += 2;

	if (!badct)
		return(s);

	if (NULL != work)
		free(work);

	work = alloc(badct + ct);
	for (p = work; c = *s++;) {
		if (c > '~') {
			*p++ = '~';
			c &= 0x7f;
		}
		if (c <= ' ') {
			*p++ = '^';
			c |= '@';
		}
		*p++ = c;
	}
	return (work);
}

/*
 * Read and print optional file header.
 */
optHeader()
{
	AOUTHDR	oh;

	if (1 != fread(&oh, sizeof(oh), 1, fd))
		fatal("Error reading optional header");

	printf("\nOPTIONAL HEADER VALUES\n");
	printf("magic            = 0x%x\n", oh.magic);
	printf("version stamp    = %d\n", oh.vstamp);
	printf("text size        = 0x%lx\n", oh.tsize);
	printf("init data size   = 0x%lx\n", oh.dsize);
	printf("uninit data size = 0x%lx\n", oh.bsize);
	printf("entry point      = 0x%lx\n", oh.entry);
	printf("text start       = 0x%lx\n", oh.text_start);
	printf("data start       = 0x%lx\n", oh.data_start);
}

/*
 * Read and print file header.
 */
readHeaders(fn)
char *fn;
{
	FILEHDR	fh;

	fd = xopen(fn, "rb");

	if (1 != fread(&fh, sizeof(fh), 1, fd))
		fatal("Error reading coff header");

	printf("FILE %s HEADER VALUES\n", fn);
	printf("magic no       = 0x%x\n", fh.f_magic);
	printf("sections       = %ld\n", num_sections = fh.f_nscns);
	printf("file date      = %s", ctime(&fh.f_timdat));
	printf("sym ptr        = 0x%lx\n", symptr = fh.f_symptr);
	printf("symbols        = %ld\n", num_symbols = fh.f_nsyms);
	printf("sizeof(opthdr) = %d\n", fh.f_opthdr);
	printf("FLAGS          = 0x%x\n", fh.f_flags);
	show(F_RELFLG, "Relocation info stripped from file");
	show(F_EXEC, "File is executable");
	show(F_LNNO, "Line numbers stripped from file");
	show(F_LSYMS, "Local symbols stripped from file");
	show(F_MINMAL, "Minamal object file");

	if (fh.f_opthdr)
		optHeader();
	section_seek = sizeof(FILEHDR) + fh.f_opthdr;
}

/*
 * Process Shared library.
 */
shrLib()
{
	long i;
	char *pathn;
	SHRLIB shr;

	if (1 != fread(&shr, sizeof(shr), 1, fd))
		fatal("Error reading Library Section");

	if (shr.pathndx -= 2) {
		long j;
		char	buf[VHSZ];	/* buffer for hex dump */
		printf("\nExtra Library info");

		for (j = shr.pathndx * 4;
		     j && (i = fread(buf, 1, ((j > VHSZ) ? VHSZ : (int)j), fd));
		     j -= i) {
			if (!i)
				fatal("Unexpected EOF in .lib data");
			dump(buf, (int)i);
		}
		putchar('\n');
	}

	pathn = alloc(i = (shr.entsz - 2) * 4);
	if (1 != fread(pathn, i, 1, fd))
		fatal("Error reading Library name");
	printf("\nReferences %s\n", pathn);
}

/*
 * Process sections.
 */
readSections()
{
	SCNHDR	sh;	/* Section header structure */
	long i;

	fseek(fd, section_seek, 0);
	if (1 != fread(&sh, sizeof(SCNHDR), 1, fd))
		fatal("Error reading section header");

	section_seek += sizeof(SCNHDR);
	fseek(fd, sh.s_scnptr, 0);

	printf("\n %.8s - SECTION HEADER -\n", checkStr(sh.s_name));
	printf("physical address   = 0x%lx\n", sh.s_paddr);
	printf("virtual address    = 0x%lx\n", sh.s_vaddr);
	printf("section size       = 0x%lx\n", sh.s_size);
	printf("file ptr to data   = 0x%lx\n", sh.s_scnptr);
	printf("file ptr to reloc  = 0x%lx\n", sh.s_relptr);
	printf("file ptr to lines  = 0x%lx\n", sh.s_lnnoptr);
	printf("relocation entrys  = %u\n", sh.s_nreloc);
	printf("line entrys        = %u\n", sh.s_nlnno);
	printf("flags              = 0x%lx\n", sh.s_flags);
	printf("\t");
	switch((int)sh.s_flags) {
#if 0
	case STYP_GROUP:
		printf("grouped section"); break;
	case STYP_PAD:
		printf("padding section"); break;
	case STYP_COPY:
		printf("copy section"); break;
	case STYP_INFO:
		printf("comment section"); break;
	case STYP_OVER:
		printf("overlay section"); break;
#endif
	case STYP_LIB:
		printf(".lib section\n");
		shrLib();
		return;

	case STYP_TEXT:
		printf("text only"); break;

	case STYP_DATA:
		printf("data only"); break;

	case STYP_BSS:
		printf("bss only"); break;

	default:
		printf("unrecognized section");
		if (uswitch)
			dswitch = 0;
	}
	putchar('\n');

	if (!dswitch && strcmp(sh.s_name, ".bss")) { /* don't output bss */
		long j;
		char	buf[VHSZ];	/* buffer for hex dump */

		fseek(fd, sh.s_scnptr, 0);
		printf("\nRAW DATA");

		for (j = sh.s_size;
		     j && (i = fread(buf, 1, ((j > VHSZ) ? VHSZ : (int)j), fd));
		     j -= i) {
			if (!i)
				fatal("Unexpected EOF in %.8s data",
				      checkStr(sh.s_name));
			dump(buf, (int)i);
		}
		putchar('\n');
	}

	if (!rswitch && sh.s_nreloc) {
		fseek(fd, sh.s_relptr, 0);
		printf("\nRELOCATION ENTRYS\n");
		for (i = 0; i < sh.s_nreloc; i++) {
			RELOC	re;	/* Relocation entry structure */

			if (1 != fread(&re, RELSZ, 1, fd))
				fatal("Error reading relocation entry");

			printf("address 0x%lx\tindex %ld \ttype 0x%x\n",
				re.r_vaddr, re.r_symndx, re.r_type);
		}
	}

	if (!lswitch && sh.s_nlnno) {
		fseek(fd, sh.s_lnnoptr, 0);
		printf("\n LINE NUMBER ENTRIES\n");

		for (i = 0; i < sh.s_nlnno; i++) {
			LINENO	le;	/* Line number entry structure */

			if (1 != fread(&le, LINESZ, 1, fd))
				fatal("Error reading line number entry");

			if (le.l_lnno)
				printf("line %d at 0x%lx\n",
					le.l_lnno, le.l_addr.l_paddr);
			else
				printf("function address 0x%lx\n",
					le.l_addr.l_symndx);
		}
	}
}

/*
 * Read the string table into memory.
 * This allows the read_symbols function to work.
 */
readStrings()
{
	register unsigned char *str_ptr, c;
	long	strings;
	unsigned len;

	strings = symptr + (SYMESZ * num_symbols);
	fseek(fd, strings, 0);

	if (1 != fread(&str_length, sizeof(str_length), 1, fd))
		str_length = 0;

	if (!str_length) {
		printf("\n NO STRING TABLE\n");
		return;
	}
	printf("\n STRING TABLE DUMP\n");
	len = str_length -= 4;
	if (len != str_length)
		fatal("File is corrupt");

	str_tab = alloc(len);

	if (1 != fread(str_tab, len, 1, fd))
		fatal("Error reading string table %lx %d", ftell(fd), len);

	for (str_ptr = str_tab; str_ptr < (str_tab + str_length);) {
		putchar('\t');
		while (c = *str_ptr++) {
			if (c > '~') {
				c &= 0x7f;
				putchar('~');
			}
			if (c < ' ') {
				c |= '@';
				putchar('^');
			}
			putchar(c);
		}
		putchar('\n');
	}
}

/*
 * Read symbol table.
 */
readSymbols()
{
	SYMENT se;
	AUXENT ae;
	long	i, j;

	if (sswitch)
		return;
	fseek(fd, symptr, 0);
	printf("\n SYMBOL TABLE ENTRIES\n");
	for (i = 0; i < num_symbols; i++) {
		if (1 != fread(&se, SYMESZ, 1, fd))
			fatal("Error reading symbol entry");

		if (!lswitch)
			printf("%4ld\t", i);
		print_se(&se);

		for (j = 0; j < se.n_numaux; j++) {
			if (i >= num_symbols)
				fatal("Inconsistant sym table");

			if (1 != fread(&ae, AUXESZ, 1, fd))
				fatal("Error reading aux symbol entry");

			if (aswitch)
				continue;
			if (!lswitch)
				printf("\n%4ld\t", ++i);
			switch (se.n_sclass) {
			case C_EXT:
				if (ISFCN(se.n_type)) {
					printf("function size 0x%lx\n",
						ae.x_sym.x_misc.x_fsize);
					continue;
				}
				break;
			case C_FCN:	/* .bf or .ef */
				printf("line %d\n",
					ae.x_sym.x_misc.x_lnsz.x_lnno);
				continue;
			case C_FILE:
				printf("file name %.8s\n",
					checkStr(ae.x_file.x_fname));
				continue;
			case C_STAT:
				if (!se.n_type) {
					printf(
		"section length %lx  reloc entrys %d  line numbers %d\n",
						ae.x_scn.x_scnlen,
						ae.x_scn.x_nreloc,
						ae.x_scn.x_nlinno);
					continue;
				}
			}
			printf("AUX ENTRY DUMP");
			dump(&ae, sizeof(ae));
			putchar('\n');
		}
		putchar('\n');
	}
}

/*
 * Print symbol table entry
 */
print_se(se)
SYMENT *se;
{
	register i, c;
	char flag = 0;

	if (se->n_zeroes) { /* name in place */
		for (i = 0; i < SYMNMLEN; i++) {
			if ((flag != -1) && 
			   (' ' < (c = se->n_name[i])) &&
			   ('~' >= c))
				putchar(c);
			else {
				putchar(' ');
				if (!c)
					flag = -1;
				else if (!flag)
					flag = 1;
			}
		}
	}
	else
		printf("%s", checkStr(str_tab + se->n_offset - 4));

	printf(" section %d ", se->n_scnum);
	switch (c = (i = se->n_type) & 15) {
	cs(T_CHAR)
	cs(T_SHORT)
	cs(T_INT)
	cs(T_LONG)
	cs(T_FLOAT)
	cs(T_DOUBLE)
	cs(T_STRUCT)
	cs(T_UNION)
	cs(T_ENUM)
	cs(T_MOE)
	cs(T_UCHAR)
	cs(T_USHORT)
	cs(T_UINT)
	cs(T_ULONG)
	default:
		printf("type = %d ", c);
	}

	if (ISFCN(i))
		printf(", function ");
	if (ISPTR(i))
		printf(", pointer ");
	if (ISARY(i))
		printf(", array ");

	switch (i = se->n_sclass) {
	cx(C_NULL)
	cx(C_STAT)
	cx(C_EXTDEF)
	case C_EXT:
		if (se->n_scnum)
			printf("C_EXT value 0x%lx ", se->n_value);
		else {
			if (se->n_value)
				printf("Common length %ld ", se->n_value);
			else
				printf("External reference ");
		}
		break;
	cx(C_AUTO)
	cs(C_FILE)
	cx(C_STRTAG)
	cx(C_EOS)
	cx(C_FCN)
	cx(C_EFCN)
	default:
		printf(" class = 0x%x ", i);
	}

	switch (se->n_numaux) {
	case 1:
		printf(" 1 aux entry");
	case 0:
		break;
	default:
		printf(" %d aux entries", se->n_numaux);
	}

	putchar('\n');

	if (1 == flag) {
		printf("*** Bad data in name **\n");
		dump(se, SYMESZ);
	}
}

dump(buf, p)
char *buf;
int p; /* p is the number of bytes to dump */
{
	register int i;

	printf ("\n\n%6x ", ftell(fd) - p);

	for (i = 0; i < p; i++ )
		outc(clean(buf[i]), i, ' ');

	printf("\n       ");
	for (i = 0; i < p; i++)
		outc(hex((buf[i] >> 4) & 0x0f), i, '.');

	printf("\n       ");
	for (i = 0; i < p; i++)
		outc(hex(buf[i]& 0x0f), i, '.');

}

clean(c)
char c;
{
	if (c >= ' ' && c <= '~' )
		return c;
	else
		return '.';
}

outc( c, i, s)
int i;
char c, s;
{
	if ((i&3) == 0 && i != 0 )
		putchar(s);
	putchar(c);
}

hex(c)
char c;
{
	if ( c <= 9 )
		return c + '0';
	else
		return c + 'A' - 10;
}

/*
 * Mainline.
 */
main(argc, argv)
char *argv[];
{
	int i, c;

	while (EOF != (c = getargs(argc, argv, "drlsau?"))) {
		switch (c) {
		case 0:
			readHeaders(optarg);

			for (i = 0; i < num_sections; i++)
				readSections();
			if (num_symbols) {
				readStrings();
				readSymbols();
			}

			fclose(fd);
			break;

		case 'u':
			uswitch++; break;

		case 'd':
			dswitch++; break;

		case 'r':
			rswitch++; break;

		case 'l':
			lswitch++; break;

		case 's':
			sswitch++; break;

		case 'a':
			aswitch++; break;

		case '?':
		default:
			fprintf(stderr, "usage: cdump -drlsa filename ...\n");
			fprintf(stderr, "-d supress data dumps\n");
			fprintf(stderr, "-r supress relocation entries\n");
			fprintf(stderr, "-l supress line numbers\n");
			fprintf(stderr, "-s supress symbol entries\n");
			fprintf(stderr, "-a supress aux symbol entries\n");
			exit(1);
		}
	}

	return (0);
}
