/* tboot.c -- tertiary boot
 * This is invoked by the secondary boot to do all the things we can't
 * do in just 512 bytes.
 *
 * Includes an interpreter for builtin commands.  Just type "info" or "dir"
 * to get disk information, or a directory listing of "/".
 *
 * Can load an image up to 1 gigabyte in length.  Segments can be as
 * big as the whole file.
 *
 * La Monte H. Yarroll <piggy@mwc.com>, September 1991
 */

#include <canon.h>
#include <sys/types.h>
#include <sys/dir.h>
#include <sys/ino.h>
#include <sys/inode.h>
#include <l.out.h>
#include <coff/filehdr.h>

#include "tboot.h"

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
#define DEF_SYS_BASE	0x0060		/* System load base paragraph. */
#define SYS_START	0x0100		/* System entry point. */
#define FIRST	8			/* Relative start of partition. */
#define FULLSEG	0xffff			/* Size of a whole 8086 segment. */
#define PPMASK	(unsigned short) 0xfff0 /* Mask for rounding to paragraph.  */

unsigned short sys_base;	/* Segment into which to load the kernel.  */

main()
{
	char imagename[5*DIRSIZ+1] = "autoboot";	/* File to boot.  */
	ino_t imageinum;		/* inode number of the boot image.  */
	struct inode imageinode;	/* Inode structure for the boot image.  */
	int imageok;			/* Flag to identify usable executables.  */

	unsigned int filemagic;		/* Magic number from file.  */
	struct load_segment imagetable[MAX_SEGS]; /* How to load a file.  */
	struct load_segment *cur_segment; /* Pointer for walking imagetable.  */ 

	unsigned short data_seg;	/* Data segment register for image.  */

	sys_base = DEF_SYS_BASE;

	puts("\r\nCOHERENT Tertiary boot Version 0.9b\r\n");

	/* Look for a valid executable.  */
	do {
		/* Find the file in the file system.  */
		while  ((ino_t) 0 == (imageinum = namei(imagename))){
			/* Ask for another name.  */
			/* Don't generate a message for name "".  */
			if (imagename[0] != '\0') {
				puts("\r\nCan't find ");
				puts(imagename);
				puts(".  Please choose another.\r\n");
			}

			/* Fetch new file names, executing them
			 * if they are builtins.  Terminate loop
			 * when we want to try another file name.
			 */
			do {
				puts("? ");
				gets(imagename, DIRSIZ);
				puts("\r\n");
			} while (interpret(imagename));
		}
	
		/* We've found the image we want to boot--let's open it.  */
		if (0 == iopen(&imageinode, imageinum)) {
			puts("Can't open ");
			puts(imagename);
			puts(".\r\n");
			continue;
		}

		/* Read the magic number.  */
		iread(&imageinode, &filemagic,
			(fsize_t) 0, (unsigned short) sizeof (int));
		canint(filemagic);	/* Harmless on 80386.  */

		switch (filemagic) {
		/* Is this an i386 COFF executable?  */
		case I386MAGIC:
			puts("COFF!  COFF!\r\n");
			imageok = 
				coff2load(&imageinode, imagetable, &data_seg);
			break;
			
		/* Is this an l.out executable?  */
		case L_MAGIC:
			puts("l.out!\r\n");
			imageok =
				lout2load(&imageinode, imagetable, &data_seg);
			break;

		default:
			imageok = (1==2);
			puts("File ");
			puts(imagename);
			puts(" is not an executable.\r\n");

			puts("Please choose another.\r\n");
			imagename[0] = '\0';
			break;
		} /* switch (filemagic) */
	} while (!imageok);

	/* ASSERTION: imageinode and imagetable describe a valid executable.  */

	puts("OK!  Loading ");
	puts(imagename);
	puts("...\r\n");

	/* Now actually load everything into memory.  */
	for (cur_segment = &imagetable[0]; cur_segment->valid; ++cur_segment) {
		puts(cur_segment->message);

		ifread(&imageinode,
			cur_segment->load_toseg,
			cur_segment->load_tooffset,
			cur_segment->load_offset,
			cur_segment->load_length);
	}

	puts("\r\nRunning ");
	puts(imagename);
	puts("...\r\n");

	/* Run the image (the kernel).  */
	gotoker(SYS_START, sys_base, data_seg);
}
