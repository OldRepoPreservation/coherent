/*
 * badscan.c
 * 3/22/90
 * Usage: badscan [-v] [-o proto] [-b boot] device count
 *	device	  device to be scanned (e.g. /dev/rat0a)
 *	count	  size of file system to be scanned
 *	sdev	  master boot record for device size info (e.g. /dev/at0x)
 * Options:
 *	-b boot	  use "boot" for boot in prototype file (default: /conf/boot)
 *	-o proto  output file name for prototype file  (default: stdout)
 *	-v	  verbose messages as to the percentage of file system scanned
 *
 * $Log: $
 * 88/03/23	Allan Cornish	/usr/src/cmd/etc/badscan.c
 * Reads are no longer attempted past logical end of partition,
 * when a logical track straddles the partition boundary.
 *
 * 88/01/27	Jim Belton	/usr/src/cmd/etc/badscan.c
 * Fixed bug for counts >= 32K by declaring atol().
 *
 * 86/11/03	Joe Iu
 * Added -o and -b options, where the protofile and the boot file name
 * can be specified by the user explicitly.
 *
 * 85/12/03	Allan Cornish
 * Multi-sector read algorithm adjusted to ensure track alignment,
 * even on track 0 where master boot block is not included in partition.
 *
 * 85/11/16	Allan Cornish
 * Added track reads, with block by block retry on error.
 *
 * 85/11/13	Allan Cornish
 * Added estimate of time remaining to each progress report.
 *
 * 85/11/06	Allan Cornish
 * Added -v flag to report progress every 10 percent of scan.
 *
 * 88/07/11     Jim Napier
 * Modified sput() to retry write if returns zero.  This was causing an
 * invalid prototype file to be created when using the Seagate ST-251
 * hard disk which contains bad blocks.
 */

#include <sys/stat.h>
#include <sys/fdisk.h>
#include <sys/hdioctl.h>

#define	BUFSIZ	512
#define	MAXUINT	65535
#define	NULL	((char *)0)
#define	TRUE	(0 == 0)

/*
 * External functions.
 */
extern	long	atol();
extern	char	*malloc();

/*
 * Forward referenced functions.
 */
void		bad();
void		fatal();
void		lput();
void		sput();
void		usage();

/*
 * Global variables.
 */
char		*bfile = "/conf/boot";
int		bflag;
int 		oflag;
struct stat	sb;
int		vflag;

/**
 * main 	- validate number and type of arguments.
 *		- scan device (arg1) for bad blocks until lim (arg2) reached.
 *		- generate a mkfs prototype file on standard output or the
 *		  designated protofile.
 */
main(argc, argv) register int argc; register char *argv[];
{
	register int n;
	long bno;
	long incr;
	long lim;
	long t0, t1;
	int percent;
	int nspt;
	char *buf;
	hdparm_t hdparms;
	unsigned int tracksize;
#if	1
	unsigned long base;
#endif

	/*
	 * Read options declared.
	 */
	while((--argc > 0) && ((*++argv)[0] == '-')) {

		if((*argv)[2] != '\0')
			usage();

		switch((*argv)[1]) {

		case 'v':
			if(vflag)
				usage();
			vflag = TRUE;
			break;

		case 'o':
			if(oflag  ||  (--argc <= 0))
				usage();
			oflag = TRUE;

 			/*
			 * Open protofile to store badscan information.
			 */
			close(1);
			if(creat(*++argv, 0644) == -1)
				fatal("cannot create ", *argv);
			break;

		case 'b':
			if(bflag  ||  (--argc <= 0))
				usage();

			bflag = TRUE;
			/*
			 * Set boot file name.
			 */
			bfile = *++argv;
			break;

		default:
			usage();
			break;
		}
	}

 	/*
	 * Check to ensure that only arg1 and arg2 are available.
	 */
	if (argc != 2)
		usage();

	/*
	 * The first argument not associated with an option must be a
	 * character or block special file.
	 */
	if (stat(argv[0], &sb) < 0)
		fatal("cannot stat: ", argv[0]);
	sb.st_mode &= S_IFMT;
	if ((sb.st_mode != S_IFCHR) && (sb.st_mode != S_IFBLK))
		fatal("not a special file: ", argv[0]);

	/*
	 * Open the special file (arg1).
	 * Find the size of a track.
	 */
	close(0);
	if (open(argv[0], 0) == -1)
		fatal("cannot open: ", argv[0]);
	if (ioctl(0, HDGETA, (char *)&hdparms) == -1)
		fatal("cannot get hard disk parameters: ", argv[0]);
	tracksize = hdparms.nspt;

	/*
	 * Allocate a track buffer.
	 */
	if (tracksize > MAXUINT/BUFSIZ || (buf = malloc(tracksize * BUFSIZ)) == NULL)
		fatal("cannot allocate track buffer", "");

	/*
	 * Validate and evaluate length (arg2)
	 */
	lim = atol(argv[1]);
	if (lim <= 0) {

		register struct hdisk_s *hp;
		int f2;

		if ((f2 = open(argv[1], 0)) < 0)
			fatal("bad size: ", argv[1]);
		if (read(f2, buf, 512) != 512)
			fatal("cannot read: ", argv[1]);
		close(f2);
		hp = buf;
		if (hp->hd_sig != HDSIG)
			fatal("bad partition table: ", argv[1]);
		lim  = hp->hd_partn[ sb.st_rdev & 3 ].p_size;
#if	0
#define	NSPT	26
		/* The following line produces an internal cc error 3/22/90. */
		nspt = NSPT - (hp->hd_partn[ sb.st_rdev & 3 ].p_base % NSPT);
#else
		base = hp->hd_partn[ sb.st_rdev & 3 ].p_base;
		nspt = tracksize - base % tracksize;
#endif
		if (lim <= 0)
			fatal("null partition: ", argv[1]);
	}

	/*
	 * Create header for mkfs prototype file.
	 */
	sput(1, bfile);
	sput(1, "\n");
	lput(1, lim);
	sput(1, " ");
	lput(1, (lim/6 + 7) & ~7L);	/* ensure ninode is multiple of 8 */
	percent = 10;
	incr = lim / 10;
	time(&t0);

	/*
	 * Scan for bad blocks.
	 * The first and last tracks may have less than tracksize sectors.
	 */
	for (bno=0; bno < lim; (bno += nspt), (nspt = tracksize)) {

		/*
		 * Try a track read first.
		 */
		lseek(0, bno * BUFSIZ, 0);

		/*
		 * Avoid reading past end of partition.
		 */
		if (bno + nspt > lim)
			nspt = lim - bno;

		if (read(0, buf, (nspt * BUFSIZ)) != (nspt * BUFSIZ)) {

			/*
			 * Try to read each block in a bad track.
			 */
			for (n=0; n < nspt; ++n) {

				/*
				 * Check for partial track.
				 */
				if ((bno+n) >= lim)
					break;
				lseek(0, (bno+n) * BUFSIZ, 0);
				/*
				 * Append bad blocks to mkfs prototype file.
				 */
				if (read(0, buf, BUFSIZ) != BUFSIZ)
					bad(bno+n);
			}
		}

		/*
		 * Periodically generate reports
		 */
		if (vflag && (bno >= incr) && (bno < lim)) {

			/*
			 * Estimate seconds remaining to next 1/10 minute.
			 */
			time(&t1);
			t1 = (t1 - t0) * (100 - percent);
			t1 += percent - 1;
			t1 /= percent;
			t1 += 5;

			lput(2, (long) percent);
			sput(2, " % done: ");
			if (t1 <  6000)
				sput(2, " ");
			if (t1 <   600)
				sput(2, " ");
			lput(2, t1 / 60);
			sput(2, ".");
			lput(2, (t1 % 60) / 6);
			sput(2, " minutes remaining ...\n");

			percent += 10;
			incr =  (lim * percent) / 100;
		}
	}
	sput(1, "\nd--755 0 0\n$\n");
	exit(0);
}

/**
 * void
 * bad(n)
 * long	n;
 *	Input:	n	- bad block location.
 *	Action:	Flag block n as being bad.
 *	Return:	None.
 *	Note:	None.
 */
void
bad(n) register long n;
{
	static int  nbad =  0;
	static long last = -1;

	if ((last+1) != n)
		nbad = 0;
	last = n;
	if ((nbad & 7) == 0)
		sput(1, "\n%b");
	sput(1, " ");
	lput(1, n);
	++nbad;
}

/**
 * void
 * fatal(s1, s2)
 * char	* s1, *s2;
 *	Input:	s1, s2	- pointer to error message strings.
 *	Action:	Print fatal message, terminate with extreme prejudice.
 *	Return:	Never return. Always exit.
 *	Note:	None.
 */
void
fatal(s1, s2) char * s1, * s2;
{
	sput(2, s1);
	sput(2, s2);
	sput(2, "\n");
	exit(1);
}

/**
 * void
 * lput(fd, num)
 * int	fd;
 * unsigned long num;
 *
 *	Input:	fd	- output file descriptor.
 *		num	-
 *	Action:	Convert long num to ascii string sent to file fd.
 *	Return:	None.
 *	Note:	None.
 */
void
lput(fd, num) int	fd; unsigned long num;
{
	register char * cp;
	static char buf[16];

	cp = &buf[15];

	/*
	 * Compute character equivalent value of long num.
	 */
	do {
		*--cp = (num % 10) + '0';
	} while (num /= 10);
	sput(fd, cp);
}

/**
 * void
 * sput(fd, s)
 * int	fd;
 * char	* s;
 *
 *	Input:	fd	- output file descriptor.
 *		s	- pointer to character string.
 *	Action:	Write string s to file descriptor fd.
 *	Return:	None.
 *	Note:	None.
 */
void
sput(fd, s) int	fd; register char * s;
{
	register char * cp;
	int i,j;

	/*
	 * Get location of end of string.
	 */
	for (cp = s; *cp != '\0'; ++cp)
		;
	
	/* WRITE WITH RETRY IF WRITE() RETURNS 0 */
	i = cp-s;
	while ((j = write(fd, s, i)) == 0) ;
	  
}

/**
 * void
 * usage()
 *	Input:	None.
 *	Action:	Display command format.
 *	return:	None.
 *	Note:	None.
 */
void
usage()
{
	fatal(	"Usage: badscan [-v] [-o proto] [-b boot] dev size\n",
		"       badscan [-v] [-o proto] [-b boot] dev sdev");
}

/* end of badscan.c */
