/*
 * dos0.c
 * cc -o dos dos[01234].c
 * Read, write or format an MS-DOS filesystem (default: INFILE in dos1.h).
 * For usage, see USAGE in dos0.h.
 * Understands DOS 2.0 tree structured disks.
 * Understands 1.5 and 2-byte FAT entries;
 * does not understand 4-byte FAT entries.
 * Prints some debugging information if compiled -DDEBUG.
 */

#include "dos0.h"

/* Globals. */
int		aflag;			/* ASCII text file		*/
unsigned char	cohfile[NAMEMAX];	/* COHERENT filename		*/
unsigned char	cmd[6 + NAMEMAX];	/* system() command buffer	*/
char *		device = INFILE;	/* Input device filename	*/
int		estat;			/* Exit status			*/
int		(*fun)();		/* Function to execute		*/
int		kflag;			/* Use mtime, not current time	*/
int		nflag;			/* Sort by time, newest first	*/
long		partseek;		/* Extended MS-DOS part. seek	*/
int		pflag;			/* Extract/replace is piped	*/
DIR *		root;			/* Root directory		*/
int		sflag;			/* Suppress subdirectory x/r	*/
MDIR *		volume;			/* Volume label			*/
int		xpart;			/* Extended MS-DOS partition	*/

main(argc, argv) int argc; char *argv[];
{
	register int mode;

	argv0 = argv[0];
	usagemsg = USAGE;
	if (argc < 2)
		usage();
	if (argc == 2)
		++argc;
	else
		device = argv[2];
	mode = key(argv[1]);
	if ((fsfd = open(device, mode)) < 0)
		fatal("cannot open device %s", device);
	if (fun != format) {
		/* Read the FAT and the root directory. */
		readfat();
		root = newdir(NULL, NULL, bpb->b_files);
		readmdir(root);
	}
	(*fun)(argc-3, &argv[3]);
	if (fun != format) {
		writedir(root);
		if (fatcflag)
			writefat();
		free(fatcache);
	}
	exit(estat);
}

/* end of dos0.c */
