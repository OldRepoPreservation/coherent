/*
 * Directory entry on an MS-DOS diskette.
 * Used by file extraction program "dos"
 * and by the MS-DOS emulator.
 */
typedef	struct	mdir	{
	unsigned char	m_name[8];	/* File name		*/
	unsigned char	m_ext[3];	/* File extension	*/
	char		m_attr;		/* File attribute	*/
	unsigned char	m_junk[10];	/* Reserved		*/
	unsigned	m_sec:5,	/* Seconds/2		*/
			m_min:6,	/* Minutes		*/
			m_hour:5;	/* Hour (creation time)	*/
	unsigned	m_day:5,	/* Day of month (1-31)	*/
			m_mon:4,	/* Month (1-12)		*/
			m_year:7;	/* Year-1980		*/
	int		m_cluster;	/* Starting cluster	*/
	long		m_size;		/* File size in bytes	*/
} MDIR;

/* Special values for m_name[0]. */
#define MFREE	0x00			/* Never used		*/
#define MMDIR	0x2E			/* Directory file	*/
#define	MEMPTY	0xE5			/* Empty name		*/

/* Attributes in m_attr. */
#define MRONLY	0x01			/* Read only		*/
#define	MHIDDEN	0x02			/* Hidden file		*/
#define	MSYSTEM	0x04			/* System file		*/
#define MVOLUME	0x08			/* Volume identifier	*/
#define MSUBDIR	0x10			/* Sub directory	*/
#define MARCHIV	0x20			/* Archive bit		*/
