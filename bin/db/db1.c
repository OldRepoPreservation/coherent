/*
 * db/db1.c
 * A debugger.
 * Initialization and command line parsing.
 */

#include <stddef.h>
#include <canon.h>
#include <signal.h>
#include <sys/uproc.h>
#include "db.h"

int
main(argc, argv) int argc; char *argv[];
{
	signal(SIGINT, &arm_sigint);
	signal(SIGQUIT, SIG_IGN);
	initialize();
	setup(argc, argv);
	process();
}

/*
 * Catch and flag interrupts (SIGINT).
 */
void
arm_sigint()
{
	signal(SIGINT, &arm_sigint);
	intflag++;
}

/*
 * Canonicalize an l.out header.
 */
void
canlout()
{
	register int i;

	canint(ldh.l_magic);
	canint(ldh.l_flag);
	canint(ldh.l_machine);
	canvaddr(ldh.l_entry);
	for (i=0; i<NLSEG; i++)
		cansize(ldh.l_ssize[i]);
}

/*
 * Initialize segment formats, clear the breakpoint table,
 * invalidate the child process register image.
 */
void
initialize()
{
	strcpy(seg_format[DSEG], "w");
	strcpy(seg_format[ISEG], "i");
	strcpy(seg_format[USEG], "w");
	bpt_init();
	reg_flag = R_INVALID;
}

/*
 * Leave.
 */
void
leave()
{
	killc();
	exit(0);
}

/*
 * Open the given file.
 * If 'rflag' is set, the file is opened for read only.
 */
FILE *
openfile(name, rflag) char *name; int rflag;
{
	register FILE *fp;

#if	__I386__
again:
#endif
	if (!rflag && (fp = fopen(name, "r+w")) != (FILE *)NULL)
		return fp;
	else if ((fp = fopen(name, "r")) != (FILE *)NULL) {
		if (!rflag)
			printr("%s: opened read only", name);
		return fp;
	}
#if	__I386__
	if (strcmp(name, DEFLT_OBJ) == 0) {
		name = DEFLT_OBJ2;	/* l.out not found, try a.out */
		goto again;
	}
#endif
	panic("Cannot open %s", name);
}

/*
 * Set up segmentation for a core dump.
 * The registers are also read.
 */
void
set_core(name) char *name;
{
	register unsigned i;
	register char *cp;
	register ADDR_T size;
	register off_t offt;
	ADDR_T regl;		/* an address, int * in <sys/uproc.h> */
	int signo;
	char ucomm[U_COMM_LEN+1];
	SR usegs[NUSEG];
	int iflag;

	/* Open the core file and read the file name. */
	cfn = name;
	cfp = openfile(name, rflag);
	fseek(cfp, (long)U_OFFSET+offsetof(UPROC, u_comm[0]), SEEK_SET);
	if ((cp = lfn) != NULL && fread(ucomm, sizeof(ucomm), 1, cfp) == 1) {

		/* Compare object filename to core filename. */
		while (strchr(cp, '/') != NULL)
			cp = strchr(cp, '/') + 1;	/* skip past '/' */
		if (strncmp(cp, ucomm, sizeof(ucomm)) != 0) {
			ucomm[U_COMM_LEN] = '\0';
			printr("Core file name \"%s\" different from object file name \"%s\"",
				ucomm, lfn);
		}
	}

	/* Seek to the segment information and read it. */
	fseek(cfp, (long)U_OFFSET+offsetof(UPROC, u_segl[0]), SEEK_SET);
	if (fread(usegs, sizeof(usegs), 1, cfp) != 1)
		panic("Bad core file");

	/* Read the core signal number and register pointer. */
	fseek(cfp, (long)U_OFFSET+offsetof(UPROC, u_signo), SEEK_SET);
	if (fread(&signo, sizeof(signo), 1, cfp) != 1)
		panic("cannot read signo");
	dbprintf(("signo=%d\n", signo));
	if (fread(&regl, sizeof(regl), 1, cfp) != 1)
		panic("cannot read regl");
	regl &= 0xFFF;
	dbprintf(("regl=%d\n", regl));

	/* Set up segmentation. */
	iflag = ISPACE == DSPACE;
	map_set(USEG, MIN_ADDR, (ADDR_T)UPASIZE, (off_t)regl, MAP_CORE);
	map_clear(DSEG, endpure);
	offt = usegs[0].sr_size;
#if	__I386__
	usegs[SISTACK].sr_base -= usegs[SISTACK].sr_size;
	dbprintf(("adjust usegs[SISTACK].sr_base to %x\n", usegs[SISTACK].sr_base));
#endif
	for (i=1; i<NUSEG; i++) {
		if (usegs[i].sr_segp == (SEG *)NULL)
			continue;
		if ((~usegs[i].sr_flag) & (SRFDUMP|SRFPMAP))
			continue;
		size = usegs[i].sr_size;
		map_set(DSEG, (ADDR_T)usegs[i].sr_base, size, offt, MAP_CORE);
		offt += size;
	}
	if (iflag)
		ISPACE = DSPACE;
	get_regs(R_ALL);			/* read the registers */
	set_sig(signo);				/* and correct signal number */
}

/*
 * Set up segmentation for an ordinary file.
 * This is really easy.
 */
void
set_file(name) char *name;
{
	lfp = openfile(name, rflag);
	map_set(DSEG, MIN_ADDR, MAX_ADDR, (off_t)0, MAP_PROG);
	ISPACE = DSPACE;
}

/*
 * Setup object file "name".
 * The flag is bit mapped:
 *	1 bit	read symbol table
 *	2 bit	read segment information
 */
void
set_prog(name, flag) char *name; int flag;
{
	dbprintf(("set_prog(%s, %d)\n", name, flag));
	lfp = openfile(name, (flag&2) ? rflag : 1);
	if (fread(&coff_hdr, sizeof(coff_hdr), 1, lfp) != 1)
		panic("Cannot read object file header");
	if (coff_hdr.f_magic == C_386_MAGIC) {
		/* The object is a COFF file. */
		dbprintf(("IS_COFF!\n"));
		file_type = COFF_FILE;
		addr_fmt = ADDR_FMT;
		aop_size = 32;
	} else {
		/* Not a COFF file, might be an l.out. */
		fseek(lfp, 0L, SEEK_SET);
		if (fread(&ldh, sizeof(ldh), 1, lfp) != 1)
			panic("Cannot read object file");
		canlout();
		if (ldh.l_magic != L_MAGIC)
			panic("Bad object file");

		/* The object is an l.out file. */
		dbprintf(("IS_LOUT!\n"));
		file_type = LOUT_FILE;
		addr_fmt = ADDR16_FMT;
		aop_size = 16;
	}
	if ((flag&1) != 0 && !sflag) {
		sfp = lfp;
		if (IS_LOUT) {
			nsyms = ldh.l_ssize[L_SYM] / sizeof(struct ldsym);
			if (nsyms != 0)
				read_lout_sym((long) sizeof(ldh)
					 + ldh.l_ssize[L_SHRI] + ldh.l_ssize[L_PRVI]
					 + ldh.l_ssize[L_SHRD] + ldh.l_ssize[L_PRVD]);
		} else {
			if (coff_hdr.f_nsyms != 0)
				read_coff_sym();
		}
	}
	if ((flag&2) != 0) {
		lfn = name;
		if (IS_LOUT)
			setloutseg();
		else
			setcoffseg();
	}
}

/*
 * Setup arguments.
 */
void
setup(argc, argv) int argc; char *argv[];
{
	register char *cp;
	register int c;
	register int t;
	register int u;
	register int tflag;

	t = '\0';
	tflag = 0;

	/* Process command line switches -[cdefkorstV]. */
	for (; argc > 1; argc--, argv++) {
		cp = argv[1];
		if (*cp++ != '-')
			break;
		while ((c = *cp++) != '\0') {
			switch (c) {
			case 'c':
			case 'd':
			case 'e':
			case 'f':
			case 'k':
			case 'o':
				/* only one of [cdefko] is allowed */
				if (t != '\0')
					usage();
				t = c;
				continue;
			case 'p':
				if (argc < 3)
					usage();
				--argc;
				prompt = argv[2];
				++argv;
				continue;
			case 'r':
				rflag = 1;
				continue;
			case 's':
				sflag = 1;
				continue;
			case 't':
				tflag = 1;
				continue;
			case 'V':
				fprintf(stderr,
					"db: " MCHNAME " "
#if	DEBUG
					"DEBUG "
#endif
#ifdef	NOCANON
				/*	"NOCANON "	*/
#endif
#ifdef	NOFP
					"NOFP "
#endif
					"V%s\n", VERSION);
				continue;
			default:
				usage();
			}
		}
	}
	switch (t) {
	case '\0':
		switch (argc) {
		case 1:
			set_prog(DEFLT_OBJ, 3);
			set_core(DEFLT_AUX);
			break;
		case 2:
			set_prog(argv[1], 3);
			break;
		case 3:
			set_prog(argv[1], 3);
			set_core(argv[2]);
			break;
		default:
			usage();
		}
		break;
	case 'c':
		switch (argc) {
		case 1:
			set_prog(DEFLT_OBJ, 3);
			set_core(DEFLT_AUX);
			break;
		case 2:
			set_core(argv[1]);
			break;
		case 3:
			set_prog(argv[1], 3);
			set_core(argv[2]);
			break;
		default:
			usage();
		}
		break;
	case 'd':
		switch (argc) {
		case 1:
			set_prog("/coherent", 3);
			setdump("/dev/dump");
			break;
		case 3:
			set_prog(argv[1], 3);
			setdump(argv[2]);
			break;
		default:
			usage();
		}
		break;
	case 'e':
		if (argc < 2)
			usage();
		set_prog(argv[1], 3);
		if (startc(&argv[1], NULL, NULL, 0) == 0)
			leave();
		break;
	case 'f':
		switch (argc) {
		case 2:
			set_file(argv[1]);
			break;
		case 3:
			set_prog(argv[1], 1);
			set_file(argv[2]);
			break;
		default:
			usage();
		}
		break;
	case 'k':
		switch (argc) {
		case 1:
			set_prog("/coherent", 1);
			setkmem("/dev/mem");
			break;
		case 2:
			setkmem(argv[1]);
			break;
		case 3:
			set_prog(argv[1], 1);
			setkmem(argv[2]);
			break;
		default:
			usage();
		}
		break;
	case 'o':
		switch (argc) {
		case 1:
			set_prog(DEFLT_OBJ, 3);
			break;
		case 2:
			set_prog(argv[1], 3);
			break;
		case 3:
			set_prog(argv[1], 1);
			set_prog(argv[2], 2);
			break;
		default:
			usage();
		}
	}
	if (tflag) {
		if ((u=open("/dev/tty", 2)) < 0)
			panic("Cannot open /dev/tty");
		dup2(u, 0);
		dup2(u, 1);
		dup2(u, 2);
	}
}

/*
 * Check for interrupts and clear flag.
 */
int
testint()
{
	register int n;

	if ((n = intflag) != 0) {
		printf("Interrupted\n");
		intflag = 0;
	}
	return n;
}

/*
 * Generate a verbose usage message.
 */
void
usage()
{
	panic(
		"Usage: db [ -cdefkorst ] [ [ mapfile ] program ]\n"
		"Options:\n"
		"\t-c\tMap program as a core file\n"
		"\t-d\tMap program as a system dump; mapfile defaults to /coherent\n"
		"\t-e\tNext argument is object file and rest of command line is passed\n"
		"\t\tto the child process\n"
		"\t-f\tMap program as binary data\n"
		"\t-k\tMap program as a kernel process; mapfile defaults to /coherent\n"
		"\t-o\tprogram is an object file\n"
		"\t-p str\tArgument str is interactive command prompt (default: \"db: \")\n"
		"\t-r\tAccess all files read-only\n"
		"\t-s\tDo not load symbol table\n"
		"\t-t\tPerform input and output via /dev/tty\n"
		"\tmapfile defaults to a.out or l.out.\n"
		"\tprogram defaults to core."
	);
}

/* end of db/db1.c */
