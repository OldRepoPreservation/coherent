/* dos0.h */ 

#include "dos1.h"
#include <access.h>
#include <ctype.h>
#include <string.h>
#include <sys/devices.h>
#include <sys/dir.h>
#include <sys/fdisk.h>
#undef DIR
#include <sys/stat.h>
#include <time.h>
#include <sys/types.h>

/* Definitions. */
#define	VERSION	"4.1"			/* Version id			*/
#define	CTRLZ	'\032'			/* ASCII <Control-Z> character	*/
#define	NAMEMAX	128			/* Maximum filename length	*/

#define	OUSAGE	\
"Usage:	dos [-][dFlrtx][acknpsv[1-9]] [ device ] [ file ... ]\n"\
"Functions:\n"\
"	d	Delete specified files\n"\
"	F	Format disk			(optional argument: boot block)\n"\
"	l	Label disk			(one argument: label)\n"\
"	r	Replace files			(default: all files in '.')\n"\
"	t	Table of contents listing	(default: all files)\n"\
"	x	Extract specified files		(default: all files)\n"\
"Flags:\n"\
"	a	ASCII data extract/replace	(default: binary data)\n"\
"	c	Check only; do not write changes to MS-DOS filesystem\n"\
"	k	Keep mtime on extract/replace	(default: current time)\n"\
"	n	Newest files first in list	(default: alphabetized)\n"\
"	p	Piped extract/replace\n"\
"	s	Suppress subdirectory x/r	(default: recursive descent)\n"\
"	v	Verbose\n"\
"	[1-9]	Specify desired drive on extended MS-DOS partition\n"\
"The default device is /dev/dos.\n\n"\
"See also doscp, doscpdir, dosdel, dosdir, dosformat, doslabel.\n"

#define	USAGE	\
"Usage: Copy:        doscp [-abkmrv] src dest\n"\
"       Copy Tree:   doscpdir [-akmrv] src dest\n"\
"       Cat files:   doscat src\n"\
"       directory:   dosdir [-vn] src\n"\
"       mkdir:       dosmkdir dest\n"\
"       delete:      dosdel src\n"\
"       format:      dosformat dest [boot block]\n"\
"       label:       doslabel dest \"label\"\n\n"\
"Flags:	a or m	ASCII copies - always convert CRLF to LF\n"\
"	b or r	Binary copies - never convert CRLF to LF\n"\
"	k	Keep mtime on copies		   (default: current time)\n"\
"	n	Newest files first in list	(default: alphabetized)\n"\
"	v	Verbose\n\n"\
"       The src and dest arguments have the format \"[device:]name\" where\n"\
"       device is a COHERENT pathname for the special device file, and name\n"\
"       is a pathname to a file or directory. Note that all paths must be\n"\
"       specified with forward slashes. Arguments without a device are assumed\n"\
"       to be COHERENT files.\n"\
"       See also /etc/default/msdos\n"


/*
 * Directories.
 * A DIR is allocated the root and for each subdirectory on a disk.
 * Rather than having pointers to multiple children,
 * a DIR has a linked list of siblings rooted at a child.
 * The MS-DOS directory is not read into d_dir until required.
 */
typedef	struct	dir {
	struct dir *	d_parent;	/* Parent, NULL if root		*/
	struct dir *	d_child;	/* Child			*/
	struct dir *	d_sibling;	/* Sibling			*/
	unsigned int	d_cluster;	/* First cluster		*/
	unsigned int	d_files;	/* Max number of files		*/
	unsigned int	d_dirblocks;	/* Number of directory blocks	*/
	MDIR *		d_dir;		/* MS-DOS directory		*/
	MDIR *		d_edp;		/* Pointer past end of d_dir	*/
	unsigned char	d_dname[12];	/* NUL-terminated name		*/
	char		d_dirflag;	/* Directory must be written	*/
} DIR;

/* Externals. */
extern	char *		calloc();
extern	struct tm *	localtime();
extern	void		qsort();
extern	char *		realloc();
extern	char *		strcat();
extern	char *		strcpy();
extern	char *		strncpy();
extern	time_t		time();
extern	char *		base;
extern	char *		dest;

/* Globals in dos0.c. */
/* dos1.c globals are declared in dos1.h, included above. */
extern	int		aflag;
extern	int		bflag;
extern	unsigned char	cohfile[NAMEMAX];
extern	unsigned char	cmd[6 + NAMEMAX];
extern	char *		device;
extern	int		estat;
extern	int		(*fun)();
extern	int		kflag;
extern	int		nflag;
extern	long		partseek;
extern	int		pflag;
extern	DIR *		root;
extern	int		sflag;
extern	MDIR *		volume;
extern	int		xpart;
extern	char		ext[];
extern	int		sext;

/* Functions. */
/* dos1.c functions are declared in dos1.h, included above. */
/* dos2.c */
extern	char *		cohname();
extern	void		cohtime();
extern	DIR *		creatdir();
extern	MDIR *		creatfile();
extern	void		delete();
extern	void		deletedir();
extern	void		deletefile();
extern	unsigned int	dirclusters();
extern	char *		dosname();
extern	void		dostime();
extern	void		extract();
extern	void		createdir();
extern	void		extractdir();
extern	void		extractfile();
/* dos3.c */
extern	MDIR *		find();
extern	MDIR *		findnext();
extern  char * 		cohn();
extern	int		finput();
extern	void		format();
extern	int		foutput();
extern	unsigned int	freecluster();
extern	int		key();
extern	void		label();
extern	void		mdirinit();
/* dos4.c */
extern	int		namecmp();
extern	DIR *		newdir();
extern	void		nonfatal();
extern	void		readmdir();
extern	void		replace();
extern	void		replacedir();
extern	void		replacefile();
extern	void		table();
extern	void		tabledir();
extern	void		tablefile();
extern	char *		uppercase();
extern	void		writedir();
extern  char * 		makef();

/* end of dos0.h */

