#include <stdio.h>
#include "ipcs.h"

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
	total_sems = 0;		/* total semaphores found */
	total_msgs = 0;		/* total message queues found */

int	NSHMID,			/* total # shared memory segments */
	NSEMID,			/* total # semaphores */
	NMSQID;			/* total # message queues */

main(argc, argv)
int	argc;
char	*argv[];
{
	char		*opstring = "qmsbcoptavC:N: ";
	extern char	*optarg;
	char		*namelist = NULL,
			*corefile = NULL;
	int		c;

	while ((c = getopt(argc, argv, opstring)) != EOF)
		switch (c) {
		case 'q':
			qflag = 1;
			printf("Qflag set!\n");
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
			printf("ipcs: Corefile option NOT yet supported\n");
			exit(1);
		case 'N':
			Nflag = 1;
			namelist = optarg;
			break;
		case 'v':
			printf("ipcs version %s\n", VERSION);
			exit(0);
		default: 
			usage(c);
	}

	set_flags();

	/* get max values for shared memory, semaphores and message queues */

	NSHMID = get_krnl_vals("NSHMID",Nflag ? namelist : pick_nfile() );
	NSEMID = get_krnl_vals("NSEMID",Nflag ? namelist : pick_nfile() );
	NMSQID = get_krnl_vals("NMSQID",Nflag ? namelist : pick_nfile() );

	/* Get ipc ids and data */
	/* We should check the return status from them */

	get_data();

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

