/*
 * File:	ipcs.c
 *
 * Purpose:	main driver for ipcs utility.
 * Revision 1.1  92/10/08 bin
 * Initial revision
 * 
 */

/*
 * ----------------------------------------------------------------------
 * Includes.
 */
#include <stdio.h>
#include <coff.h>
#include <fcntl.h>
#include "ipcs.h"

/*
 * ----------------------------------------------------------------------
 *	Global data
 */

/* Option's flags. See man pages for more info */
short	qflag =	0,	/* message q */
	mflag =	0,	/* shared memory */
	sflag =	0,	/* semaphores */
	bflag =	0,	/* biggest */
	cflag =	0,	/* creator name */ 
	oflag = 0,	/* outstanding usage */
	pflag = 0,	/* process ID */
	tflag = 0,	/* time */
	aflag = 0,	/* include b, c, o, p, & t */
	Cflag = 0,	/* corefile */
	Nflag = 0;	/* namelist */

int	total_shmids = 0,	/* total shared memory segs found */
	total_sems = 0,		/* total semaphores found */
	usemsqs = 0;		/* is msq in use */

int	NSHMID,			/* total # shared memory segments */
	NSEMID,			/* total # semaphores */
	NMSQID;			/* total # message queues */

main(argc, argv)
int	argc;
char	*argv[];
{
	char		*opstring = "qmsbcoptaVC:N: ";
	extern char	*optarg;
	char		*namelist = NULL,
			*corefile = "/dev/kmem";/* default vlue */
	char		*fname;			/* kernel name */
	int		c;

	while ((c = getopt(argc, argv, opstring)) != EOF)
		switch (c) {
		case 'q':
			qflag = 1;
			break;
		case 'm':
			mflag = 1;
			break;
		case 's':
			sflag = 1;
			break;
		case 'b':
			bflag = 1;
			break;	
		case 'c':
			cflag = 1;
			break;
		case 'o':
			oflag = 1;
			break;
		case 'p':
			pflag = 1;
			break;
		case 't':
			tflag = 1;
			break;
		case 'a':
			aflag = 1;
			break;
		case 'C':
/*			Cflag = 1;
 *			corefile = optarg;
 *			break;
 */
			fprintf(stderr, 
				"ipcs: Corefile option NOT yet supported\n");
			exit(1);
		case 'N':
			Nflag = 1;
			namelist = optarg;
			break;
		case 'V':
			printf("ipcs version %s\n", VERSION);
			exit(0);
		default: 
			usage(c);
	}

	set_flags();
	fname = Nflag ? namelist : pick_nfile();
	getmaxnum(fname, corefile);

	get_data(fname, corefile);

	/* Now we can print */
	if (qflag)
		print_q();
	if (mflag)
		print_m();
	if (sflag)
		print_s();

	exit(0);
}

/* 
 * set_flags does some additional processing for flags 
 */
set_flags() 
{
	/* Default is all ipc */
	if (!(qflag + mflag + sflag))
		qflag = mflag = sflag = 1;

	/* use all print options */
	if (aflag)
		bflag = cflag = oflag = pflag = tflag = 1;
}

/*
 * Get the following values from the corefile:
 *	NSHMID:		max number of allowable shared memory segments
 *	NSEMID:		max number of allowable semaphores
 *	NMSQID:		max number of allowable message queues
 */
getmaxnum(fname, corefile)
char	*fname;		/* Kernel file name */
char	*corefile;	/* Core file name (/dev/kmem by default) */
{
	SYMENT 	sym[3];	/* The table of names to find */
	int	fd;	/* corefile file descriptor */
	int	val;	/* Read values buffer */
	int	i;	/* Loop index */

	/* Initialise SYMENT array */
	for (i = 0; i < 3; i++) {
		sym[i]._n._n_n._n_zeroes = 0;	/* stuff for coffnlist */
		sym[i].n_type = -1;
	}
	strcpy(sym[0].n_name, "NSHMID");
	strcpy(sym[1].n_name, "NSEMID");
	strcpy(sym[2].n_name, "NMSQID");

	/* do lookups. coffnlist returns 0 on error. */
	if (!coffnlist(fname, sym, NULL, 3)) {
		fprintf(stderr, "ipcs: error obtaining values from %s\n", 
									fname);
		exit(1);
	}

	/* Now we got addresses of the variables. So, we can go to corefile
	 * and read proper values. sym[i].n_value contains addresses of
	 * variables.
	 */
	if ((fd = open(corefile, O_RDONLY)) < 0) {
		fprintf(stderr, "ipcs: cannot open %s\n", corefile);
		exit(1);
	}
	/* Get max number of allowable shared memory segments */
	lseek(fd, sym[0].n_value, 0);
	if (read(fd, &val, sizeof(int)) != sizeof(int)) {
		fprintf(stderr, "ipcs: read value of NSHMID error\n");
		exit(1);
	}
	NSHMID = val;
	/* Get max number of allowable semaphores */
	lseek(fd, sym[1].n_value, 0);
	if (read(fd, &val, sizeof(int)) != sizeof(int)) {
		fprintf(stderr, "ipcs: read value of NSEMID error\n");
		exit(1);
	}
	NSEMID = val;
	/* Get max number of allowable message queues */
	lseek(fd, sym[2].n_value, 0);
	if (read(fd, &val, sizeof(int)) != sizeof(int)) {
		fprintf(stderr, "ipcs: read value of NSHMID error\n");
		exit(1);
	}
	NMSQID = val;
	close(fd);
}


/* ipcs usage. Print message and die */
usage(c) 
int	c;
{
	fprintf(stderr, "ipcs:  illegal option - %c\n", c);
	fprintf(stderr, "usage: "
		 "ipcs [-abcmopqstV] [-C corefile] [-N namelist]\n");
	exit(1);
}
