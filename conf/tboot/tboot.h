/* Include file for tertiary boot programs.
 *
 * This is a real hodge-podge of symbols.  If you are looking to improve
 * code readability, start by hacking this file into many tiny pieces.
 *
 * La Monte H. Yarroll <piggy@mwc.com>, September 1991
 */

#ifdef MAIN
#define EXTERN
#else
#define EXTERN extern
#endif

#include <sys/types.h>
#include <sys/buf.h>
#include "ptypes.h"

#define TRUE	(1==1)
#define FALSE	(1==2)
#define WS	" \t"
#define BIGINT	((int32) 65535L)	/* Largest unsigned int.  */
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


#ifndef	NHD
#define	NHD	1			/* # of heads per drive [1 for f9d0]. */
#endif

#ifndef	NSPT
#define	NSPT	9			/* # of sectors per track on floppy. */
#define	NTRK	40			/* # of tracks on floppy. */
#endif

#define ROOTINO 2			/* Root inode # */
#define INOORG	2			/* First inode block. */
#define IBSHIFT 3			/* Shift, inode to blocks */
#define IOSHIFT 6			/* Shift, inode to bytes */
#define INOMASK 0x0007			/* Mask, inode to offset */
#define BUFSIZE 512			/* Block size. */
#define DISK	0x13			/* Disk Interrupt */
#define KEYBD	0x16			/* Keyboard Interrupt */
#define READ1	0x0201			/* read one sector */
#define FIRST	8			/* Relative start of partition. */
#define FULLSEG	0xffff			/* Size of a whole 8086 segment. */
#define PPMASK	(unsigned short) 0xfff0 /* Mask for rounding to paragraph.  */

#define COFF_SYS_BASE	0x0200		/* System load base paragraph for 386.  */
#define DEF_SYS_BASE	0x0060		/* System load base paragraph. */
#define SYS_START	0x0100		/* System entry point. */

#define THE_DEV		((dev_t)0x01)	/* The one disk device we recognize.  */
#define THE_XDEV	((dev_t)0x02)	/* The whole disk device, rather than partition.  */

/* WAIT_DELAY is how long to wait after finding autoboot before booting.  */
#define WAIT_DELAY	182	/* 10 seconds * 18.2 clicks per second.  */

/* Useful macros.  */
#define GREATEST(a, b, c) (a > (b>c?b:c) ? a : (b>c?b:c))
#define LESSER(a, b) (a < b ? a : b)
#define HIGH(x)	(x >> 8)	/* High byte of 16 bit number.  */
#define LOW(x)	(x & 0xff)	/* Low byte of 16 bit number.  */

/* Register structure used by call_bios().  */
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
	uint16 load_toseg;	/* Where in memory to		*/
	uint16 load_tooffset;	/* load this segment.		*/
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
extern uint16 basetoi(); /* Convert an arbitrary base string to an integer.  */
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
extern uint16 object_nlist();	/* Look up a symbol in an object file.  */
extern uint16 object_sys_base(); /* Generate a default sys_base.  */
extern uint32 wrap_coffnlist();	/* Candy coated coff nlist().  */
extern int wait_for_keystrok();	/* Wait a time delay for a keystroke.  */
extern BUF *bread();		/* Read a disk block.  */
extern BUF *bclaim();		/* Claim a disk buffer.  */
extern BUF *bpick();		/* Pick a buffer to trash.  */
extern void bufinit();		/* Initialize disk buffers.  */
extern int better_buf();	/* Compare the stealability of two buffers.  */
extern void brelease();		/* Free a disk buffer.  */
extern int gate_lock();		/* Attempt to lock a GATE.  */
extern int gate_locked();	/* Check to see if a GATE is locked.  */
extern void gate_unlock();	/* Unlock a GATE.  */
extern void print32();		/* Print a 32 bit integer, base 16.  */
extern void sanity_check();	/* Check for insane conditions.  */
extern void seg_align();	/* Align a far address.  */

extern int errno;	/* Error number for "system" calls.  */
EXTERN uint16 sys_base;	/* Segment into which to load the kernel.  */
EXTERN int sys_base_set;	/* Has sys_base been explicitly set?  */
