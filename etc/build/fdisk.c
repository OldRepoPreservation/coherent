/*
 * fdisk.c
 * 4/4/90
 * cc -o fdisk fdisk.c query.c -f
 * Change partitioning of IBM-XT or IBM-AT hard disk.
 * Usage: /etc/fdisk [ -rvx ] [ -b bootb ] [ device ... ]
 * Options:
 *	-b	Add master boot block code from "bootb"
 *	-r	Read only
 *	-v	Print c:h:s start and end values
 *	-x	Use devices /dev/xt[01]x instead of /dev/at[01]x
 * If no device argument is given, fdisk supplies "/dev/[ax]t[01]x"
 * as appropriate.
 *
 * UNDONE:
 *	allow <Esc> to return to main menu options
 */

#include <stdio.h>
#include <sys/fdisk.h>
#include <sys/hdioctl.h>

#define	USAGE	"Usage: /etc/fdisk [ -rvx ] [ -b mboot ] [ device ... ]\n"
#define	VERSION	"2.4"
#define	NBUF	256		/* buffer size			*/
#define	SSIZE	512		/* sector size			*/

/*
 * Conversions.
 * (unsigned) c:h:s to (ulong) sectors,
 * (ulong) sectors to (unsigned) c:h:s.
 */
#define	chs_to_sec(c,h,s) ((((unsigned long)(c)*nheads) + (h)) * nspt + (s) - 1)
#define	sec_to_c(sec)	((unsigned)((sec) / cylsize))
#define	sec_to_h(sec)	((unsigned)(((sec) / nspt) % nheads))
#define	sec_to_s(sec)	((unsigned)(((sec) % nspt) + 1))
/* (ulong) Sectors to (double) megabytes. */
#define	meg(sec)	(((double)(sec)) * SSIZE / 1000000L)
/* (ulong) Sectors to (unsigned) cylinders and tracks, rounding up. */
#define	sec_upto_c(sec)	(sec_to_c((sec) + nspt * nheads - 1))
#define	sec_upto_t(sec)	((unsigned)(((sec) + nspt - 1) / nspt))

/* Externals. */
extern	long	lseek();
extern	char	*malloc();
extern	void	qsort();

/* Functions. */
void		change_active();
void		change_part();
void		check_chs();
void		cls();
void		dos_shrink();
void		drive_info();
void		fatal();
void		fdisk();
int		get_boot();
void		get_uint();
void		get_ulong();
int		pcompare();
void		print_part();
int		quit();
void		sanity();
void		unused();
void		usage();

/* Globals. */
char		*argv0;		/* Command name, for error messages.	*/
int		badflag;	/* Partition table is bad.		*/
char		buf[NBUF];	/* Input buffer.			*/
int		cylflag;	/* Specify base and size in cylinders.	*/
unsigned int	cylsize;	/* Cylinder size in sectors.		*/
unsigned char	*defargs[3] = { "/dev/at0x", "/dev/at1x", NULL };
unsigned char	*device;	/* Partition table device name.		*/
unsigned char	*drivename;	/* Disk drive name.			*/
int		freepart;	/* Free partition.			*/
unsigned long	freesize;	/* Free size.				*/
unsigned long	freestart;	/* First free sector.			*/
HDISK_S		hd;		/* Structure to house boot block.	*/
int		megflag;	/* Specify sizes in megabytes.		*/
unsigned int	nspt;		/* Number of sectors per track.		*/
unsigned int	ncyls;		/* Number of cylinders.			*/
HDISK_S		newhd;		/* Structure to house new boot block.	*/
unsigned int	nheads;		/* Number of heads per track.		*/
int		nmods;		/* Modifications to the table.		*/
unsigned long	nsectors;	/* Total sectors.			*/
char		*mboot;		/* Name of new master boot file.	*/
int		openmode = 2;	/* Default open mode: read/write.	*/
int		partbase;	/* Partition number base (0 or 4).	*/
int		rflag;		/* Readonly.				*/
int		vflag;		/* Print c:h:s start and end values.	*/

main(argc, argv) int argc; char *argv[];
{
	register char *s;
	int fd0, fd1;

	/* Sanity check. */
	argv0 = argv[0];
	if (sizeof hd != SSIZE)
		fatal("invalid HDISK_S size %u != %u", sizeof hd, SSIZE);
	while (argc > 1 && **++argv == '-') {
		--argc;
		for (s = &argv[0][1]; *s; ++s) {
			switch(*s) {
			case 'b':
				if (argc-- < 2)
					usage();
				mboot = *++argv;
				break;
			case 'r':
				++rflag;
				openmode = 0;
				break;
			case 'v':
				++vflag;
				break;
			case 'V':
				fprintf(stderr, "%s: V%s\n", argv0, VERSION);
				break;
			case 'x':
				defargs[0][5] = defargs[1][5] = 'x';
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
		/* No arguments specified, take defaults. */
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
	cls(0);
	printf(
		"This program lets you change partition information for each disk drive.\n"
		"A disk drive can be divided into one to four logical partitions.\n"
		"You can change the active partition (the partition which your\n"
		"system boots by default) or change the layout of logical partitions.\n"
		"Other programs which change hard disk partition information\n"
		"may list logical partitions in a different order.\n"
		);
	while ((device = *argv++) != NULL)
		fdisk();
	exit(0);
}

/*
 * Change the active partition.
 */
void
change_active()
{
	int active, oactive, i, flag;

	active = oactive = -1;
	for (i=0; i < NPARTN; i++) 
		if (hd.hd_partn[i].p_boot == 0x80) {
			hd.hd_partn[i].p_boot = 0;	/* make inactive */
			active = oactive = i;		/* remember old */
		}
	flag = 'y';
	queryc("Do you want to make a partition active", &flag);
	if (flag == 'n') {
		active = -1;
		if (active != oactive)
			++nmods;
		return;
	}
	if (active == -1)
		active = 0;				/* default */
	active += partbase;
	get_uint("Active partition", &active, partbase, partbase + NPARTN-1);
	active -= partbase;
	hd.hd_partn[active].p_boot = 0x80;		/* make active */
	if (active != oactive)
		++nmods;
}

/*
 * Interactively change the table entry for logical partition n.
 * Grunge city.
 */
void
change_part(n) int n;
{
	register FDISK_S *p;
	int sys, old, flag;
	unsigned int c, h, s;
	unsigned long size, osize, base, obase, end;
	static int optflag;

	/* Get options first time through. */
	if (optflag == 0) {
		++optflag;
		cls(0);
		printf(
			"Existing data on a partition will be lost if you change the\n"
			"base or the size of the partition.  Be sure you have backed up\n"
			"all data from any partition which you are going to change.\n"
			"\n"
			"You may specify partition bases in cylinders or in tracks.\n"
			);
		flag = 'y';
		queryc("Do you want to specify bases in cylinders", &flag);
		cylflag = (flag == 'y');
		printf("You may specify partition sizes in %s or in megabytes.\n",
			cylflag ? "cylinders" : "tracks");
		flag = 'n';
		queryc("Do you want to specify sizes in megabytes", &flag);
		megflag = (flag == 'y');
	}
	p = &hd.hd_partn[n];
	printf("\nPartition %d:\n", n + partbase);
	size = p->p_size;
			
	/* Display possible system types. */
	printf("Operating system types:\n");
	printf("\t%d  = <Empty>\n", SYS_EMPTY);
	printf("\t%d  = Coherent\n", SYS_COH);
	printf("\tn  = Others\n");

	/* Get new system type. */
	old = p->p_sys;
	if (size == 0L)
		sys = SYS_COH;
	else {
		sys = old;
		printf("The current operating system type is %d.\n", sys);
	}
	get_uint("Operating system type", &sys, 0, 255);
	if (sys != old) {
		++nmods;
		p->p_sys = sys;
	}

getbase:
	/* Specify the base. */
	/* Default: old or first free or track 1. */
	obase = p->p_base;
	base = (size != 0L) ? obase : (freesize != 0) ? freestart : nspt;
	if (cylflag) {				/* in cylinders */
		base = sec_to_c(base);
		get_ulong("Base cylinder", &base, 0L, (long) ncyls - 1);
		if (base == 0)
			base = nspt;		/* skip first track for cyl 0 */
		else
			base *= nspt * nheads;	/* cylinders to sectors */
	} else {				/* in tracks */
		base = sec_upto_t(base);
		get_ulong("Base track", &base, 1L, (long)ncyls * nheads - 1);
		base *= nspt;			/* tracks to sectors */
	}

	/* Check that base falls at a track boundary. */
	/* This should only happen if the disk was previously partitioned. */
	c = sec_to_c(base);
	h = sec_to_h(base);
	s = sec_to_s(base);
	if (s != 1) {
		printf("Partitions should begin at a track boundary.\n");
		printf("The partition does not begin at a track boundary with the selected base.\n");
		printf("The next track boundary is at track %u\n", sec_upto_t(base));
		flag = 'y';
		queryc("Do you want to change the partition base", &flag);
		if (flag == 'y')
			goto getbase;
	}

	/* Update the partition table base and start information. */
	if (base != obase) {
		++nmods;
		p->p_base = base;
		p->p_bcyl = c & 0xFF;
		p->p_bhd = h;
		p->p_bsec = ((c >> 2) & CYLMASK ) | s;
	}

	/* Specify the partition size. */
	/* Default size: free block size, old size, largest possible. */
	osize = size;
	size = (base == freestart) ? freesize : (osize != 0L) ? osize : nsectors - base;
	if (megflag) {				/* in megabytes */
		size = meg(size);
		if ((long)meg(nsectors - base) == 0) {
			printf("Less than a megabyte of space remains.\n");
			size = nsectors - base;
		} else {
			get_ulong("Partition size in megabytes", &size, 0L,
				(long) meg(nsectors - base));
			size *= 1000000L;	/* megabytes to bytes */
			size /= SSIZE;		/* to sectors */
			size = sec_upto_t(size); /* round up to tracks */
			size *= nspt;		/* tracks to sectors */
		}
	} else if (cylflag) {			/* in cylinders */
		/* Tricky stuff again. */
		end = base + size - 1;
		size = sec_to_c(end) - sec_to_c(base) + 1;
		get_ulong("Partition size in cylinders", &size, 0L,
			(long) ncyls - sec_to_c(base));
		size *= nspt * nheads;		/* cylinders to sectors */
		/*
		 * Adjust size to end at cylinder boundary
		 * if it did not start at cylinder boundary.
		 */
		if (size != 0 && base % cylsize != 0)
			size -= base % cylsize;
	} else {				/* in tracks */
		size = sec_upto_t(size);
		get_ulong("Partition size in tracks", &size, 0L,
			(long) sec_upto_t(nsectors - base));
		size *= nspt;			/* tracks to sectors */
	}

	/* Check the size. */
	if (base + size > nsectors)
		size = nsectors - base;		/* roundup too big */
	end = base + size - 1;
	c = sec_to_c(end);
	h = sec_to_h(end);
	s = sec_to_s(end);
	if (s != nspt) {
		printf("Partitions should end at a track boundary.\n");
		printf("A partition with %u more sectors would end at a track boundary.\n", nspt - s);
		printf("Do you want to add %u sectors", nspt - s);
		flag = 'y';
		queryc("to the partition size", &flag);
		if (flag == 'y') {
			size += nspt - s;
			s = nspt;
		}
	}

	/* Update the partition table size. */
	if (size != osize) {
		++nmods;
		p->p_size = size;
		p->p_ecyl = c & 0xFF;
		p->p_ehd = h;
		p->p_esec = ((c >> 2) & CYLMASK ) | s;
	}
	printf("\n");
}

/*
 * Check a c:h:s entry in the partition table for consistency.
 * Try correcting any inconsistency found, with warning to the user.
 * The flag is 1 for beginning, 0 for end.
 */
void
check_chs(p, flag) FDISK_S *p; int flag;
{
	unsigned int c, h, s, nc, nh, ns;
	unsigned long n;

	if (flag) {
		c = bcyl(p);
		h = bhd(p);
		s = bsec(p);
		n = p->p_base;
	} else {
		c = ecyl(p);
		h = ehd(p);
		s = esec(p);
		n = p->p_base + p->p_size - 1;
	}
	if (c >= ncyls
	 || h >= nheads
	 || (s == 0 || s > nspt)
	 || n != chs_to_sec(c, h, s)) {
		nc = sec_to_c(n);
		nh = sec_to_h(n);
		ns = sec_to_s(n);
		cls(1);
		printf("According to the hard disk controller, the disk contains\n");
		printf("%u cylinders (0 to %u), %u heads (0 to %u), and %u sectors\n",
			ncyls, ncyls - 1, nheads, nheads -1, nspt);
		printf("per track (1 to %u).  According to the partition table, a partition\n",
			nspt);
		printf("%s at sector %lu, which corresponds to a c:h:s of %u:%u:%u.\n",
			(flag) ? "begins" : "ends" , n, nc, nh, ns);
		printf("But the partition table entry gives a c:h:s of %u:%u:%u.\n",
			c, h, s);
		printf("This program will change the c:h:s of the entry to %u:%u:%u\n",
			nc, nh, ns);
		printf("to resolve this inconsistency.  If you feel this change is\n");
		printf("incorrect, exit from this program without saving the\n");
		printf("partition table to the disk.\n");
		flag = 'n';
		queryc("Do you want to exit from this program", &flag);
		if (flag == 'y')
			exit(1);
		++nmods;
		if (flag) {
			p->p_bcyl = nc & 0xFF;
			p->p_bhd = nh;
			p->p_bsec = ((nc >> 2) & CYLMASK ) | ns;
		} else {
			p->p_ecyl = nc & 0xFF;
			p->p_ehd = nh;
			p->p_esec = ((nc >> 2) & CYLMASK ) | ns;
		}
	}
}

/*
 * Clear the IBM-AT console screen.
 */
void
cls(flag) register int flag;
{
	if (flag || rflag) {
		printf("\nHit <Enter> to continue...");
		fflush(stdout);		
		fgets(buf, sizeof buf, stdin);
	}
	putchar(0x1B);
	putchar('[');
	putchar('2');
	putchar('J');
	fflush(stdout);
}

/*
 * Shrink an MS-DOS partition.
 * PFM.
 */
void
dos_shrink(fd, n) int fd, n;
{
	int flag;

	cls(0);
	printf(
		"You can sometimes shrink an existing MS-DOS partition to make room for\n"
		"a COHERENT partition if your disk is entirely allocated to MS-DOS.\n"
		"This program will attempt to shrink the MS-DOS partition without destroying\n"
		"the data on it.  However, you should BACK UP ALL DATA from the MS-DOS\n"
		"partition to diskettes before you try to shrink it.\n"
		);
	if (freepart == -1) {
		printf("%s", drivename);
		printf(
			" does not contain an unused partition.  Shrinking an MS-DOS\n"
			"partition will create additional free space on the disk, but there\n"
			"is currently no partition table entry available for the freed space.\n"
			);
		flag = 'n';
		queryc("Do you want to shrink the MS-DOS partition anyway", &flag);
		if (flag == 'n')
			return;
	}

	/* Go for smoke. */
	sprintf(buf, "/etc/dosshrink %s %d %s\n", device, n, device);
	buf[strlen(buf) - 2] = 'a' + n;
	if (system(buf) != 0) {
		printf("Shrinking of MS-DOS partition failed.\n");
		return;
	}

	/* Read the partition table again to get the changed entry. */
	if (lseek(fd, 0L, 0) != 0L)
		fatal("%s: seek failed", device);
	else if (read(fd, &newhd, sizeof hd) != sizeof hd) {
		close(fd);
		fatal("%s: read error", device);
	} else
		memcpy(&hd.hd_partn[n], &newhd.hd_partn[n], sizeof(FDISK_S));
}

/*
 * Print drive information.
 */
void
drive_info()
{
	printf("%s has %u cylinders, %u heads, and %u sectors per track.\n",
		drivename, ncyls, nheads, nspt);
	printf("It contains:\n");
	printf("\t%u cylinders of %lu bytes each,\n",
		ncyls, (long)cylsize * SSIZE);
	printf("\t%u tracks of %lu bytes each,\n",
		ncyls * nheads, (long)nspt * SSIZE);
	printf("\t%lu sectors of %d bytes each,\n",
		nsectors, SSIZE);
	printf("or a total of %ld bytes (%.2f megabytes).\n",
		nsectors * SSIZE, meg(nsectors));
}

/*
 * Print a fatal error message and die.
 */
void
fatal(args) char *args;
{
	fprintf(stderr, "%s: %r\n", argv0, &args);
	exit(1);
}

/*
 * Print/change configuration for given device.
 */
void
fdisk()
{
	hdparm_t	hdparms;
	int 		fd, nfd, p, flag;
	unsigned	action;
	char		drive;

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

	/* Obtain drive characteristics. */
	if (ioctl(fd, HDGETA, (char *)&hdparms) == -1)
		fatal("cannot get \"%s\" drive characteristics", device);
	ncyls = (hdparms.ncyl[1] << 8) | hdparms.ncyl[0];
	nheads = hdparms.nhead;
	nspt = hdparms.nspt;
	cylsize = nheads * nspt;
	nsectors = (long)ncyls * cylsize;
	drive = device[strlen(device) - 2];
	partbase = 0;
	if (drive == '0')
		drivename = "Drive 0";
	else if (drive == '1') {
		drivename = "Drive 1";
		partbase = 4;
	} else
		drivename = "The disk";

	/* If no signature, zap the partition entries. */
	if (hd.hd_sig != HDSIG) {
		memset(hd.hd_partn, 0, sizeof(FDISK_S));
		hd.hd_sig = HDSIG;
		nmods++;
	}

	/* If readonly, print information and return. */
	if (openmode == 0) {
		print_part(0);
		close(fd);
		return;
	}

	/* Interactive input loop. */
	for (flag = 1; ; ) {
		print_part(flag);
		flag = 0;
		printf(
			"Possible actions:\n"
			"\t1 = Change active partition\n"
			"\t2 = Change one logical partition\n"
			"\t3 = Change all logical partitions\n"
			"\t4 = Shrink an MS-DOS logical parition\n"
			"\t5 = Display drive information\n"
			"\t6 = Quit\n"
			);
		action = 6;
		get_uint("Action", &action, 1, 6);

		switch(action) {
		case 1:
			printf("Change active partition:\n");
			change_active();
			continue;
		case 2:
		case 4:
			p = (freepart != -1) ? freepart : 0;
			p += partbase;
			get_uint("Which partition", &p, partbase, partbase + NPARTN - 1);
			p -= partbase;
			if (action == 2)
				change_part(p);
			else {
				dos_shrink(fd, p);
				flag = 1;
			}
			continue;
		case 3:
			for (p=0; p < NPARTN; ) {
				change_part(p++);
				if (p < NPARTN)
					print_part(0);
			}
			continue;

		case 5:
			cls(0);
			drive_info();
			flag = 1;
			continue;
		case 6:
			if (quit(device, fd) == 1)
				return;
			continue;
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
		printf("Enter a value between %lu and %lu.\n", min, max);
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
print_part(flag) int flag;
{
	register FDISK_S *p;
	register char c, *s;
	int i;
	unsigned long end;

	cls(flag);
	printf("%s currently has the following logical partitions:\n", drivename);
	printf("                     Cylinders             Tracks\n");
	printf("Number     Type   Start  End  Size  Start    End   Size Megabytes  Name\n");
	for (i = 0; i < NPARTN; ++i) {
		p = &hd.hd_partn[i];
		if (p->p_size == 0L)
			end = p->p_base = 0L;
		else
			end = p->p_base + p->p_size - 1;
		printf("%d", partbase + i);
		printf("%s\t", (p->p_boot == 0x80) ? " Boot" : "");
		s = NULL;
		switch (p->p_sys) {
		case SYS_EMPTY:		s = "<Empty>";	break;
		case SYS_DOS_12:
		case SYS_DOS_16:
		case SYS_DOS_LARGE:
					s = "MS-DOS";	break;
		case SYS_DOS_XP:
					s = "Ext.DOS";	break;
		case SYS_XENIX:		s = "Xenix";	break;
		case SYS_COH:		s = "Coherent";	break;
		case SYS_SWAP:		s = "Swap";	break;
		default:				break;	
		};
		if (s == NULL)
			printf("%8u ", p->p_sys);
		else
			printf("%8s ", s);
		printf("%5u ", sec_to_c(p->p_base));
		printf("%5u ", sec_to_c(end));
		printf("%5u ", sec_upto_c(p->p_size));
		printf("%6lu ", p->p_base / nspt);
		printf("%6lu ", end / nspt);
		printf("%6u ", sec_upto_t(p->p_size));
		printf("%7.2f ", meg(p->p_size));
		s = &device[strlen(device) - 1];
		c = *s;
		*s = 'a' + i;
		printf("%10s ", device);
		*s = c;
		if (vflag) {
			printf("%3u:%u:%u ", bcyl(p), bhd(p), bsec(p));
			printf("%3u:%u:%u ", ecyl(p), ehd(p), esec(p));
		}
		printf("\n");
	}
	sanity();
	printf("\n");
}

/*
 * Done.
 * If changes, prompt for confirmation and save.
 * Return 1 to quit, 0 to not quit.
 */
int
quit(fname, fd) char *fname; int fd;
{
	char flag;

	if (badflag) {
		printf("Because the partition table defines overlapping disk\n");
		printf("partitions, it will not be saved to the disk if you quit.\n");
		flag = 'y';
		queryc("Do you wish to quit without saving the changes", &flag);
		if (flag == 'n')
			return 0;
	} else if (nmods != 0) {
		flag = 'n';
		queryc("\nSave changes", &flag);
		if (flag == 'y') {
			if (lseek(fd, 0L, 0) != 0L)
				fatal("seek failed on \"%s\"", fname);
			else if (write(fd, &hd, sizeof hd) != sizeof hd)
				fatal("write error on \"%s\"", fname);
			sync();
		} else
			printf("Changes not saved.\n");
	} else
		printf("The partition table is unchanged.\n");
	close(fd);
	return 1;
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
	freepart = -1;
	freesize = freestart = 0;
	for (i = 0; i < NPARTN; i++) {
		p[i] = &hd.hd_partn[i];
		if (p[i]->p_size != 0) {
			check_chs(p[i], 1);	/* check start c:h:s */
			check_chs(p[i], 0);	/* check end c:h:s */
		} else if (freepart == -1)
			freepart = i;		/* first free partition */
	}
	qsort(p, NPARTN, sizeof(FDISK_S *), pcompare);
	next = 1;		/* next block available after boot sector */
	for (i = 0; i < NPARTN; i++) {
		base = p[i]->p_base;
		size = p[i]->p_size;
		if (size == 0)
			break;			/* done when empty reached */
		if (base < next) {
			if (next == 1)
				printf("Partition overlaps boot sector.\n");
			else if (cylflag)
				printf("Partitions overlap starting at cylinder %lu.\n", base / cylsize);
			else
				printf("Partitions overlap starting at track %lu.\n", base / nspt);
			++badflag;
		} else if (base != next) {
			if (i == 0 && (base == nspt || base == cylsize))
				;	/* first partition at 0:1:1 or 1:0:1 */
			else
				unused(base, next);
		}
		if (base + size > next)
			next = base + size;
	}
	if (next != nsectors)
		unused(nsectors, next);
}

/*
 * Report unused portion of disk.
 */
void
unused(base, next) unsigned long base, next;
{
	register unsigned long n, x, y;
	register char *s;

	n = base - next;
	if (cylflag && n >= cylsize) {
		s = "cylinder";
		x = sec_to_c(n);
		y = sec_upto_c(next);
	} else if (n >= nspt) {
		s = "track";
		x = n / nspt;
		y = sec_upto_t(next);
	} else {
		s = "sector";
		x = n;
		y = next;
	}
	if (x == 1)
		printf("%lu %s (%.2f megabytes) is unused starting at %s %lu.\n",
			x, s, meg(n), s, y);
	else 
		printf("%lu %ss (%.2f megabytes) are unused starting at %s %lu.\n",
			x, s, meg(n), s, y);
	if (freesize < n) {
		freesize = n;
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
