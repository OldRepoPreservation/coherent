/*
 * fdisk.c
 * 3/20/90
 * cc -o fdisk fdisk.c query.c -f
 * Change partitioning of IBM-XT or IBM-AT hard disk.
 * Usage: /etc/fdisk [ -qrx ] [ -b bootb ] [ device ... ]
 * Options:
 *	-b	Add master boot block code from "bootb"
 *	-q	Quiet mode; the default is verbose
 *	-r	Read only
 *	-x	Use devices /dev/xt[01]x instead of /dev/at[01]x
 * If no device argument is given, fdisk supplies "/dev/[ax]t[01]x"
 * as appropriate.
 *
 * UNDONE:
 *	consistency() failure
 *	dosshrink execution
 */

#include <stdio.h>
#include <sys/fdisk.h>
#include <sys/hdioctl.h>

#define	USAGE	"Usage: /etc/fdisk [ -qrx ] [ -b mboot ] [ device ... ]\n"
#define	VERSION	"2.0"

/* Conversions: c:h:s to/from sector number, sectors to (double) megabytes. */
#define	SSIZE	512			/* sector size */
#define	chs_to_sec(c,h,s) ((((unsigned long)(c)*nheads) + (h)) * nspt + (s) - 1)
#define	meg(sec)	(((double)(sec)) * SSIZE / 1000000L)
#define	sec_to_c(sec)	((unsigned)((sec) / (nheads * nspt)))
#define	sec_to_h(sec)	((unsigned)(((sec) / nspt) % nheads))
#define	sec_to_s(sec)	((unsigned)(((sec) % nspt) + 1))

/* Externals. */
extern	long	lseek();
extern	void	qsort();

/* Functions. */
void		change_active();
void		change_logical();
void		change_part();
void		consistency();
void		fatal();
void		fdisk();
int		get_boot();
void		get_uint();
void		get_ulong();
int		pcompare();
void		print_part();
void		quit();
void		sanity();
void		unused();
void		usage();

/* Globals. */
int		badflag;	/* Partition table is bad.		*/
unsigned char	*defargs[3] = { "/dev/at0x", "/dev/at1x", NULL };
int		freepart;	/* Free partition.			*/
unsigned long	freesize;	/* Free size.				*/
unsigned long	freestart;	/* First free sector.			*/
HDISK_S		hd;		/* Structure to house boot block.	*/
unsigned int	nspt;		/* Number of sectors per track.		*/
unsigned int	ncyls;		/* Number of cylinders.			*/
HDISK_S		newhd;		/* Structure to house new boot block.	*/
unsigned int	nheads;		/* Number of heads per track.		*/
int		nmods;		/* Modifications to the table.		*/
unsigned long	nsectors;	/* Total sectors.			*/
char		*mboot;		/* Name of new master boot file.	*/
int		openmode = 2;	/* Default open mode: read/write.	*/
int		qflag;		/* Quiet mode; default is verbose.	*/

main(argc, argv) int argc; char *argv[];
{
	register char *s;
	int fd0, fd1;

	/* Sanity check. */
	if (sizeof hd != SSIZE)
		fatal("invalid HDISK_S size %u != %u", sizeof hd, SSIZE);

	while (argc > 1 && **++argv == '-') {
		--argc;
		for (s = &argv[0][1]; *s; ++s) {
			switch(*s) {
			case 'b':
				if (argc < 2)
					usage();
				mboot = *++argv;
				break;
			case 'q':
				++qflag;
				break;
			case 'r':
				openmode = 0;
				break;
			case 'V':
				fprintf(stderr, "/etc/fdisk: V%s\n", VERSION);
				break;
			case 'x':
				argv[0][5] = argv[1][5] = 'x';
				break;
			default:
				usage();
				break;
			}
		}
	}
	if (openmode == 0 && mboot != NULL)
		fatal("cannot specify both 'b' and 'r' options");
	if (--argc == 0) {
		/* No arguments specified, try defaults. */
		argv = defargs;
		fd0 = open(argv[0], 0);
		fd1 = open(argv[1], 0);
		if (fd1 >= 0) {
			++argc;
			close(fd1);
		} else
			argv[1] = NULL;
		if (fd0 >= 0) {
			++argc;
			close(fd0);
		} else
			++argv;
		if (argc == 0)
			fatal("cannot open default devices");
	}
	if (!qflag) {
		printf(
"This program lets you change hard disk partition information.\n"
"A disk drive can be divided into one to four logical partitions.\n"
"You can change the active partition (the partition which your\n"
"system boots by default) or change the layout of logical partitions.\n"
			);
		if (argc > 1) {
			printf("Since your system includes %d hard disk drives,\n", argc);
			printf("you can change the partition information for each drive.\n");
		}
		printf("\n");
	}
	while (*argv != NULL)
		fdisk(*argv++);
	exit(0);
}

/*
 * Change the active partition.
 */
void
change_active()
{
	int active, oactive, i;

	active = oactive = -1;
	for (i=0; i < NPARTN; i++) 
		if (hd.hd_partn[i].p_boot == 0x80) {
			hd.hd_partn[i].p_boot = 0;	/* make inactive */
			active = oactive = i;		/* remember old */
		}
	if (active == -1)
		active = 0;				/* default */
	get_uint("Active partition", &active, 0, NPARTN-1);
	hd.hd_partn[active].p_boot = 0x80;		/* make active */
	if (active != oactive)
		++nmods;
}

/*
 * Change logical partition information.
 */
void
change_logical()
{
	unsigned action, p;	

	printf("Possible actions:\n");
	printf("\t1 = Change attributes of one partition\n");
	printf("\t2 = Change attributes of all partitions\n");	
	printf("\t3 = Return to main menu\n");
	action = 3;
	get_uint("Action", &action, 1, 3);
	switch (action) {
	case 1:
		p = (freepart != -1) ? freepart : 0;
		get_uint("Which partition", &p, 0, NPARTN - 1);
		change_part(p);
		printf("\n");
		break;
	case 2:
		for (p=0; p < NPARTN; p++)
			change_part(p);
		printf("\n");
		break;
	default:
		break;
	}
}

/*
 * Change the table entry for logical partition n.
 */
void
change_part(n) int n;
{
	register FDISK_S *p;
	int sys, old, flag;
	unsigned int c, h, s;
	unsigned long size, osize, base, obase, end;

	p = &hd.hd_partn[n];
	printf("\nPartition %d:\n", n);
	size = p->p_size;
	if (size != 0L) {
		printf("Partition %d currently begins at sector %lu\n", n, p->p_base);
		printf("(cylinder %u, head %u, sector %u).\n", bcyl(p), bhd(p), bsec(p));
		printf("It contains %ld sectors (%.2f megabytes).\n", size, meg(size));
	}
			
	/* Display possible system types. */
	printf("Operating system types:\n");
	printf("\t0  = <Unused>\n");
	printf("\t9  = Coherent\n");
	printf("\tn  = Others\n");

	/* Get new system type. */
	old = p->p_sys;
	sys = (size != 0L) ? old : 9;
	get_uint("Operating system type", &sys, 0, 255);
	p->p_sys = sys;
	if (sys != old)
		++nmods;

	/* Specify the base. */
getbase:
	obase = p->p_base;
	base = (size != 0L) ? obase : (freesize != 0) ? freestart : nspt;
	get_ulong("Base sector", &base, 1L, nsectors - 1);
	if (base != obase) {
		c = sec_to_c(base);
		h = sec_to_h(base);
		s = sec_to_s(base);
		if (s != 1) {
			printf("For efficiency, partitions should start at a track boundary.\n");
			printf("Base sector %lh does not fall on a track boundary.\n", base);
			printf("The next track boundary is at sector %lu.\n",
				chs_to_sec(c, h+1, 1));
			flag = 'n';
			queryc("Do you want to change the base sector", &flag);
			if (flag == 'y')
				goto getbase;
		}
		++nmods;
		p->p_base = base;
		p->p_bcyl = c & 0xFF;
		p->p_bhd = h;
		p->p_bsec = ((c >> 2) & CYLMASK ) | s;
	}
			
	/* Specify the length. */
getsize:
	osize = size;
	size = (size != 0L) ? size : (freesize != 0) ? freesize : nsectors - base;
	get_ulong("Partition size", &size, 0L, nsectors - base);
	if (size != osize) {
		end = base + size - 1;
		c = sec_to_c(end);
		h = sec_to_h(end);
		s = sec_to_s(end);
		if (s != nspt) {
			printf("For efficiency, partitions should end at a track boundary.\n");
			printf("The partition does not end at a track boundary with the size you selected.\n");
			printf("A partition of size %lu ends at the previous track boundary.\n",
				chs_to_sec(c, h, 0) - base + 1);
			printf("A partition of size %lu ends at the next track boundary.\n",
				chs_to_sec(c, h, nspt) - base + 1);
			flag = 'n';
			queryc("Do you want to change the partition size", &flag);
			if (flag == 'y')
				goto getsize;
		}
		++nmods;
		p->p_size = size;
		p->p_ecyl = c & 0xFF;
		p->p_ehd = h;
		p->p_esec = ((c >> 2) & CYLMASK ) | s;
	}
}

/*
 * Check the consistency of a partition table entry.
 */
void
consistency(p) FDISK_S *p;
{
	unsigned int c, h, s;
	int flag;

	if (p->p_size == 0)
		return;				/* empty entry */
	flag = 0;
	c = bcyl(p);
	h = bhd(p);
	s = bsec(p);
	if (c >= ncyls) {
		printf("bad cylinder number %u in partition table\n");
		++flag;
	}
	if (h >= nheads) {
		printf("bad head number %u in partition table\n");
		++flag;
	}
	if (s == 0 || s > nspt) {
		printf("bad sector number %u in partition table\n");
		++flag;
	}
	if (p->p_base != chs_to_sec(c, h, s)) {
		printf("bad start sector %u:%u:%u in partition table\n",
			c, h, s);
		++flag;
	}
	c = ecyl(p);
	h = ehd(p);
	s = esec(p);
	if (c >= ncyls) {
		printf("bad cylinder number %u in partition table\n");
		++flag;
	}
	if (h >= nheads) {
		printf("bad head number %u in partition table\n");
		++flag;
	}
	if (s == 0 || s > nspt) {
		printf("bad sector number %u in partition table\n");
		++flag;
	}
	if (p->p_base + p->p_size - 1 != chs_to_sec(c, h, s)) {
		printf("bad end sector %u:%u:%u in partition table\n",
			c, h, s);
		++flag;
	}
	if (flag) {
		/* UNDONE: do something about it. */
	}
}

/*
 * Print a fatal error message and die.
 */
void
fatal(args) char *args;
{
	fprintf(stderr, "/etc/fdisk: %r\n", &args);
	exit(1);
}

/*
 * Print/change configuration for given device.
 */
void
fdisk(device) char *device;
{
	hdparm_t	hdparms;
	int 		fd, nfd;
	unsigned	action;

	nmods = 0;
	fd = get_boot(device, openmode, &hd);		/* read boot */
	if (mboot != NULL) {
		nfd = get_boot(mboot, 0, &newhd);	/* read new boot */
		close(nfd);
		if (newhd.hd_sig != HDSIG)
			fatal("invalid signature in \"%s\"", mboot);
		memcpy(hd.hd_boot, newhd.hd_boot, sizeof hd.hd_boot);
		nmods++;
	}

	/* Obtain and print drive characteristics. */
	if (ioctl(fd, HDGETA, (char *)&hdparms) == -1)
		fatal("cannot get \"%s\" drive characteristics", device);
	ncyls = (hdparms.ncyl[1] << 8) | hdparms.ncyl[0];
	nheads = hdparms.nhead;
	nspt = hdparms.nspt;
	nsectors = (long)ncyls * nheads * nspt;
	if (!qflag) {
		printf("Disk %s has %u cylinders, %u heads, and %u sectors per track.\n",
			device, ncyls, nheads, nspt);
		printf("It contains a total of %lu sectors containing %d bytes each,\n",
			nsectors, SSIZE);
		printf("or a total of %ld bytes (%.2f megabytes).\n",
			nsectors * SSIZE, meg(nsectors));
		printf("\n");
	}

	/* If no signature, zap the partition entries. */
	if (hd.hd_sig != HDSIG) {
		memset(hd.hd_partn, 0, sizeof hd.hd_partn);
		hd.hd_sig = HDSIG;
		nmods++;
	}

	/* If readonly, print information and return. */
	if (openmode == 0) {
		print_part();
		quit(device, fd);
		return;
	}

	/* Interactive input loop. */
	for (;;) {
		print_part();
		printf("Possible actions:\n");
		printf("\t1 = Change active partition\n");
		printf("\t2 = Change logical partition attributes\n");
		printf("\t3 = Quit\n");
		action = 3;
		get_uint("Action", &action, 1, 3);

		switch(action) {
		case 1:
			printf("Change active partition:\n");
			change_active();
			continue;	
		case 2:
			printf("Change logical partition attributes:\n");
			change_logical();
			continue;
		case 3:
			quit(device, fd);
			return;
		default:
			continue;
		}
	}	
}

/*
 * Read boot block from a file into the given structure.
 * Return a file descriptor.
 */
int
get_boot(name, mode, hdp) char *name; HDISK_S *hdp;
{
	int	fd;

	/* Open the file. */
	if ((fd = open(name, mode)) < 0)
		fatal("cannot open \"%s\"", name);
	/* Read the current boot block into the hd structure. */
	if (read(fd, hdp, sizeof hd) != sizeof hd) {
		close(fd);
		fatal("read error on \"%s\"", name);
	}
	return fd;
}

/*
 * Prompt for unsigned int input from the user.
 * Accept data in range min to max.
 * Store the result through dp.
 */
void
get_uint(prompt, dp, min, max) char *prompt; register unsigned int *dp; unsigned min, max;
{
	unsigned int defval;

	for (defval = *dp; ; *dp = defval) {
		if (queryu(prompt, dp) >= 0 && *dp >= min && *dp <= max)
			return;
		printf("Enter a value between %u and %u.\n", min, max);
	}
}

/*
 * Prompt for unsigned long input from the user.
 * Accept data in range min to max.
 * Store the result through dp.
 */
void
get_ulong(prompt, dp, min, max) char *prompt; register unsigned long *dp; unsigned long min, max;
{
	unsigned long defval;

	for (defval = *dp; ; *dp = defval) {
		if (queryl(prompt, dp) >= 0 && *dp >= min && *dp <= max)
			return;
		printf("Value must be between %lu and %lu\n", min, max);
	}
}

/*
 * Compare two partition table entries.
 * Called by qsort.
 * The result is sorted by base, with empty entries at the end.
 */
int
pcompare(pp1, pp2) FDISK_S **pp1, **pp2;
{
	register FDISK_S *p1, *p2;

	p1 = *pp1;
	p2 = *pp2;
	if (p1->p_size == 0)
		return (p2->p_size == 0) ? 0 : 1;
	else if (p2->p_size == 0)
		return -1;
	else if (p1->p_base < p2->p_base)
		return -1;
	else if (p1->p_base == p2->p_base)
		return 0;
	else
		return 1;
}

/* 
 * Output partition information.
 */
void
print_part()
{
	register FDISK_S *p;

	if (!qflag)
		printf("The current logical partitions are:\n\n");
	printf("Number\tType\t\t   Base\tSectors\tMegabytes\n");
	for (p = &hd.hd_partn[0]; p < &hd.hd_partn[NPARTN]; ++p) {
		printf("%d", p - &hd.hd_partn[0]);
		printf("%s\t", (p->p_boot == 0x80) ? " Boot" : "");
		switch (p->p_sys) {
		case SYS_EMPTY:		printf("<Empty>\t\t");	break;
		case SYS_DOS_12:
		case SYS_DOS_16:	printf("MS-DOS\t\t");	break;
		case SYS_XENIX:		printf("Xenix\t\t");	break;
		case SYS_COH:		printf("Coherent\t");	break;
		case SYS_SWAP:		printf("Swap\t\t");	break;
		default:		printf("%u\t\t", p->p_sys);	break;
		};
		printf("%7ld\t", p->p_base);
		printf("%7ld\t", p->p_size);
		printf("%7.2f", meg(p->p_size));
#if	0
		printf("  %3u:%u:%u", bcyl(p), bhd(p), bsec(p));
		printf("  %3u:%u:%u", ecyl(p), ehd(p), esec(p));
#endif
		printf("\n");
	}
	sanity();
	printf("\n");
}

/*
 * Done.
 * If changes, prompt for confirmation and save.
 */
void
quit(fname, fd) char *fname; int fd;
{
	char flag;

	if (nmods != 0) {
		flag = 'n';
		queryc("\nSave changes", &flag);
		if (flag == 'y') {
			if (lseek(fd, 0L, 0) != 0L)
				fatal("seek failed on \"%s\"", fname);
			else if (write(fd, &hd, sizeof hd) != sizeof hd)
				fatal("write error on \"%s\"", fname);
			sync();
			printf("Changes saved to \"%s\".\n", fname);
		} else
			printf("Changes not saved.\n");
	} 
	close(fd);
}

/*
 * Check a partition table for sanity.
 * Sort the partitions, look for gaps and overlaps.
 */
void
sanity()
{
	register int i;
	FDISK_S *p[NPARTN];
	unsigned long base, next, size;

	badflag = 0;
	for (i = 0; i < NPARTN; i++) {
		p[i] = &hd.hd_partn[i];
		consistency(p[i]);
	}
	qsort(p, NPARTN, sizeof p[0], pcompare);
	freepart = -1;
	freesize = freestart = 0;
	next = 1;		/* next block available after boot sector */
	for (i = 0; i < NPARTN; i++) {
		base = p[i]->p_base;
		size = p[i]->p_size;
		if (size == 0) {
			if (freepart == -1)
				freepart  = i;
			break;
		}
		if (base < next) {
			if (next == 1)
				printf("Partition overlaps boot sector.\n");
			else
				printf("Partitions overlap starting at %lu.\n", base);
			++badflag;
		} else if (base != next) {
			if (i == 0 && (base == nspt || base == nheads * nspt))
				;	/* first partition at 0:1:1 or 1:0:1 */
			else
				unused(base, next);
		}
		next = base + size;
	}
	if (next != nsectors + 1)
		unused(nsectors, next);
	return badflag;
}

/*
 * Report unused sectors.
 */
void
unused(base, next) unsigned long base, next;
{
	printf("%lu sectors (%.2f megabytes) are unused starting at sector %lu.\n",
		base - next, meg(base - next), next);
	if (freesize < base - next) {
		freesize = base - next;
		freestart = next;
	}
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

/* end of fdisk.c */
