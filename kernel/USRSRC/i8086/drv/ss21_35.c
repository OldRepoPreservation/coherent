/*
 * This is a driver for Seagate ST01/ST02 scsi host adapters.
 *
 * To do:
 *	figure out a better storage class for rqs
 *      make input buffer for commands dynamic (?)
 *
 * $Log:	/usr/src/sys/i8086/drv/RCS/ss.c,v $
 * Revision 1.28	91/04/16  20:48:47	root
 * Kernel fdisk works, fdisk command gets garbage.
 * 
 * Revision 1.27	91/04/16  16:38:47	root
 * Typos fixed.  Locks up CPU on first open
 * 
 * Revision 1.26	91/04/16  16:13:10	root
 * First clean compile with block routine.
 * 
 * Revision 1.25	91/04/16  01:45:35	root
 * lots of strategy code added - but not ready to compile
 * 
 * Revision 1.24	91/04/12  17:19:25	root
 * Some rearrangements - still working on block & related routines
 * 
 * Revision 1.23	91/04/11  12:51:50	root
 * ssopen compiles - not tested
 * 
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
 * Definitions.
 *	Constants.
 *	Macros with argument lists.
 *	Typedefs.
 *	Enums.
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
#define WATCHDOG_SECONDS  4

#define IN_BUF_SIZE	512	/* buffer size in "ss" structs */

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
	printf(" %x", ffbyte(ssp->in_buf+i));printf(" data_bytes_in=%d\n",\
	ssp->data_bytes_in);}
#else
#define PUSHI
#define POPI
#define SSTELL(foo)
#define SSTATUS
#define SSDUMP(ssp, text)
#endif

typedef unsigned char	uchar;
typedef unsigned long	ulong;
typedef struct ss {
	ulong	capacity;
	ulong	blocklen;
	ulong	bno;
	int	msg_in;
	int	dr_watch;	/* number of seconds for pending timeout */
	uchar	cmdbuf[G1CMDLEN];
	int	cmdlen;
	int	cmd_bytes_out;
	int	cmdstat;
	faddr_t	in_buf;
	int	in_buf_len;
	int	data_bytes_in;
	faddr_t	out_buf;
	int	out_buf_len;
	int	data_bytes_out;
	BUF	*bp;		/* current I/O request node, or NULL */
	struct	fdisk_s parmp[NPARTN+1];
	unsigned int	ptab_read:1;  /* 1 if partition table has been read */
	unsigned int	id_busy:1;  /* 1 if device with this SCSI id busy */
}	ss_type;

/*
 * Functions.
 *	Import Functions.
 *	Export Functions.
 *	Local Functions.
 */
extern int	nulldev();
extern int	nonedev();
extern unsigned char ffbyte();

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
static int	chk_reconn();
static int	inquiry();
static int	read_cap();
static void	reconnect();
static int	req_sense();
static int	scsicmd();
static void	scsireset();
static void	ss_done();
static int	ss_rw();
static void	ss_start();
static void	ss_start_timing();
static void	ss_stop_timing();
static void	ssdelay();
static int	ssinit();
static void	ssintr();

/*
 * Global Data.
 *	Import Variables.
 *	Export Variables.
 *	Local Variables.
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

	/* Patch these Export Variables to configure the driver. */
int	NSDRIVE = 1;		/* Bitmap of attached SCSI drives. */
int	SS_INT = 5;		/* ST0[12] use either IRQ3 or IRQ5 */
int	SS_BASE = 0xDE00;	/* Segment addr of ST0x communication area */

static BUF	dbuf;		/* For raw I/O */
static paddr_t	ss_base;	/* physical address of ST0x comm area */
static faddr_t	ss_fp;		/* (far *) to ST0x comm area */

static faddr_t	ss_ram;		/* (far *) to parameter RAM */
static faddr_t	ss_csr;		/* (far *) to control/status */
static faddr_t	ss_dat;		/* (far *) to data port */

static int	num_drives;	/* number of controller SCSI id's */
static ss_type *ss_tbl;		/* points to block of "ss" structs */
static int	st0x_busy;	/* 1 if SCSI host adapter busy */

static TIM	delay_tim;	/* needed for calls to ssdelay() */
static TIM	timeout_tim;	/* needed for calls to timeout() */
static int	ss_expired;	/* 1 after local timeout */
static ss_type  *ss[MAX_SCSI_ID-1], rqs;
static int	dr_watch[MAX_SCSI_ID-1];

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
		} else if ((ss_tbl = kalloc(num_drives*sizeof(ss_type)))
		== NULL) {
			printf("Error - ss can't allocate structs\n");
			erf = 1;
		} else
			kclear(ss_tbl, num_drives * sizeof(ss_type));
	}
	if (!erf) {
		ss_type *foo = ss_tbl;

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
	if (ss_tbl)
		kfree(ss_tbl);

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
	ss_type * ssp;
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
devmsg(dev, "ssopen");
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
printf("adj partn=%d\n", partn);
	/*
	 * If not accessing whole drive and the partition table has not
	 * been read yet, try to read it now.
	 * Do this by calling fdisk() with partition table device on the drive
	 * that is being accessed.
	 */
	if (valid_open && partn != WHOLE_DRIVE && !(ssp->ptab_read)) {
		int fdisk_dev;

		fdisk_dev = (dev | SDEV) & 0xfff0;
devmsg(fdisk_dev, "calling fdisk");
		if (fdisk(fdisk_dev, fdp)) {
int p;
			fdp[WHOLE_DRIVE].p_size = ssp->capacity;
			fdp[WHOLE_DRIVE].p_base = 0;
printf("fdisk() succeeded\n");
for (p=0; p<=WHOLE_DRIVE; p++)
	printf("p=%d base=%ld size=%ld\n", p, fdp[p].p_base, fdp[p].p_size);
			ssp->ptab_read = 1;
		} else {
printf("fdisk() failed\n");
			u.u_error = ENXIO;
			valid_open = 0;
		}
	}

	/*
	 * Ensure partition lies within drive boundaries and is non-zero size.
	 */
	if (valid_open && partn != WHOLE_DRIVE
	&& (fdp[partn].p_base+fdp[partn].p_size) > fdp[WHOLE_DRIVE].p_size) {
		u.u_error = EBADFMT;
		valid_open = 0;
printf("BARF\n");
	}

	if (valid_open && partn != WHOLE_DRIVE && fdp[partn].p_size == 0) {
		u.u_error = ENODEV;
		valid_open = 0;
	}

	/*
	 * OK to open the device.
	 * Start watchdog timer (if not already started) for the host adapter.
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
	/*
	 * Decrement the number of watchdog timer requests open for host board.
	 */
	--drvl[SCSI_MAJOR].d_time;	
devmsg(dev, "ssclose");
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
#define NHEAD	7
#define NSEC	28
#define NCYL	1066

static int ssioctl(dev, cmd, vec)
register dev_t	dev;
int cmd;
char * vec;
{
	int ret = 0;
	hdparm_t hdparm;
	struct	fdisk_s	*fdp;
	int s_id;
	ss_type * ssp;

	s_id = DEV_SCSI_ID(dev);
	ssp = ss[s_id];
	fdp = ssp->parmp;

	switch(cmd) {
	case HDGETA:
printf("HDGETA\n");
		fdp = ssp->parmp;
		*(short *)&hdparm.landc[0] =
		*(short *)&hdparm.ncyl[0] = NCYL;
		hdparm.nhead = NHEAD;
		hdparm.nspt = NSEC;
printf("ncyl=%d nhead=%d nspt=%d\n",
  hdparm.ncyl[0] + hdparm.ncyl[1]<<8, (int)hdparm.nhead, (int)hdparm.nspt);
		kucopy( &hdparm, vec, sizeof hdparm );
		ret = 0;
		break;
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
	register int s;
	struct	fdisk_s	*fdp;
	int partition, drive, s_id;
	dev_t dev;
	ss_type * ssp;
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
printf("BFERR 1\n");
				bp->b_flag |= BFERR;
				valid_op = 0;
			}
		} else {
printf("BFERR 2\n");
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
printf("BFERR 3\n");
		bp->b_flag |= BFERR;
		valid_op = 0;
	}

	/*
	 * Operation appears valid.
	 * Fill fields in the node and queue the request.
	 */
	if (valid_op) {

printf("ssblock: drv=%x bno=%lx bp=%x flag=%x\n",
	drive, bp->b_bno, bp, bp->b_flag);

		ssq_wr_tail(bp);
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
 *
 * If we have been reselected by a recognized target device
 *	let kernel get out of interrupt mode (defer) and do SCSI
 *	reconnect stuff.
 */
static void ssintr()
{
	int s_id;

printf("@");
	s_id = chk_reconn();
	if (s_id != -1)
		defer(reconnect, s_id);
}

/*
 * sswatch()
 *
 * Invoked once per second if any devices going through this driver are open.
 * Poll for any reselect, in case interrupt got lost.
 */
static void sswatch()
{
	int s_id;
	ss_type * ssp;

printf("*");
	for (s_id = 0; s_id < MAX_SCSI_ID-1; s_id++) {
		ssp = ss[s_id];
		if (ssp && ssp->dr_watch) {
			ssp->dr_watch--;
			if (ssp->dr_watch == 0) {
printf("BFERR 4\n");
				bus_dev_reset(s_id);
				ssp->bp->b_flag |= BFERR;
				ss_done(s_id);
printf("SCSI id #%d: bno=%lu <Watchdog Timeout>\n", s_id, ss[s_id]->bp->b_bno);
			} else {
				while (1) {
					s_id = chk_reconn();
					if (s_id == -1)
						break;
					else
						reconnect(s_id);
				} /* endwhile */
			}
		}
	} /* endfor */
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
	uchar query_buf[INQUIRYLEN + 1];
	ss_type * ssp = ss[s_id];
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
		if (inquiry(s_id, query_buf)) {
			query_buf[INQUIRYLEN] = 0;
			devmsg(dev, query_buf + 8);
			if (query_buf[0] == 0) {
				retval = 1;
			} else
				devmsg(dev, "Not Direct Access Device");
		} else
			devmsg(dev, "Inquiry Failed");

	if (retval)
		if (read_cap(s_id, query_buf)) {
			retval = 1;
			ssp->capacity = query_buf[3] | (query_buf[2] << 8)
			| (((long)(query_buf[1])) << 16)
			| (((long)(query_buf[0])) << 24);
			ssp->blocklen = query_buf[7] | (query_buf[6] << 8)
			| (((long)(query_buf[5])) << 16)
			| (((long)(query_buf[4])) << 24);
printf("capacity=%ld   block length=%ld\n", ssp->capacity, ssp->blocklen);
		} else
			devmsg(dev, "Read Capacity Failed");

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
	ss_type * ssp = ss[s_id];

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
	ss_type *ssp = ss[s_id];

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
ss_type *ssp;
{
	int bus_timeout;
	uchar phase_type;
	uchar msg_in;
	int no_msg_rcvd = 1;
	int s;
	int bytes_to_send;

	ssp->cmdstat = -1;
	ssp->data_bytes_in = 0;
	ssp->data_bytes_out = 0;
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
			if (ssp->data_bytes_in < ssp->in_buf_len && ssp->in_buf) {
				uchar dat;

				dat = ffbyte(ss_dat);
				sfbyte(ssp->in_buf + ssp->data_bytes_in, dat);
				ssp->data_bytes_in++;
			} else
				ffbyte(ss_dat);
			break;
		case XP_DATA_OUT:
			/*
			 * Copy output buffer bytes to data register.
			 */
			if (ssp->data_bytes_out < ssp->out_buf_len && ssp->out_buf) {
				uchar dat;

				dat = ffbyte(ssp->out_buf + ssp->data_bytes_out);
				sfbyte(ss_dat, dat);
				ssp->data_bytes_out++;
			} else { /* This case should not happen. */
SSDUMP(ssp, "Data out overrun");
				scsireset();
			}
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
	uchar sense_buf[SENSELEN];
	int ret = 0;

	rqs.cmdbuf[0] = ScmdREQUESTSENSE;
	rqs.cmdbuf[1] = rqs.cmdbuf[2] = rqs.cmdbuf[3] =
		rqs.cmdbuf[5] = 0;
		rqs.cmdbuf[4] = SENSELEN;
	rqs.cmdlen = G0CMDLEN;
	rqs.in_buf_len = SENSELEN;
	FP_OFF(rqs.in_buf) = sense_buf;
	FP_SEL(rqs.in_buf) = sds;

	if (bus_pre_xfer(s_id)) {
		bus_info_xfer(&rqs);
		if (rqs.data_bytes_in == SENSELEN) {
			if (sense_buf[2] == 0x00)	/* No Sense.  AOK */
				ret = 1;
			else if (sense_buf[2] == 0x06 && sense_buf[12] == 0x29)
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
 * Put result of inquiry into supplied buffer.
 * Return 1 if command succeeds, else 0.
 */
static int inquiry(s_id, buf)
int s_id;
uchar * buf;
{
	int ret = 0;
	ss_type * ssp = ss[s_id];

	ssp->id_busy = 1;
	ssp->cmdbuf[0] = ScmdINQUIRY;
	ssp->cmdbuf[1] = ssp->cmdbuf[2] = ssp->cmdbuf[3] =
		ssp->cmdbuf[5] = 0;
		ssp->cmdbuf[4] = INQUIRYLEN;
	ssp->cmdlen = G0CMDLEN;
	FP_OFF(ssp->in_buf) = buf;
	FP_SEL(ssp->in_buf) = sds;
	ssp->in_buf_len = INQUIRYLEN;

	ret = scsicmd(s_id);
	ssp->id_busy = 0;

	return ret;
}

/*
 * read_cap()
 *
 * Read Capacity command for a device.
 *
 * Return 1 if command succeeds, else 0.
 */
static int read_cap(s_id, buf)
int s_id;
uchar * buf;
{
	int ret = 0;
	ss_type * ssp = ss[s_id];

	ssp->id_busy = 1;
	ssp->cmdbuf[0] = ScmdREADCAPACITY;
	ssp->cmdbuf[1] = ssp->cmdbuf[2] = ssp->cmdbuf[3] = ssp->cmdbuf[4] = 0;
	ssp->cmdbuf[5] = ssp->cmdbuf[6] = ssp->cmdbuf[7] = ssp->cmdbuf[8] = 0;
	ssp->cmdbuf[9] = 0;
	ssp->cmdlen = G1CMDLEN;
	FP_OFF(ssp->in_buf) = buf;
	FP_SEL(ssp->in_buf) = sds;
	ssp->in_buf_len = 8;

	ret = scsicmd(s_id);
	ssp->id_busy = 0;

	return ret;
}

/*
 * ss_start()
 *
 * Invoked whenever there might be I/O to do.
 *
 * Disallow re-entrancy in this routine (variable "locked").
 * If there is a next I/O request queued (peek at head of queue)
 *   get the target SCSI ID.
 *   If target is not busy
 *     remove request from queue
 *     mark target device busy
 *     start watchdog timer
 *     send command to host adapter
 *     if command succeeded
 *       cleanup after command
 *       adjust b_resid field
 *     else if command failed
 *       set error flag
 *       cleanup after command
 *     else (disconnected)
 *       do nothing
 */
static void ss_start()
{
	int s;
	BUF * bp;
	static char locked;
	int s_id;
	ss_type * ssp;
	struct	fdisk_s	*fdp;
	int partition;
	dev_t dev;

	s = sphi();
	if(locked) {
		spl(s);
		return;
	}
	++locked;
	spl(s);

	if((bp = ssq_rd_head()) != NULL) {
		s_id = DEV_SCSI_ID(bp->b_dev);
		ssp = ss[s_id];
		ssp->bp = bp;
		dev = bp->b_dev;
		partition = DEV_PARTN(dev);
		if (dev & SDEV)
			partition = WHOLE_DRIVE;
		fdp = ssp->parmp;
		if (partition != WHOLE_DRIVE)
			ssp->bno = fdp[partition].p_base + bp->b_bno;
		else
			ssp->bno = bp->b_bno;
		if (!(ssp->id_busy)) {
			ssq_rm_head();
			ssp->id_busy = 1;
			ssp->dr_watch = WATCHDOG_SECONDS;
			if (ss_rw(s_id)) {
				if (bp->b_req == BREAD)
					bp->b_resid -= ssp->data_bytes_in;
				else
					bp->b_resid -= ssp->data_bytes_out;
				if (ssp->msg_in != MSG_DISCONNECT)
					ss_done(s_id);
			} else {
printf("BFERR 5\n");
				bp->b_flag |= BFERR;
				ss_done(s_id);
			}
		}
	}
	--locked;
}

/*
 * ss_done
 *
 * Release current i/o buffer to the O/S.
 */
static void ss_done(s_id)
int s_id;
{
	ss_type * ssp = ss[s_id];
	BUF * bp = ssp->bp;

	ssp->id_busy = 0;
	ssp->dr_watch = 0;
	ssp->in_buf = ssp->out_buf = NULL;
	if (bp) {
if (bp->b_flag & BFERR)
  printf("BFERR\n");
		bdone(bp);
		ssp->bp = NULL;
	}
	ss_start();
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

/*
 * chk_reconn()
 *
 * Check SELECT to see if any SCSI device has tried to reconnect to the host
 * adapter.  Called if there is an interrupt, and by the timer in case
 * we somehow lose an interrupt.
 *
 * Return -1 if no reselect detected, or the SCSI ID of the reselecting
 * target if there is one.
 *
 * Call reconnect() after this if reselect has occurred.
 */
static int chk_reconn()
{
	uchar dat;
	int s_id = -1;

	if (ffbyte(ss_csr) && RS_SELECT) {
		dat = ffbyte(ss_dat);
		if ((dat & HOST_ID) && (dat & NSDRIVE)) {
			dat &= ~HOST_ID;
			s_id = 0;
			while (dat >>=1)
				s_id++;
printf("R%d", s_id);
		}
	}

	return s_id;
}

/*
 * reconnect()
 *
 * Given SCSI ID of target device that is issuing reselect, do reconnect
 * SCSI bus stuff.
 */
static void reconnect(s_id)
int s_id;
{
	uchar dat;
	int cmd_ok = 0;
	ss_type * ssp = ss[s_id];
	BUF * bp = ssp->bp;

	dat = ffbyte(ss_dat);
	if ((dat & HOST_ID) && (dat & (1 << s_id))) {
		sfbyte(ss_csr, WC_ENABLE_SCSI | WC_BUSY);
		if (bus_wait(RS_SELECT << 8 | 0)) {
			sfbyte(ss_csr, WC_ENABLE_SCSI);
			cmd_ok = bus_info_xfer(ssp);
			if (bp->b_req == BREAD)
				bp->b_resid -= ssp->data_bytes_in;
			else
				bp->b_resid -= ssp->data_bytes_out;
			if (cmd_ok && ssp->cmdstat == CS_GOOD) {
				if (ssp->msg_in == MSG_DISCONNECT)
					ssp->dr_watch = WATCHDOG_SECONDS;
				else
					ss_done(s_id);
			} else {
printf("BFERR 6\n");
				bp->b_flag |= BFERR;
				ss_done(s_id);
			}
		}
	}
}

/*
 * ss_rw()
 *
 * Send read or write command to the host adapter.
 */
static int ss_rw(s_id)
int s_id;
{
	ss_type * ssp = ss[s_id];
	BUF * bp = ssp->bp;
	int retval;
printf("ss_rw(%d)\n", s_id);
	if (bp->b_req == BREAD) {
		ssp->cmdbuf[0] = ScmdREADEXTENDED;
		ssp->in_buf_len = bp->b_count;
		ssp->in_buf = bp->b_faddr;
	} else {
		ssp->cmdbuf[0] = ScmdWRITEXTENDED;
		ssp->out_buf_len = bp->b_count;
		ssp->out_buf = bp->b_faddr;
	}
	ssp->cmdbuf[1] = 0;
	ssp->cmdbuf[2] = ssp->bno >> 24;
	ssp->cmdbuf[3] = ssp->bno >> 16;
	ssp->cmdbuf[4] = ssp->bno >>  8;
	ssp->cmdbuf[5] = ssp->bno;
	ssp->cmdbuf[6] = 0;
	ssp->cmdbuf[7] = bp->b_count / (BSIZE * 256L);
	ssp->cmdbuf[8] = bp->b_count / BSIZE;
	ssp->cmdbuf[9] = 0;
	ssp->cmdlen = G1CMDLEN;
	if (retval = bus_pre_xfer(s_id)) {
printf("ss_rw(): bus_pre_xfer ok\n");
		bus_info_xfer(ssp);
printf("cmdlen=%d cmd_bytes_out=%d cmdstat=%d\n", ssp->cmdlen,
	ssp->cmd_bytes_out, ssp->cmdstat);
		retval = (ssp->cmdlen == ssp->cmd_bytes_out);
	} else
printf("ss_rw(): bus_pre_xfer not ok\n");

	if (ssp->cmdstat == CS_CHECK) {
printf("ss_rw(): requesting sense\n");
		if (req_sense(s_id))
			retval = (ssp->cmdlen == ssp->cmd_bytes_out);
	}

	retval = (retval &&
		(ssp->cmdstat == CS_GOOD || ssp->msg_in == MSG_DISCONNECT));
printf("ss_rw(): retval=%d\n", retval);
	return retval;
}
