/*
 * build.c
 * 3/30/90
 * Build (install) COHERENT on a system, part 1.
 * The second part of the install procedure is in install.c.
 * Requires floating point output: cc build.c -f
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

#define	VERSION		"1.3"
#define	USAGE		"Usage: build [ -dvx ]\n"
#define	AINDEX		5		/* index of 'a' in "/dev/at0x"	*/
#define	BSIZE		512		/* sector size			*/
#define	MINSIZE		5		/* minimum root size in MB	*/
#define	NBUF		256		/* buffer size			*/
#define	NDEV		(NPARTN+NPARTN)	/* number of devices		*/
#define	NSIZE		10		/* strlen("/dev/at0x") + 1	*/
#define	MAJOR		11		/* AT device major number	*/

/* Flags for sys(). */
#define	S_IGNORE	1
#define	S_NONFATAL	2
#define	S_FATAL		3

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
void	cls();
void	copy();
void	done();
int	exists();
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
char	buf[NBUF];			/* input buffer		*/
char	cmd[NBUF];			/* command buffer	*/
int	dflag;				/* debug		*/
HDISK_S	hd;				/* hard disk boot block	*/
int	ndevices;			/* number of COH devices */
int	root;				/* root partition	*/
int	vflag;				/* verbose		*/
char	*xdev[2] = { "/dev/at0x", "/dev/at1x" };
int	xflag;				/* use XT not AT	*/

main(argc, argv) int argc; char *argv[];
{
	register DEVICE *pp;
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
				fprintf(stderr, "%s: V%s\n", argv0, VERSION);
				break;
			default:	usage();	break;
			}
		}
	}
	if (argc != 1)
		usage();
	if (xflag) {
		/* Hot patch XT device names. */
		xdev[0][AINDEX] = xdev[1][AINDEX] = 'x';
		for (pp = device; pp < &device[NDEV]; pp++)
			pp->d_name[AINDEX] = pp->d_rname[AINDEX] = pp->d_pname[AINDEX] = 'x';
	}

	fdisk();
	badscan();
	mkfs();
	copy();
	if (ndevices > 1)
		user_devices();
	done();
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
 * Clear the IBM AT console screen.
 * Prompt for <Enter> if the flag is true or if dflag.
 */
void
cls(flag) register int flag;
{
	if (flag || dflag)
		get_line("\nType <Enter> to continue...");
	putchar(0x1B);		/* ESC */
	putchar('c');
	fflush(stdout);
}

/*
 * Patch /coherent, mount the root filesystem, copy files to it.
 */
void
copy()
{
	FILE *fp;
	register int i;

	cls(0);
	printf(
"The next step is to copy some COHERENT files from the diskette to\n"
"the root partition of your hard disk.  This will take a few minutes.\n"
		);

	/* Mount the filesystem and copy the boot floppy to it. */
	sprintf(cmd, "/etc/mount %s /mnt", device[root].d_name);
	sys(cmd, S_FATAL);
	sprintf(cmd, "/bin/cpdir -d%s -smnt -sbegin / /mnt", (vflag) ? "v" : "");
	sys(cmd, S_FATAL);
	sys("/bin/mkdir /mnt/mnt", S_FATAL);

	/*
	 * Patch the /coherent image on the hard disk.
	 * The parameters are straight from the Inetco version, for now.
	 */
	sprintf(cmd, "/conf/patch /mnt/coherent ALLSIZE_=16384 NBUF_=32 NCLIST_=24 rootdev_=makedev\\(%d,%d\\) pipedev_=makedev\\(%d,%d\\)",
		MAJOR, root, MAJOR, root);
	sys(cmd, S_FATAL);

	/* Create /autoboot. */
	sys("/bin/ln -f /mnt/coherent /mnt/autoboot", S_FATAL);

	/* Replace the build version of /etc/brc with the install version. */
	sys("/bin/rm /mnt/etc/brc", S_NONFATAL);
	sys("/bin/ln -f /mnt/etc/brc.install /mnt/etc/brc", S_FATAL);

	/* Link root device to /dev/root. */
	sprintf(cmd, "/bin/ln /mnt%s /mnt/dev/root", device[root].d_name);
	sys(cmd, S_NONFATAL);

	/* Create /etc/checklist. */
	if (dflag)
		fp = NULL;
	else if ((fp = fopen("/mnt/etc/checklist", "w")) != NULL) {
		for (i = NDEV - 1; i >= 0; i--)
			if (isflag(i, F_COH) && isflag(i, F_FS) && notflag(i, F_ROOT))
				fprintf(fp, "%s\n", device[i].d_rname);
		fprintf(fp, "%s\n", device[root].d_name);
		fclose(fp);
	} else
		nonfatal("/mnt/etc/checklist: open failed");
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
"After you finish reading this information, type <Enter>.\n"
"Your system will automatically reboot.  When the screen goes blank,\n"
"remove the floppy disk from the disk drive.\n"
"\n"
"If you type a partition number (0 to 7) while\n"
"the boot procedure is trying to read the floppy disk,\n"
"your system will boot the operating system on that partition.\n"
		);
	if (active != -1) {
		printf("If you type nothing, your system will boot ");
		if (active == root)
			printf("COHERENT (partition %d).\n", active);
		else
			printf("active partition %d.\n", active);
	}
	if (root != active)
		printf("\nYou MUST type %d during the boot to boot the COHERENT operating system.\n",
			root);
	printf("\n");
}

/*
 * Print a fatal error message.
 */
void
fatal(s) char *s;
{
	fprintf(stderr, "%s: %r\nInstallation aborted before completion.\n",
		argv0, &s);
	exit(1);
}

/*
 * Return 1 if file exists, 0 if not.
 */
int
exists(file) register char *file;
{
	register int fd;

	if ((fd = open(file, 0)) < 0)
		return 0;
	close(fd);
	return 1;
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
"space for COHERENT (%d megabytes), you can install COHERENT into the\n"
"unused space.  If it has fewer than four partitions but no unused space,\n"
"you MAY be able to split an existing MS-DOS partition into two partitions\n"
"to create a partition for COHERENT.  If you intend to install MS-DOS\n"
"after installing COHERENT, you must leave the first physical partition\n"
"free for MS-DOS.\n"
"\n"
"The first part of the installation procedure will let you change the\n"
"partitions on your hard disk.  Data on unchanged hard disk partitions\n"
"will not be changed.  However, data already on your hard disk may be\n"
"destroyed if you change the base or the size of a logical partition,\n"
"change the order of partition table entries, or try to shrink an MS-DOS\n"
"partition.  If you need to back up existing data from the hard disk,\n"
"type <Ctrl-C> now to interrupt COHERENT installation; then reboot your\n"
"system and back up your hard disk data onto diskettes.\n"
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
			if (check_special(s))
				setflag(j, F_FS);
			device[j].d_size = hd.hd_partn[i].p_size;

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
		if (isflag(i, F_COH))
			printf("%3d\t%3d\t%s\t%.2f\n",
				i/4, i, device[i].d_name, meg(device[i].d_size));
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
 * Print the args and get a line from the user to buf[].
 * Strip the trailing newline and return a pointer to the first non-space.
 */
char *
get_line(args) char *args;
{
	register char *s;

	printf("%r ", &args);
	fflush(stdout);
	fgets(buf, sizeof buf, stdin);
	buf[strlen(buf) - 1] = '\0';
	for (s = buf; ; ++s)
		if (*s != ' ' && *s != '\t')
			return s;
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
				sys(cmd, S_NONFATAL);
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
 * Print a nonfatal error message.
 */
void
nonfatal(s) char *s;
{
	fprintf(stderr, "%s: %r\n", argv0, &s);
}

/*
 * Execute the given command and return its exit status.
 * The flag tells what to do if the command returns an error status:
 *	S_IGNORE	ignore it
 *	S_NONFATAL		report it
 *	S_FATAL		report it and die
 */
int
sys(command, flag) char *command; int flag;
{
	register int status;

	if (dflag || vflag)
		printf("%s\n", command);
	if (dflag)
		return 0;
	if ((status = system(command)) != 0) {
		if (flag == S_NONFATAL)
			nonfatal("command \"%s\" failed", command);
		else if (flag == S_FATAL)
			fatal("command \"%s\" failed", command);
	}
	return status;
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
 * Append lines to /etc/rc to mount the user devices.
 */
void
user_devices()
{
	FILE *fp;
	register int i;
	register char *s, *s2, *name;

	/* Create user device names. */
	cls(0);
	if (dflag)
		fp = NULL;
	else if ((fp = fopen("/mnt/etc/rc", "a")) == NULL)
		nonfatal("/mnt/etc/rc: open failed");
	printf(
"Your system includes %d partition%s in addition to the root partition.\n"
"These partitions are usually mounted on directories in the COHERENT\n"
"filesystem when the system goes into multiuser mode.\n"
"For example, /dev/at0b (partition 1) might be mounted on\n"
"directory \"/u\", /dev/at0c (partition 2) on \"/v\", and so on.\n"
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
			if (exists(s)) {
				printf("Directory %s already exists.\n", s);
				if (!yes_no("Are you sure you want %s mounted on %s\n", name, s))
					goto again;
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

			/* Change e.g. /usr/src to .usr.src */
			while ((s2 = strchr(s, '/')) != NULL)
				*s2 = '.';

			/* Make link to pseudo-device, e.g. "/dev/usr.src". */
			sprintf(cmd, "/bin/ln /mnt%s /mnt/dev%s", name, ++s);
			sys(cmd, S_NONFATAL);
			if (fp != NULL)
				fprintf(fp, "/etc/mount %s %s\n", name, s);
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
