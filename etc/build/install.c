/*
 * install.c
 * 4/24/90
 * Install COHERENT disks on a system.
 * The first part of the initial install procedure is in build.c.
 * Uses common routines in build0.o: cc install.c build0.c
 * Usage: install [ -bdv ] id device ndisks
 * Options:
 *	-b	Build: special processing for build, part 2.
 *	-d	Debug, echo commands without executing
 *	-v	Verbose
 */

#include <stdio.h>
#include "build0.h"

#define	VERSION		"1.6"
#define	USAGE		"Usage: /etc/install [ -bdv ] id device ndisks\n"

/* Forward. */
void	config();
void	done();
void	install();
int	newdisk();
void	newusr();

/* Globals. */
int	bflag;				/* build flag		*/
char	*device;			/* special device name	*/
char	*id;				/* disk id		*/
int	ndisks;				/* number of disks	*/

main(argc, argv) int argc; char *argv[];
{
	register char *s;
	register int i;

	argv0 = argv[0];
	usagemsg = USAGE;
	if (argc > 1 && argv[1][0] == '-') {
		for (s = &argv[1][1]; *s; ++s) {
			switch(*s) {
			case 'b':	++bflag;	break;
			case 'd':	++dflag;	break;
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
	if (argc != 4)
		usage();
	id = argv[1];
	device = argv[2];
	ndisks = atoi(argv[3]);

	/* Add line to /etc/install.log. */
	sprintf(cmd, "/bin/echo /etc/install: %s %s %s >>/etc/install.log",
		argv[1], argv[2], argv[3]);
	sys(cmd, S_NONFATAL);
	sys("/bin/date >>/etc/install.log", S_NONFATAL);

	/* Remove old ids and postfile if present. */
	sprintf(cmd, "/bin/rm -f /%s.* /conf/%s.post", id, id);
	sys(cmd, S_IGNORE);
	if (bflag)
		sys("/etc/mount.all", S_NONFATAL);
	cls(0);

	/* Install disks. */
	for (i = 1; i <= ndisks; ++i)
		install(i);
	if (bflag) {
		newusr();
		config();
		done();
	}

	/* Delete ids and execute postfile if present. */
	sprintf(cmd, "/bin/rm -f /%s.*", id);
	sys(cmd, S_NONFATAL);
	sprintf(cmd, "/conf/%s.post", id);
	if (exists(cmd)) {
		cls(0);
		sys(cmd, S_NONFATAL);
	}
	sys("/bin/echo /etc/install: success >>/etc/install.log", S_NONFATAL);
	sys("/bin/date >>/etc/install.log", S_NONFATAL);
	sys("/bin/echo >>/etc/install.log", S_NONFATAL);
	if (bflag)
		sys("/etc/umount.all", S_NONFATAL);
	cls(0);
	printf("You have completed the installation procedure successfully.\n");
	sync();
	exit(0);
}

/*
 * System-specific configuration.
 */
void
config()
{
	register char *s;
	char c1, c2;

	cls(1);
	if (yes_no("Does your computer system have a modem")) {
		do {
			s = get_line("Enter 1 if your modem is on serial port COM1, 2 if on COM2:");
		} while ((*s != '1' && *s != '2') || *(s+1) != '\0');
		sprintf(cmd, "/bin/ln -f /dev/com%s /dev/modem", s);
		if (sys(cmd, S_NONFATAL) == 0)
			printf("/dev/modem is now linked to /dev/com%s.\n",
				s);
		printf("\n");
	}
	if (yes_no("Does your computer system have a line printer")) {
		printf(
"Your printer is connected to your computer system either through a\n"
"parallel port or through a serial port; most printers are connected\n"
"through parallel port LPT1.\n"
			);
		if (yes_no("Is your printer connected through a parallel port")) {
			do {
				s = get_line("Enter 1, 2 or 3 for port LPT1, LPT2 or LPT3:");
			} while (*s < '1' || *s > '3' || *(s+1) != '\0');
			sprintf(cmd, "/bin/ln -f /dev/lpt%s /dev/lp", s);
			if (sys(cmd, S_NONFATAL) == 0)
				printf("/dev/lp is now linked to /dev/lpt%s.\n",
					s);
		} else {
			do {
				s = get_line("Enter 1 or 2 for port COM1 or COM2:");
			} while ((*s != '1' && *s != '2') || *(s+1) != '\0');
			sprintf(cmd, "/bin/ln -f /dev/com%s /dev/lp", s);
			if (sys(cmd, S_NONFATAL) == 0)
				printf("/dev/lp is now linked to /dev/com%s.\n",
					s);
		}
		printf("\n");
	}
	if (yes_no("Do you use both COHERENT and MS-DOS on your hard disk")) {
		do {
			s = get_line("Enter the partition number (0 to 7) of your MS-DOS partition:");
		} while (*s < '0' || *s > '7' || *(s+1) != '\0');
		*s -= '0';
		c1 = *s < 4 ? '0' : '1';
		c2 = 'a' + *s % 4;
		sprintf(cmd, "/bin/ln -f /dev/rat%c%c /dev/dos", c1, c2);
		if (sys(cmd, S_NONFATAL) == 0)
			printf(
"/dev/dos is now linked to /dev/rat%c%c.\n"
"You can use the \"dos\" command to transfer files\n"
"to and from the MS-DOS partition.\n",
			c1, c2);
		printf("\n");
	}
}

/*
 * Finish up.
 */
void
done()
{
	cls(1);

	/* Replace the install version of /etc/brc with the normal one. */
	sys("/bin/rm /etc/brc", S_NONFATAL);
	sys("/bin/ln -f /etc/brc.coh /etc/brc", S_NONFATAL);
}

/*
 * Install disk n.
 */
void
install(n) int n;
{
	register int i;

again:
	get_line("Insert a disk from the installation kit into the drive and hit <Enter>.",
		n);
	sprintf(cmd, "/etc/mount %s /mnt -r", device);
	if (sys(cmd, S_NONFATAL))
		goto again;
	if ((i = newdisk()) == 0) {
		sprintf(cmd, "/etc/umount %s", device);
		sys(cmd, S_NONFATAL);
		goto again;
	}
	printf("Copying disk %d.  This will take a few minutes...\n", i);
	sprintf(cmd, "cpdir -ad%s -smnt /mnt /", (vflag) ? "v" : "");
	sys(cmd, S_FATAL);
	sprintf(cmd, "/etc/umount %s", device);
	sys(cmd, S_NONFATAL);
	sprintf(cmd, "/bin/echo /etc/install: disk %d installed >>/etc/install.log",
		i);
	sys(cmd, S_NONFATAL);
}

/*
 * Check for an appropriate id on the disk on /mnt.
 * Return 0 if not found, disk number otherwise.
 */
int
newdisk()
{
	register int i;
	static int n;

	if (dflag)
		return (n >= ndisks) ? 0 : ++n;
	for (i = 1; i <= ndisks; i++) {
		sprintf(buf, "/mnt/%s.%d", id, i);
		if (!exists(buf))
			continue;			/* not disk i */
		sprintf(buf, "/%s.%d", id, i);
		if (exists(buf)) {			/* exists on root */
			printf(
				"The disk you inserted is disk %d of the kit;\n"
				"it has already been copied to the hard disk.\n"
				"Please try again.\n",
				i);
			return 0;			/* wrong disk */
		}
		return i;				/* ok */
	}
	printf(
		"The disk you inserted is not part of the kit.\n"
		"Please try again.\n"
		);
	return 0;					/* no id found */
}

/*
 * Install new users.
 */
void
newusr()
{
	register int n, status;
	register char *s;
	char homedir[NBUF];

	cls(0);
	printf(
"Your COHERENT system initially allows logins by users \"root\" (superuser)\n"
"and \"bin\" (system administrator).  In addition, the password file contains\n"
"special entries for \"remacc\" (to control remote access, e.g. via modem),\n"
"\"daemon\" (the spooler), \"sys\" (to access system information), and\n"
"\"uucp\" (for communication with other COHERENT systems).\n"
"\n"
"If your system has multiple users or allows remote logins, you should use\n"
"the \"passwd\" program after you finish installing COHERENT to assign a\n"
"password to each user, including users \"root\" and \"bin\".\n"
"\n"
"You should create a login for each additional user of your system.\n"
		);
	for (n = 0; ;) {
		if (!yes_no("Do you want to create another login"))
			break;
		if (++n == 1) {
			printf(
"You must specify a login name and a full name for each user.\n"
"Joe Smith might have login name \"joe\" and full name \"Joseph H. Smith.\"\n"
"His home directory would be in \"/usr\" by default, namely \"/usr/joe\".\n"
"Do not type quotation marks around the names you enter.\n"
				);
			if (yes_no("Do you want home directories in \"/usr\"")) 
				strcpy(homedir, "/usr");
			else {
again:
				s = get_line("Where do you want home directories?");
				if (*s != '/') {
					printf(
"Please enter a name beginning with '/', such as \"/u\".\n"
						);
					goto again;
				} else
					strcpy(homedir, s);
			}
			if ((status = is_dir(homedir)) == -1) {
				printf("%s is not a directory, try again.\n",
					homedir);
				goto again;
			} else if (status == 0) {
				sprintf(cmd, "/bin/mkdir -r %s", homedir);
				if (sys(cmd, S_NONFATAL) != 0)
					goto again;
			}
		}
		s = get_line("Login name:");
		sprintf(cmd, "/etc/newusr %s ", s);
		s = get_line("Full name:");
		sprintf(&cmd[strlen(cmd)], "\"%s\" %s", s, homedir);
		sys(cmd, S_NONFATAL);
	}
	printf("\n");
}

/* end of install.c */

