/* (lgl-
 *	The information contained herein is a trade secret of Mark Williams
 *	Company, and  is confidential information.  It is provided  under a
 *	license agreement,  and may be  copied or disclosed  only under the
 *	terms of  that agreement.  Any  reproduction or disclosure  of this
 *	material without the express written authorization of Mark Williams
 *	Company or pursuant to the license agreement is unlawful.
 *
 *	COHERENT Version 4.0
 *	Copyright (c) 1982, 1992.
 *	All rights reserved.
 -lgl) */
/*
 * uname.c print information about current COHERENT system.
 */
#include	<stdio.h>
#include	<fcntl.h>
#include	<sys/utsname.h>
#include	<string.h>

main(argc, argv)
int	argc;
char	*argv[];
{
	extern char	*optarg;
	extern int	optind;
	static int	snf = 0,	/* Default */
			nnf = 0,
			srf = 0, 
			svf = 0, 
			mhf = 0,
			Snf = 0;
	char		*sname;
	int		c;
	struct utsname	tsname;

	while ((c = getopt(argc, argv, "snrvmaS:")) != EOF)
		switch (c) {
		case 's':	/* Print system name (default). */
			snf = 1;
			break;
		case 'n':	/* Print node name */
			nnf = 1;
			break;
		case 'r':	/* Print system release */
			srf = 1;
			break;
		case 'v':	/* Print system version */
			svf = 1;
			break;
		case 'm':	/* Print machine hardware name */
			mhf = 1;
			break;
		case 'a':	/* Print all above */
			nnf = srf = svf = mhf = 1;
			break;
		case 'S':	/* Change system name */
			Snf = 1;
			sname = optarg;			
			break;
		default:
			usage();
		}
	if ((snf || nnf || srf || svf || mhf) && Snf)
		usage();
	if (Snf)
		changename(sname);
	else {
		if (uname(&tsname) < 0) {
			perror("uname");
			exit(1);
		}
		if (snf)
			printf("%.*s ", SYS_NMLN, tsname.sysname);
		if (nnf)
			printf("%.*s ", SYS_NMLN, tsname.nodename);
		if (srf)
			printf("%.*s ", SYS_NMLN, tsname.release);
		if (svf)
			printf("%.*s ", SYS_NMLN, tsname.version);
		if (mhf)
			printf("%.*s ", SYS_NMLN, tsname.machine);
		puts("");
	}
		
	exit(0);
}

/*
 * Change system name.
 */
changename(sname)
char	*sname;
{
	char	newname[SYS_NMLN + 2];
	int	fd;
	char	*uucpname = "/etc/uucpname";

	if (strlen(sname) >= SYS_NMLN) {
		fprintf(stderr, "uname: name must be <= 8 characters.\n");
		exit(1);
	}
	strcpy(newname, sname);
	strcat(newname, "\n");
	if ((fd = open(uucpname, O_RDWR | O_TRUNC, 0644)) < 0) {
		perror("uname");
		exit(1);
	}
	if (write(fd, newname, strlen(newname)) != strlen(newname)) {
		perror("uname");
		exit(1);
	}
}

usage()
{
	printf("usage:\tuname [-snrvma]\n\tuname [-S system_name]\n");
	exit(1);
}
