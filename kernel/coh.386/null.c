/* $Header: /v4a/coh/RCS/null.c,v 1.2 92/01/06 11:59:49 hal Exp $ */
/* (lgl-
 *	The information contained herein is a trade secret of Mark Williams
 *	Company, and  is confidential information.  It is provided  under a
 *	license agreement,  and may be  copied or disclosed  only under the
 *	terms of  that agreement.  Any  reproduction or disclosure  of this
 *	material without the express written authorization of Mark Williams
 *	Company or persuant to the license agreement is unlawful.
 *
 *	COHERENT Version 2.3.37
 *	Copyright (c) 1982, 1983, 1984.
 *	An unpublished work by Mark Williams Company, Chicago.
 *	All rights reserved.
 -lgl) */
/*
 * Null and memory driver.
 *  Minor device 0 is /dev/null
 *  Minor device 1 is /dev/mem, physical memory
 *  Minor device 2 is /dev/kmem, kernel data
 *  Minor device 3 is /dev/cmos
 *  Minor device 4 is /dev/boot_gift
 *  Minor device 5 is /dev/clock
 *  Minor device 6 is /dev/proc
 *
 * $Log:	null.c,v $
 * Revision 1.2  92/01/06  11:59:49  hal
 * Compile with cc.mwc.
 * 
 * Revision 1.1	88/03/24  16:14:04	src
 * Initial revision
 * 
 */

/*
 * The symbol "DANGEROUS" should be undefined for a production system.
 */
#ifdef TRACER
#define NULL_IOCTL	/* Allow ioctl()s for /dev/kmem.  */
#define DANGEROUS	/* Allow dangerous ioctl()s for /dev/null.  */
#endif

#include <sys/coherent.h>
#include <sys/con.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/typed.h>
#include <sys/inode.h>
#ifdef NULL_IOCTL
#include <sys/null.h>
#endif /* NULL_IOCTL */

/* These are minor numbers.  */
#define DEV_NULL	0	/* /dev/null	*/
#define DEV_MEM		1	/* /dev/mem	*/
#define DEV_KMEM	2	/* /dev/kmem	*/
#define DEV_CMOS	3	/* /dev/cmos	*/
#define DEV_BOOTGIFT	4	/* /dev/bootgift  */
#define DEV_CLOCK	5	/* /dev/clock  */
#define DEV_PROC	6	/* /dev/proc  */

/*
 * CMOS devices are limited by an 8 bit address.
 */
#define MAX_CMOS	255
#define CMOS_LEN	256

/*
 * The first 14 bytes of the CMOS are the clock.
 */
#define MAX_CLOCK	13
#define CLOCK_LEN	14

/*
 * These are definitions for mucking with the CMOS clock.
 */
#define SRA	10	/* Status Register A */
#define SRB	11	/* Status Register B */
#define SRC	12	/* Status Register C */
#define SRD	13	/* Status Register D */

#define UIP	0x80	/* Update In Progress bit of SRA.	*/
#define NO_UPD	0x80	/* No Update bit of SRB.		*/

/*
 * Functions for configuration.
 */
void	nlopen();
void	nlclose();
void	nlread();
void	nlwrite();
int	nlioctl();
int	nulldev();
int	nonedev();

/*
 * Configuration table.
 */
CON nlcon ={
	DFCHR,				/* Flags */
	0,				/* Major index */
	nlopen,				/* Open */
	nlclose,			/* Close */
	nulldev,			/* Block */
	nlread,				/* Read */
	nlwrite,			/* Write */
#ifdef NULL_IOCTL
	nlioctl,			/* Ioctl */
#else /* NULL_IOCTL */
	nonedev,			/* Ioctl */
#endif /* NULL_IOCTL */
	nulldev,			/* Powerfail */
	nulldev,			/* Timeout */
	nulldev,			/* Load */
	nulldev				/* Unload */
};

int lock_clock();
void unlock_clock();

/*
 * These variables are used by /dev/proc.
 */
static	int proc_open_c = 0;	/* How many times is this device open?  */
static	int proc_valid = 0;	/* Do we have a valid snapshot?  */
static	PROC *proc_snapshot;	/* This is a snapshot of procq.  */
static	int proc_size;		/* How long is proc_snapshot (in bytes)?  */

/*
 * Null/memory open routine.
 */
void
nlopen(dev, mode)
dev_t dev;
int mode;
{
	register PROC *pp1;
	int count;		/* How many processes are there?  */

	switch (minor(dev)) {
	case DEV_PROC:
		if ( IPR == (IPR & mode) ){
			T_PIGGY( 0x4000000, printf("proc open "); );
			/*
			 * Lock the process table.
			 * We lock the process table first to avoid a race
			 * condition.  We really only want to muck with the
			 * process table on the first open.
			 */
			lock(pnxgate);
	
			/*
			 * If this is the first open, take a snapshot.
			 */
			if ( 0 == proc_open_c ) {
				T_PIGGY( 0x4000000, printf("snapshot of "); );
				/*
				 * Find out how long the process table is.
				 */
				for (count = 0, pp1 = &procq;
				     (pp1=pp1->p_nforw) != &procq;
				     ++count) {
					/* Do nothing else.  */
				}

				T_PIGGY( 0x4000000,
					printf("%d entries of %d, ",
						count, sizeof(PROC));
				);

				/*
				 *	Allocate memory for a snapshot.
				 */
				proc_size = count * sizeof(PROC);
				T_PIGGY( 0x4000000,
					printf("allocating %d, ", proc_size);
				);
				if ( (proc_snapshot = kalloc(proc_size)) !=
					NULL) {
					/*
					 *	Take a snapshot.
					 */
					for ( count = 0, pp1 = &procq;
					      (pp1=pp1->p_nforw) != &procq;
					      ++count) {
						T_PIGGY( 0x4000000,
						    printf("&proc[%d]: %x, ",
						    	count,
							&proc_snapshot[count]);
						);
						kkcopy(pp1,
						       &proc_snapshot[count],
						       sizeof(PROC)
						);
					}
					proc_valid = 1;
				}
			} /* First open?  */
	
			/*
			 * Unlock the process table.
			 */
			unlock(pnxgate);
	
			/*
			 * If we have a valid snapshot, the open succeeded,
			 * so increment the count.  Otherwise, fail the open.
			 */
			if ( proc_valid ) {
				proc_open_c++;
			} else {
				SET_U_ERROR( ENOMEM,
					"Not enough memory for a snapshot");
			}
		} else {
			SET_U_ERROR( EACCES, "/dev/proc is read only" );
		}
		break;
	default:
		/*
		 * For minor devices on NULL there is
		 * usually no action for open().
		 */
		break;
	}
	return;
} /* nlopen() */

/*
 * Null/memory close routine.
 */
void
nlclose(dev, mode)
dev_t dev;
int mode;
{
	switch (minor(dev)) {
	case DEV_PROC:
		if (proc_open_c > 0) {
			T_PIGGY( 0x4000000, printf(" last proc close, "); );

			/*
			 * Lock the process table.
			 *
			 * We lock the process table first to avoid a race
			 * condition.  We don't muck with the process table
			 * at all, but on the last close we want to lock out
			 * opens on this device.
			 */
			lock(pnxgate);
			/*
			 * If this is the last close:
			 *	Free the snapshot of the process table.
			 */
			if ( 1 == proc_open_c ) {
				kfree( proc_snapshot );
				proc_valid = 0;
			}

			/*
			 * Unlock the process table.
			 */
			unlock(pnxgate);
	
			/*
			 * Record the close.
			 */
			proc_open_c--;
		}
		break;
	default:
		/*
		 * For minor devices on NULL there is
		 * Usually no action for close().
		 */
		break;
	}
	return;
} /* nlclose() */


/*
 * Null/memory read routine.
 */
void
nlread(dev, iop)
dev_t dev;
register IO *iop;
{
	register unsigned bytes_read;
	unsigned int seek;
	unsigned char read_cmos();
	extern typed_space boot_gift;

	switch (minor(dev)) {
	case DEV_NULL:
		/*
		 * Read nothing.
		 * Do NOT update iop->io_ioc.
		 * This way, caller knows 0 bytes were read.
		 */
		break;

	case DEV_MEM:
		bytes_read = pxcopy((long)iop->io_seek, iop->io.pbase,
		  iop->io_ioc, SEG_386_UD);
		iop->io_ioc -= bytes_read;
		if (u.u_error == EFAULT)
			u.u_error = 0;
		break;

	case DEV_KMEM:
		iowrite(iop, iop->io_seek, iop->io_ioc);
		if (u.u_error == EFAULT)
			u.u_error = 0;
		break;

	case DEV_CLOCK:
		/*
		 * Don't go past the end of the CLOCK.
		 */
		if (iop->io_seek >= CLOCK_LEN)
			break;

		/*
		 * Lock the clock before any reading.
		 */
		if (lock_clock() == 0) {
			SET_U_ERROR(EIO, "RT clock will not settle.");
			break;
		}

		/*
		 * Read the requested data out of the CMOS.
		 */
		for (seek = iop->io_seek; seek < CLOCK_LEN; seek++) {
			if(ioputc(read_cmos(seek), iop) == -1)
				break;
		}

		/*
		 * Now that we are done reading the CMOS, let
		 * the clock loose.
		 */
		unlock_clock();
		break;

	case DEV_CMOS:
		/*
		 * Don't go past the end of the CMOS.
		 */
		if (iop->io_seek >= CMOS_LEN)
			break;

		/*
		 * Read the requested data out of the CMOS.
		 */
		for (seek = iop->io_seek; seek < CMOS_LEN; seek++) {
			if(ioputc(read_cmos(seek), iop) == -1)
				break;
		}
		break;

	case DEV_BOOTGIFT:
		/*
		 * Reads all from the data structure boot_gift.
		 */
		if (iop->io_seek < BG_LEN) {
			bytes_read = iop->io_ioc;
			/*
			 * Copy no more than to the end of boot_gift.
			 */
			if (iop->io_seek + bytes_read > BG_LEN) {
				bytes_read = BG_LEN - (iop->io_seek);
			}

			iowrite(iop,
				(char *)(&boot_gift) + iop->io_seek,
				bytes_read);
		}
		break;

	case DEV_PROC:
		/*
		 * Reads are all from the data structure *proc_snapshot.
		 */
		T_PIGGY( 0x4000000,
			printf("reading %d proc bytes, ", iop->io_ioc);
		);

		if (iop->io_seek < proc_size) {
			bytes_read = iop->io_ioc;
			/* Copy no more than to the end of the snapshot.  */
			if (iop->io_seek + bytes_read > proc_size) {
				bytes_read = proc_size - (iop->io_seek);
			}

			iowrite(iop,
				(char *)(proc_snapshot) + iop->io_seek,
				bytes_read);
		}
		break;
	default:
		SET_U_ERROR(ENXIO, "nlread(): illegal minor device for null");
	}
	return;
}

/*
 * Null/memory write routine.
 */
void
nlwrite(dev, iop)
dev_t dev;
register IO *iop;
{
	register unsigned n;
	unsigned write_cmos();
	unsigned seek;
	int	ch;

	switch (minor(dev)) {
	case DEV_NULL:
		/*
		 * Tell caller all bytes were written.
		 */
		iop->io_ioc = 0;
		break;

	case DEV_MEM:
		n = xpcopy(iop->io.pbase, (long)iop->io_seek, iop->io_ioc,
			SEG_386_UD);
		iop->io_ioc -= n;
		if (u.u_error == EFAULT)
			u.u_error = 0;
		break;

	case DEV_KMEM:
		ioread(iop, iop->io_seek, iop->io_ioc);
		break;

	case DEV_CLOCK:
		/*
		 * Don't go past the end of the CLOCK.
		 */
		if (iop->io_seek >= CLOCK_LEN)
			break;

		/*
		 * Lock the clock before any writing.
		 */
		if (lock_clock() == 0) {
			SET_U_ERROR(EIO, "RT clock will not settle.");
			break;
		}

		/*
		 * Write the requested data into the CMOS.
		 */
		for (seek = iop->io_seek; seek < CLOCK_LEN; seek++) {
			if((ch = iogetc(iop)) == -1)
				break;
			write_cmos(seek, ch);
		}

		/*
		 * Now that we are done writing the CMOS, let
		 * the clock loose.
		 */
		unlock_clock();
		break;

	case DEV_CMOS:
		/*
		 * Don't go past the end of the CMOS.
		 */
		if (iop->io_seek >= CMOS_LEN)
			break;

		/*
		 * Write the requested data into the CMOS.
		 */
		for (seek = iop->io_seek; seek < CMOS_LEN; seek++) {
			if((ch = iogetc(iop)) == -1)
				break;
			write_cmos(seek, ch);
		}
		break;

	case DEV_BOOTGIFT:
		/*
		 * /dev/bootgift is not writable.
		 */
		break;

	case DEV_PROC:
		/*
		 * /dev/proc is not writable.
		 */
		T_PIGGY( 0x4000000, printf("/dev/proc is not writable.\n"); );
		break;

	default:
		SET_U_ERROR(ENXIO,
			     "nlwrite(): illegal minor device for null");
	}
	return;
}

#ifdef NULL_IOCTL /* Includes all of nlioctl().  */

/*
 * Do an ioctl call for /dev/null.
 */
int
nlioctl(dev, cmd, vec)
	dev_t dev;
	int cmd;
	char * vec;
{
	/* Only /dev/kmem has an ioctl.  */
	switch (minor(dev)) {
	case DEV_KMEM:
		switch (cmd) {
#ifdef DANGEROUS
		case NLCALL:	/* Call a function.  */
		return docall(vec);
#endif /* DANGEROUS */
		default:
			SET_U_ERROR(EINVAL,
				     "nioctl(): illegal command for kmem");
			return(-1);
		}
	default:
		SET_U_ERROR(EINVAL, "illegal minor device for null ioctl");
		return (-1);
	} /* switch on minor device */

} /* nlioctl() */

#endif /* NULL_IOCTL */

#ifdef DANGEROUS /* Includes all of docall().  */
/*
 * MASSIVE SECURITY HOLE!  This should NOT be included in a distribution
 * system.  Among other problems, it becomes possible to do "setuid(0)".
 *
 * Call a function with arguments.
 *
 * Takes an array of unsigned ints.  The first element is the length of
 * the whole array, the second element is a pointer to the function to
 * call, all other elements are arguments.  At most 5 arguments may be
 * passed.
 *
 * Returns the return value of the called fuction in uvec[0].
 */
int
docall(uvec)
	unsigned uvec[];
{
	int (* func)();
	unsigned kvec[7];
	int retval;

	printf("NLCALL security hole.\n");

	/* Fetch the first element of vec.  */
	ukcopy(uvec, kvec, sizeof(unsigned));

	if ((kvec[0] < 2) || (kvec[0] > 7)) {
		/* Invalid number of elements in uvec.  */
		SET_U_ERROR(EINVAL, "Invalid number of elements in uvec");
		return(-1);
	}
	
	/* Fetch the whole vector.  */
	ukcopy(uvec, kvec, kvec[0] * sizeof(unsigned));

	/* Extract the function.  */
	func = (int (*)()) kvec[1];

	/* Call the function with all arguments.  */
	retval = (*func)(kvec[2], kvec[3], kvec[4], kvec[5], kvec[6]);

	kucopy(&retval, uvec, sizeof(unsigned));

} /* docall() */

#endif /* DANGEROUS */

/*
 * int lock_clock() -- Stop the update cycle on the CMOS RT clock and
 * wait for it to settle.  Returns 0 if the clock would not settle
 * in time.
 */
int
lock_clock()
{
	register int i;

	/*
	 * Wait for the clock to settle.  If it does not settle in
	 * a reasonable amount of time, give up.
	 */
	i = 65536;	/* Loop for a longish time.  */
	while (--i > 0) {
		if (0 == (UIP & read_cmos(SRA))) {
			break;	/* Break if there is no update in progress.  */
		}
	}
	
	if (0 == i) {
		/* The clock would not settle.  */
		return 0;
	}

	/*
	 * There is a tiny race here--an interrupt could conceivably
	 * come here, thus allowing enough delay for another update to
	 * begin.  But if we take interrupts that take a full second
	 * to process, other things are going to break horribly.
	 */
	
	/*
	 * Lock out updates.
	 * We set the No Updates bit in Clock Status Register B.
	 */
	write_cmos(SRB, (NO_UPD | read_cmos(SRB)));

	return 1;
} /* lock_clock() */

/*
 * void unlock_clock() -- Restart the update cycle on the CMOS RT clock.
 */ 
void
unlock_clock()
{
	/*
	 * We clear the No Updates bit in Clock Status Register B.
	 */
	write_cmos(SRB, ((~ NO_UPD) & read_cmos(SRB)));
} /* unlock_clock() */
