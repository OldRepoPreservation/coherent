/*
 * This is a driver for Seagate ST01/ST02 scsi hard disk controllers.
 *
 */
 
/*
 * Definitions.
 */
#define SS_RAM		0x1800	/* Offset of parameter RAM */
#define SS_CSR		0x1A00	/* Offset of control/status register */
#define SS_DAT		0x1C00	/* Offset of data port */

#define SS_RAM_LEN	128	/* ST0x has 128 bytes of RAM */
#define SS_DAT_LEN	0x400	/* Byte range mapped to data port */
#define SS_SEL_LEN	0x2000	/* Total size of memory-mapped area */
 
/*
 * Includes.
 */ 
#include	<coherent.h>
#include	<sys/io.h>
#include	<sys/sched.h>
#include	<sys/uproc.h>
#include	<sys/proc.h>
#include	<sys/con.h>
#include	<sys/stat.h>
#include	<devices.h>		/* SCSI_MAJOR */
#include	<errno.h>

#include 	<sys/fdisk.h>
#include	<sys/hdioctl.h>
#include	<sys/buf.h>

/*
 * Export Functions.
 */
 
/*
 * Export Variables - patch these to configure the driver.
 */
int	SS_INT = 5;		/* ST01/ST02 use either IRQ3 or IRQ5 */
int	SS_BASE_SEG = 0xDE00;	/* Start of memory-mapped communication area */

/*
 * Import Functions.
 */
int	nulldev();
int	nonedev();

/*
 * Local Functions.
 */
static void	ssload();
static void	ssunload();
static void	ssopen();
static void	ssread();
static void	sswrite();
static int	ssioctl();
static void	sswatch();
static void	ssblock();

static void	ssreset();
static void	ssintr();

/*
 * Local Variables.
 */
static BUF	dbuf;		/* For raw I/O */
static paddr_t	ss_base;	/* physical address of ST0x comm area */
static faddr_t	ss_fp;		/* (far *) to ST0x comm area */

static faddr_t	ss_ram;		/* (far *) to parameter RAM */
static faddr_t	ss_csr;		/* (far *) to control/status */
static faddr_t	ss_dat;		/* (far *) to data port */

/*
 * Driver CON entry - an export variable.
 */
CON	sscon	= {
	DFBLK|DFCHR,			/* Flags */
	SCSI_MAJOR,			/* Major index */
	ssopen,				/* Open */
	nulldev,			/* Close */
	ssblock,			/* Block */
	ssread,				/* Read */
	sswrite,			/* Write */
	ssioctl,			/* Ioctl */
	nulldev,			/* Powerfail */
	sswatch,			/* Timeout */
	ssload,				/* Load */
	ssunload,			/* Unload */
	nulldev				/* Poll */
};

/**
 *
 * void
 * ssload()	- load routine.
 *
 *	Action:	The controller is reset and the interrupt vector is grabbed.
 *		The drive characteristics are set up at this time.
 */
static void
ssload()
{
	/*
	 * Claim IRQ vector.
	 */
	setivec(SS_INT, ssintr); 
	
	/*
	 * Allocate a selector to map into ST0x memory-mapped comm area.
	 */
	ss_base = (paddr_t)((long)(unsigned)SS_BASE_SEG << 4);
	ss_fp = ptov(ss_base, (fsize_t)SS_SEL_LEN);

	ss_ram = ss_fp + SS_RAM;
	ss_csr = ss_fp + SS_CSR;
	ss_dat = ss_fp + SS_DAT;

	/*
	 * Primitive test of ST0x RAM.
	 */
	sfword(ss_ram, 0xA55A);
	sfword(ss_ram + 2, 0x3CC3);
	sfword(ss_ram + SS_RAM_LEN - 4, 0xA55A);
	sfword(ss_ram + SS_RAM_LEN - 2, 0x3CC3);
	if (ffword(ss_ram) != 0xA55A		/* fetch a "far" word */
	||  ffword(ss_ram + 2) != 0x3CC3
	||  ffword(ss_ram + SS_RAM_LEN - 4) != 0xA55A
	||  ffword(ss_ram + SS_RAM_LEN - 2) != 0x3CC3) {
		printf("Error - ST0x failed memory test\n");
		return;
	} else
		printf("ST0x passed memory test\n");

	/*
	 * Initialize Drive Size.
	 */

	/*
	 * Initialize Drive Controller.
	 */
}

/*
 * void
 * ssunload()	- unload routine.
 */
static void
ssunload()
{
	/*
	 * Release IRQ vector.
	 */
	clrivec(SS_INT); 
	 
	/*
	 * Free the ST0x selector.
	 */
	vrelse(ss_fp); 
}

/**
 *
 * void
 * ssreset()	-- reset hard disk controller, define drive characteristics.
 */
static void
ssreset()
{
}

/**
 *
 * void
 * ssopen( dev, mode )
 * dev_t dev;
 * int mode;
 *
 *	Input:	dev = disk device to be opened.
 *		mode = access mode [IPR,IPW, IPR+IPW].
 *
 *	Action:	Validate the minor device.
 *		Update the paritition table if necessary.
 */
static void
ssopen( dev, mode )
register dev_t	dev;
{
}

/**
 *
 * void
 * ssread( dev, iop )	- write a block to the raw disk
 * dev_t dev;
 * IO * iop;
 *
 *	Input:	dev = disk device to be written to.
 *		iop = pointer to source I/O structure.
 *
 *	Action:	Invoke the common raw I/O processing code.
 */
static void
ssread( dev, iop )
dev_t	dev;
IO	*iop;
{
	ioreq( &dbuf, iop, dev, BREAD, BFRAW|BFBLK|BFIOC );
}

/**
 *
 * void
 * sswrite( dev, iop )	- write a block to the raw disk
 * dev_t dev;
 * IO * iop;
 *
 *	Input:	dev = disk device to be written to.
 *		iop = pointer to source I/O structure.
 *
 *	Action:	Invoke the common raw I/O processing code.
 */
static void
sswrite( dev, iop )
dev_t	dev;
IO	*iop;
{
	ioreq( &dbuf, iop, dev, BWRITE, BFRAW|BFBLK|BFIOC );
}

/**
 *
 * int
 * ssioctl( dev, cmd, arg )
 * dev_t dev;
 * int cmd;
 * char * vec;
 *
 *	Input:	dev = disk device to be operated on.
 *		cmd = input/output request to be performed.
 *		vec = (pointer to) optional argument.
 *
 *	Action:	Validate the minor device.
 *		Update the paritition table if necessary.
 */
static int
ssioctl( dev, cmd, vec )
register dev_t	dev;
int cmd;
char * vec;
{
}

/**
 *
 * void
 * ssblock( bp )	- queue a block to the disk
 *
 *	Input:	bp = pointer to block to be queued.
 *
 *	Action:	Queue a block to the disk.
 *		Make sure that the transfer is within the disk partition.
 */
static void
ssblock(bp)
register BUF	*bp;
{
}

/**
 *
 * void
 * ssintr()	- Interrupt routine.
 *
 */
static void
ssintr()
{
}

static void	sswatch()
{
}
