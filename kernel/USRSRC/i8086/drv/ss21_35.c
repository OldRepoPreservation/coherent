int rpt_irpt;
int busted;
/*
 * This is a driver for Seagate ST01/ST02 scsi hard disk controllers.
 *
 * To do:
 *	figure out a better storage class for rqs
 *      make input buffer for commands dynamic (?)
 *
 * $Log:	/usr/src/sys/i8086/drv/RCS/ss.c,v $
 * Revision 1.22	91/04/10  16:55:59	root
 * Starting to get block operations working.
 * 
 * Revision 1.21	91/04/10  15:21:41	root
 * Move define's to ss.h and scsiwork.h.  Other cleanup
 * 
 * Revision 1.20	91/04/09  14:23:49	root
 * Reads boot sector 100 times using IRQ on reconnect
 */

/*
 * Definitions.
 */
#define DEV_SCSI_ID(dev)	((dev >> 4) & 0x0007)
#define DEV_LUN(dev)		((dev >> 2) & 0x0003)
#define DEV_DRIVE(dev)		((dev >> 2) & 0x001F)
#define DEV_PARTN(dev)		(dev & 0x0003)
#define DEV_SPECIAL(dev)	(dev & 0x0080)

#define HOST_ID		0x80	/* Host adapter is SCSI ID #7 */
#define HIPRI_RETRIES	400	/* # of times to retry while hogging CPU */
#define LOPRI_RETRIES	5	/* # of retries with sleep between tries */
#define WHOLE_DRIVE	NPARTN

				/* Device States */
#define	SIDLE		0	/* controller idle */
#define	SRETRY		1	/* seeking */
#define	SREAD		2	/* reading */
#define	SWRITE		3	/* writing */

#define DEBUG	1
#if DEBUG
int stats[100], statsptr;
#define PUSHI		{ if(statsptr<100)stats[statsptr++] = i; }
#define POPI		{ printf("%d:",statsptr);while(statsptr)\
				printf("%d ",stats[--statsptr]);printf("\n");}
#define SSTELL(foo)	printf(foo)
#define SSTATUS		{uchar status = ffbyte(ss_csr);printf("status=%x\n", status);}
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
#include	<sys/devices.h>		/* SCSI_MAJOR */
#include	<errno.h>

#include 	<sys/fdisk.h>
#include	<sys/hdioctl.h>
#include	<sys/buf.h>
#include	<scsiwork.h>
#include	<ss.h>

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
extern int	nulldev();
extern int	nonedev();
extern unsigned char ffbyte();

/*
 * Local Functions.
 */
static void	ssopen();		/* CON functions */
static void	ssclose();
static void	ssblock();
static void	ssread();
static void	sswrite();
static int	ssioctl();
static void	sswatch();
static void	ssload();
static void	ssunload();

static void	bus_dev_reset();	/* additional support functions */
static int	bus_info_xfer();
static int	bus_pre_xfer();
static void	do_ss();
static int	inquiry();
static int	read_cap();
static int	req_sense();
static int	scsicmd();
static void	scsireset();
static void	ss_done();
static void	ss_start();
static void	ss_start_timing();
static void	ss_stop_timing();
static void	ssdelay();
static int	ssinit();
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

static int	num_drives;	/* number of controller SCSI id's */
static struct ss *ss_block;	/* points to block of "ss" structs */
static int	st0x_busy;	/* 1 if SCSI host adapter busy */

static TIM	delay_tim;	/* needed for calls to ssdelay() */
static TIM	timeout_tim;	/* needed for calls to timeout() */
static int	ss_expired;	/* 1 after local timeout */
static int	ss_state;	/* starts at SIDLE */

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
#define IN_BUF_SIZE	512
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
	unsigned int	id_busy:1;  /* 1 if device with this SCSI id busy */
} *ss[MAX_SCSI_ID-1], rqs;

/*
 * ssload()	- load routine.
 *
 *	Action:	The controller is reset and the interrupt vector is grabbed.
 *		The drive characteristics are set up at this time.
 */
static void ssload()
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
 * ssunload()	- unload routine.
 */
static void ssunload()
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
 * ssopen()
 *
 *	Input:	dev = disk device to be opened.
 *		mode = access mode [IPR,IPW, IPR+IPW].
 *
 *	Action:	Validate the minor device.
 *		Update the paritition table if necessary.
 */
static void ssopen(dev, mode)
register dev_t	dev;
{
	int drive, partn;
	int valid_open;
	struct	fdisk_s	*fdp;
	struct ss * ssp;
	int s_id;

	/*
	 * Set up local variables.
	 */
	valid_open = 1;
	drive = DEV_SCSI_ID(dev);
	partn = DEV_PARTN(dev);
	s_id = DEV_SCSI_ID(dev);
	ssp = ss[s_id];
	fdp = ssp->parmp;

	/*
	 * LUN must be zero.
	 * SCSI id must have corresponding 1 in NSDRIVE bitmapped variable.
	 */
	if (DEV_LUN(dev) != 0 || ((1 << drive) & NSDRIVE) == 0) {
		u.u_error = ENXIO;
		valid_open = 0;
	}

	/*
	 * If "special" bit is set, partition field must be zero.
	 */
	if (valid_open && DEV_SPECIAL(dev) && partn != 0) {
		u.u_error = ENXIO;
		valid_open = 0;
	}

	/*
	 * Subscripting gimmick for partition table.
	 */
	if (valid_open && dev & SDEV)
		partn = WHOLE_DRIVE;

	/*
	 * If not accessing whole drive and the partition table has not
	 * been read yet, try to read it now.
	 */
	if (valid_open && partn != WHOLE_DRIVE && !(ssp->ptab_read))
		if (fdisk(dev, fdp)) {
int p;
printf("fdisk() succeeded\n");
for (p=0; p<WHOLE_DRIVE; p++)
	printf("p=%d base=%ld size=%ld\n", p, fdp[p].p_base, fdp[p].p_size);
			ssp->ptab_read = 1;
		} else {
printf("fdisk() failed\n");
			u.u_error = ENXIO;
			valid_open = 0;
		}

	/*
	 * Ensure partition lies within drive boundaries and is non-zero size.
	 */
	if (valid_open
	&& (fdp[partn].p_base+fdp[partn].p_size) > fdp[WHOLE_DRIVE].p_size) {
		u.u_error = EBADFMT;
		valid_open = 0;
	}

	if (valid_open && fdp[partn].p_size == 0) {
		u.u_error = ENODEV;
		valid_open = 0;
	}

	/*
	 * OK - open the device.
	 */
	if (valid_open) {
		++drvl[SCSI_MAJOR].d_time;
	}
}

/*
 * ssclose()
 */
static void ssclose(dev)
dev_t dev;
{
	--drvl[SCSI_MAJOR].d_time;	
}

/*
 * ssread()	- read a block from the raw disk
 *
 *	Input:	dev = disk device to be written to.
 *		iop = pointer to source I/O structure.
 *
 *	Action:	Invoke the common raw I/O processing code.
 */
static void ssread(dev, iop)
dev_t	dev;
IO	*iop;
{
	ioreq( &dbuf, iop, dev, BREAD, BFRAW|BFBLK|BFIOC );
}

/*
 * sswrite()	- write a block to the raw disk
 *
 *	Input:	dev = disk device to be written to.
 *		iop = pointer to source I/O structure.
 *
 *	Action:	Invoke the common raw I/O processing code.
 */
static void sswrite(dev, iop)
dev_t	dev;
IO	*iop;
{
	ioreq( &dbuf, iop, dev, BWRITE, BFRAW|BFBLK|BFIOC );
}

/*
 * ssioctl()
 *
 *	Input:	dev = disk device to be operated on.
 *		cmd = input/output request to be performed.
 *		vec = (pointer to) optional argument.
 *
 *	Action:	Validate the minor device.
 *		Update the paritition table if necessary.
 */
static int ssioctl(dev, cmd, vec)
register dev_t	dev;
int cmd;
char * vec;
{
	int ret = 0;

	switch(cmd) {
	default:
		u.u_error = EINVAL;
		ret = -1;
	}

	return ret;
}

/*
 * ssblock()	- queue a block to the disk
 *
 *	Input:	bp = pointer to block to be queued.
 *
 *	Action:	Queue a block to the disk.
 *		Make sure that the transfer is within the disk partition.
 */
static void ssblock(bp)
register BUF	*bp;
{
	register scsi_work_t *sw;
	register int s;
	struct	fdisk_s	*fdp;
	int partition, drive, s_id;
	dev_t dev;
	struct ss * ssp;
	int valid_op = 1;

	bp->b_resid = bp->b_count;

	/*
	 * Set up local variables.
	 */
	dev = bp->b_dev;
	partition = DEV_PARTN(dev);
	drive = DEV_DRIVE(dev);
	s_id = DEV_SCSI_ID(dev);
	ssp = ss[s_id];
	if (dev & SDEV)
		partition = WHOLE_DRIVE;
	fdp = ssp->parmp;

	/*
	 * Range check disk region.
	 */
	if (!(ssp->ptab_read)) {
		if ( partition == WHOLE_DRIVE ) {
			if ((bp->b_bno != 0) || (bp->b_count != BSIZE)) {
				bp->b_flag |= BFERR;
				valid_op = 0;
			}
		} else {
			devmsg(dev, "no partition table");
			bp->b_flag |= BFERR;
			valid_op = 0;
		}
	}
	/*
	 * Check for read at end of partition.
	 * (Need to return with b_resid = BSIZE to signal end of volume.)
	 */
	else if ((bp->b_req == BREAD) && (bp->b_bno == fdp[partition].p_size)) {
		valid_op = 0;
	}
	/*
	 * Check for read past end of partition.
	 */
	else if ( (bp->b_bno + (bp->b_count/BSIZE))
	> fdp[partition].p_size ) {
		bp->b_flag |= BFERR;
		valid_op = 0;
	}

	/*
	 * See if we can allocate a request node for this operation.
	 */
	if (valid_op) {
		bp->b_actf = NULL;
		sw = (scsi_work_t *)kalloc( sizeof(*sw) );
		if (sw == NULL) {
			devmsg(dev, "out of kernel memory");
			bp->b_flag |= BFERR;
			valid_op = 0;
		}
	}

	/*
	 * Operation appears valid and we have a node for it.
	 * Fill fields in the node and queue the request.
	 */
	if (valid_op) {
		sw->sw_bp = bp;
		sw->sw_drv = drive;
		if (partition != WHOLE_DRIVE)
			sw->sw_bno = fdp[partition].p_base + bp->b_bno;
		else
			sw->sw_bno = bp->b_bno;
		sw->sw_retry = 1;

printf("ssblock: drv %x bno %x:%x  bp=%x, flag = %o\n",
	drive, (long)sw->sw_bno, bp, bp->b_flag);

		ssq_wr_tail(sw);
		if (ss_state == SIDLE)
			ss_start();
	/*
	 * Operation cannot be done.  Release the kernel buffer structure.
	 * Value of "bp->b_flag" tells caller if error occurred.
	 */
	} else { 	/* "valid_op" is FALSE */
		bdone(bp);
	}
}

/*
 * ssintr()	- Interrupt routine.
 */
static void ssintr()
{
	printf("@");
	rpt_irpt=1;
	wakeup(&rpt_irpt);
}

/*
 * sswatch()
 */
static void sswatch()
{
	printf("*");
	busted = 1;
	drvl[SCSI_MAJOR].d_time=0;
	wakeup(&rpt_irpt);
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

	return found;
}

/*
 * ssinit()
 *
 * Attempt to initialize the (unique) drive with a given SCSI id.
 * Assume only one drive per SCSI id, having LUN = 0.
 * 
 * Return 1 if success, 0 if failure.
 */
static int ssinit(s_id)
int s_id;
{
	int retval = 0;
	int dev = ((sscon.c_mind << 8) | 0x80 | (s_id << 4));

	/*
	 * Try Test Unit Ready command.
	 * If it fails, reset SCSI bus and target device, and try again.
	 */
	if (testready(s_id))
		retval = 1;
	else {
		scsireset();
		bus_dev_reset(s_id);
		if (testready(s_id))
			retval = 1;
		else
			devmsg(dev, "Test Unit Ready Failed");
	}

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

#if 1
	/*
	 * For test purposes only, try to read the partition table.
	 */
	if (retval) {
#define READ_PTS	1
int foo,fof;
for (foo=0,fof=0;foo<READ_PTS;){
	rpt_irpt=0;
	busted=0;
	drvl[SCSI_MAJOR].d_time=1;
		if (read_pt(s_id)) {
			retval = 1;
		} else {
			devmsg(dev, "Read Partition Table Failed");
			break;
		}
foo++;
	if (!rpt_irpt){
		fof++;
		if (fof>=3) {
			printf("3 irq's lost\n");
			break;
		}
	}
} /*endfor*/
printf("%d read_pt's\n",foo);
	}
#endif

	return retval;
}

/*
 * testready()
 *
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
	int tries;

	tries = 0;
	do {
		if (tries > 0)
			ssdelay(100);

		if (retval = bus_pre_xfer(s_id)) {
			bus_info_xfer(ssp);
			retval = (ssp->cmdlen == ssp->cmd_bytes_out
				&& ssp->cmdstat == CS_GOOD);
		}

		if (ssp->cmdstat == CS_CHECK) {
			if (req_sense(s_id))
				retval = (ssp->cmdlen == ssp->cmd_bytes_out);
		}

		tries++;
	} while (ssp->cmdstat == CS_BUSY && tries < LOPRI_RETRIES);

	if (ssp->msg_in == MSG_DISCONNECT) {
		int connected = 0;
		uchar dat, csr;

printf("Disconnected ");
{
	int s;
	s=sphi();
	while(!rpt_irpt && !busted)
		sleep(&rpt_irpt, CVBLKIO,IVBLKIO,SVBLKIO);
	spl(s);
}
		for (tries = 0; tries < 10; tries++) {
			csr = ffbyte(ss_csr);
			if (csr & RS_SELECT) {
				dat = ffbyte(ss_dat);
				if (dat & HOST_ID) {
printf("%d tries Reconnected\n",tries);
					connected = 1;
					break;
				} else {
					int t;
printf("Host not selected\n");
					for (t = 0; t < 10; t++) {
						if (ffbyte(ss_csr) & RS_SELECT == 0) {
printf("Select dropped by target\n");
							break;
						}
						ssdelay(10);
					}
				}
			}
			ssdelay(10);
		}
		if (connected) {
			sfbyte(ss_csr, WC_ENABLE_SCSI | WC_BUSY);
			if (bus_wait(RS_SELECT << 8 | 0)) {
				sfbyte(ss_csr, WC_ENABLE_SCSI);
				bus_info_xfer(ssp);
				retval = (ssp->cmdstat == CS_GOOD);
			}
		}
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
printf("scsireset\n");
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
 * ss_start_timing()
 *
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
 * ss_stop_timing()
 *
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

		sfbyte(ss_dat, MSG_IDENT_DC);
		sfbyte(ss_csr, WC_ENABLE_SCSI | WC_ENABLE_IRPT);
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
static int bus_info_xfer(ssp)
struct ss *ssp;
{
	int bus_timeout;
	uchar phase_type;
	uchar msg_in;
	int no_msg_rcvd = 1;
	int s;
	int bytes_to_send;

	ssp->cmdstat = -1;
	ssp->data_bytes_in = 0;
	ssp->cmd_bytes_out = 0;
	ssp->msg_in = -1;
	s = sphi();
	while(req_wait(&bus_timeout)) {
		phase_type = ffbyte(ss_csr) & (RS_MESSAGE|RS_I_O|RS_CTRL_DATA);
		switch (phase_type) {
		case XP_MSG_IN:
			msg_in = ffbyte(ss_dat);
			switch(msg_in){
			case MSG_CMD_CMPLT:
				ssp->msg_in = msg_in;
				break;
			case MSG_SAVE_DPTR:
				break;
			case MSG_RSTOR_DPTR:
				break;
			case MSG_DISCONNECT:
				ssp->msg_in = msg_in;
				break;
			case MSG_ABORT:
				break;
			case MSG_DEV_RESET:
				break;
			case MSG_IDENT_DC:
				break;
			}
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
 * off the queue, send it to the drive, and delete it from the queue.
 *
 * Disallow re-entrancy in this routine (variable "locked").
 */
static void ss_start()
{
	int s;
	scsi_work_t *sw;
	static char locked;

	s = sphi();
	if(locked) {
		spl(s);
		return;
	}
	++locked;
	spl(s);

	if((sw = ssq_rm_head()) != NULL) {
		if (sw->sw_bp->b_req == BWRITE)
			ss_state = SWRITE;
		else if (sw->sw_bp->b_req == BREAD)
			ss_state = SREAD;
		else
			printf("Error:  b_req=%d\n", sw->sw_bp->b_req);
		do_ss(sw);
	}
	--locked;
}

/*
 * do_ss()
 *
 * Begin a block read or write command as found in an "sw" queue entry.
 */
static void do_ss(sw)
struct scsi_work_t * sw;
{
	BUF * bp;

printf("do_ss\n");
	bp = sw->sw_bp;
	switch(ss_state) {
	case SREAD:
		bp->b_resid -= BSIZE;
		ss_done(sw);
		break;
	case SWRITE:
		bp->b_resid -= BSIZE;
		ss_done(sw);
		break;
	}
}

/*
 * ss_done
 *
 * Release current i/o buffer to the O/S.
 */
static void ss_done(sw)
struct scsi_work_t * sw;
{
	BUF * bp;

printf("ss_done\n");
	bp = sw->sw_bp;

	ss_state = SIDLE;
	bdone(bp);
	kfree(sw);

	if (ssq_rd_head())
		ss_start();
}

/*
 * read_pt()
 *
 * Read partition table for a device.
 *
 * Return 1 if command succeeds, else 0.
 */
static int read_pt(s_id)
int s_id;
{
	int ret = 0;
	struct ss * ssp = ss[s_id];

	ssp->cmdbuf[0] = ScmdREADEXTENDED;
	ssp->cmdbuf[1] = ssp->cmdbuf[2] = ssp->cmdbuf[3] = ssp->cmdbuf[4] = 0;
	ssp->cmdbuf[5] = ssp->cmdbuf[6] = ssp->cmdbuf[7] = ssp->cmdbuf[9] = 0;
	ssp->cmdbuf[8] = 1;	/* transfer 1 block */
	ssp->cmdlen = G1CMDLEN;
	ssp->in_buf_len = BSIZE;

	ret = scsicmd(s_id);
	if (ret) {
printf("signature low:%x high:%x\n", ssp->in_buf[510], ssp->in_buf[511]);
	}

	return ret;
}

/*
 * BDR_CHECK_INTERVAL is the number of ticks to wait between checks for
 * SCSI Bus Free after sending Bus Device Reset.
 * BDR_CHECK_COUNT is the number of times to check for SCSI Bus Free
 * before giving up.
 */
#define BDR_CHECK_INTERVAL	10
#define BDR_CHECK_COUNT		100

/*
 * bus_dev_reset()
 *
 * Send Bus Device Reset message to the given SCSI id.
 */
static void	bus_dev_reset(s_id)
{
	int tries;
	int dev = ((sscon.c_mind << 8) | 0x80 | (s_id << 4));
printf("bus_dev_reset\n");
	for (tries = 0; tries < LOPRI_RETRIES; tries++) {
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

		sfbyte(ss_csr, WC_ENABLE_SCSI | WC_ATTENTION);

		if (!bus_wait(((RS_REQUEST|RS_CTRL_DATA|RS_I_O|RS_MESSAGE) << 8)
		| (RS_REQUEST|RS_CTRL_DATA|RS_MESSAGE)))
			continue;

		sfbyte(ss_csr, WC_ENABLE_SCSI);
		sfbyte(ss_dat, MSG_DEV_RESET);
		break;
	}
	for (tries = 0; tries < BDR_CHECK_COUNT; tries++) {
		if (ffbyte(ss_csr) == 0) {
			printf("bus device reset done\n");
			break;
		}
		ssdelay(BDR_CHECK_INTERVAL);
	}
}
