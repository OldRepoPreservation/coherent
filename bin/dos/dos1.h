/* dos1.h */
/* Common header for "dos" and "dosshrink" commands. */

#include <stdio.h>
#include "bpb.h"
#include "mdir.h"

/* Manifest constants. */
#define	BBSIZE		512		/* Boot block size		*/
#define	CLBAD		0xFFF7		/* FAT bad cluster		*/
#define	CLEOF		0xFFFF		/* FAT end of file marker	*/
#define	CLFREE		0x0000		/* FAT free cluster		*/
#define	CLMAX		0xFFEF		/* Maximum FAT normal cluster	*/
/*
 * The FAT cache must hold a complete 1.5-byte FAT expanded to 2 bytes/entry,
 * so it must contain at least 4096 * 2 = 8192 bytes or 16 sectors.
 */
#define	FATCSECS	(16)		/* Sectors in FAT cache		*/
#define	FATCNPSEC	(BBSIZE/sizeof(int))	/* 2-byte FAT clusters per sector */
#define	FATCCOUNT	(FATCSECS*FATCNPSEC)	/* 2-byte FAT cache entries */
#define	FATMASK		0x0FFF		/* Mask for 1.5-byte FAT entry	*/
#define	INFILE		"/dev/dos"	/* Default input file name	*/
#define	isdir(mdp)	((mdp)->m_attr&MSUBDIR) /* MS-DOS directory test */
#define	ishidden(mdp)	((mdp)->m_attr&MHIDDEN) /* MS-DOS directory test */
#define	is_media_id(n)	(((n)&0xF0)==0xF0) /* Media descriptor test	*/

#if	DEBUG
#define	dbprintf(arglist) printf arglist
#else
#define	dbprintf(arglist)
#endif

/* Externals. */
extern	void	exit();
extern	long	lseek();
extern	char	*malloc();

/* Globals in dos1.c. */
/* Diskette parameters. */
extern	BPB		d8floppy;
extern	BPB		d9floppy;
extern	BPB		d15floppy;
extern	BPB		d18floppy;
extern	BPB		q9floppy;
extern	BPB		s8floppy;
extern	BPB		s9floppy;

/* Other globals. */
extern	char		*argv0;
extern	unsigned char	bootb[BBSIZE];
extern	BPB		*bpb;
extern	int		cflag;
extern	unsigned char	*clbuf;
extern	unsigned int	clsize;
extern	unsigned int	dirbase;
extern	unsigned int	dirsize;
extern	unsigned int	fatbase;
extern	unsigned int	fatbytes;
extern	unsigned int	*fatcache;
extern	unsigned int	fatccount;
extern	unsigned int	fatcfirst;
extern	int		fatcflag;
extern	unsigned int	fatcmax;
extern	unsigned int	fatcmin;
extern	unsigned int	fatsize;
extern	unsigned int	filebase;
extern	int		fsfd;
extern	unsigned int	heads;
extern	unsigned int	maxcluster;
extern	unsigned int	mdirsize;
extern	unsigned int	nspt;
extern	unsigned int	sectors;
extern	unsigned int	ssize;
extern	char		*usagemsg;
extern	int		vflag;

/* Functions in dos1.c. */
extern	unsigned int	cltosec();
extern	void		decodefat();
extern	void		diskread();
extern	void		diskseek();
extern	void		diskwrite();
extern	void		fatal();
extern	void		fatcflush();
extern	void		fatcread();
extern	unsigned int	getcluster();
extern	char		*lcname();
extern	void		putcluster();
extern	void		readfat();
extern	void		setglobals();
extern	void		usage();
extern	void		writefat();
extern	void		xpartition();

/* end of dos1.h */

