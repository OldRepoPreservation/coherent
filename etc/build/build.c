/*
 * build.c
 * 3/28/90
 * Requires floating point output: cc build.c -f
 * Usage: build [ -vx ]
 * Options:
 *	-d	Debug, echo commands without executing them
 *	-v	Verbose
 *	-x	XT instead of AT
 *
 * Build (install) COHERENT on a new machine:
 *	Read the hard disk partition tables /dev/[ax]t[01]x.
 *	Print a list of all COHERENT partitions (0 through 7).
 *	Allow the user to determine the root partition.
 *
 * Presumes:
 *	In /bin:	chgrp, chown, cpdir, ln, mkdir
 *	In /conf:	mboot, patch
 *	In /etc:	badscan, fdisk, mkfs
 *
 * Undone:
 *	/conf/patch arguments?
 *	always supply /autoboot?
 *	skip badscan if already filesystem present
 *	install more disks?
 */

#include <stdio.h>
#include <canon.h>
#include <sys/fdisk.h>
#include <sys/filsys.h>

#define	VERSION		"1.0"
#define	USAGE		"Usage: build [ -dvx ]\n"
#define	BSIZE		512		/* sector size			*/
#define	NBUF		256		/* buffer size			*/
#define	NSIZE		10		/* strlen("/dev/at0x") + 1	*/
#define	MAJOR		11		/* device major number		*/

#define	AINDEX		5		/* index of 'a' in "/dev/at0x"	*/
#define	MAJINDEX	7		/* index of '0' in "/dev/at0x"	*/
#define	MININDEX	8		/* index of 'x' in "/dev/at0x"	*/

/* (unsigned long) sectors to (double) megabytes. */
#define	meg(sec)	((double)sec * BSIZE / 1000000.)

/* Partition table structure. */
typedef	struct	ptable	{
	int		d_flags;		/* flags		*/
	char		d_name[NSIZE];		/* cooked device name	*/
	char		d_rname[NSIZE+1];	/* raw device name	*/
	char		d_pname[NSIZE];		/* prototype name	*/
	unsigned long	d_size;			/* size in blocks	*/
}	PTABLE;
/* Flag bits. */
#define	F_COH	0x01				/* COHERENT partition	*/
#define	F_BOOT	0x02				/* Active		*/
#define	F_ROOT	0x04				/* Root			*/
#define	F_FS	0x08				/* File system exists	*/
#define	F_MOUNT	0x10				/* Mounted by /etc/rc	*/
#define	F_BAD	0x20				/* badscan failed	*/

/* Table.  The index in this table corresponds to the device minor number. */
PTABLE	ptable	[NPARTN + NPARTN] = {
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
void	cls();
void	copyfiles();
void	fdisk();
void	fatal();
char	*get_line();
void	mkfs();
void	nonfatal();
int	sys();
void	usage();
void	user_devices();
int	yes_no();

/* Globals. */
int	active = -1;			/* active partition	*/
char	*argv0;				/* for error messages	*/
char	buf[NBUF];			/* buffer		*/
char	cmd[NBUF];			/* command buffer	*/
char	*devname[2] = { "/dev/at0x", "/dev/at1x" };
int	dflag;				/* debug		*/
HDISK_S	hd;				/* hard disk boot block	*/
int	ndevices;			/* number of COH devices */
int	root;				/* root partition	*/
int	status;				/* return status	*/
int	vflag;				/* verbose		*/
int	xflag;				/* use XT not AT	*/

main(argc, argv) int argc; char *argv[];
{
	register PTABLE *pp;
	register char *s;

	argv0 = argv[0];
	if (argc > 1 && argv[1][0] == '-') {
		--argc;
		for (s = &argv[1][1]; *s; ++s) {
			switch(*s) {
			case 'd':	++dflag;	break;
			case 'x':	++xflag;	break;
			case 'v':	++vflag;	break;
			case 'V':
				fprintf(stderr, "build: V%s\n", VERSION);
				break;
			default:	usage();	break;
			}
		}
	}
	if (argc != 1)
		usage();
	if (xflag) {
		devname[0][AINDEX] = devname[1][AINDEX] = 'x';
		for (pp = ptable; pp < &ptable[NPARTN + NPARTN]; pp++)
			pp->d_name[AINDEX] = pp->d_rname[AINDEX] = pp->d_pname[AINDEX] = 'x';
	}

	fdisk();
	badscan();
	mkfs();
	copyfiles();
	user_devices();
	cls(0);
	printf(
"The COHERENT operating system is now installed on your hard disk.\n"
"Type \"sync\", remove the floppy disk from the disk drive, and\n"
"then reboot your system by typing <Ctrl><Alt><Del> or by pressing\n"
"the reset button.  If you type a partition number (0 to 7) while\n"
"the boot procedure is trying to read the floppy disk,\n"
"your system will boot the operating system on that partition.\n"
"If you type nothing, your system will boot the active partition.\n"
		);
	sync();
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

	cls(0);
	printf(
"The next step in installation is to scan each COHERENT partition\n"
"for bad blocks.  Be patient, this will take a few minutes.\n"
		);
	for (i = 0; i < NPARTN + NPARTN; i++) {
		if ((ptable[i].d_flags & F_COH) == 0)
			continue;
		sprintf(cmd, "/etc/badscan -v -o %s %s %s",
			ptable[i].d_name, ptable[i].d_rname, devname[i/NPARTN]);
		printf("Scanning partition %d:\n", i);
		if (sys(cmd) != 0) {
			nonfatal("%s: badscan failed", ptable[i].d_name);
			ptable[i].d_flags |= F_BAD;
		}
	}
}

/*
 * Check if a special file is a well-formed filesystem.
 * This routine is derived from code in "mount.c".
 * Here the check that "special" is a block special file is eliminated.
 */
int
check_special(special) char *special;
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

	/* Canonicalization. */
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
 * Clear the screen.
 * Prompt for <Enter> if the flag is true.
 */
void
cls(flag) register int flag;
{
	if (flag || dflag)
		get_line("Type <Enter> to continue...");
	putchar(0x1B);		/* ESC */
	putchar('c');
	fflush(stdout);
}

/*
 * Copy files to the hard disk.
 */
void
copyfiles()
{
	FILE *fp;
	register int i, flags;

	cls(0);
	printf(
"The next step is to copy some COHERENT files from the diskette to\n"
"the root partition of your hard disk.  This will take a few minutes.\n"
		);

	/*
	 * Patch /coherent.
	 * This is straight from the Inetco version, for now.
	 */
	sprintf(cmd, "/conf/patch /coherent ALLSIZE_=16384 NBUF_=32 NCLIST_=24 rootdev_=makedev(%d,%d) pipedev_=makedev(%d,%d)",
		MAJOR, root, MAJOR, root);
	if (sys(cmd) != 0)
		fatal("cannot patch /coherent");

	/* Mount the file system and copy to it. */
	sprintf(cmd, "/etc/mount %s /mnt", ptable[root].d_name);
	if (sys(cmd) != 0)
		fatal("cannot mount root partition");
	if (sys("/bin/cpdir -d -smnt / /mnt") != 0)
		fatal("cannot copy build disk");

	/* Create /autoboot if desired. */
	if (active == root) {
		printf("The COHERENT root partition is marked as active in the partition table.\n");
		if (yes_no("Do you want your system to boot COHERENT automatically"))
			sys("/bin/ln /mnt/coherent /mnt/autoboot");
		else
			active = -1;
	}

	/* Link /dev/root. */
	sprintf(cmd, "/bin/ln /mnt%s /mnt/dev/root", ptable[root].d_name);
	if (sys(cmd) != 0)
		nonfatal("%s: cannot link to /dev/root", ptable[root].d_name);

	/* Create /etc/checklist. */
	if ((fp = fopen("/mnt/etc/checklist", "w")) == NULL) {
		nonfatal("/mnt/etc/checklist: open failed");
		return;
	}
	for (i = NPARTN + NPARTN - 1; i >= 0; i--) {
		flags = ptable[i].d_flags;
		if ((flags & F_COH) == 0
		 || (flags & F_FS) == 0
		 || (flags & F_ROOT) != 0)
			continue;
		fprintf(fp, "%s\n", ptable[i].d_rname);
	}
	fprintf(fp, "%s\n", ptable[root].d_name);
	fclose(fp);
}

/*
 * Print a fatal error message.
 */
void
fatal(s) char *s;
{
	fprintf(stderr, "%s: %r\n", argv0, &s);
	exit(1);
}

/*
 * Get partition table information.
 */
void
fdisk()
{
	register int fd, drive, i, j, opened;
	char *fname, *s;

	cls(0);
	printf(
"This installation procedure allows you to create one or more partitions\n"
"on your hard disk to contain the COHERENT system and its files.\n"
"Each disk drive may contain no more than four logical partitions.\n"
"If all four partitions on your disk are already in use,\n"
"you will have to overwrite at least one of them to install COHERENT.\n"
"If your disk uses fewer than four partitions and has enough unused\n"
"space for COHERENT (ten megabytes), you can install COHERENT into the\n"
"unused space.  If it has fewer than four partitions but no unused space,\n"
"you MAY be able to split an existing MS-DOS partition into two partitions\n"
"to create a partition for COHERENT.\n"
"\n"
"The first part of the installation procedure will let you change the\n"
"partitions on your hard disk.  Data on unchanged hard disk partitions\n"
"will not be changed.  However, data already on your hard disk may be\n"
"destroyed if you change the base or the size of a logical partition,\n"
"change the order of partition table entries, or try to shrink an MS-DOS\n"
"partition.  If you need to back up existing data from the hard disk,\n"
"type <Ctrl-C> now to interrupt COHERENT installation; then reboot your\n"
"system and back up your data onto diskettes.\n"
"\n"
		);
	cls(1);
	if (sys("/etc/fdisk -b /conf/mboot") != 0)
		fatal("fdisk failed");
	for (drive = opened = 0; drive < 2; ++drive) {
		fname = devname[drive];
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
			if (hd.hd_partn[i].p_sys != SYS_COH)
				continue;
			/* Make sure the device can be accessed. */
			j = 4 * drive + i;
			if ((fd = open(ptable[j].d_name, 0)) < 0) {
				nonfatal("cannot open COHERENT partition %d (%s)",
					j, ptable[j].d_name);
				continue;
			} else
				close(fd);
			++ndevices;
			ptable[j].d_flags = F_COH;
			ptable[j].d_size = hd.hd_partn[i].p_size;
			if (hd.hd_partn[i].p_boot != 0) {
				ptable[j].d_flags |= F_BOOT;
				active = j;
			}
		}
	}
	if (opened == 0)
		fatal("cannot open devices %s, %s", devname[0], devname[1]);
	else if (ndevices == 0)
		fatal("no COHERENT partition found");
again:
	cls(0);
	printf("Your system includes %d COHERENT partition%s:\n",
		ndevices, (ndevices == 1) ? "" : "s");
	printf("Drive Partition\tDevice\tMegabytes\n");
	for (i = 0; i < NPARTN + NPARTN; i++)
		if (ptable[i].d_flags & F_COH)
			printf("%d\t%d\t%s\t%.2f\n",
				i/4, i, ptable[i].d_name, meg(ptable[i].d_size));
	printf(
"You must specify one COHERENT partition as the root filesystem.\n"
"The root filesystem contains the files normally used by COHERENT.\n"
"The root filesystem should contain at least five megabytes.\n"
		);
	if (active != -1) {
		printf("COHERENT partition %d is marked as active in the partition table.\n",
			active);
		printf("If you choose it as the root, you can boot COHERENT automatically.\n");
	}
	s = get_line("Which partition do you want to be the root partition?");
	root = *s - '0';
	if (root < 0 || root >= NPARTN + NPARTN || (ptable[root].d_flags & F_COH) == 0) {
		printf("Enter a number between 0 and 7 which specifies a COHERENT partition.\n");
		goto again;
	}
	ptable[root].d_flags |= F_ROOT;
}

/*
 * Print the args and get a line from the user to buf[].
 * Return a pointer to the first non-space character.
 */
char *
get_line(args) char *args;
{
	register char *s;

	printf("%r ", &args);
	fflush(stdout);
	fgets(buf, sizeof buf, stdin);
	for (s = buf; ; ++s)
		if (*s != ' ' && *s != '\t')
			return s;
}

/*
 * Make file systems on COHERENT partitions.
 */
void
mkfs()
{
	register int i;
	char *name;

	cls(1);
	printf(
"You must create an empty COHERENT filesystem on each COHERENT partition\n"
"before you can use it.  This will destroy all previously existing\n"
"data on the partition.\n"
		);
	for (i = 0; i < NPARTN + NPARTN; i++) {
		if ((ptable[i].d_flags & F_COH) == 0)
			continue;
		name = ptable[i].d_name;
		if (check_special(name)) {
			printf("Partition %d (%s) apparently already contains a COHERENT filesystem.\n",
				i, name);
			ptable[i].d_flags |= F_FS;
		}
		if ((ptable[i].d_flags & F_BAD) != 0)
			continue;			/* badscan failed */
again:
		if (yes_no("Create a new COHERENT filesystem on partition %d", i)) {
			sprintf(cmd, "/etc/mkfs %s %s", name, ptable[i].d_pname);
			if (sys(cmd) == 0)
				ptable[i].d_flags |= F_FS;
			else if (i == root)
				fatal("%s: mkfs failed", name);
			else
				nonfatal("%s: mkfs failed", name);
		} else if (i == root && (ptable[i].d_flags & F_FS) == 0) {
			printf("You must create a filesystem on the root partition.\n");
			goto again;
		}
	}
}

/*
 * Print a nonfatal error message.
 */
void
nonfatal(s) char *s;
{
	fprintf(stderr, "%s: %r\n", argv0, &s);
}

/*
 * Execute the given command and return its exit status.
 */
int
sys(command) char *command;
{
	if (dflag || vflag)
		printf("%s\n", command);
	return ((dflag) ? 0 : system(command));
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

/*
 * Configure user devices.
 * Assumes hard disk filesystem mounted on /mnt.
 */
void
user_devices()
{
	FILE *fp;
	register int i, flags;
	register char *s;

	/* Create user device names. */
	if (ndevices < 2)
		return;			/* no user devices */
	cls(1);
	if ((fp = fopen("/mnt/etc/rc", "a")) == NULL)
		nonfatal("/mnt/etc/rc: open failed");
	printf(
"Your system includes %d partition%s in addition to the root partition.\n"
"These partitions are usually mounted on directories in the COHERENT\n"
"filesystem.  For example, /dev/at0b (partition 1) might be mounted on\n"
"directory \"/u\", /dev/at0c (partition 2) on \"/v\", and so on.\n"
"You can now specify where you want each partition mounted.\n",
		ndevices - 1, ndevices == 2 ? "" : "s");
	for (i = 0; i < NPARTN + NPARTN; i++) {
		flags = ptable[i].d_flags;
		if ((flags & F_COH) == 0
		 || (flags & F_FS) == 0
		 || (flags & F_ROOT) != 0)
			continue;
		printf("Partition %d (%s):\n", i, ptable[i].d_name);
		if (yes_no("Do you want %s mounted", ptable[i].d_name)) {
			ptable[i].d_flags |= F_MOUNT;
again:
			s = get_line("Where do you want to mount it?");
			if (*s != '/') {
				printf("Type a directory name beginning with '/', such as \"/u\".\n");
				goto again;
			} else
				s[strlen(s) - 1] = '\0';	/* zap '\n' */
			sprintf(cmd, "/bin/mkdir /mnt%s", s);
			if (sys(cmd) != 0) {
				nonfatal("cannot make directory %s", s);
				goto again;
			}
			sprintf(cmd, "/bin/chown bin /mnt%s", s);
			sys(cmd);
			sprintf(cmd, "/bin/chgrp bin /mnt%s", s);
			sys(cmd);
			sprintf(cmd, "/bin/ln /mnt%s /mnt/dev%s",
				ptable[i].d_name, s);
			if (fp != NULL)
				fprintf(fp, "/etc/mount %s %s\n",
					ptable[i].d_name, s);
		}
	}
	if (fp != NULL)
	fclose(fp);
}

/*
 * Get the answer to a yes/no question.
 * Return 1 for yes, 0 for no.
 */
int
yes_no(args) char *args;
{
	register char *s;

	for (;;) {
		printf("%r", &args);
		s = get_line(" [y or n]?");
		if (*s == 'y')
			return 1;
		else if (*s == 'n')
			return 0;
	}
}

/* end of build.c */
