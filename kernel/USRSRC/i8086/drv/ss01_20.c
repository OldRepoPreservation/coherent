/*
 * This is a driver for Seagate ST01/ST02 scsi hard disk controllers.
 *
 * To do:
 *	turn on interrupts
 *	figure out a better storage class for rqs
 *      make input buffer for commands dynamic (?)
 *
 * $Log:	/usr/src/sys/i8086/drv/RCS/ss.c,v $
 * Revision 1.15	91/03/21  16:44:03	root
 * getting ready to call fdisk - finish ss_start next
 * 
 * Revision 1.14	91/03/20  17:25:14	root
 * Inquiry and Read Capacity working
 * 
 * Revision 1.13	91/03/18  17:43:18	root
 * add retry logic to scsicmd(); general cleanup
 * 
 * Revision 1.12	91/03/14  17:22:28	root
 * Test Ready now works, including Req Sense
 * 
 * Revision 1.11	91/03/14  15:45:12	root
 * has trouble with Test Ready using bus_info_xfer fsa
 * 
 * Revision 1.10	91/03/13  17:08:03	root
 * still more to do on bus_info_xfer
 * 
 * Revision 1.9	91/03/12  16:08:23	root
 * need to finish bus_info_xfer()
 * 
 * Revision 1.8	91/03/11  17:41:10	root
 * started ssopen()/wrote stub for ssinit()
 * 
 * Revision 1.7	91/03/08  17:07:28	root
 * Does Test Read and Request Sense properly.
 * 
 * Revision 1.6	91/03/07  16:41:31	root
 * sends Test Ready, Starts to Request Sense
 * 
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
#define DEV_SCSI_ID(dev)	((dev >> 4) & 0x0007)
#define DEV_LUN(dev)		((dev >> 2) & 0x0003)
#define DEV_DRIVE(dev)		((dev >> 2) & 0x001F)
#define DEV_PARTN(dev)		(dev & 0x0003)
#define DEV_SPECIAL(dev)	(dev & 0x0080)

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
#define HIPRI_RETRIES	400	/* # of times to retry while hogging CPU */
#define LOPRI_RETRIES	5	/* # of retries with sleep between tries */

#define G0CMDLEN	6	/* Group 0 commands are 6 bytes long  */
#define G1CMDLEN	10	/* Group 1 commands are 10 bytes long */
#define SENSELEN	22	/* number of bytes returned w/ req sense */
#define INQUIRYLEN	54	/* number of bytes returned w/ inquiry */

				/* Message types */
#define MSG_IDENT_DC	0xC0	/* Identify, with Disconnect allowed */
#define MSG_ABORT	0x06	/* End the current SCSI bus cycle */

#define CS_GOOD		0x00	/* Command Status from the drive */
#define CS_CHECK	0x02
#define CS_BUSY		0x08
#define CS_RESERVED	0x18

/*
 * Information Transfer Phase masks -
 * setting of RS_MESSAGE, RS_I_O, and RS_CTRL_DATA determines which of six
 * possible info transfer phases is occurring.
 */
#define XP_MSG_IN	(RS_MESSAGE | RS_I_O | RS_CTRL_DATA)
#define XP_MSG_OUT	(RS_MESSAGE          | RS_CTRL_DATA)
#define XP_STAT_IN	(             RS_I_O | RS_CTRL_DATA)
#define XP_CMD_OUT	(                      RS_CTRL_DATA)
#define XP_DATA_IN	(             RS_I_O               )
#define XP_DATA_OUT	(                                 0)

#define DEBUG	1
#if DEBUG
int stats[100], statsptr;
#define PUSHI		{ if(statsptr<100)stats[statsptr++] = i; }
#define POPI		{ printf("%d:",statsptr);while(statsptr)\
				printf("%d ",stats[--statsptr]);printf("\n");}
#define SSTELL(foo)	printf(foo)
#define SSTATUS		printf("status=%x\n", (int)(unsigned char)status)
#define SSDUMP(ssp, text) {int i;\
	printf("%s: msg_in=%x cmdstat=%x\n", text, ssp->msg_in,\
	ssp->cmdstat);if(ssp->cmdlen)for(i=0;i<ssp->cmdlen;i++)\
	printf(" %x", ssp->cmdbuf[i]);printf(" cmd_bytes_out=%d",\
	ssp->cmd_bytes_out);\
	if(ssp->data_bytes_in)for(i=0;i<ssp->data_bytes_in;i++)\
	printf(" %x", ssp->in_buf[i]);printf(" data_bytes_in=%d\n",\
	ssp->data_bytes_in);}
#else
#define PUSHI
#define POPI
#define SSTELL(foo)
#define SSTATUS
#define SSDUMP(ssp, text)
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
int	NSDRIVE = 1;		/* Bitmap of attached SCSI drives. */
int	SS_INT = 5;		/* ST0[12] use either IRQ3 or IRQ5 */
int	SS_BASE = 0xDE00;	/* Segment addr of ST0x communication area */

/*
 * Import Functions.
 */
int	nulldev();
int	nonedev();
unsigned char ffbyte();

/*
 * Local Functions.
 */
static void	ssload();
static void	ssunload();
static void	ssopen();
static void	ssclose();
static void	ssread();
static void	sswrite();
static int	ssioctl();
static void	sswatch();
static void	ssblock();
static int	ssinit();
static int	scsicmd();
static void	scsireset();
static void	ssdelay();
static int	bus_pre_xfer();
static int	bus_info_xfer();
static void	ss_start_timing();
static void	ss_stop_timing();
static int	req_sense();
static int	inquiry();
static int	read_cap();
static void	ssintr();
static void	ss_start();

/*
 * Local Variables.
 */
static BUF	dbuf;		/* For raw I/O */
static paddr_t	ss_base;	/* physical address of ST0x comm area */
static faddr_t	ss_fp;		/* (far *) to ST0x comm area */

static faddr_t	ss_ram;		/* (far *) to parameter RAM */
static faddr_t	ss_csr;		/* (far *) to control/status */
static faddr_t	ss_dat;		/* (far *) to data port */

static int	num_drives;	/* number of controller SCSI id's */
static struct ss *ss_block;	/* points to block of "ss" structs */
static TIM	delay_tim;	/* needed for calls to ssdelay() */
static TIM	timeout_tim;	/* needed for calls to timeout() */

static int	ss_expired;	/* 1 after local timeout */
static scsi_work_t	*scsi_work_queue;

/*
 * Driver CON entry - an export variable.
 */
CON	sscon	= {
	DFBLK|DFCHR,			/* Flags */
	SCSI_MAJOR,			/* Major index */
	ssopen,				/* Open */
	ssclose,			/* Close */
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

/*
 * A per-drive structure - ss
 */
#define IN_BUF_SIZE	100
typedef unsigned char	uchar;

static struct ss	{
	long	capacity;
	long	blocklen;
	int	msg_in;
	uchar	cmdbuf[G1CMDLEN];
	int	cmdlen;
	int	cmd_bytes_out;
	int	cmdstat;
	uchar	in_buf[IN_BUF_SIZE];
	int	in_buf_len;
	int	data_bytes_in;
	struct	fdisk_s parmp[NPARTN+1];
	unsigned int	ptab_read:1;  /* 1 if partition table has been read */
} *ss[MAX_SCSI_ID-1], rqs;

/*
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
	int erf = 0;  /* 1 if error occurs */
	int i;

	/*
	 * Claim IRQ vector.
	 */
	setivec(SS_INT, ssintr);

	/*
	 * Allocate a selector to map into ST0x memory-mapped comm area.
	 */
	ss_base = (paddr_t)((long)(unsigned)SS_BASE << 4);
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
		erf = 1;
	}

	/*
	 * Allocate drive structs.
	 *
	 * Do a single call to kalloc() then put allocated pieces into
	 * array ss.
	 */
	if (!erf) {
		for (i = 0; i < MAX_SCSI_ID -1; i++)
			if ((NSDRIVE >> i) & 1)
				num_drives++;
		if (num_drives == 0) {
			printf("Error - ss has no valid target id's\n");
			erf = 1;
		} else if ((ss_block = kalloc(num_drives*sizeof(struct ss)))
		== NULL) {
			printf("Error - ss can't allocate structs\n");
			erf = 1;
		} else
			kclear(ss_block, num_drives * sizeof(struct ss));
	}
	if (!erf) {
		struct ss *foo = ss_block;

		for (i = 0; i < MAX_SCSI_ID -1; i++)
			if ((NSDRIVE >> i) & 1)
				ss[i] = foo++;
	}

	/*
	 * Initialize drives we know about (i.e. in NSDRIVE bitmap).
	 */
	if (!erf) {
		for (i = 0; i < MAX_SCSI_ID -1; i++)
			if ((NSDRIVE >> i) & 1)
				ssinit(i);
	}
}

/*
 * void
 * ssunload()	- unload routine.
 */
static void
ssunload()
{
	/*
	 * Deallocate driver heap space.
	 */
	if (ss_block)
		kfree(ss_block);

	/*
	 * Free the ST0x selector.
	 */
	vrelse(ss_fp);

	/*
	 * Release IRQ vector.
	 */
	clrivec(SS_INT);
}

/*
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
	int drive, partn;
	int erf = 0;

	drive = DEV_SCSI_ID(dev);
	partn = DEV_PARTN(dev);

	/*
	 * LUN must be zero.
	 * SCSI id must have corresponding 1 in NSDRIVE bitmapped variable.
	 */
	if (DEV_LUN(dev) != 0 || ((1 << drive) & NSDRIVE) == 0) {
		u.u_error = ENXIO;
		erf = 1;
	}

	/*
	 * If "special" bit is set, partition must be zero.
	 */
	if (!erf && DEV_SPECIAL(dev) && partn != 0) {
		u.u_error = ENXIO;
		erf = 1;
	}

	/*
	 * If "special" bit is NOT set, error return for now.
	 */
	if (!erf && !DEV_SPECIAL(dev)) {
		u.u_error = ENXIO;
		erf = 1;
	}

	/*
	 * OK - open the device.
	 */
	if (!erf) {
		++drvl[SCSI_MAJOR].d_time;
	}
#if 0
	/*
	 * Ensure partition lies within drive boundaries and is non-zero size.
	 */
	if ((pparm[p].p_base+pparm[p].p_size) > pparm[d+NDRIVE*NPARTN].p_size)
		u.u_error = EBADFMT;
	else if ( pparm[p].p_size == 0 )
		u.u_error = ENODEV;
#endif
}

/*
 * ssclose()
 */
static void ssclose( dev )
dev_t dev;
{
	--drvl[SDMAJOR].d_time;	
}

/*
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

/*
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

/*
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
	int ret = 0;

	switch(cmd) {
	default:
		u.uerror = EINVAL;
		ret = -1;
	}

	return ret;
}

/*
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
	register scsi_work_t *sw;
	register int s;
	struct	fdisk_s	*fdp;
	int partition, drive, s_id;
	dev_t dev;
	struct ss * ssp;

	dev = bp->b_dev;
	partition = DEV_PARTN(dev);
	drive = DEV_DRIVE(dev);
	s_id = DEV_SCSI_ID(dev);
	ssp = ss[s_id];

	if (dev & SDEV)
		partition = WHOLE_DRIVE;
	bp->b_resid = bp->b_count;
	
	fdp = ssp->parmp;

	/*
	 * Range check disk region.
	 */
	if (!(ssp->ptab_read)) {
		if ( partition == WHOLE_DRIVE ) {
			if ((bp->b_bno != 0) || (bp->b_count != BSIZE)) {
				bp->b_flag |= BFERR;
				bdone(bp);
				return;
			}
		} else {
			devmsg(dev, "no partition table");
			bp->b_flag |= BFERR;
			bdone(bp);
			return;
		}
	} else if ( (bp->b_bno + (bp->b_count/BSIZE))
	> fdp[partition].p_size ) {
		bp->b_flag |= BFERR;
		bdone(bp);
		return;
	}

	bp->b_actf = NULL;
	sw = (scsi_work_t *)kalloc( sizeof(*sw) );
	if (sw == (scsi_work_t *)0) {
		devmsg(dev, "out of kernel memory");
		bp->b_flag |= BFERR;
		bdone(bp);
		return;
	}
	sw->sw_bp = bp;
	sw->sw_drv = drive;
	if (partition != WHOLE_DRIVE)
		sw->sw_bno = fdp[partition].p_base + bp->b_bno;
	else
		sw->sw_bno = bp->b_bno;
	sw->sw_retry = 1;

printf("ssblock: drv %x bno %x:%x  bp=%x, flag = %o\n",
	drv, (long)sw->sw_bno, bp, bp->b_flag);

	s = sphi();
	if (sd.sw_actf == NULL)
		sd.sw_actf = sw;
	else
		sd.sw_actl->sw_actf = sw;
	sd.sw_actl = sw;
	spl(s);

	ss_start();
}

/*
 *
 * void
 * ssintr()	- Interrupt routine.
 *
 */
#if 0
static int irpted;
static long x;
for (x = 0, irpted = 0; x < 100000L; x++)  if (irpted) break;
#endif

static void
ssintr()
{
	printf("@");
}

/*
 * sswatch()
 */
static void	sswatch()
{
	static int calls;

	if (calls == 0)
		printf("*");
	calls++;
	if (calls >= 60)
		calls = 0;
}

/*
 * bus_wait()
 *
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
	for ( i = 0; i < HIPRI_RETRIES; i++) {
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

/*
 * Attempt to initialize the (unique) drive with a given SCSI id.
 * Assume only one drive per SCSI id, having LUN = 0.
 * 
 * Return 1 if success, 0 if failure.
 *
 * Pseudocode:
 *
 * retval = 0
 * if Test Unit Ready command fails
 *   print "Test Unit Ready fails"
 * else if Request Sense command fails
 *   print "Request Sense fails"
 * else if Read Capacity command succeeds
 *   print "Read Capacity fails"
 * else if partition table can't be read
 *   print "can't get partition table"
 * else
 *   print "SCSI id #n initialized"
 *   retval = 1
 * return retval
 */
static int ssinit(s_id)
int s_id;
{
	int retval = 0;
	int dev = ((sscon.c_mind << 8) | 0x80 | (s_id << 4));

	if (testready(s_id)) {
		retval = 1;
	} else
		devmsg(dev, "Test Unit Ready Failed");

	if (retval)
		if (req_sense(s_id)) {
			retval = 1;
		} else
			devmsg(dev, "Request Sense Failed");

	if (retval)
		if (inquiry(s_id)) {
			ss[s_id]->in_buf[INQUIRYLEN] = 0;
			devmsg(dev, ss[s_id]->in_buf + 8);
			if (ss[s_id]->in_buf[0] == 0) {
				retval = 1;
			} else
				devmsg(dev, "Not Direct Access Device");
		} else
			devmsg(dev, "Inquiry Failed");

	if (retval)
		if (read_cap(s_id)) {
			retval = 1;
		} else
			devmsg(dev, "Read Capacity Failed");

	if (retval) {
		retval = fdisk(dev, ss[s_id]->parmp);
		if (retval)
			printf("fdisk succeeded\n");
		else
			printf("fdisk failed\n");
	}

	return retval;
}

/*
 * Send Test Unit Ready command.
 * Retry after bus reset if necessary.
 *
 * Return 1 if unit is ready, 0 if not.
 */
static int testready(s_id)
int s_id;
{
	int retval;
	struct ss * ssp = ss[s_id];

	ssp->cmdbuf[0] = ScmdTESTREADY;
	ssp->cmdbuf[1] = ssp->cmdbuf[2] = ssp->cmdbuf[3] = ssp->cmdbuf[4] =
		ssp->cmdbuf[5] = 0;
	ssp->cmdlen = G0CMDLEN;
	retval = scsicmd(s_id);

	return retval;
}

/*
 * scsicmd()
 *
 * Send command packet to target device.
 * Start a new SCSI bus cycle when this routine is called.
 * If command status after sending is Device Check (CS_CHECK), do a
 * Request Sense to find out what happened and clear check status.
 *
 * Return 1 if command was send and status was good, else 0.
 */
static int scsicmd(s_id)
int s_id;
{
	int retval;
	struct ss *ssp = ss[s_id];

	if (retval = bus_pre_xfer(s_id)) {
		bus_info_xfer(ssp);
		retval = (ssp->cmdlen == ssp->cmd_bytes_out
			&& ssp->cmdstat == CS_GOOD);
	}

	if (ssp->cmdstat == CS_CHECK) {
		if (req_sense(s_id))
			retval = (ssp->cmdlen == ssp->cmd_bytes_out);
	}

	return retval;
}

/*
 * scsireset()
 *
 * Reset the SCSI bus.
 * Allow settling time when turning reset on/off.
 * Settling times were determined empirically.
 * Each tick is 10 msec.
 */
#define RESET_TICKS	40
static void scsireset()
{
	sfbyte(ss_csr, WC_ENABLE_SCSI | WC_SCSI_RESET);
	ssdelay(RESET_TICKS);
	sfbyte(ss_csr, 0);
	ssdelay(RESET_TICKS);
}

/*
 * ssdelay()
 *
 * Delay for some number of clock ticks.
 * 286/386 kernel ticks are at 100Hz
 */
static void ssdelay(ticks)
int ticks;
{
	timeout(&delay_tim, ticks, wakeup, (int)&delay_tim);
	sleep((char *)&delay_tim, CVPAUSE, IVPAUSE, SVPAUSE);
}

/*
 * Start a timeout for some number of ticks.
 * Caller knows timer has expired when "ss_expired" goes to 1.
 *
 * Sample invocation:
 *	ss_start_timing(n);
 *	while (check for desired event fails) {
 *		if (ss_expired) {
 *			...failure stuff..
 *			break;
 *		}
 *		ssdelay(m); <= needed to allow kernel to update timers
 *	}
 */
static void ss_start_timing(ticks)
int ticks;
{
	ss_expired = 0;
	timeout(&timeout_tim, ticks, ss_stop_timing, 1);
}

/*
 * Stub function called only by ss_start_timing()
 */
static void ss_stop_timing(flagval)
int flagval;
{
	ss_expired = flagval;
}

/*
 * bus_pre_xfer()
 *
 * Do bus cycle phases prior to the information transfer phases.
 * This includes arbitration and selection.
 */
static int bus_pre_xfer(s_id)
int s_id;
{
	int tries;
	int dev = ((sscon.c_mind << 8) | 0x80 | (s_id << 4));
	int ret;

	for (ret = 0, tries = 0; !ret && tries < LOPRI_RETRIES; tries++) {
		/*
		 * Do ST0x arbitration.
		 */
		sfbyte(ss_csr, 0);		/* De-assert SCSI enable bit */
		sfbyte(ss_dat, HOST_ID);	/* Write my SCSI id to port */
		sfbyte(ss_csr, WC_ARBITRATE);	/* Start arbitration */

		/*
		 * SCSI spec says there is "no maximum" to the wait for arbitration
		 * complete.
		 */
		if (!bus_wait(RS_ARBIT_COMPL << 8 | RS_ARBIT_COMPL)) {
			scsireset();
			continue;
		}

		/*
		 * Arbitration complete.  Now select, with ATN to allow messages.
		 */
		sfbyte(ss_dat, HOST_ID | (1 << s_id));	/* Write both SCSI id's */
		sfbyte(ss_csr, WC_ENABLE_SCSI | WC_ATTENTION | WC_SELECT);

		if (!bus_wait(RS_BUSY << 8 | RS_BUSY))
			continue;

		/*
		 * Send "Identify" Message with Disconnect allowed.
		 */
		sfbyte(ss_csr, WC_ENABLE_SCSI | WC_ATTENTION);

		if (!bus_wait(((RS_REQUEST|RS_CTRL_DATA|RS_I_O|RS_MESSAGE) << 8)
		| (RS_REQUEST|RS_CTRL_DATA|RS_MESSAGE)))
			continue;

		sfbyte(ss_csr, WC_ENABLE_SCSI);
		sfbyte(ss_dat, MSG_IDENT_DC);
		ret = 1;
	}

	return ret;
}

/*
 * bus_info_xfer()
 *
 * Do bus cycle information transfer phases.
 * This includes message in/out, command in/out, and data in/out.
 *
 * If cmdlen is nonzero, cmdbuf is an array of bytes of that length,
 * to be sent to the target.
 *
 * Return 1 if bus timeout did not occur, else 0.
 *
 * pseudocode:
 *
 * while (wait for REQ true or BUSY false on SCSI bus)
 *   if (BUSY false)
 *     break from while loop
 *   else
 *     switch (xfer phase = RS_CTRL_DATA|RS_I_O|RS_MESSAGE)
 *       case XP_MSG_IN/XP_MSG_OUT/...
 *         handle the indicated information transfer phase
 *     endswitch
 *   endif
 * endwhile
 */
int zzzz;

static int bus_info_xfer(ssp)
struct ss *ssp;
{
	int bus_timeout;
	unsigned char phase_type;
	int no_msg_rcvd = 1;
	int s;
	int bytes_to_send;
int zzgo=0;

	ssp->cmdstat = -1;
	ssp->data_bytes_in = 0;
	ssp->cmd_bytes_out = 0;
	s = sphi();
	while(req_wait(&bus_timeout)) {
if(zzgo) {
	zzgo = 0;
	printf("zzzz=%d\n", zzzz);
}
		phase_type = ffbyte(ss_csr) & (RS_MESSAGE|RS_I_O|RS_CTRL_DATA);
		switch (phase_type) {
		case XP_MSG_IN:
			/*
			 * Only pay attention to first msg byte in.
			 * Don't care about extended messages.
			 */
			if (no_msg_rcvd) {
				no_msg_rcvd = 0;
				ssp->msg_in = ffbyte(ss_dat);
			} else
				ffbyte(ss_dat);
			break;
		case XP_MSG_OUT:
			/*
			 * This case shouldn't happen.  We weren't
			 * asserting ATTENTION.  Abort the bus cycle.
			 */
			sfbyte(ss_csr, WC_ENABLE_SCSI);
			sfbyte(ss_dat, MSG_ABORT); 
			break;
		case XP_STAT_IN:
			ssp->cmdstat = ffbyte(ss_dat);
			break;
		case XP_CMD_OUT:
			/*
			 * Ship out command bytes.
			 * Reset SCSI bus if too many command bytes are wanted.
			 */
			bytes_to_send = ssp->cmdlen - ssp->cmd_bytes_out;
			if(bytes_to_send > 0) {
				sfbyte(ss_dat, ssp->cmdbuf[ssp->cmd_bytes_out++]);
				/*
				 * If just sent last byte, allow interrupts.
				 */
				if (bytes_to_send == 1) {
					spl(s);
zzgo=1;
					s = sphi();
				}
			} else {	/* This case should not happen. */
SSDUMP(ssp, "Command overrun");
				scsireset();
			}
			break;
		case XP_DATA_IN:
			/*
			 * If caller's buffer has room, keep incoming
			 * data byte.  Else toss it.
			 */
			if (ssp->data_bytes_in < ssp->in_buf_len)
				ssp->in_buf[ssp->data_bytes_in]
				= ffbyte(ss_dat);
			else
				ffbyte(ss_dat);
			ssp->data_bytes_in++;
			break;
		case XP_DATA_OUT:
			/*
			 * Temporary filler.
			 */
			sfbyte(ss_dat, 0xAA);
			break;
		default:
			break;
		} /* endswitch */
	}
	spl(s);
POPI;
	return (bus_timeout) ? 0 : 1 ;
}
/*
 * req_wait()
 *
 * This routine is called at the start of each information transfer
 * phase and after the last such phase.
 *
 * It returns 1 if REQ is asserted on the SCSI bus, meaning another phase
 * may begin, and 0 otherwise.  A REQ signal will not be seen if the function
 * times out or if BUSY drops.  A value of 1 is written to the pointer argument
 * if timeout occurred, else 0 is written.
 */
static int req_wait(to_ptr)
int *to_ptr;
{
	int req_found, i;
	unsigned char status;

	*to_ptr = 1;
	req_found = 0;
	for ( i = 0; i < HIPRI_RETRIES; i++) {
		status = ffbyte(ss_csr);
		if (status & RS_REQUEST) {
			req_found = 1;
			*to_ptr = 0;
			break;
		} else if ((status & RS_BUSY) == 0) {
			*to_ptr = 0;
			break;
		}
	}

	if (*to_ptr)
		printf("ST0x info xfer timeout;  status=%x\n", status);
PUSHI;
zzzz=i;
	return req_found;
}

/*
 * req_sense()
 *
 * Request Sense for a device.  The main reason for doing this is to
 * clear a standing Command Status of Device Check.
 *
 * Full results are discarded.  Return 1 if Device returns No Sense or
 * or Unit Attention.  Else return 0.
 *
 */
static int req_sense(s_id)
int s_id;
{
	int ret = 0;

	rqs.cmdbuf[0] = ScmdREQUESTSENSE;
	rqs.cmdbuf[1] = rqs.cmdbuf[2] = rqs.cmdbuf[3] =
		rqs.cmdbuf[5] = 0;
		rqs.cmdbuf[4] = SENSELEN;
	rqs.cmdlen = G0CMDLEN;
	rqs.in_buf_len = SENSELEN;

	if (bus_pre_xfer(s_id)) {
		bus_info_xfer(&rqs);
		if (rqs.data_bytes_in == SENSELEN) {
			if (rqs.in_buf[2] == 0x00)	/* No Sense.  AOK */
				ret = 1;
			else if (rqs.in_buf[2] == 0x06 && rqs.in_buf[12] == 0x29)
				ret = 1;
		}
	}

	return ret;
}

/*
 * inquiry()
 *
 * Inquiry command for a device.
 * Find out if device is direct access, removable, etc.
 *
 * Return 1 if command succeeds, else 0.
 */
static int inquiry(s_id)
int s_id;
{
	int ret = 0;
	struct ss * ssp = ss[s_id];

	ssp->cmdbuf[0] = ScmdINQUIRY;
	ssp->cmdbuf[1] = ssp->cmdbuf[2] = ssp->cmdbuf[3] =
		ssp->cmdbuf[5] = 0;
		ssp->cmdbuf[4] = INQUIRYLEN;
	ssp->cmdlen = G0CMDLEN;
	ssp->in_buf_len = INQUIRYLEN;

	ret = scsicmd(s_id);

	return ret;
}

/*
 * read_cap()
 *
 * Read Capacity command for a device.
 *
 * Return 1 if command succeeds, else 0.
 */
static int read_cap(s_id)
int s_id;
{
	int ret = 0;
	struct ss * ssp = ss[s_id];

	ssp->cmdbuf[0] = ScmdREADCAPACITY;
	ssp->cmdbuf[1] = ssp->cmdbuf[2] = ssp->cmdbuf[3] = ssp->cmdbuf[4] = 0;
	ssp->cmdbuf[5] = ssp->cmdbuf[6] = ssp->cmdbuf[7] = ssp->cmdbuf[8] = 0;
	ssp->cmdbuf[9] = 0;
	ssp->cmdlen = G1CMDLEN;
	ssp->in_buf_len = 8;

	ret = scsicmd(s_id);
	if (ret) {
		ssp->capacity = ssp->in_buf[3] | (ssp->in_buf[2] << 8)
		| (((long)(ssp->in_buf[1])) << 16)
		| (((long)(ssp->in_buf[0])) << 24);
		ssp->blocklen = ssp->in_buf[7] | (ssp->in_buf[6] << 8)
		| (((long)(ssp->in_buf[5])) << 16)
		| (((long)(ssp->in_buf[4])) << 24);
printf("capacity=%ld   block length=%ld\n", ssp->capacity, ssp->blocklen);
	}

	return ret;
}

/*
 * ss_start()
 *
 * Invoked whenever there is I/O to do.  Pull first request, if any,
 * off the queue and try to send it to the drive.
 * If request is sent, delete it from the queue.
 *
 * Disallow re-entrancy in this routine (variable "locked").
 */
static void ss_start()
{
	int s;
	scsi_work_t *sw;
	static char locked;

	s = sphi();
	if( locked ) {
		spl(s);
		return;
	}
	++locked;
	spl(s);

	if( (sw = scsi_work_queue->sw_actf) != NULL ) {
		if (do_ss(sw)) {
			s = sphi();
			sw = scsi_work_queue->sw_actf = sw->sw_actf;
			if( sw == NULL )
				scsi_work_queue->sw_actl = NULL;
			spl(s);
		}
	}
	--locked;
}
