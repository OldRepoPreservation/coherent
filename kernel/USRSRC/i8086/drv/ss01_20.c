/*
 * This is a driver for Seagate ST01/ST02 scsi hard disk controllers.
 *
 * $Log:	/usr/src/sys/i8086/drv/RCS/ss.c,v $
 * Revision 1.5	91/03/07  11:48:39	root
 * Now sends Identify and Abort messages & completes a SCSI bus cycle
 *
 * Revision 1.4	91/03/06  16:31:45	root
 * tried to send Identify message - get status 0x40 & fail
 *
 * Revision 1.3	91/03/05  17:03:43	root
 * Goes thru arbitration (sans IRQ) successfully
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

#define WC_ENABLE_SCSI	0x80	/* Write Control (WC) register bits */
#define WC_ENABLE_IRPT	0x40
#define WC_ENABLE_PRTY	0x20
#define WC_ARBITRATE	0x10
#define WC_ATTENTION	0x08
#define WC_BUSY  	0x04
#define WC_SELECT  	0x02
#define WC_SCSI_RESET  	0x01

#define RS_ARBIT_COMPL	0x80	/* Read STATUS (RS) register bits */
#define RS_PRTY_ERROR	0x40
#define RS_SELECT	0x20
#define RS_REQUEST	0x10
#define RS_CTRL_DATA	0x08
#define RS_I_O  	0x04
#define RS_MESSAGE  	0x02
#define RS_BUSY  	0x01

#define HOST_ID		0x80	/* Host adapter is SCSI ID #7 */
#define BUS_RETRIES	1000

				/* Message types */
#define MSG_IDENT_DC	0xC0	/* Identify, with Disconnect allowed */
#define MSG_ABORT	0x06	/* End the current SCSI bus cycle */

#define CS_GOOD		0x00	/* Command Status from the drive */
#define CS_CHECK	0x02
#define CS_BUSY		0x08
#define CS_RESERVED	0x18

#if 1
int stats[40], statsptr;
#define PUSHI		{ stats[statsptr++] = i; }
#define POPI		{ printf("%d:",statsptr);while(statsptr)\
				printf("%d ",stats[--statsptr]);printf("\n");}
#define SSTELL(foo)	printf(foo)
#define SSTATUS		printf("status=%x\n", (int)(unsigned char)status)
#else
#define PUSHI
#define POPI
#define SSTELL(foo)
#define SSTATUS
#endif

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
#include	<scsiwork.h>

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
	int i;
	char status;
	int await_bus;
	int data1, data2;
#define CMDLEN 6
	unsigned char cmd[CMDLEN];

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

#if 1
{ long x;
	/*
	 * Reset the SCSI bus.
	 */
	sfbyte(ss_csr, WC_ENABLE_SCSI | WC_SCSI_RESET);
	for (x=0; x<100000L; x++);
	sfbyte(ss_csr, 0);
	for (x=0; x<100000L; x++);
}
#endif

	/*
	 * Do ST0x arbitration.
	 */
	sfbyte(ss_csr, 0);		/* De-assert SCSI enable bit */
	sfbyte(ss_dat, HOST_ID);	/* Write my SCSI id to port */
	sfbyte(ss_csr, WC_ARBITRATE);	/* Start arbitration */

	bus_wait(RS_ARBIT_COMPL << 8 | RS_ARBIT_COMPL);

	/*
	 * Arbitration complete.  Now select, with ATN to allow messages.
	 */
	sfbyte(ss_dat, HOST_ID | 1);	/* Write two SCSI id's to port */
	sfbyte(ss_csr, WC_ENABLE_SCSI | WC_ATTENTION | WC_SELECT);

	bus_wait(RS_BUSY << 8 | RS_BUSY);

	/*
	 * Send "Identify" Message with Disconnect allowed.
	 */
	sfbyte(ss_csr, WC_ENABLE_SCSI | WC_ATTENTION);

	bus_wait(((RS_REQUEST|RS_CTRL_DATA|RS_I_O|RS_MESSAGE) << 8) |
		(RS_REQUEST|RS_CTRL_DATA|RS_MESSAGE));

#ifdef SEND_ABORT_MESSAGE
	sfbyte(ss_csr, WC_ENABLE_SCSI | WC_ATTENTION);
	sfbyte(ss_dat, MSG_IDENT_DC);

	/*
	 * Second message is Abort.
	 */
	bus_wait(((RS_REQUEST|RS_CTRL_DATA|RS_I_O|RS_MESSAGE) << 8) |
		(RS_REQUEST|RS_CTRL_DATA|RS_MESSAGE));
	sfbyte(ss_csr, WC_ENABLE_SCSI);
	sfbyte(ss_dat, MSG_ABORT);

	bus_wait(RS_BUSY << 8 | RS_BUSY);
#else
	sfbyte(ss_csr, WC_ENABLE_SCSI);
	sfbyte(ss_dat, MSG_IDENT_DC);

for (i = 0; i < CMDLEN; i++) cmd[i] = 0; /* send Test Unit Ready command */

	for (i = 0; i < CMDLEN; i++) {
		bus_wait(((RS_REQUEST|RS_CTRL_DATA|RS_I_O|RS_MESSAGE) << 8) |
			(RS_REQUEST|RS_CTRL_DATA));
		sfbyte(ss_dat, cmd[i]);
	}
#endif

	bus_wait(((RS_REQUEST|RS_CTRL_DATA|RS_I_O|RS_MESSAGE) << 8) |
		(RS_REQUEST|RS_CTRL_DATA|RS_I_O));
	data1 = ffbyte(ss_dat);
	bus_wait(((RS_REQUEST|RS_CTRL_DATA|RS_I_O|RS_MESSAGE) << 8) |
		(RS_REQUEST|RS_CTRL_DATA|RS_I_O|RS_MESSAGE));
	data2 = ffbyte(ss_dat);
bus_wait(RS_BUSY, 0);
status = ffbyte(ss_csr);
SSTATUS;
printf("data1=%x data2=%x\n",data1,data2);
POPI;
	/*
	 * If there was a check condition on the previous commmand,
	 * do a Request Sense command.
	 */
	if (data1 & CS_CHECK) {
		sfbyte(ss_csr, 0);		/* De-assert SCSI enable bit */
		sfbyte(ss_dat, HOST_ID);	/* Write my SCSI id to port */
		sfbyte(ss_csr, WC_ARBITRATE);	/* Start arbitration */

		bus_wait(RS_ARBIT_COMPL << 8 | RS_ARBIT_COMPL);

		/*
		 * Arbitration complete.  Now select, with ATN to allow messages.
		 */
		sfbyte(ss_dat, HOST_ID | 1);	/* Write two SCSI id's to port */
		sfbyte(ss_csr, WC_ENABLE_SCSI | WC_ATTENTION | WC_SELECT);

		bus_wait(RS_BUSY << 8 | RS_BUSY);

		/*
		 * Send "Identify" Message with Disconnect allowed.
		 */
		sfbyte(ss_csr, WC_ENABLE_SCSI | WC_ATTENTION);

		bus_wait(((RS_REQUEST|RS_CTRL_DATA|RS_I_O|RS_MESSAGE) << 8) |
			(RS_REQUEST|RS_CTRL_DATA|RS_MESSAGE));

		sfbyte(ss_csr, WC_ENABLE_SCSI);
		sfbyte(ss_dat, MSG_IDENT_DC);

for (i = 0; i < CMDLEN; i++) cmd[i] = 0; /* send Test Unit Ready command */
cmd[0] = 3;

		for (i = 0; i < CMDLEN; i++) {
			bus_wait(((RS_REQUEST|RS_CTRL_DATA|RS_I_O|RS_MESSAGE) << 8) |
				(RS_REQUEST|RS_CTRL_DATA));
			sfbyte(ss_dat, cmd[i]);
		}

		bus_wait(((RS_REQUEST|RS_CTRL_DATA|RS_I_O|RS_MESSAGE) << 8) |
			(RS_REQUEST|RS_CTRL_DATA|RS_I_O));
		data1 = ffbyte(ss_dat);
		bus_wait(((RS_REQUEST|RS_CTRL_DATA|RS_I_O|RS_MESSAGE) << 8) |
			(RS_REQUEST|RS_CTRL_DATA|RS_I_O|RS_MESSAGE));
		data2 = ffbyte(ss_dat);
bus_wait(RS_BUSY, 0);
status = ffbyte(ss_csr);
SSTATUS;
printf("data1=%x data2=%x\n",data1,data2);
POPI;
	}

#if 0
bus_wait(RS_REQUEST << 8 | RS_REQUEST);
#endif
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
	printf("ss IRPT\n");
}

static void	sswatch()
{
}

/*
 * Wait for specified bit values to appear in Status Register.
 * This uses a tight loop and does not expect to be interrupted.
 *
 * Argument "flags" is a double-byte value;  the high byte is ANDed with
 * status register contents, and the result is tested for equality with
 * the low byte.
 *
 * Return 1 if values wanted appeared, 0 if timeout occurred.
 */
static int bus_wait(flags)
unsigned short flags;
{
	int found, i;
	unsigned char status;

	found = 0;
	for ( i = 0; i < BUS_RETRIES; i++) {
		status = ffbyte(ss_csr);
		if ((status & (flags >> 8)) == (flags & 0xff)) {
			found = 1;
			break;
		}
	}

	if (!found)
		printf("ST0x timeout;  flags=%x status=%x\n", flags, status);

PUSHI;
	return found;
}
