/* Include file for tertiary boot programs.
 *
 * La Monte H. Yarroll <piggy@mwc.com>, September 1991
 */

#include <sys/types.h>

#define TRUE	(1==1)
#define FALSE	(1==2)
#define	NULL	((char *)0)
#define	FOURK	0x1000	/* 4k page.  Needed for alignment purposes.  */
#define FOURKBOUNDRY	0xf000
#define BLOCK	512	/* 512 bytes per disk block.  */
#define LINESIZE 81	/* Size of typical line with NUL terminator.  */
#define	MAX_SEGS 8	/* Maximum number of executable file segs + 1.  */
#define NORMAL_MAGIC 0x10B	/* Value of optional header magic
				 * for normal executable file.
				 */

#define DISKINT	0x13		/* Disk drive interrupt.  */
#define DISK_PARAMS (8 << 8)	/* Return Disk Drive Parameters function.  */
#define HARD_DRIVE 0x80		/* Select fixed disks.  */

#define SIXBITS 0x3f		/* Lower six bits of a byte.  */

#define INODES_PER_BLOCK 8

#define GREATEST(a, b, c) (a > (b>c?b:c) ? a : (b>c?b:c))
#define LESSER(a, b) (a < b ? a : b)
#define HIGH(x)	(x >> 8)	/* High byte of 16 bit number.  */
#define LOW(x)	(x & 0xff)	/* Low byte of 16 bit number.  */

struct reg {
	unsigned r_ax;
	unsigned r_bx;
	unsigned r_cx;
	unsigned r_dx;
	unsigned r_si;
	unsigned r_di;
	unsigned r_ds;
	unsigned r_es;
	unsigned r_flags;
};

/* Table entry describing a generic segment in an executable file.  */
struct load_segment {
	int valid;			/* Is this a valid table entry?	*/
	char *message;			/* Message to print while loading.  */
	unsigned short load_toseg;	/* Where in memory to		*/
	unsigned short load_tooffset;	/* load this segment.		*/
	fsize_t load_offset;	/* Where in file to get it.	*/
	fsize_t load_length;	/* How long it is.		*/
};

extern int intcall();	/* Provide C interface to bios interrupts.  */
/* int intcall(reg *srcreg, reg *destreg, int intnum);  */
extern void puts();	/* Put a string on the screen.  */
extern char *gets();	/* Get a string from the keyboard.  */
extern void reverse();	/* Reverse a string in place.  */
extern void itoa();	/* Convert an integer to a decimal string.  */
extern void itobase();	/* Convert an integer to an arbitrary base string.  */
extern unsigned int basetoi(); /* Convert an arbitrary base string to an integer.  */
extern int abread();	/* Read a physical block into an unaligned buffer.  */
extern ino_t namei();	/* Convert from a name to an inode.  */
extern void iread();	/* Read from a file, given an inode.  */
extern void ifread();	/* Read from a file into a far buffer, given an inode.  */
extern daddr_t fbno2pbno();	/* Convert file block number to physical block number.  */
extern daddr_t vmap();	/* Convert file block number to physical block number.  */
extern int interpret();	/* Attempt to execute a builtin command.  */
extern int coff2lout();	/* Generate an l.out header for a COFF file.  */
extern void dpb();	/* Display parameters from bios.  */
extern void dir();	/* List contents of /.  */
extern char *lpad();	/* Pad a string on the left.  */
extern unsigned short sys_base;	/* System load base paragraph.  */
