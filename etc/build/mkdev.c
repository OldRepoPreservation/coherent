/*
 * mkdev.c
 * 10/25/90
 * Allow the user to configure devices requiring loadable drivers.
 * Uses common routines in build0.c: cc -o mkdev mkdev.c build0.c
 * Usage: mkdev [ -bdv ] scsi
 * Options:
 *	-b	Use special processing when invoked from /etc/build
 *	-d	Debug; echo commands without executing
 *	-v	Verbose
 */

#include <stdio.h>
#include <sys/devices.h>
#include "build0.h"

#define	VERSION		"V1.1"		/* version number */
#define	USAGEMSG	"Usage:\t/etc/mkdev [ -bdv ] [ scsi ]\n"

/* Forward. */
void	scsi();

/* Globals. */
int	bflag;				/* Invoked from /etc/build. */

main(argc, argv) int argc; char *argv[];
{
	register char *s;

	argv0 = argv[0];
	usagemsg = USAGEMSG;
	if (argc > 1 && argv[1][0] == '-') {
		for (s = &argv[1][1]; *s; ++s) {
			switch(*s) {
			case 'b':	++bflag;	break;
			case 'd':	++dflag;	break;
			case 'v':	++vflag;	break;
			case 'V':
				fprintf(stderr, "mkdev: %s\n", VERSION);
				break;
			default:	usage();	break;
			}
		}
		--argc;
		++argv;
	}

	if (argc == 1) {
		/* Do everything mkdev knows how to do. */
		scsi();
	} else {
		/* Do specified things. */
		while (--argc > 0) {
			if (strcmp(argv[1], "scsi") == 0)
				scsi();
			else
				usage();
			++argv;
		}
	}
	exit(0);
}

void
scsi()
{
	char *drv, *coh, *dev;
	int i, id, lun, rootflag;

	rootflag = 0;
#if	0
	/* For future use, not much use with only one supported host adapter. */
again:
#endif
	cls(0);
	printf(
"COHERENT currently supports the following SCSI host adapters:\n"
"\n"
"(1) Adaptec AHA-154x series\n"
"\n"
		);
retry:
	switch(get_int(0, 1, "Enter a number from the above list or 0 to exit:")) {
	case 0:
		return;
	case 1:
		drv = "/drv/aha154x";
		coh = "aha";
		break;
	default:
		goto retry;		/* should never happen */
	}
	
	/* If rootdev is SCSI, copy /coherent.xxx to /tmp/coherent. */
	if (bflag && !rootflag) {
		cls(0);
#if	0
		printf(
"If your computer system includes both a standard AT-type hard disk and\n"
"a SCSI hard disk, you must put the COHERENT root partition on the AT disk.\n"
"If it includes only a SCSI disk, the COHERENT root partition must be on\n"
"the SCSI disk.\n"
			);
#endif
		if (yes_no("Will the COHERENT root partition be on this SCSI device")) {
			++rootflag;
			sprintf(cmd, "/bin/cp /coherent.%s /tmp/coherent", coh);
			sys(cmd, S_FATAL);
			if (yes_no(
"Does your computer system include both a standard AT-type hard disk\n"
"and a SCSI hard disk"
				)) {
				sprintf(cmd, "/bin/echo /etc/drvld -r /drv/at >>%s",
					(bflag) ? "/tmp/drvld.all" : "/etc/drvld.all");
				sys(cmd, S_FATAL);
			}
		}
	}

	/* Load the driver for the device. */
	sprintf(cmd, "/etc/drvld -r %s", drv);
	sys(cmd, S_FATAL);

	/*
	 * If the device is not the root,
	 * append a line to load the driver
	 * to /etc/drvld.all or /tmp/drvld.all.
	 */
	if (!rootflag) {
		sprintf(cmd, "/bin/echo /etc/drvld -r %s >>%s",
			drv, (bflag) ? "/tmp/drvld.all" : "/etc/drvld.all");
		sys(cmd, S_FATAL);
	}

	/* Make device nodes. */
newdev:
	cls(0);
	printf(
"You must specify a SCSI-ID (0 through 7) for each SCSI hard disk device.\n"
"Each SCSI hard disk device can contain up to four partitions.\n\n"
		);
	id = get_int(0, 7, "Enter the SCSI-ID:");
#if	1
	lun = 0;
#else
	lun = get_int(0, 3, "Enter the LUN:");
#endif

	/* Make /tmp/dev if bflag. */
	if (bflag) {
		if ((i = is_dir("/tmp/dev")) == 0)
			sys("/bin/mkdir /tmp/dev", S_FATAL);
		else if (i == -1)
			fatal("/tmp/dev is not a directory");
	}
	dev = (bflag) ? "/tmp/dev" : "/dev";

	/* Make the cooked devices. */
	for (i = 0; i < 4; i++) {
		sprintf(cmd, "/etc/mknod %s/sd%d%c b %d %d",
			dev, id, 'a'+i, SCSI_MAJOR, SCSI_minor(0, id, lun, i));
		sys(cmd, S_NONFATAL);
	}
	sprintf(cmd, "/etc/mknod %s/sd%dx b %d %d",
		dev, id, SCSI_MAJOR, SCSI_minor(1, id, lun, 0));
	sys(cmd, S_NONFATAL);

	/* Make the raw devices. */
	for (i = 0; i < 4; i++) {
		sprintf(cmd, "/etc/mknod %s/rsd%d%c c %d %d",
			dev, id, 'a'+i, SCSI_MAJOR, SCSI_minor(0, id, lun, i));
		sys(cmd, S_NONFATAL);
	}
	sprintf(cmd, "/etc/mknod %s/rsd%dx c %d %d",
		dev, id, SCSI_MAJOR, SCSI_minor(1, id, lun, 0));
	sys(cmd, S_NONFATAL);

	/* Set the device permissions. */
	sprintf(cmd, "/bin/chmod 0600 %s/sd* %s/rsd*", dev, dev);
	sys(cmd, S_NONFATAL);
	sprintf(cmd, "/bin/chown sys %s/sd*[a-d] %s/rsd*[a-d]", dev, dev);
	sys(cmd, S_NONFATAL);
	sprintf(cmd, "/bin/chgrp sys %s/sd*[a-d] %s/rsd*[a-d]", dev, dev);
	sys(cmd, S_NONFATAL);
	sprintf(cmd, "/bin/chown root %s/sd*x %s/rsd*x", dev, dev);
	sys(cmd, S_NONFATAL);
	sprintf(cmd, "/bin/chgrp root %s/sd*x %s/rsd*x", dev, dev);
	sys(cmd, S_NONFATAL);

	/* Append lines to /tmp/devices to pass device info to /etc/build. */
	if (bflag) {
		for (i = 0; i < 4; i++) {
			sprintf(cmd, "/bin/echo sd%dx sd%d%c %d %d >>/tmp/devices",
				id, id, 'a'+i, SCSI_MAJOR, SCSI_minor(0, id, lun, i));
			sys(cmd, S_NONFATAL);
		}
	}
	if (yes_no("Do you have an additional SCSI hard disk device on this host adapter"))
		goto newdev;

#if	0
	if (yes_no("Is there another SCSI host adapter in your system"))
		goto again;
#endif
}

/* end of mkdev.c */
