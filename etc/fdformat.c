char helpmessage[] = "\
\
/etc/fdformat -- format a floppy diskette\n\
Usage:	/etc/fdformat [-i n] [-o n] [-v] [-w file] special\n\
Options:\n\
	-i	use n as an interleave factor\n\
	-o	use n as an offset factor for numbering sectors\n\
	-v	verify the format by reading each track\n\
	-w	write file onto the floppy\n\
Interleave 4 and offset 0 are the default specifications, both range from\n\
0 to the number of sectors per track less 1.\n\
The number of verify options less one specifies the number of format\n\
track retries to be attempted before giving up.\n\
\
";
#include	<stdio.h>
#include	<sys/fdioctl.h>
#include	<sys/stat.h>
#include	<errno.h>

int	dvfd = 0;
char	*dvname = NULL;
char	*dvbuff = NULL;
int	wrfd = 0;
char	*wrname = NULL;
char	*wrbuff = NULL;
int	interleave = 4;
int	offset = 0;
int	verify = 0;

struct fform fform[16];

struct fdata {
	int fd_cyladdress;	/* Use cylinder addressing */
	int fd_heads;		/* Number of heads */
	int fd_tracks;		/* Number of tracks per surface */
	int fd_sectors;		/* Number of sectors per track */
	int fd_size;		/* Sector size parameter */
	int fd_tsz;		/* Track size in bytes */
} fdata[] = {
	{ 0, 1, 40, 8, 2 , 8*512 },	/* minor device 0 - /dev/f8s0 */
	{ 0, 2, 40, 8, 2 , 8*512 },	/* minor device 1 - /dev/f8d0 */
	{ 0, 2, 80, 8, 2 , 8*512 },	/* minor device 2 - /dev/f8q0 */

	{ 0, 1, 40, 9, 2,  9*512 },	/* minor device 3 - /dev/f9s0 */
	{ 0, 2, 40, 9, 2,  9*512 },	/* minor device 4 - /dev/f9d0 */

	{ 0, 2, 80, 9, 2,  9*512 },	/* minor device 5 - /dev/fqd0 */
	{ 0, 2, 80,15, 2, 15*512 },	/* minor device 6 - /dev/fhd0 */

	{ 1, 2, 40,16, 1, 16*256 },	/* Micral double */
	{ 1, 2, 80,16, 1, 16*256 },	/* Micral quad */

	{ 1, 2, 40, 9, 2,  9*512 },	/* minor device 9 - /dev/f9a0 */

	{ 0, 1, 80,10, 2, 10*512 },	/* Rainbow */
	{ 0, 0,  0, 0, 0,      0 },	/* Future */

	{ 1, 2, 40, 9, 2,  9*512 },	/* minor device 12 - /dev/f9a0 */
	{ 1, 2, 80, 9, 2,  9*512 },	/* minor device 13 - /dev/fqa0 */
	{ 1, 2, 80,15, 2, 15*512 },	/* minor device 14 - /dev/fha0 */
	{ 0, 0,  0,  0, 0,     0 }	/* Future */
};

struct fdata fkind;
#define NKINDS (sizeof(fdata)/sizeof(fdata[0]))
struct stat sbuf;

main(argc, argv)
char *argv[];
{
	register int i;
	register char *p;
	int retry;

	for (i=1; i<argc && argv[i][0]=='-'; ++i) {
		for (p=&argv[i][1]; *p!='\0'; p+=1) {
			switch (*p) {

			case	'i':
				if (++i>argc
				 || (interleave = atoi(argv[i]))<0)
					usage();
				break;

			case	'o':
				if (++i>argc
				 || (offset = atoi(argv[i]))<0)
					usage();
				break;

			case	'v':
				verify += 1;
				break;

			case	'w':
				if (++i>argc)
					usage();

				wrname = argv[i];

				if ((wrfd = open(wrname, 0)) < 0) {
					dvname = wrname;
					xxerror("open", 1);
				}

				break;

			default:
				usage();
			}
		}
	}

	dvname = argv[i++];

	if (i != argc)
		usage();

	if ((dvfd = open(dvname, 2)) < 0)
		xxerror("open", 1);

	if (fstat(dvfd, &sbuf) < 0)
		xxerror("fstat", 1);

	sbuf.st_rdev &= 0xF;

	if (sbuf.st_rdev >= NKINDS) {
		errno = ENXIO;
		xxerror("kind check", 1);
	}

	fkind = fdata[sbuf.st_rdev];

	if (offset >= fkind.fd_sectors || interleave >= fkind.fd_sectors)
		usage();

	if (verify && (dvbuff = malloc(fkind.fd_tsz)) == NULL)
		xxerror("malloc verify buffer", 1);

	if (wrfd && (wrbuff = malloc(fkind.fd_tsz)) == NULL)
		xxerror("malloc copy buffer", 1);

	for (i = 0; i < fkind.fd_tracks; i += 1) {
		retry = 0;
		makeform(i, 0, offset, interleave);

		do
			if (ioctl(dvfd, FDFORMAT, (char *)fform) < 0)
				xxerror("ioctl", 1);
		while ((verify || wrfd)
		 && doextra(i, 0) < 0
		 && ++retry < verify);

		if (verify && (retry == verify)) {
			errno = 0;
			xxerror("verify failed", 1);
		}

		if (fkind.fd_heads < 2)
			continue;

		makeform(i, 1, offset, interleave);
		retry = 0;

		do
			if (ioctl(dvfd, FDFORMAT, (char *)fform) < 0)
				xxerror("ioctl", 1);
		while ((verify || wrfd)
		 && doextra(i, 1) < 0
		 && ++retry < verify);

		if (verify && (retry == verify)) {
			errno = 0;
			xxerror("verify failed", 1);
		}
	}

	exit(0);
}

/*
 * Construct a track format buffer for the floopy.
 */
makeform(c, h, o, i)
{
	register int psect, lsect;
	register int nspt;

	nspt = fkind.fd_sectors;

	for (psect=0; psect<nspt; psect+=1)
		fform[psect].ff_sect = 0;

	psect = 0;
	lsect = c * o;

	while (psect<nspt) {
		while (lsect>=nspt || fform[lsect].ff_sect!=0)
			if (lsect>=nspt)
				lsect-=nspt;
			else
				lsect+=1;

		fform[lsect].ff_cylin = c;
		fform[lsect].ff_head = h;
		fform[lsect].ff_sect = psect+1;
		fform[lsect].ff_size = fkind.fd_size;
		psect += 1;
		lsect += i;
	}
	/* printform(); */
}

/*
printform()
{
	register int i;
	register struct fform *f;
	for (i = 0; i < fkind.fd_sectors; i += 1) {
		f = &fform[i];
		printf("%x %x %x %x\n",
			f->ff_cylin, f->ff_head, f->ff_sect, f->ff_size);
	}
}
*/

doextra(track, head)
int track, head;
{
	long pos;
	register int size;
	register char *p, *q;

	if (fkind.fd_cyladdress)
		pos = 2 * track + head;
	else
		pos = head * fkind.fd_tracks + track;

	pos *= fkind.fd_tsz;
	size = fkind.fd_tsz;

	if (wrfd) {
		lseek(wrfd, pos, 0);

		if (read(wrfd, wrbuff, size) != size)
			return (xxerror("read file", 0));

		lseek(dvfd, pos, 0);

		if (write(dvfd, wrbuff, size) != size)
			return (xxerror("write floppy", 0));
	}

	if (verify) {
		lseek(dvfd, pos, 0);

		if (read(dvfd, dvbuff, size) != size)
			return (xxerror("read floppy", 0));
	}

	if (verify && wrfd) {
		p = dvbuff;
		q = wrbuff;

		while (--size >= 0)
			if (*p++ != *q++)
				return (xxerror("verify compare", 0));
	} else if (verify) {
		p = dvbuff;

		while (--size >= 0)
			if (*p++ != 0xF6)
				return (xxerror("verify compare", 0));
	}

	return (0);
}

usage()
{
	printf(helpmessage);
	exit(1);
}

xxerror(s, f)
char *s;
{
	extern int errno;
	int uerror;

	uerror = errno;
	fprintf(stderr, "fdformat %s: ", dvname);

	if ((errno = uerror) > 0 && errno < sys_nerr)
		perror(s);

	else if (errno != 0)
		fprintf(stderr, "%s: errno==%d\n", s, errno);

	else
		fprintf(stderr, "%s\n", s);

	if (f)
		exit(1);

	return (-1);
}
