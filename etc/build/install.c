/*
 * install.c
 * 4/5/90
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

#define	VERSION		"1.5"
#define	USAGE		"Usage: /etc/install [ -bdv ] id device ndisks\n"

/* Forward. */
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
	sprintf(cmd, "/bin/rm -f /%s.* /tmp/%s.*", id, id);
	sys(cmd, S_IGNORE);
	if (bflag)
		sys("/etc/mount.all", S_NONFATAL);
	cls(0);
	for (i = 1; i <= ndisks; ++i)
		install(i);
	if (bflag) {
		newusr();
		done();
	}
	sprintf(cmd, "/tmp/%s.post", id);
	if (exists(cmd)) {
		cls(0);
		sys(cmd, S_NONFATAL);
	}
	if (bflag)
		sys("/etc/umount.all", S_NONFATAL);
	cls(0);
	printf("You have completed the installation procedure successfully.\n");
	sync();
	exit(0);
}

/*
 * Finish up.
 */
void
done()
{
	FILE *fp;
	char *s;

	cls(0);

	/* Replace the install version of /etc/brc with the normal one. */
	sys("/bin/rm /etc/brc", S_NONFATAL);
	sys("/bin/ln -f /etc/brc.coh /etc/brc", S_NONFATAL);

	/* Serial number. */
	printf(
"A card included with your distribution gives the serial number\n"
"of your copy of COHERENT.\n"
		);
	s = get_line("Type in the serial number from the card:");
	if (dflag)
		fp = NULL;
	else if ((fp = fopen("/etc/serialno", "w")) != NULL) {
		fprintf(fp, "%s\n", s);
		fclose(fp);
		chmod("/etc/serialno", 0444);
	} else
		nonfatal("/etc/serialno: open failed");
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
	register int n;
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
"You should create a login for each additional user.\n"
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
			if (!exists(homedir)) {
				sprintf(cmd, "/bin/mkdir %s", homedir);
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
}

/* end of install.c */

