/*
 * This is the generic SCSI part of the
 * Adaptec AHA154x host adaptor driver for the AT.
 *
 * $Log$
 */

#include	<coherent.h>
#include 	<sys/fdisk.h>
#include	<sys/hdioctl.h>
#include	<sys/sdioctl.h>
#include	<sys/buf.h>
#include	<sys/con.h>
#include	<sys/stat.h>
#include	<sys/uproc.h>
#include	<sys/mmu.h>
#include	<errno.h>
#include	<scsiwork.h>

extern	saddr_t sds;

/*
 * Configurable parameters
 */
#define NDRIVE	(8 * 4)			/* 8 SCSI ids and 4 LUNs */
#define	SDMAJOR	13			/* Major Device Number */
#define	NHEAD	64			/* controller fakes this value */
#define	NSEC	32			/* likewise... */
#define	SDDMA	5			/* Used for first party DMA */

/*
 * user configurable paramters
 */
int	SDIRQ	= 11;			/* Interrupt */
int	SDBASE	= 0x0330;		/* Port base */

/*
 *					LUN --------++
 * device macros			Special-+   ||
 * minor device bits are of the form:		76543210
 *						 |||  ||
 *					SCSI ID--+++  ||
 *					Partition ----++
 * Partition mapping:
 *
 * Description	   Special Bit	   Partition #		Device		Type
 * -----------	   -----------	   -----------		------		----
 * partition a		0		00		/dev/sd??a	disk
 * partition b		0		01		/dev/sd??b	disk
 * partition c		0		10		/dev/sd??c	disk
 * partition d		0		11		/dev/sd??d	disk
 * partition table	1		00		/dev/sd??x	disk
 * no rewind tape	1		01		/dev/sd??n	tape
 * UNALLOCATED		1		10		  ---		????
 * rewind tape device	1		11		/dev/sd??	tape
 */
#define	DRIVENO(minor)	(((minor) >> 2) & 0x1F)	/* SCSI ID + LUN */
#define	SCSIID(minor)	(((minor) >> 4) & 0x7)	/* SCSI ID */
#define	LUN(minor)	(((minor) >> 2) & 0x3)	/* Logical Unit Number */
#define	PARTITION(minor) ((minor) & 0x3)	/* Partition */
#define	sdmkdev(maj, s, drv)	makedev((maj), ((s)|((drv)<<2)))

/*
 * Driver configuration.
 */
void	sdload();
void	sdunload();
void	sdopen();
void	sdclose();
void	sdread();
void	sdwrite();
int	sdioctl();
void	sdblock();
int	sdwatch();
int	nulldev();
int	nonedev();

CON	sdcon	= {
	DFBLK|DFCHR,			/* Flags */
	SDMAJOR,			/* Major index */
	sdopen,				/* Open */
	sdclose,			/* Close */
	sdblock,			/* Block */
	sdread,				/* Read */
	sdwrite,			/* Write */
	sdioctl,			/* Ioctl */
	nulldev,			/* Powerfail */
	sdwatch,			/* Timeout */
	sdload,				/* Load */
	sdunload			/* Unload */
};

/*
 *	hostadaptor routines
 */
int	aha_load();		/* initialize hostadaptor, DMA */
void	aha_unload();		/* shutdown the host adaptor */
int	aha_start();		/* see if there's work */
int	aha_command();

/*
 * Partition Parameters - copied from disk.
 *
 *	There are NPARTN positions for the user partitions in array PPARM,
 *	plus 1 additional position to span the entire drive.
 *	Array pparmp[] contains a pointer to a kalloc()'ed PPARM
 *	entry if the drive actually exists, is a disk drive and if someone
 *	has attmpted to read a partition table from the drive.
 */
typedef	struct	fdisk_s	PPARM[NPARTN + 1];	/* 4 partitions + whole drive */
static	PPARM *pparmp[NDRIVE];			/* one per possible drive */
#define	WHOLE_DRIVE	NPARTN			/* index for whole drive */
#define	PNULL	((PPARM *)0)

/*
 * Per disk controller data.
 * Only one host adapter; no more, no less.
 */
static
scsi_work_t	sd;

static	BUF	dbuf;			/* For raw I/O */
static	int	sw_active;

/**
 *
 * void
 * sdload()	- load routine.
 *
 *	Action:	The controller is reset and the interrupt vector is grabbed.
 *		The drive characteristics are set up at this time.
 */
static void
sdload()
{
	/*
	 * Initialize Drive Controller.
	 */
	sw_active = 0;
	if (aha_load( SDDMA, SDIRQ, SDBASE, &sd ) < 0) {
		u.u_error = ENXIO;
		return;
	}
/*	aha_device_info(); */		/* enable after this gets fixed */
}

/**
 *
 * void
 * sdunload()	- unload routine.
 */
static void
sdunload()
{
	register int i;

	if( sw_active > 0 )
		printf( "aha154x: sdunload() athough %d active\n", sw_active );
	aha_unload( SDIRQ );
	for (i = 0; i < NDRIVE; ++i)
		if (pparmp[i] != PNULL)
			kfree(pparmp[i]);	/* free any partition tables */
}

/*
 * int
 * sdgetpartitions(dev)	- load partition table for specified drive
 *
 *			- return 1 on success and 0 on failure
 */
int sdgetpartitions(dev)
dev_t	dev;
{
	register int 	i;
	scsi_cmd_t	sc;
	unsigned char	*buffer;
	struct fdisk_s	*fdp;
	int	d = DRIVENO(minor(dev));

	pparmp[d] = kalloc(sizeof *pparmp[0]);
	fdp = (struct fdisk_s *) pparmp[d];	/* point to first entry */
	buffer = kalloc(36+1);
	if (buffer == NULL || pparmp[d] == PNULL) {
		printf("aha154x: out of kernel memory\n");
		u.u_error = EKSPACE;
		return 0;
	}
	kclear(pparmp[d], sizeof *pparmp[0]);
	sc.unit = d;
	sc.block = 0L;
	sc.blklen = 0;
	sc.buffer = vtop( buffer, sds );
	++drvl[SDMAJOR].d_time;	
#if	0
	sc.cmd = ScmdINQUIRY;
	sc.buflen = 36;
	aha_command( &sc );
	aha_command( &sc );
	buffer[36] = 0;
	printf( "SCSI Disk %s", &buffer[8] );
#endif
	sc.cmd = ScmdREADCAPACITY;
	sc.buflen = 8;

	for( i = 0; i < sc.buflen; ++i )
		buffer[i] = 0;
	aha_command( &sc );
	aha_command( &sc );
#if	VERBOSE
	printf( "buffer =" );
	for( i = 0; i < sc.buflen; ++i )
		printf( " %x", buffer[i] );
	printf( "\n" );
#endif
	sc.block = (buffer[0]<<8) | buffer[1];
	sc.block <<= 16;
	sc.block |= (buffer[2]<<8) | buffer[3];

	sc.blklen = (buffer[6]<<8) | buffer[7];
#if	VERBOSE
	printf( "SCSI %D. blocks of size %d\n", sc.block, sc.blklen );
#endif
	kfree(buffer);
	fdp[WHOLE_DRIVE].p_size = sc.block;
	--drvl[SDMAJOR].d_time;	
	return fdisk( sdmkdev(major(dev), SDEV, d), pparmp[d]);
}

/**
 *
 * void
 * sdopen( dev, mode )
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
sdopen( dev, mode )
register dev_t	dev;
{
	register int p;			/* partition */
	register int d;			/* drive (SCSI ID + LUN) */
	struct fdisk_s	*fdp;		/* one partition entry */

	if ( minor(dev) & SDEV ) {
		if ( PARTITION(minor(dev)) != 0 )	/* tape device ? */
			u.u_error = ENXIO;		/* not yet! */
		return;
	}

	d = DRIVENO(minor(dev));
	p = PARTITION(minor(dev));

	/*
	 * If partition not defined read partition characteristics.
	 */
	if ( pparmp[d] == PNULL )   /* no entry yet for this drive ? */
		if (!sdgetpartitions(dev)) {
			u.u_error = ENXIO;
			return;
		}
	/*
	 * Ensure partition lies within drive boundaries and is non-zero size.
	 */
	fdp = (struct fdisk_s *) pparmp[d];
	if ((fdp[p].p_base+fdp[p].p_size) > fdp[WHOLE_DRIVE].p_size)
		u.u_error = EBADFMT;
	else if ( fdp[p].p_size == 0 )
		u.u_error = ENODEV;
	else {
		++drvl[SDMAJOR].d_time;	
		++sw_active;
	}
}

void sdclose( dev )
{
	--drvl[SDMAJOR].d_time;	
	--sw_active;	
}

/**
 *
 * void
 * sdread( dev, iop )	- write a block to the raw disk
 * dev_t dev;
 * IO * iop;
 *
 *	Input:	dev = disk device to be written to.
 *		iop = pointer to source I/O structure.
 *
 *	Action:	Invoke the common raw I/O processing code.
 */
static void
sdread( dev, iop )
dev_t	dev;
IO	*iop;
{
	ioreq( &dbuf, iop, dev, BREAD, BFRAW|BFBLK|BFIOC );
}

/**
 *
 * void
 * sdwrite( dev, iop )	- write a block to the raw disk
 * dev_t dev;
 * IO * iop;
 *
 *	Input:	dev = disk device to be written to.
 *		iop = pointer to source I/O structure.
 *
 *	Action:	Invoke the common raw I/O processing code.
 */
static void
sdwrite( dev, iop )
dev_t	dev;
IO	*iop;
{
	ioreq( &dbuf, iop, dev, BWRITE, BFRAW|BFBLK|BFIOC );
}

/**
 *
 * int
 * sdioctl( dev, cmd, arg )
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
sdioctl( dev, cmd, vec )
register dev_t	dev;
int cmd;
char * vec;
{
	int d;
	hdparm_t hdparm;
	struct fdisk_s	*fdp;

	d = DRIVENO(minor(dev));

	/*
	 * Identify input/output request.
	 */
	switch ( cmd ) {

	case HDGETA:
		if (pparmp[d] == PNULL)
			if (!sdgetpartitions(dev)) {
				u.u_error = ENXIO;
				return -1;
			}
		fdp = (struct fdisk_s *) pparmp[d];
		*(short *)&hdparm.landc[0] =
		*(short *)&hdparm.ncyl[0] = fdp[WHOLE_DRIVE].p_size
						/ (NHEAD * NSEC);
		hdparm.nhead = NHEAD;
		hdparm.nspt = NSEC;
		kucopy( &hdparm, vec, sizeof hdparm );
		return 0;
	case SCSI_HA_CMD:
		return aha_ioctl( cmd, vec );
	case SCSI_CMD:
		return 0;
	case SCSI_CMD_IN:
		return 0;
	case SCSI_CMD_OUT:
		return 0;

	default:
		u.u_error = EINVAL;
		return -1;
	}
}

/**
 *
 * void
 * sdblock( bp )	- queue a block to the disk
 *
 *	Input:	bp = pointer to block to be queued.
 *
 *	Action:	Queue a block to the disk.
 *		Make sure that the transfer is within the disk partition.
 */
static void
sdblock(bp)
register BUF	*bp;
{
	register scsi_work_t *sw;
	register int s;
	struct	fdisk_s	*fdp;

	int p = PARTITION(minor(bp->b_dev));
	int drv = DRIVENO(minor(bp->b_dev));

	if ( minor(bp->b_dev) & SDEV )
		p = WHOLE_DRIVE;
	bp->b_resid = bp->b_count;
	
	fdp = (struct fdisk_s *) pparmp[drv];

	/*
	 * Range check disk region.
	 */
	if ( pparmp[drv] == PNULL ) {
		if ( p == WHOLE_DRIVE ) {
			if ((bp->b_bno != 0) || (bp->b_count != BSIZE)) {
				bp->b_flag |= BFERR;
				bdone(bp);
				return;
			}
		} else {
			printf("aha154x: no partition table\n");
			bp->b_flag |= BFERR;
			bdone(bp);
			return;
		}
	} else if ( (bp->b_bno + (bp->b_count/BSIZE)) > fdp[p].p_size ) {
		bp->b_flag |= BFERR;
		bdone(bp);
		return;
	}

	bp->b_actf = NULL;
	sw = (scsi_work_t *)kalloc( sizeof(*sw) );
	if (sw == (scsi_work_t *)0) {
		printf("aha154x: out of kernel memory\n");
		bp->b_flag |= BFERR;
		bdone(bp);
		return;
	}
	sw->sw_bp = bp;
	sw->sw_drv = drv;
	sw->sw_type = 0;
	if ( p != WHOLE_DRIVE )
		sw->sw_bno   = fdp[p].p_base + bp->b_bno;
	else
		sw->sw_bno   = bp->b_bno;
	sw->sw_retry = 1;

#if	VERBOSE
	printf( "sdblock: drv %x bno %x:%x  bp=%x, flag = %o\n",
		drv, (long)sw->sw_bno, bp, bp->b_flag );
#endif

	s = sphi();
	if (sd.sw_actf == NULL)
		sd.sw_actf = sw;
	else
		sd.sw_actl->sw_actf = sw;
	sd.sw_actl = sw;
	++drvl[SDMAJOR].d_time;	
	spl(s);

	aha_start();
}

sdwatch()
{
	register i;

	if( i = aha_start() )
#if	VERBOSE
		printf( "sdwatch: started %d actions\n", i );
#else
		;
#endif
	if( i = aha_completed() )
#if	VERBOSE
		printf( "sdwatch: completed %d actions\n", i );
#else
		;
#endif
}
