/*
 * This is a driver for Seagate ST01/ST02 scsi hard disk controllers.
 *
 * $Log:	/usr/src/sys/i8086/drv/RCS/ss.c,v $
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
#define BUS_RETRIES	1000

#define G0CMDLEN	6	/* Group 0 commands are 6 bytes long  */
#define G1CMDLEN	10	/* Group 1 commands are 10 bytes long */
#define SENSELEN	22	/* number of bytes returned w/ req sense */

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
int stats[40], statsptr;
#define PUSHI		{ stats[statsptr++] = i; }
#define POPI		{ printf("%d:",statsptr);while(statsptr)\
				printf("%d ",stats[--statsptr]);printf("\n");}
#define SSTELL(foo)	printf(foo)
#define SSTATUS		printf("status=%x\n", (int)(unsigned char)status)
#define SSDUMP(ssp, text) {int i;\
	printf("%s: msg_in=%x cmdstat=%x\n", text, ssp->msg_in,\
	ssp->cmdstat);if(ssp->cmdlen)for(i=0;i<ssp->cmdlen;i++)\
	printf(" %x", ssp->cmdbuf[i]);printf(" cmd_bytes_out=%d\n",\
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
static TIM	delay_tim;	/* needed for calls to ssdelay() */

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

/*
 * A per-drive structure - ss
 */
#define IN_BUF_SIZE	22
typedef unsigned char	uchar;

static struct ss	{
	long	capacity;
	int	msg_in;
	uchar	cmdbuf[G1CMDLEN];
	int	cmdlen;
	int	cmd_bytes_out;
	int	cmdstat;
	uchar	in_buf[IN_BUF_SIZE];
	int	in_buf_len;
	int	data_bytes_in;
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
#if 0
	if ( minor(dev) & SDEV ) {
		d = minor(dev) % NDRIVE;
		p += NDRIVE * NPARTN;
	}
	else
		d = minor(dev) / NPARTN;

	if ( (d >= NDRIVE) || (at.at_dtype[d] == 0) ) {
		return;
	}

	if ( minor(dev) & SDEV )
		return;

	/*
	 * If partition not defined read partition characteristics.
	 */
	if ( pparm[p].p_size == 0 )
		fdisk( makedev( major(dev), SDEV + d), &pparm[ d * NPARTN ] );

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
}

/*
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
	for ( i = 0; i < BUS_RETRIES; i++) {
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
#if DEBUG
devmsg(dev, "ssinit invoked");
#endif
	if (!testready(s_id))
		devmsg(dev, "Test Unit Ready failed");
	else {
		devmsg(dev, "Unit successfully initialized");
		retval = 1;
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
#define XXXX
	if (1 || !(retval = scsicmd(s_id))) {
printf("First Test Unit Ready failed.  Will reset SCSI bus.\n");
		scsireset();
		retval = scsicmd(s_id);
	}
	return retval;
}

/*
 * scsicmd()
 *
 * Send command packet to target device.
 * Start a new SCSI bus cycle when this routine is called.
 *
 * Return 1 if command succeeds, else 0.
 */
static int scsicmd(s_id)
int s_id;
{
	int retval;
	struct ss *ssp = ss[s_id];
SSTELL("enter scsicmd\n");
	if (retval = bus_pre_xfer(s_id)) {
		bus_info_xfer(ssp);
		retval = (ssp->cmdlen == ssp->cmd_bytes_out
			&& ssp->cmdstat == CS_GOOD);
	}
SSDUMP(ssp, "command sent");
	if (ssp->cmdstat == CS_CHECK) {
		if (bus_pre_xfer(s_id)) {
			rqs.cmdbuf[0] = ScmdREQUESTSENSE;
			rqs.cmdbuf[1] = rqs.cmdbuf[2] = rqs.cmdbuf[3] =
				rqs.cmdbuf[5] = 0;
				rqs.cmdbuf[4] = SENSELEN;
			rqs.cmdlen = G0CMDLEN;
			rqs.in_buf_len = SENSELEN;
			bus_info_xfer(&rqs);
			if (rqs.data_bytes_in == SENSELEN
			&& (rqs.in_buf[2] & 0x0F) == 0x06
			&& rqs.in_buf[12] == 0x29)
				retval = (ssp->cmdlen == ssp->cmd_bytes_out);
SSDUMP((&rqs), "sense req");
		}
	}
	return retval;
}

/*
 * scsireset()
 *
 * Assert reset for 1 clock tick.
 */
static void scsireset()
{
	sfbyte(ss_csr, WC_ENABLE_SCSI | WC_SCSI_RESET);
	ssdelay(50);
	sfbyte(ss_csr, 0);
	ssdelay(50);
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
 * bus_pre_xfer()
 *
 * Do bus cycle phases prior to the information transfer phases.
 * This includes arbitration and selection.
 */
static int bus_pre_xfer(s_id)
int s_id;
{
	/*
	 * Do ST0x arbitration.
	 */
	sfbyte(ss_csr, 0);		/* De-assert SCSI enable bit */
	sfbyte(ss_dat, HOST_ID);	/* Write my SCSI id to port */
	sfbyte(ss_csr, WC_ARBITRATE);	/* Start arbitration */

	if (!bus_wait(RS_ARBIT_COMPL << 8 | RS_ARBIT_COMPL))
		return 0;

	/*
	 * Arbitration complete.  Now select, with ATN to allow messages.
	 */
	sfbyte(ss_dat, HOST_ID | (1 << s_id));	/* Write both SCSI id's */
	sfbyte(ss_csr, WC_ENABLE_SCSI | WC_ATTENTION | WC_SELECT);

	if (!bus_wait(RS_BUSY << 8 | RS_BUSY))
		return 0;

	/*
	 * Send "Identify" Message with Disconnect allowed.
	 */
	sfbyte(ss_csr, WC_ENABLE_SCSI | WC_ATTENTION);

	if (!bus_wait(((RS_REQUEST|RS_CTRL_DATA|RS_I_O|RS_MESSAGE) << 8)
	| (RS_REQUEST|RS_CTRL_DATA|RS_MESSAGE)))
		return 0;

	sfbyte(ss_csr, WC_ENABLE_SCSI);
	sfbyte(ss_dat, MSG_IDENT_DC);

	return 1;
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
	unsigned char phase_type;
	int no_msg_rcvd = 1;

	ssp->cmdstat = -1;
	ssp->data_bytes_in = 0;
	ssp->cmd_bytes_out = 0;
	while(req_wait(&bus_timeout)) {
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
			if (ssp->cmd_bytes_out < ssp->cmdlen)
				sfbyte(ss_dat, ssp->cmdbuf[ssp->cmd_bytes_out++]);
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
	for ( i = 0; i < BUS_RETRIES; i++) {
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

#define STUFF 0
/* pieces of code temporarily without a home */
#if STUFF
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
bus_wait(RS_BUSY << 8 | 0);
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

for (i = 0; i < CMDLEN; i++) cmd[i] = 0; /* send Request Sense command */
cmd[0] = 3;  cmd[4] = SENSELEN;

		for (i = 0; i < CMDLEN; i++) {
			bus_wait(((RS_REQUEST|RS_CTRL_DATA|RS_I_O|RS_MESSAGE) << 8) |
				(RS_REQUEST|RS_CTRL_DATA));
			sfbyte(ss_dat, cmd[i]);
		}

		for (inbytes = 0; inbytes < SENSELEN;  inbytes++) {
			if (bus_wait(
			((RS_REQUEST|RS_CTRL_DATA|RS_I_O|RS_MESSAGE) << 8) |
			(RS_I_O)))
				sense[inbytes] = ffbyte(ss_dat);
			else
				break;
		}
		bus_wait(((RS_REQUEST|RS_CTRL_DATA|RS_I_O|RS_MESSAGE) << 8) |
			(RS_REQUEST|RS_CTRL_DATA|RS_I_O));
		data1 = ffbyte(ss_dat);
		bus_wait(((RS_REQUEST|RS_CTRL_DATA|RS_I_O|RS_MESSAGE) << 8) |
			(RS_REQUEST|RS_CTRL_DATA|RS_I_O|RS_MESSAGE));
		data2 = ffbyte(ss_dat);
bus_wait(RS_BUSY << 8 | 0);
status = ffbyte(ss_csr);
SSTATUS;
printf("data1=%x data2=%x\n",data1,data2);
printf("%d:", inbytes);
for (i = 0; i < inbytes; i++) printf(" %x",sense[i]);
printf("\n");
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
#endif
