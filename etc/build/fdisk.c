/*
 * fdisk.c
 * 11/2/90
 * cc -o fdisk fdisk.c -f
 * Change partitioning IBM-AT (or other type) hard disk.
 * Usage: /etc/fdisk [ -crvx ] [ -b bootb ] [ device ... ]
 * Options:
 *	-b	Add master boot block code from "bootb"
 *	-c	Configure hard disk geometry
 *	-r	Read only
 *	-v	Print c:h:s start and end values
 *	-x	Use devices /dev/xt[01]x instead of /dev/at[01]x
 * If no device argument is given, fdisk supplies "/dev/[ax]t[01]x"
 * as appropriate.
 */

#include <stdio.h>
#include <setjmp.h>
#include <sys/devices.h>
#include <sys/fdisk.h>
#include <sys/hdioctl.h>
#include <sys/stat.h>
#include "fdisk0.h"

/* Globals. */
char		*argv0;		/* Command name, for error messages.	*/
int		badflag;	/* Partition table is bad.		*/
char		buf[NBUF];	/* Input buffer.			*/
int		cfd;		/* Current file descriptor.		*/
int		cflag;		/* Configure disk geometry.		*/
int		cylflag;	/* Specify base and size in cylinders.	*/
unsigned int	cylsize;	/* Cylinder size in sectors.		*/
unsigned char	*defargs[3] = { "/dev/at0x", "/dev/at1x", NULL };
unsigned char	*device;	/* Partition table device name.		*/
unsigned char	*drivename;	/* Disk drive name.			*/
int		drivenum;	/* Drive number.			*/
int		freepart;	/* Free partition.			*/
unsigned long	freesize;	/* Free size.				*/
unsigned long	freestart;	/* First free sector.			*/
HDISK_S		hd;		/* Structure to house boot block.	*/
hdparm_t	hdparms;	/* Hard disk parameter block.		*/
int		isatflag;	/* Device is an AT-type disk.		*/
jmp_buf		loop;		/* Interactive input loop entry point.	*/
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
	int fd0, fd1, i;
	struct stat sb;

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
			case 'c':
				++cflag;
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
		"This program will let you change partition information for each disk drive.\n"
		"A disk drive can be divided into one to four logical partitions.\n"
		"You can change the active partition (the partition which your\n"
		"system boots by default) or change the layout of logical partitions.\n"
		"Other programs which change hard disk partition information\n"
		"may list logical partitions in a different order.\n"
		"Hit <Esc><Enter> to return to the main menu at any time.\n"
		);
	for (i = 0; (device = *argv++) != NULL; ++i) {
		/*
		 * Set the drive number, drive name, partition base.
		 * /etc/build calls /etc/fdisk with an ordered list of args
		 * which correspond to the drive number, so this usually works.
		 * But there is no obvious way to find the correct drive number
		 * when the user invokes /etc/fdisk directly; hence the
		 * kludge below for AT disks.
		 */
		drivenum = i;
		partbase = i * NPARTN;
		drivename = "Drive x";
		drivename[6] = drivenum + '0';

		/*
		 * Check if device is an AT device.
		 * The fix_chs() kludge only works for AT-type disks.
		 */
		if (stat(device, &sb) < 0)
			fatal("cannot stat \"%s\"", device);
		isatflag = (major(sb.st_rdev) == AT_MAJOR);
		if (isatflag) {
			/* Kludge, see comment above... */
			if (minor(sb.st_rdev) == AT0X_MINOR) {
				drivenum = partbase = 0;
				drivename = "Drive 0";
			} else if (minor(sb.st_rdev) == AT1X_MINOR) {
				drivenum = 1;
				partbase = 4;
				drivename = "Drive 1";
			} /* else huh? */
		}

		/* Do it. */
		fdisk();
	}
	exit(0);
}

/*
 * Copy /coherent to /tmp/coherent and
 * patch /tmp/coherent disk parameters "atparms" with hdparms.
 * Patch /conf/pboot to /tmp/pboot.[01].
 */
void
atpatch()
{
	register int	i, fd;
	int		dbase;
	unsigned char	*cp, *hdp;
	static int	patched;

	if (drivenum == 0)
		dbase = 0;
	else if (drivenum == 1)
		dbase = sizeof hdparms;
	else
		fatal("unrecognized drive number");

	/* Copy /coherent to /tmp/coherent and patch appropriately. */
	/* Not pretty, but better than what's coming next. */
	if (!patched)
		sys("/bin/cp -d /coherent /tmp/coherent");
	sprintf(buf, "/conf/patch /tmp/coherent ");
	for (i = 0, hdp = (char *)&hdparms; i < sizeof hdparms; i++, hdp++) {
		if (*hdp != 0) {
			cp = &buf[strlen(buf)];
			sprintf(cp, "atparm_+%d=%u:c ", dbase + i, *hdp);
		}
	}
	sys(buf);

	/* Copy /conf/pboot to /tmp/pboot.[01], patched appropriately. */
	if ((fd = open("/conf/pboot", 0)) < 0)
		fatal("cannot open /conf/pboot");
	else if (read(fd, buf, NBUF) != NBUF)
		fatal("read error on /conf/pboot");
	else
		close(fd);
	/* Hot patch; if the boot changes this will go down in flames. */
	cp = &buf[0x1F0];
	*cp++ = ncyls & 0xFF;		/* traks lo */
	*cp++ = ncyls >> 8;		/* traks hi */
	*cp++ = nspt;			/* sects */
	*cp++ = nheads;			/* heads */
	*cp++ = hdparms.ctrl;		/* control byte */
	*cp++ = hdparms.wpcc[0];	/* wpcc lo */
	*cp++ = hdparms.wpcc[1];	/* wpcc hi */
	*cp++ = drivenum;		/* drive number */
	if ((fd = creat("/tmp/pboot", 0644)) < 0)
		fatal("cannot create /tmp/pboot");
	else if (write(fd, buf, NBUF) != NBUF)
		fatal("write error on /tmp/pboot");
	else
		close(fd);
	sprintf(buf, "/bin/mv /tmp/pboot /tmp/pboot.%d", drivenum);
	sys(buf);
	++patched;
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
	if (!yes_no("Do you want to make a partition active")) {
		active = -1;
		if (active != oactive)
			++nmods;
		return;
	}
	if (active == -1)
		active = 0;				/* default */
	active = get_int("Active partition", active + partbase, partbase, partbase + NPARTN-1);
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
	int sys, old;
	unsigned int c, h, s;
	unsigned long size, osize, base, obase, end;
	static int optflag;

	/* Get options first time through. */
	if (optflag == 0) {
		cls(0);
		printf(
			"Existing data on a partition will be lost if you change the\n"
			"base or the size of the partition.  Be sure you have backed up\n"
			"all data from any partition which you are going to change.\n"
			"\n"
			"You may specify partition bases in cylinders or in tracks.\n"
			);
		cylflag = yes_no("Do you want to specify bases in cylinders");
		printf("You may specify partition sizes in %s or in megabytes.\n",
			cylflag ? "cylinders" : "tracks");
		megflag = !yes_no("Do you want to specify partition sizes in %s",
			cylflag ? "cylinders" : "tracks");
		++optflag;
		print_part(0);
	}
	p = &hd.hd_partn[n];
	printf("\nPartition %d:\n", n + partbase);
	size = p->p_size;
			
	/* Get new system type. */
	old = p->p_sys;
again:
	if (old != SYS_EMPTY)
		printf("The current operating system type is %s.\n", sys_type(old));
	if (yes_no("Do you want this to be a COHERENT partition"))
		sys = SYS_COH;
	else if (old != SYS_COH && yes_no("Do you want the partition type left unchanged"))
		sys = old;
	else if (old != SYS_EMPTY && yes_no("Do you want the partition marked as unused"))
		sys = SYS_EMPTY;
	else {
		printf(
"This program can mark a partition as a COHERENT partition,\n"
"leave its type unchanged, or mark it as unused.  It cannot\n"
"initialize a partition for use by any other operating system;\n"
"to do so, you must mark it as unused now and subsequently use\n"
"the disk partitioning program provided by the other system\n"
"to initialize it correctly.\n"
			);
		goto again;
	}
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
		base = get_long("Base cylinder", base, 0L, (long) ncyls - 1);
		if (base == 0)
			base = nspt;		/* skip first track for cyl 0 */
		else
			base *= nspt * nheads;	/* cylinders to sectors */
	} else {				/* in tracks */
		base = sec_upto_t(base);
		base = get_long("Base track", base, 1L, (long)ncyls * nheads - 1);
		base *= nspt;			/* tracks to sectors */
	}

	/* Check that base falls at a track boundary. */
	/* It might not if the disk was previously partitioned. */
	c = sec_to_c(base);
	h = sec_to_h(base);
	s = sec_to_s(base);
	if (s != 1) {
		printf("Partitions should begin at a track boundary.\n");
		printf("The partition does not begin at a track boundary with the selected base.\n");
		printf("The next track boundary is at track %u\n", sec_upto_t(base));
		if (yes_no("Do you want to change the partition base"))
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
			size = get_long("Partition size in megabytes", size, 0L,
				(long) meg(nsectors - base));
			size *= 1000000L;	/* megabytes to bytes */
			size /= SSIZE;		/* to sectors */
			size = sec_upto_c(size); /* round up to cylinders */
			size *= nspt * nheads;	/* cylinders to sectors */
		}
	} else if (cylflag) {			/* in cylinders */
		/* Tricky stuff again. */
		end = base + size - 1;
		size = sec_to_c(end) - sec_to_c(base) + 1;
		size = get_long("Partition size in cylinders", size, 0L,
			(long) ncyls - sec_to_c(base));
		size *= nspt * nheads;		/* cylinders to sectors */
	} else {				/* in tracks */
		size = sec_upto_t(size);
		size = get_long("Partition size in tracks", size, 0L,
			(long) sec_upto_t(nsectors - base));
		size *= nspt;			/* tracks to sectors */
	}
	/*
	 * Adjust size to end at cylinder boundary
	 * if it did not start at cylinder boundary.
	 */
	if ((megflag || cylflag) && size != 0 && base % cylsize != 0 && size > base % cylsize)
		size -= base % cylsize;

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
		if (yes_no("Do you want to add %u sectors to the partition size",
			nspt - s)) {
			size += nspt - s;
			s = nspt;
		}
	}

	/* Update the partition table size and end. */
	if (base != obase || size != osize) {
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

again:
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
		printf("According to your computer system, the disk contains\n");
		printf("%u cylinders (0 to %u), %u heads (0 to %u), and %u sectors\n",
			ncyls, ncyls - 1, nheads, nheads -1, nspt);
		printf("per track (1 to %u).  According to the partition table, a partition\n",
			nspt);
		printf("%s at sector %lu, which corresponds to a c:h:s of %u:%u:%u.\n",
			(flag) ? "begins" : "ends" , n, nc, nh, ns);
		printf("But the partition table entry gives a c:h:s of %u:%u:%u.\n",
			c, h, s);

		if (flag) {
			printf(
"\n"
"An inconsistency in a partition table entry usually occurs because\n"
"the system CMOS RAM area specifies the hard disk geometry incorrectly;\n"
"that is, your disk does not contain %u cylinders, %u heads, and %u sectors.\n",
				ncyls, nheads, nspt);
			if (!cflag) {
				printf(
"If you think the above values are wrong, invoke this program again\n"
"using the \"-c\" option to correct them.  Because changing these values\n"
"is dangerous and you have not specified the \"-c\" option, this program\n"
"will now terminate.\n"
					);
				exit(1);
			}
			if (isatflag) {
				printf(
"This program lets you to resolve the inconsistency by specifying correct\n"
"values for the disk geometry (number of cylinders, heads and sectors) or by\n"
"making the partition table entry consistent with the given values.\n"
					);
				if (yes_no("Do you think the above values are wrong")) {
					fix_chs();
					goto again;
				}
			}
		}

		printf("This program will now change the c:h:s of the entry to %u:%u:%u\n",
			nc, nh, ns);
		printf(
"to resolve this inconsistency.  Changing a partition table entry can\n"
"make data on existing filesystems inaccessible.  If you feel this change is\n"
"incorrect, exit from this program now without updating the partition table.\n"
			);
		if (yes_no("Do you want to exit from this program"))
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
 * Prompt for <Enter> if the flag is true or if rflag.
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

#if	DOSSHRINK
/*
 * Shrink an MS-DOS partition.
 * PFM.
 */
void
dos_shrink(n) int n;
{
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
		if (!yes_no("Do you want to shrink the MS-DOS partition anyway"))
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
	if (lseek(cfd, 0L, 0) != 0L)
		fatal("%s: seek failed", device);
	else if (read(cfd, &newhd, sizeof hd) != sizeof hd) {
		close(cfd);
		fatal("%s: read error", device);
	} else
		memcpy(&hd.hd_partn[n], &newhd.hd_partn[n], sizeof(FDISK_S));
}
#endif

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
	int 		nfd, p, flag;
	unsigned	action;

	nmods = 0;
	if ((cfd = open(device, openmode)) < 0)
		fatal("cannot open \"%s\"", device);

	/* Obtain drive characteristics. */
	if (ioctl(cfd, HDGETA, (char *)&hdparms) == -1)
		fatal("cannot get \"%s\" drive characteristics", device);
	ncyls = (hdparms.ncyl[1] << 8) | hdparms.ncyl[0];
	if (ncyls > 1024) {
		printf(
"\n"
"The disk controller says your disk has %d cylinders.\n"
"COHERENT requires cylinder numbers in the range 0 to 1023.\n"
"Accordingly, this program will use 1024 as the effective\n"
"number of cylinders on your disk.\n"
			, ncyls);
		ncyls = 1024;
	}
	nheads = hdparms.nhead;
	nspt = hdparms.nspt;
	cylsize = nheads * nspt;
	nsectors = (long)ncyls * cylsize;

	/* Print drive characteristics and allow user to patch. */
	if (cflag && isatflag) {
		cls(1);
		printf("According to your computer system:\n");
		drive_info();
		if (!yes_no("Do you think the above values are correct"))
			fix_chs();
	}
	close(cfd);

	/* Read the current boot block. */
	cfd = get_boot(device, openmode, &hd);		/* read boot */
	if (mboot != NULL) {
		nfd = get_boot(mboot, 0, &newhd);	/* read new boot */
		close(nfd);
		if (newhd.hd_sig != HDSIG)
			fatal("invalid signature in \"%s\"", mboot);
		memcpy(hd.hd_boot, newhd.hd_boot, sizeof hd.hd_boot);
		nmods++;
	}

	/* If no signature, zap the partition entries. */
	if (hd.hd_sig != HDSIG) {
		memset(hd.hd_partn, 0, sizeof(FDISK_S));
		hd.hd_sig = HDSIG;
		nmods++;
	}

	/* If readonly, print information and return. */
	if (openmode == 0) {
		print_part(0);
		close(cfd);
		return;
	}

	/* Interactive input loop. */
	for (flag = 1; ; ) {
		setjmp(loop);
		print_part(flag);
		flag = 0;
		printf(
			"Possible actions:\n"
			"\t1 = Change active partition (or make no partition active)\n"
			"\t2 = Change one logical partition\n"
			"\t3 = Change all logical partitions\n"
#if	DOSSHRINK
#define	NACTIONS	6
			"\t4 = Shrink an MS-DOS logical parition\n"
			"\t5 = Display drive information\n"
			"\t6 = Quit\n"
#else
#define	NACTIONS	5
			"\t4 = Display drive information\n"
			"\t5 = Quit\n"
#endif
			);
		action = get_int("Action", NACTIONS, 1, NACTIONS);

		switch(action) {
		case 1:
			printf("Change active partition:\n");
			change_active();
			continue;
		case 2:
#if	DOSSHRINK
		case 4:
#endif
			p = (freepart != -1) ? freepart : 0;
			p = get_int("Which partition", p + partbase, partbase, partbase + NPARTN - 1);
			p -= partbase;
			if (action == 2)
				change_part(p);
#if	DOSSHRINK
			else {
				dos_shrink(cfd, p);
				flag = 1;
			}
#endif
			continue;
		case 3:
			for (p=0; p < NPARTN; ) {
				change_part(p++);
				if (p < NPARTN)
					print_part(0);
			}
			continue;

		case NACTIONS-1:
			cls(0);
			drive_info();
			flag = 1;
			continue;
		case NACTIONS:
			if (quit(device, cfd) == 1)
				return;
			continue;
		default:
			continue;
		}
	}	
}

/*
 * Interactively obtain new disk geometry values.
 * Update running /coherent with correct values using HDSETA.
 * Call atpatch to create patched /tmp/coherent and /tmp/boot.[01].
 */
void
fix_chs()
{
	register int i;

	printf(
"Warning: if you specify incorrect disk parameter values, data on\n"
"existing partitions may be lost or your disk may not operate correctly.\n"
"Consult your disk controller manual or call your disk vendor\n"
"if you do not know the correct values.\n"
		);
	if (!yes_no("Are you sure you want to change the disk parameter values"))
		return;
	ncyls = get_int("Number of cylinders", ncyls, 1, 1024);
	nheads = get_int("Number of heads", nheads, 1, 255);
	nspt = get_int("Number of sectors per track", nspt, 1, 255);
	hdparms.ctrl = get_int("Control byte", hdparms.ctrl, 0, 255);
	i = (hdparms.wpcc[1] << 8) | (hdparms.wpcc[0]);
	i = get_int("Write pre-compensation cylinder", i, -1, ncyls+1);
	hdparms.wpcc[1] = i >> 8;
	hdparms.wpcc[0] = i & 0xFF;
	cylsize = nheads * nspt;
	nsectors = (long)ncyls * cylsize;
	hdparms.ncyl[1] = ncyls >> 8;
	hdparms.ncyl[0] = ncyls & 0xFF;
	hdparms.nhead = nheads;
	hdparms.nspt = nspt;
	if (ioctl(cfd, HDSETA, (char *)&hdparms) == -1)
		fatal("cannot set \"%s\" drive characteristics", device);
	atpatch();
}

/*
 * Read boot block from a file into the given structure.
 * Return a file descriptor to the open file.
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
 * Prompt for integer input from the user.
 * Accept data in range min to max.
 * Return a valid result.
 */
int
get_int(prompt, defval, min, max) char *prompt; register int defval, min, max;
{
	int val;
	char *s;

	for (;;) {
		s = get_line("%s [%u]?", prompt, defval);
		if (*s == '\0')
			return defval;		/* take default */
		val = atoi(s);
		if (val >= min && val <= max)
			return val;
		printf("Please enter a value between %u and %u.\n", min, max);
	}
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
	for (s = buf; ; ++s) {
		if (*s == 0x1B)			/* <Esc> returns to loop */
			longjmp(loop, 1);
		else if (*s != ' ' && *s != '\t')
			return s;
	}
}

/*
 * Prompt for long input from the user.
 * Accept data in range min to max.
 * Return the result.
 */
long
get_long(prompt, defval, min, max) char *prompt; register long defval, min, max;
{
	long val;
	char *s;

	for (;;) {
		s = get_line("%s [%lu]?", prompt, defval);
		if (*s == '\0')
			return defval;		/* take default */
		val = atol(s);
		if (val >= min && val <= max)
			return val;
		printf("Please enter a value between %lu and %lu.\n", min, max);
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
	register char c, *s, *dname;
	int i;
	unsigned long end;

	cls(flag);
	printf("%s currently has the following logical partitions:\n", drivename);
	printf("                  [ In Cylinders ]  [    In Tracks    ]\n");
	printf("Number     Type   Start  End  Size  Start    End   Size Megabytes  Name\n");
	for (i = 0; i < NPARTN; ++i) {
		p = &hd.hd_partn[i];
		if (p->p_size == 0L)
			end = p->p_base = 0L;
		else
			end = p->p_base + p->p_size - 1;
		printf("%d", partbase + i);
		printf("%s\t", (p->p_boot == 0x80) ? " Boot" : "");
		printf("%8s ", sys_type(p->p_sys));
		printf("%5u ", sec_to_c(p->p_base));
		printf("%5u ", sec_to_c(end));
		printf("%5u ", sec_upto_c(p->p_size));
		printf("%6lu ", p->p_base / nspt);
		printf("%6lu ", end / nspt);
		printf("%6u ", sec_upto_t(p->p_size));
		printf("%7.2f ", meg(p->p_size));
		dname = device;
		if (strncmp(dname, "/tmp", 4) == 0)
			dname += 4;
		s = &dname[strlen(dname) - 1];
		c = *s;
		*s = 'a' + i;
		printf("%10s ", dname);
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
quit(fname) char *fname;
{
	if (badflag) {
		printf("Because the partition table defines overlapping disk\n");
		printf("partitions, it will not be saved to the disk if you quit.\n");
		if (!yes_no("Do you wish to quit without saving the changes"))
			return 0;
	} else if (nmods != 0) {
		if (yes_no("\nAre you sure you want to write the updated partition table")) {
			if (lseek(cfd, 0L, 0) != 0L)
				fatal("seek failed on \"%s\"", fname);
			else if (write(cfd, &hd, sizeof hd) != sizeof hd)
				fatal("write error on \"%s\"", fname);
			/*
			 * This HDGETA is for the benefit of the SCSI driver,
			 * which needs to reset the parameters if they changed.
			 */
			if (ioctl(cfd, HDGETA, (char *)&hdparms) == -1)
				fprintf(stderr, "HDGETA failed on \"%s\"\n", fname);
			sync();
		} else if (!yes_no("Changes will not be saved.  Quit anyway"))
			longjmp(loop, 2);
		else
			printf("Changes not saved.\n");
	} else
		printf("The partition table is unchanged.\n");
	close(cfd);
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
	unsigned long base, next, size, safe;

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
	safe = nsectors - nspt * nheads;	/* safely usable sectors */
	if (next < safe)
		unused(safe, next);
	else if (next > safe)
		printf(
"\n"
"Warning: the last cylinder of a hard disk is usually reserved for use by\n"
"disk diagnostic programs.  The current disk partitioning uses part of the\n"
"the last cylinder in a disk partition.  Mark Williams strongly recommends\n"
"that you change the partitioning to avoid using the last cylinder.\n"
			);
}

/*
 * Execute a command.
 */
void
sys(cmd) char *cmd;
{
	if (system(cmd) != 0)
		fatal("command \"%s\" failed", cmd);
}

/*
 * Convert system type code i to a string describing the system type.
 * Return a pointer to statically allocated buffer.
 */
char *
sys_type(i) register int i;
{
	static char buf[8+1];	/* longest name is "COHERENT" or "<Unused>"XS */
	register char *s;

	switch (i) {
	case SYS_EMPTY:		s = "<Unused>";	break;
	case SYS_DOS_12:
	case SYS_DOS_16:
	case SYS_DOS_LARGE:
				s = "MS-DOS";	break;
	case SYS_DOS_XP:
				s = "Ext.DOS";	break;
	case SYS_XENIX:		s = "Xenix";	break;
	case SYS_COH:		s = "Coherent";	break;
	case SYS_SWAP:		s = "Swap";	break;
	default:		s = NULL;	break;	
	}

	if (s == NULL)
		sprintf(buf, "%8u ", i);
	else
		strcpy(buf, s);
	return buf;
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

/* end of fdisk.c */
