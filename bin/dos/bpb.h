/*
 * bpb.h
 * BIOS parameter block (BPB).
 * Cf. Duncan, p. 164.
 * Cf. "Norton Disk Companion", pp. 143-144, for DOS 3.3 and later.
 */
#define	BPBOFF	0x0B			/* Offset of BPB in boot sector	*/
typedef	struct	bpb {
	unsigned int	b_ssize;	/* Bytes per sector		*/
	unsigned char	b_clsize;	/* Sectors per cluster		*/
	unsigned int	b_reserved;	/* Reserved sectors, from 0	*/
	unsigned char	b_fats;		/* FATs				*/
	unsigned int	b_files;	/* Root directory file entries	*/
	unsigned int	b_sectors;	/* Sectors			*/
	unsigned char	b_media;	/* Media descriptor		*/
	unsigned int	b_fatsize;	/* Sectors per FAT		*/
	unsigned int	b_tracks;	/* Sectors per track		*/
	unsigned int	b_heads;	/* Heads			*/
	unsigned int	b_hidden;	/* Hidden sectors		*/
	/* The following items are for DOS 3.3 and later only. */
	unsigned int	b_hidden2;	/* Hidden sectors, second word	*/
	unsigned long	b_bigsectors;	/* Big sectors			*/
	unsigned char	b_driveno;	/* Physical drive number	*/
	unsigned char	b_info[26];	/* More information...		*/
} BPB;

/* end of bpb.h */
