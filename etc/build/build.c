/*
 * build.c
 * 4/5/90
 * Build (install) COHERENT on a system, part 1.
 * The second part of the install procedure is in install.c.
 * Uses common routines in build0.c.
 * Requires floating point output: cc build.c build0.c -f
 * Usage: build [ -dvx ]
 * Options:
 *	-d	Debug, echo commands without executing
 *	-v	Verbose
 *	-x	XT instead of AT
 *
 * The build disk from which this program runs must contain:
 *	In /:		coherent
 *	In /bin:	chgrp, chown, cpdir, ln, mkdir
 *	In /conf:	boot, mboot, patch
 *	In /dev:	at[01][abcdx], rat[01][abcd]
 *	In /etc:	badscan, fdisk, mkfs, mount, umount
 * It must also contain /mnt, /tmp and all files necessary to boot
 * the installed /coherent system.
 *
 * Open questions:
 *	Allow user to avoid overwriting boot with mboot?
 */

#include <stdio.h>
#include <canon.h>
#include <string.h>
#include <sys/fdisk.h>
#include <sys/filsys.h>
#include "build0.h"

#define	VERSION		"1.5"
#define	USAGE		"Usage: /etc/build [ -dvx ]\n"
#define	AINDEX		5		/* index of 'a' in "/dev/at0x"	*/
#define	BSIZE		512		/* sector size			*/
#define	MINSIZE		7		/* minimum root size in MB	*/
#define	NDEV		(NPARTN+NPARTN)	/* number of devices		*/
#define	NSIZE		10		/* strlen("/dev/at0x") + 1	*/
#define	MAJOR		11		/* AT device major number	*/

/* (unsigned long) sectors to (double) megabytes. */
#define	meg(sec)	((double)sec * BSIZE / 1000000.)

/* Device table structure. */
typedef	struct	device	{
	int		d_flags;		/* flags		*/
	char		d_name[NSIZE];		/* cooked device name	*/
	char		d_rname[NSIZE+1];	/* raw device name	*/
	char		d_pname[NSIZE];		/* prototype name	*/
	unsigned long	d_size;			/* size in blocks	*/
}	DEVICE;

/* Flag bits. */
#define	F_COH	0x01				/* COHERENT partition	*/
#define	F_BOOT	0x02				/* Active		*/
#define	F_ROOT	0x04				/* Root			*/
#define	F_FS	0x08				/* File system exists	*/
#define	F_MOUNT	0x10				/* Mounted by /etc/rc	*/
#define	F_PROTO	0x20				/* Proto created	*/
#define	F_SCAN	0x40				/* Badscanned		*/
#define	isflag(i, f)	((device[i].d_flags & (f)) != 0)
#define	notflag(i, f)	((device[i].d_flags & (f)) == 0)
#define	clrflag(i, f)	device[i].d_flags &= ~(f)
#define	setflag(i, f)	device[i].d_flags |= (f)

/* Device table.  The index in this table is the device minor number. */
DEVICE	device	[NDEV] = {
	{ 0, "/dev/at0a", "/dev/rat0a", "/tmp/at0a", 0L },
	{ 0, "/dev/at0b", "/dev/rat0b", "/tmp/at0b", 0L },
	{ 0, "/dev/at0c", "/dev/rat0c", "/tmp/at0c", 0L },
	{ 0, "/dev/at0d", "/dev/rat0d", "/tmp/at0d", 0L },
	{ 0, "/dev/at1a", "/dev/rat1a", "/tmp/at1a", 0L },
	{ 0, "/dev/at1b", "/dev/rat1b", "/tmp/at1b", 0L },
	{ 0, "/dev/at1c", "/dev/rat1c", "/tmp/at1c", 0L },
	{ 0, "/dev/at1d", "/dev/rat1d", "/tmp/at1d", 0L }
};

/* Externals. */
extern	long	lseek();

/* Forward. */
void	badscan();
int	check_special();
void	copy();
void	done();
void	fdisk();
void	get_timezone();
void	mkfs();
void	set_date();
void	user_devices();

/* Globals. */
int	active = -1;			/* active partition	*/
char	*activeos;			/* active partition OS	*/
HDISK_S	hd;				/* hard disk boot block	*/
int	ndevices;			/* number of COH devices */
int	root;				/* root partition	*/
char	*timezone;			/* timezone		*/
char	*xdev[2] = { "/dev/at0x", "/dev/at1x" };
int	xflag;				/* use XT not AT	*/

main(argc, argv) int argc; char *argv[];
{
	register DEVICE *pp;
	register char *s;

	argv0 = argv[0];
	usagemsg = USAGE;
	if (argc > 1 && argv[1][0] == '-') {
		for (s = &argv[1][1]; *s; ++s) {
			switch(*s) {
			case 'd':	++dflag;	break;
			case 'x':	++xflag;	break;
			case 'v':	++vflag;	break;
			case 'V':
				fprintf(stderr, "%s: V%s\n", argv0, VERSION);
				break;
			default:	usage();	break;
			}
		}
		--argc;
		++argv;
	}
	if (argc != 1)
		usage();
	if (xflag) {
		/* Hot patch XT device names. */
		xdev[0][AINDEX] = xdev[1][AINDEX] = 'x';
		for (pp = device; pp < &device[NDEV]; pp++)
			pp->d_name[AINDEX] = pp->d_rname[AINDEX] = pp->d_pname[AINDEX] = 'x';
	}

	set_date();
	fdisk();
	badscan();
	mkfs();
	copy();
	user_devices();
	done();
	sync();
	printf(
"Hit <Enter>. Remove the floppy disk when the screen goes blank\n"
"(but not before!) so the system does not boot from the floppy.\n"
		);
	if (root != active)
		printf("You MUST type %d during the boot to boot the COHERENT operating system.\n",
			root);
	get_line("");
	sys("/etc/reboot", S_IGNORE);
	exit(0);
}

/*
 * Scan each COHERENT device for bad blocks.
 * Build prototypes in /tmp.
 */
void
badscan()
{
	register int i;
	register char *name;

	cls(0);
	printf(
"The next step in installation is to scan each COHERENT partition\n"
"for bad blocks.  Be patient, this takes a few minutes.\n"
		);
	for (i = 0; i < NDEV; i++) {
		if (notflag(i, F_COH))
			continue;
		printf("\n");
		name = device[i].d_name;
		if (isflag(i, F_FS)) {
			printf(
"Partition %d (%s) already contains a COHERENT filesystem.\n"
"If you wish to continue to use the existing filesystem, you can skip\n"
"scanning it for bad blocks.  If you want to replace it with an empty\n"
"filesystem, you must scan it for bad blocks first.\n",
				i, name);
			if (yes_no("Do you want to scan %s for bad blocks",
				name) == 0)
				continue;
		}
		sprintf(cmd, "/etc/badscan -v -o %s %s %s",
			device[i].d_pname, device[i].d_rname, xdev[i/NPARTN]);
		printf("Scanning partition %d:\n", i);
		setflag(i, F_SCAN);
		if (sys(cmd, S_NONFATAL) == 0)
			setflag(i, F_PROTO);
	}
}

/*
 * Check if a special file is a well-formed filesystem.
 * This routine is derived from code in "mount.c".
 * Here the check that "special" is a block special file is eliminated.
 */
int
check_special(special, size) char *special; unsigned long size;
{
	static struct filsys f;
	register int fd;
	register struct filsys *fp;
	register daddr_t *dp;
	register ino_t *ip, maxinode;

	if ((fd = open(special, 0)) < 0)
		return 0;			/* cannot open */
	else if (lseek(fd, (long)SUPERI*BSIZE, 0) == -1L)
		return 0;			/* seek failed */
	else if (read(fd, &f, sizeof(f)) != sizeof(f))
		return 0;			/* read failed */
	close(fd);

	/* Canonical stuff. */
	fp = &f;
	canshort(fp->s_isize);
	candaddr(fp->s_fsize);
	canshort(fp->s_nfree);
	for (dp = &fp->s_free[0]; dp < &fp->s_free[NICFREE]; dp += 1)
		candaddr(*dp);
	canshort(fp->s_ninode);
	for (ip = &fp->s_inode[0]; ip < &fp->s_inode[NICINOD]; ip += 1)
		canino(*ip);
	candaddr(fp->s_tfree);
	canino(fp->s_tinode);

	/* Test for rationality. */
	if (fp->s_fsize != (daddr_t)size)
		return 0;
	maxinode = (fp->s_isize - INODEI) * INOPB + 1;
	if (fp->s_isize >= fp->s_fsize)
		return 0;
	if ((fp->s_tfree < fp->s_nfree)
	||  (fp->s_tfree >= fp->s_fsize - fp->s_isize + 1))
		return 0;
	if ((fp->s_tinode < fp->s_ninode) || (fp->s_tinode >= maxinode-1 ))
		return 0;
	for (dp = &fp->s_free[0]; dp < &fp->s_free[fp->s_nfree]; dp += 1)
		if ((*dp < fp->s_isize) || (*dp >= fp->s_fsize))
			return 0;
	for (ip = &fp->s_inode[0]; ip < &fp->s_inode[fp->s_ninode]; ip += 1)
		if ((*ip < 1) || (*ip > maxinode))
			return 0;
	return 1;
}

/*
 * Patch /coherent, mount the root filesystem, copy files to it.
 */
void
copy()
{
	FILE *fp;

	cls(0);
	printf(
"The next step is to copy some COHERENT files from the diskette to the\n"
"root filesystem of your hard disk.  This will take a few minutes...\n"
		);

	/* Mount the filesystem and copy the boot floppy to it. */
	sprintf(cmd, "/etc/mount %s /mnt", device[root].d_name);
	sys(cmd, S_FATAL);
	sprintf(cmd, "/bin/cpdir -ad%s -smnt -sbegin / /mnt", (vflag) ? "v" : "");
	sys(cmd, S_FATAL);
	if (!exists("/mnt/mnt"))
		sys("/bin/mkdir /mnt/mnt", S_FATAL);
	sys("/bin/rm /mnt/tmp/*", S_IGNORE);

	/* Patch the /coherent image on the hard disk. */
	sprintf(cmd, "/conf/patch /mnt/coherent rootdev_=makedev\\(%d,%d\\) pipedev_=makedev\\(%d,%d\\)",
		MAJOR, root, MAJOR, root);
	sys(cmd, S_FATAL);

	/* Grow /lost+found to make room for files. */
	sys("cd /lost+found; /bin/touch a b c d e f g h i j k l; /bin/rm [a-l]",
		S_IGNORE);

	/* Create /autoboot. */
	sys("/bin/ln -f /mnt/coherent /mnt/autoboot", S_FATAL);

	/* Replace the build version of /etc/brc with the install version. */
	sys("/bin/rm /mnt/etc/brc", S_NONFATAL);
	sys("/bin/ln -f /mnt/etc/brc.install /mnt/etc/brc", S_FATAL);

	/* Link root device to /dev/root. */
	sprintf(cmd, "/bin/ln -f /mnt%s /mnt/dev/root", device[root].d_name);
	sys(cmd, S_FATAL);

	/* Write the timezone to /etc/timezone. */
	if (dflag)
		fp = NULL;
	else if ((fp = fopen("/mnt/etc/timezone", "w")) == NULL)
		nonfatal("/mnt/etc/timezone: open failed");
	else {
		fprintf(fp, "export TIMEZONE=\"%s\"\n", timezone);
		fclose(fp);
	}
}

/*
 * Done.
 * Print useful information.
 */
void
done()
{
	cls(0);
	printf(
"You have installed the COHERENT operating system onto your hard disk.\n"
"To install files from the remaining diskettes in the installation kit,\n"
"you must boot the COHERENT system from the hard disk.  It will prompt\n"
"you to install the remaining diskettes in the installation kit.\n"
"\n"
"After you finish reading this information,\n"
"hit <Enter> and your system will automatically reboot.\n"
"Remove the floppy disk from the drive when the screen goes blank.\n"
"\n"
"If you type a partition number (0 to 7) while\n"
"the boot procedure is trying to read the floppy disk,\n"
"your system will boot the operating system on that partition.\n"
		);
	if (active != -1) {
		printf("If you type nothing, your system will boot ");
		if (active == root)
			printf("COHERENT (partition %d).\n", active);
		else {
			printf("active partition %d", active);
			if (activeos != NULL)
				printf(" (%s)", activeos);
			printf(".\n", active);
		}
	}
	printf("\n");
}

/*
 * Get partition table information.
 */
void
fdisk()
{
	register int fd, drive, i, j, opened, cohpart;
	char *fname, *s;

	cls(0);
	printf(
"This installation procedure allows you to create one or more partitions\n"
"on your hard disk to contain the COHERENT system and its files.\n"
"Each disk drive may contain no more than four logical partitions.\n"
"If all four partitions on your disk are already in use,\n"
"you will have to overwrite at least one of them to install COHERENT.\n"
"If your disk uses fewer than four partitions and has enough unused\n"
"space for COHERENT (%d megabytes), you can install COHERENT into the\n"
"unused space.  If it has fewer than four partitions but no unused space,\n"
"you MAY be able to split an existing MS-DOS partition into two partitions\n"
"to create a partition for COHERENT.  If you intend to install MS-DOS\n"
"after installing COHERENT, you must leave the first physical partition\n"
"free for MS-DOS.\n"
"\n"
"The next part of the installation procedure will let you change the\n"
"partitions on your hard disk.  Data on unchanged hard disk partitions\n"
"will not be changed.  However, data already on your hard disk may be\n"
"destroyed if you change the base or the size of a logical partition,\n"
"change the order of partition table entries, or try to shrink an MS-DOS\n"
"partition.  If you need to back up existing data from the hard disk,\n"
"type <Ctrl-C> now to interrupt COHERENT installation; then reboot your\n"
"system and back up your hard disk data onto diskettes.\n"
"\n"
		, MINSIZE);
	cls(1);
	sys("/etc/fdisk -b /conf/mboot", S_FATAL);
	for (drive = opened = 0; drive < 2; ++drive) {
		fname = xdev[drive];
		if ((fd = open(fname, 0)) < 0)
			continue;
		++opened;
		if (read(fd, &hd, sizeof hd) != sizeof hd)
			fatal("%s: read failed", fname);
		close(fd);
		if (hd.hd_sig != HDSIG) {
			nonfatal("%s: invalid partition table", fname);
			continue;
		}
		for (i = 0; i < NPARTN; i++) {
			j = 4 * drive + i;
			if (hd.hd_partn[i].p_boot != 0) {
				setflag(j, F_BOOT);
				active = j;
				switch(hd.hd_partn[i].p_sys) {
				case SYS_COH:
					activeos = "COHERENT";
					break;
				case SYS_DOS_12:
				case SYS_DOS_16:
				case SYS_DOS_XP:
				case SYS_DOS_LARGE:
					activeos = "MS-DOS";
					break;
				case SYS_XENIX:
					activeos = "Xenix";
					break;
				default:
					activeos = NULL;
					break;
				}
			}
			if (hd.hd_partn[i].p_sys != SYS_COH)
				continue;

			/* Make sure the device can be accessed. */
			s = device[j].d_name;
			if (!exists(s)) {
				nonfatal("cannot open COHERENT partition %d (%s)",
					j, s);
				continue;
			} else if (hd.hd_partn[i].p_size == 0L) {
				nonfatal("COHERENT partition %d (%s) is empty",
					j, s);
				continue;
			}

			/* OK, set flags in the device table. */
			++ndevices;
			setflag(j, F_COH);
			device[j].d_size = hd.hd_partn[i].p_size;
			if (check_special(s, device[j].d_size))
				setflag(j, F_FS);

			/* Make sure the device is not mounted. */
			sprintf(cmd, "/etc/umount %s 2>/dev/null", s);
			sys(cmd, S_IGNORE);
		}
	}
	if (opened == 0)
		fatal("cannot open devices %s, %s", xdev[0], xdev[1]);
	else if (ndevices == 0)
		fatal("no COHERENT partition found");
	cls(0);
	printf("Your system includes %d COHERENT partition%s:\n",
		ndevices, (ndevices == 1) ? "" : "s");
	printf("Drive Partition\t  Device\tMegabytes\n");
	for (i = 0; i < NDEV; i++)
		if (isflag(i, F_COH)) {
			cohpart = i;
			printf("%3d\t%3d\t%s\t%.2f\n",
				i/4, i, device[i].d_name, meg(device[i].d_size));
		}
	if (ndevices == 1) {
		root = cohpart;
		setflag(root, F_ROOT);
		return;
	}
	printf(
"You must specify one COHERENT partition as the root filesystem.\n"
"The root filesystem contains the files normally used by COHERENT.\n"
"The root filesystem should contain at least %d megabytes.\n",
		MINSIZE);
	if (active != -1 && isflag(active, F_COH)) {
		printf("COHERENT partition %d is marked as active in the partition table.\n",
			active);
		printf("If you choose it as the root, you can boot COHERENT automatically.\n");
	}
	printf("\n");
again:
	s = get_line("Which partition do you want to be the root filesystem?");
	root = *s - '0';
	if (*++s != '\0' || root < 0 || root >= NDEV || notflag(root, F_COH)) {
		printf("Enter a number between 0 and 7 which specifies a COHERENT partition.\n");
		goto again;
	}
	if (meg(device[root].d_size) < (double)MINSIZE) {
		printf("Partition %d contains only %.2f megabytes.\n",
			root, meg(device[root].d_size));
		if (!yes_no("Are you sure you want it to be the root partition"))
			goto again;
	}
	setflag(root, F_ROOT);
}

/*
 * Set up a nonstandard timezone.
 */
void
get_timezone(dstflag) int dstflag;
{
	static char tz[NBUF];
	register char *s;

	timezone = tz;
	printf(
"You need to specify an abbreviation for your timezone,\n"
"whether you are east or west of Greenwich, England,\n"
"and the difference in minutes between your timezone\n"
"and Greenwich Time (called UT or GMT).  For example,\n"
"Germany is 60 minutes of time east of Greenwich.\n"
		);
	s = get_line("Abbreviation for your timezone?");
	sprintf(tz, "%s:", s);
	if (yes_no("Is your timezone east of Greenwich"))
		strcat(tz, "-");
	s = get_line("Difference in minutes from GMT?");
	strcat(tz, s);
	strcat(tz, ":");
	if (!dstflag)
		return;
	s = get_line("Abbreviation for your daylight savings timezone?");
	strcat(tz, s);
	strcat(tz, ":1.1.4");
}

/*
 * Make filesystems on COHERENT partitions.
 */
void
mkfs()
{
	register int i;
	char *name;

	cls(0);
	printf(
"You must create an empty COHERENT filesystem on each COHERENT partition\n"
"before you can use it.  Creating an empty filesystem will destroy all\n"
"previously existing data on the partition.\n"
		);
	for (i = 0; i < NDEV; i++) {
		if (notflag(i, F_COH) || notflag(i, F_SCAN))
			continue;
		name = device[i].d_name;
		printf("\n");
		if (isflag(i, F_FS))
			printf("Partition %d (%s) already contains a COHERENT filesystem.\n",
				i, name);
again:
		if (yes_no("Do you want to create a new COHERENT filesystem on partition %d", i)) {
			if (notflag(i, F_PROTO)) {
				printf("The attempt to scan %s for bad blocks previously failed.",
					name);
				if (yes_no("Do you want to create a new filesystem on it without a bad block list"))
					sprintf(cmd, "/etc/mkfs %s %lu", name, device[i].d_size);
				else
					continue;
			} else
				sprintf(cmd, "/etc/mkfs %s %s", name, device[i].d_pname);
			clrflag(i, F_FS);
			if (sys(cmd, S_NONFATAL) == 0) {
				setflag(i, F_FS);
				/*
				 * Mount the file system,
				 * create /lost+found,
				 * unmount it.
				 */
				sprintf(cmd, "/etc/mount %s /mnt", name);
				if (sys(cmd, S_NONFATAL))
					continue;
				sprintf(cmd, "/bin/mkdir /mnt/lost+found");
				if (sys(cmd, S_NONFATAL) == 0)
					sys(
"cd /mnt/lost+found; /bin/touch a b c d e f g h i j k l; /bin/rm [a-l]",
						S_IGNORE);
				sprintf(cmd, "/etc/umount %s", name);
				sys(cmd, S_NONFATAL);
			} else if (i == root)
				fatal("%s: root partition mkfs failed", name);
		} else if (i == root && notflag(i, F_FS)) {
			printf("You must create a filesystem on the root partition.\n");
			goto again;
		}
	}
}

/*
 * Date and time.
 */
void
set_date()
{
	register char *s;
	int dstflag;
	char *mytz;

	cls(0);

	/*
	 * Local time.
	 * The brc sets the COHERENT date from the system clock.
	 */
	printf(
"It is important for the COHERENT system to know the correct date and time.\n"
"You must provide information about your timezone and daylight savings time.\n"
"\n"
"You can run COHERENT with or without daylight savings time conversion.\n"
"You should normally run with daylight savings time conversion.\n"
"However, if you are going to use both COHERENT and MS-DOS\n"
"and you choose to run with daylight savings time conversion,\n"
"your time will be wrong (by one hour) during daylight savings time\n"
"while you are running under MS-DOS.\n"
		);
	dstflag = yes_no("Do you want COHERENT to use daylight savings time conversion");
	if (dstflag)
		printf(
"By default, COHERENT assumes daylight savings time begins on the\n"
"first Sunday in April and ends on the last Sunday in October.\n"
"If you want to change the defaults, edit the file \"/etc/timezone\"\n"
"after you finish installing COHERENT.\n"
			);
	mytz = (dstflag) ? "TIMEZONE='standard time:0:daylight time:1.1.4'"
			 : "TIMEZONE=";
	printf(
"\n"
"According to your computer system clock, your current local date and time are:\n"
		);
	sprintf(cmd, "%s /bin/date", mytz);
	sys(cmd, S_NONFATAL);
	if (!yes_no("Is this correct")) {
		do {
			if (dstflag)
				printf(
"Subtract one hour from the time you enter if daylight savings time is in effect.\n"
					);
			s = get_line(
				"Enter the correct date and time in the form YYMMDDHHMM.SS:"
				);
			sprintf(cmd, "/etc/ATclock %s >/dev/null", s);
			if (sys(cmd, S_NONFATAL) != 0)
				continue;
			sprintf(cmd, "%s /bin/date `/etc/ATclock`", mytz);
		} while (sys(cmd, S_NONFATAL) != 0);
	}

	/* Timezone */
	printf(
"Please choose one of the following timezones:\n"
"\t1\tAtlantic Time\n"
"\t2\tEastern Time\n"
"\t3\tCentral Time\n"
"\t4\tMountain Time\n"
"\t5\tPacific Time\n"
"\t6\tOther\n"
		);
	do {
		s = get_line("Timezone code:");
	} while (*s < '1' || *s > '6' || *(s+1) != '\0');
	switch (*s) {
	case '1':	timezone = "AST:240:ADT:1.1.4";	break;
	case '2':	timezone = "EST:300:EDT:1.1.4";	break;
	case '3':	timezone = "CST:360:CDT:1.1.4";	break;
	case '4':	timezone = "MST:420:MDT:1.1.4";	break;
	case '5':	timezone = "PST:480:PDT:1.1.4";	break;
	case '6':	timezone = NULL;		break;
	}

	if (timezone == NULL)
		get_timezone(dstflag);
	else if (!dstflag)
		timezone[8] = '\0';
}

/*
 * Configure user devices.
 * Assumes hard disk filesystem mounted on /mnt.
 * Wrtie lines to /etc/mount.all, /etc/umount.all to [u]mount the user devices.
 */
void
user_devices()
{
	FILE *mfp, *ufp, *checkfp;
	register int i, status;
	register char *s, *s2, *name;
	char fakename[NBUF];

	/* Create /etc/checklist. */
	if (dflag)
		checkfp = NULL;
	else if ((checkfp = fopen("/mnt/etc/checklist", "w")) == NULL)
		nonfatal("/mnt/etc/checklist: open failed");

	/* Create user device names. */
	cls(0);
	if (dflag)
		mfp = NULL;
	else if ((mfp = fopen("/mnt/etc/mount.all", "w")) == NULL)
		nonfatal("/mnt/etc/mount.all: open failed");
	chmod("/mnt/etc/mount.all", 0555);
	if (dflag)
		ufp = NULL;
	else if ((ufp = fopen("/mnt/etc/umount.all", "w")) == NULL)
		nonfatal("/mnt/etc/umount.all: open failed");
	chmod("/mnt/etc/umount.all", 0555);
	printf(
"Your system includes %d partition%s in addition to the root partition.\n"
"These partitions are usually mounted on directories in the COHERENT\n"
"root filesystem when the system goes into multiuser mode.\n"
"For example, one non-root partition might be mounted on\n"
"directory \"/u\", another on \"/v\", and so on.\n"
"You now may specify where you want each partition mounted.\n",
		ndevices - 1, ndevices == 2 ? "" : "s");
	for (i = 0; i < NDEV; i++) {
		if (notflag(i, F_COH) || notflag(i, F_FS) || isflag(i, F_ROOT))
			continue;
		name = device[i].d_name;
		printf("\nPartition %d (%s):\n", i, name);
		if (yes_no("Do you want %s mounted", name)) {
			setflag(i, F_MOUNT);
again:
			s = get_line("Where do you want to mount it?");
			if (*s != '/') {
				printf("Type a directory name beginning with '/', such as \"/u\".\n");
				goto again;
			} else if ((s2 = strchr(s, ' ')) != NULL)
				*s2 = '\0';
			sprintf(cmd, "/mnt/%s", s);
			if (exists(cmd)) {
				strcpy(cmd, s);
				printf("Directory %s already exists.\n", s);
				if (!yes_no("Are you sure you want %s mounted on %s", name, s))
					goto again;
				s = buf;
				strcpy(s, cmd);
			} else {
				/* Make the target directory, uid=bin, gid=bin. */
				sprintf(cmd, "/bin/mkdir /mnt%s", s);
				if (sys(cmd, S_NONFATAL))
					goto again;
				sprintf(cmd, "/bin/chown bin /mnt%s", s);
				sys(cmd, S_NONFATAL);
				sprintf(cmd, "/bin/chgrp bin /mnt%s", s);
				sys(cmd, S_NONFATAL);
			}

			/* Change e.g. /usr/src to usr_src */
			strcpy(fakename, &s[1]);
			while ((s2 = strchr(fakename, '/')) != NULL)
				*s2 = '_';

			/* Make link to pseudo-device, e.g. "/dev/usr_src". */
			sprintf(cmd, "/mnt/dev/%s", fakename);
			if (exists(cmd))
				status = 1;		/* use normal name */
			else {
				sprintf(cmd, "/bin/ln -f /mnt%s /mnt/dev/%s",
					name, fakename);
				status = sys(cmd, S_NONFATAL);
			}
			if (mfp != NULL) {
				if (status == 0)
					fprintf(mfp, "/etc/mount /dev/%s %s\n",
						fakename, s);
				else
					fprintf(mfp, "/etc/mount %s %s\n",
						name, s);
			}
			if (ufp != NULL) {
				if (status == 0)
					fprintf(ufp, "/etc/umount /dev/%s\n",
						fakename);
				else
					fprintf(ufp, "/etc/umount %s\n",
						name);
			}
			/* And again, for the raw device. */
			sprintf(cmd, "/mnt/dev/r%s", fakename);
			if (exists(cmd))
				status = 1;
			else {
				sprintf(cmd, "/bin/ln -f /mnt%s /mnt/dev/r%s",
					device[i].d_rname, fakename);
				status = sys(cmd, S_NONFATAL);
			}
			if (checkfp != NULL) {
				if (status == 0)
					fprintf(checkfp, "/dev/r%s\n", fakename);
				else
					fprintf(checkfp, "%s\n", device[i].d_rname);
			}
		} else if (checkfp != NULL)
			fprintf(checkfp, "%s\n", device[i].d_rname);
	}
	fprintf(checkfp, "/dev/root\n");
	if (checkfp != NULL)
		fclose(checkfp);
	if (mfp != NULL)
		fclose(mfp);
	if (ufp != NULL)
		fclose(ufp);
}

/* end of build.c */
