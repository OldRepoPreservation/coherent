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
	struct inode imageinode;	/* Inode structure for the boot image.  */
	ino_t imageinum;		/* inode number of the boot image.  */
	struct ldheader imageheader;	/* l.out header for boot image.  */
	int imageok;			/* Flag to identify usable executables.  */
	unsigned short data_seg;	/* Data segment register for image.  */
	int i;				/* A loop counter.  */

	/* Holders for arguments to ifread.  */
	unsigned short load_toseg;
	unsigned short load_tooffset;
	fsize_t	load_offset;
	fsize_t load_lenarg;

	char imagename[5*DIRSIZ+1] = "autoboot";	/* File to boot.  */

	sys_base = DEF_SYS_BASE;

	puts("\r\nCOHERENT Tertiary boot Version 0.9\r\n");

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
		if (0 == iattach(&imageinode, imageinum)) {
			puts("Can't open ");
			puts(imagename);
			puts(".\r\n");
			continue;
		}

		/* Read the header.  */
		iread(&imageinode, &imageheader,
		      (fsize_t) 0, (unsigned short) sizeof(struct ldheader));

		/* Canonicalize the header.  */
		canint(imageheader.l_magic);
		canint(imageheader.l_flag);
		canint(imageheader.l_machine);
		canvaddr(imageheader.l_entry);
		for(i = 0; i < NLSEG; ++i) {
			cansize(imageheader.l_ssize[i]);
		}

		/* Is this an l.out executable?  */
		if (L_MAGIC == imageheader.l_magic) {
			imageok = (1==1);
		} else {
			imageok = (1==2);
			puts("File ");
			puts(imagename);
			puts(" is not an executable.\r\n");

			puts("Please choose another.\r\n");
			imagename[0] = '\0';
		}
	} while (!imageok);

	/* ASSERTION: the inode in imageinode points at a valid l.out file.  */

	puts("OK!  Loading ");
	puts(imagename);
	puts("...\r\n");

	if (imageheader.l_flag & LF_SEP) { /* if sep i/d executable */
		puts("\r\nLoading code segments...\r\n");
		/* Load the shared and private code segments as one.  */
		load_toseg = sys_base; /* This is where we want the OS.  */
		load_tooffset = 0;
		load_offset = sizeof(struct ldheader); /* Skip the header.  */
		load_lenarg = imageheader.l_ssize[L_SHRI] + /* Both segments as one.  */
			      imageheader.l_ssize[L_PRVI];
		
		ifread(&imageinode, load_toseg, load_tooffset,
			load_offset, load_lenarg);
		
		puts("\r\nLoading data segments...\r\n");
		/* Load both data segments.  */

		/* Round up to next 16 byte paragraph.  */
		load_toseg = (sys_base +
			(imageheader.l_ssize[L_SHRI] + /* Shared code */
			imageheader.l_ssize[L_PRVI] +  /* Private code */
			15) / 16),
		load_tooffset = 0,

		load_offset = (fsize_t) sizeof(struct ldheader) + /* l.out header */
			imageheader.l_ssize[L_SHRI] + /* Shared code */
			imageheader.l_ssize[L_PRVI]; /* Private code */

		load_lenarg = imageheader.l_ssize[L_SHRD] + /* Both segments as one.  */
			imageheader.l_ssize[L_PRVD];

		ifread(&imageinode, load_toseg, load_tooffset,
			load_offset, load_lenarg);
		
	} else { /* if not sep i/d executable */
		
		puts("\r\nLoading all segments...\r\n");
		/* Load the shared and private code segments as one.  */
		load_toseg = sys_base, /* This is where we want the OS.  */
		load_tooffset = 0,
	
		load_offset = (fsize_t) sizeof(struct ldheader), /* Skip the header.  */
		load_lenarg = imageheader.l_ssize[L_SHRI] +
		       imageheader.l_ssize[L_PRVI] +
		       imageheader.l_ssize[L_SHRD] +
		       imageheader.l_ssize[L_PRVD];

		ifread(&imageinode, load_toseg, load_tooffset,
			load_offset, load_lenarg);

	} /* if not sep i/d executable */

	puts("\r\nRunning ");
	puts(imagename);
	puts("...\r\n");

	/* Be sure to set the data segement appropriately.  */
	if (imageheader.l_flag & LF_SEP) { /* if sep i/d executable */
		data_seg = (unsigned short) (sys_base +
			(imageheader.l_ssize[L_SHRI] +	/* Shared code */
			 imageheader.l_ssize[L_PRVI] +	/* Private code */
			 15) / 16);	/* Rounded up a paragraph.  */
	} else {
		/* Tiny model: ds = cs */
		data_seg = sys_base;
	}

	/* Run the image (the kernel).  */
	gotoker(SYS_START, sys_base, data_seg);
}
