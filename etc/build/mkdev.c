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
 *
 * $Log:	mkdev.c,v $
 * Revision 1.5  91/06/17  08:13:40  hal
 * Allow for older Future Domain host adapters.
 * 
 * Revision 1.4  91/06/17  08:09:22  hal
 * Shipped with 3.2.0.
 * 
 * Revision 1.3  91/05/30  12:22:24  hal
 * Patch SS_INT and SS_BASE.
 * 
 * Revision 1.2  91/05/24  03:06:43  hal
 * Add Seagate and Future Domain.
 * 
 */

#include <stdio.h>
#include <sys/devices.h>
#include "build0.h"

#define	VERSION		"V1.2"		/* version number */
#define	USAGEMSG	"Usage:\t/etc/mkdev [ -bdv ] [ scsi ]\n"
#define BUFLEN		50
#define AHA_HDS		64
#define TANDY_HDS	16

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
{ char *drv, *coh, *dev;
	int i, id, lun, rootflag;
	int ss_dev = 0;
	int fut_dev = 0;
	short nsdrive = 0;
	int ss_int = 5, new_int;
	unsigned int ss_base = 0xCA00, new_base;
	unsigned char ss_patch[80], buf[BUFLEN];
	FILE *fp;
	int aha_dev = 0, sd_hds = AHA_HDS;

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
"(2) Seagate ST01 or ST02\n"
"(3) Future Domain TMC-845/850/860/875/885\n"
"(4) Future Domain TMC-840/841/880/881\n"
"\n"
		);
retry:
	switch(get_int(0, 4, "Enter a number from the above list or 0 to exit:")) {
	case 0:
		return;
	case 1:
		aha_dev = 1;
		drv = "/drv/aha154x";
		coh = "aha";
		break;
	case 2:
		ss_dev = 1;
		drv = "/drv/ss";
		coh = "ss";
		break;
	case 3:
		ss_dev = 1;
		fut_dev = 1;
		drv = "/drv/ss";
		coh = "ss";
		nsdrive |= 0x8000;
		break;
	case 4:
		ss_dev = 1;
		fut_dev = 1;
		drv = "/drv/ss";
		coh = "ss";
		nsdrive |= 0x4000;
		break;
	default:
		goto retry;		/* should never happen */
	}

	/*
	 * If Adaptec, allow patching host adapter variables SD_HDS
	 * for Tandy variant of host BIOS.
	 */
	if (aha_dev) {
printf("\nMost versions of the Adaptec BIOS use 64-head translation mode.\n");
printf("A few, including some Tandy variants, use 16-head translation mode.\n\n");
		if (!yes_no("Do you want 64-head translation mode"))
			sd_hds = TANDY_HDS;
	}

	/*
	 * If Seagate or Future Domain, allow patching host adapter
	 * variables SS_INT and SS_BASE.
	 */
	if (ss_dev) {
printf("\nPlease refer to the installation guide for your host adapter.\n");

		/* Get value to patch for SS_INT */
printf("\nWhich IRQ number does the host adapter use [%d]? ", ss_int);
		while (1) {
			new_int = ss_int;
			fgets(buf, BUFLEN, stdin);
			sscanf(buf, "%d", &new_int);
			if (new_int < 3 || new_int > 15)
printf("Type a number between 3 and 15 or just <Enter> for the default: ");
			else
				break;
		} /* endwhile */
		ss_int = new_int;

		/* Get value to patch for SS_BASE */
printf("Your host adapter is configured for a base segment address.  Possible\n");
printf("values are: C800, CA00, CC00, CE00, DC00, and DE00.\n");
printf("What is your 4-digit hexadecimal base address [%04X]? ", ss_base);
		while (1) {
			new_base = ss_base;
			fgets(buf, BUFLEN, stdin);
			sscanf(buf, "%x", &new_base);
			if (new_base < 0xC800 || new_base > 0xDE00)
printf("Type a number between C800 and DE00 or just <Enter> for the default: ");
			else
				break;
		} /* endwhile */
		ss_base = new_base;
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

	/* Make device nodes. */
	cls(0);
	printf(
"You must specify a SCSI-ID (0 through 7) for each SCSI hard disk device.\n"
"Each SCSI hard disk device can contain up to four partitions.\n\n"
		);

newdev:
	id = get_int(0, 7, "Enter the SCSI-ID:");
#if	1
	lun = 0;
	nsdrive |= (1 << id);
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
		sprintf(cmd, "/etc/mknod -f %s/sd%d%c b %d %d",
			dev, id, 'a'+i, SCSI_MAJOR, SCSI_minor(0, id, lun, i));
		sys(cmd, S_NONFATAL);
	}
	sprintf(cmd, "/etc/mknod -f %s/sd%dx b %d %d",
		dev, id, SCSI_MAJOR, SCSI_minor(1, id, lun, 0));
	sys(cmd, S_NONFATAL);

	/* Make the raw devices. */
	for (i = 0; i < 4; i++) {
		sprintf(cmd, "/etc/mknod -f %s/rsd%d%c c %d %d",
			dev, id, 'a'+i, SCSI_MAJOR, SCSI_minor(0, id, lun, i));
		sys(cmd, S_NONFATAL);
	}
	sprintf(cmd, "/etc/mknod -f %s/rsd%dx c %d %d",
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
	if (yes_no("Do you have another SCSI hard disk device on this host adapter"))
		goto newdev;

	/*
	 * Ugly patching stuff specific to "ss" driver.
	 * At this point all SCSI id's attached to the host are known.
	 */
	if (ss_dev) {
		int unit;

		/* "ss" device driver requires patching to work at all. */
		sprintf(ss_patch,
			"NSDRIVE_=0x%04x SS_INT_=%d SS_BASE_=0x%04x",
			nsdrive, ss_int, ss_base);

		/*
		 * Write PATCHFILE which is run by build.
		 * Tell it to patch the driver.
		 * Patch /tmp/coherent if there is one.
		 */
		if (bflag) {
			fp = fopen(PATCHFILE, "a");
			fprintf(fp, "/conf/patch /mnt%s %s\n", drv, ss_patch);
			fclose(fp);

			if(rootflag) {
sprintf(cmd, "/conf/patch /tmp/coherent %s\n", ss_patch);
				sys(cmd, S_FATAL);
			}
		} else {
			sprintf(cmd, "/conf/patch %s %s", drv, ss_patch);
			sys(cmd, S_FATAL);
		}

		/* Load the driver for the device. */
		if (bflag) {
			sprintf(cmd, "/bin/mkdir /tmp/drv");
			sys(cmd, S_FATAL);
			sprintf(cmd, "/bin/cp %s /tmp%s", drv, drv);
			sys(cmd, S_FATAL);
			sprintf(cmd, "/conf/patch /tmp%s %s", drv, ss_patch);
			sys(cmd, S_FATAL);
			sprintf(cmd, "/etc/drvld -r /tmp%s", drv);
			sys(cmd, S_FATAL);
			sprintf(cmd, "/bin/rm /tmp%s", drv);
			sys(cmd, S_FATAL);
		} else {
			sprintf(cmd, "/etc/drvld -r %s", drv);
			sys(cmd, S_FATAL);
		}

		/*
		 * Allow patching of the loaded driver parameters.
		 */
		for (unit = 0; unit < 7; unit++)
			if (nsdrive & (1<<unit)) {
				sprintf(cmd, "/etc/hdparms -b%c %s/sd%dx",
					(fut_dev)?'f':'s', dev, unit);
				sys(cmd, S_NONFATAL);
			}
	} /* end of "ss" stuff */

	/*
	 * Ugly patching stuff specific to "aha154x" driver.
	 * At this point all SCSI id's attached to the host are known.
	 */
	if (aha_dev) {
		/*
		 * Tandy Adaptec BIOS spoofs different head count than
		 * Adaptec's Own Translation Mode.
		 */
		sprintf(ss_patch,
			"SD_HDS_=%d", sd_hds);

		/*
		 * Write PATCHFILE which is run by build.
		 * Tell it to patch the driver.
		 * Patch /tmp/coherent if there is one.
		 */
		if (bflag) {
			fp = fopen(PATCHFILE, "a");
			fprintf(fp, "/conf/patch /mnt%s %s\n", drv, ss_patch);
			fclose(fp);

			if(rootflag) {
sprintf(cmd, "/conf/patch /tmp/coherent %s\n", ss_patch);
				sys(cmd, S_FATAL);
			}
		} else {
			sprintf(cmd, "/conf/patch %s %s", drv, ss_patch);
			sys(cmd, S_FATAL);
		}

		/* Load the driver for the device. */
		if (bflag) {
			sprintf(cmd, "/bin/mkdir /tmp/drv");
			sys(cmd, S_FATAL);
			sprintf(cmd, "/bin/cp %s /tmp%s", drv, drv);
			sys(cmd, S_FATAL);
			sprintf(cmd, "/conf/patch /tmp%s %s", drv, ss_patch);
			sys(cmd, S_FATAL);
			sprintf(cmd, "/etc/drvld -r /tmp%s", drv);
			sys(cmd, S_FATAL);
			sprintf(cmd, "/bin/rm /tmp%s", drv);
			sys(cmd, S_FATAL);
		} else {
			sprintf(cmd, "/etc/drvld -r %s", drv);
			sys(cmd, S_FATAL);
		}
	} /* end of "aha154x" stuff */

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


#if	0
	if (yes_no("Is there another SCSI host adapter in your system"))
		goto again;
#endif
}
/* end of mkdev.c */
