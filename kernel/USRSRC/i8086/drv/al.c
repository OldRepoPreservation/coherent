/* $Header: /usr/src/sys/i8086/drv/RCS/al.c,v 2.2 89/03/31 16:16:50 src Exp $ */
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
 * Driver for an IBM PC asyncronous
 * line, using interrupts. The interface
 * uses a Natty/WD 8250 chip.
 *
 * $Log$
 * Revision 2.2	89/03/31  16:16:50 	src
 * Bug:	Did not cancel timed functions during an unload.  This could result
 * 	in a system panic.
 * Fix:	Now cancels timed functions during an unload. (ABC)
 * 
 * Revision 2.1	88/09/03  06:02:24 	src
 * *** empty log message ***
 * 
 * Revision 1.1	88/03/24  17:04:07	src
 * Initial revision
 * 
 * 88/01/23	Allan Cornish		/usr/src/sys/i8086/drv/al.c
 * Unload function added to support loadable device drivers.
 *
 * 86/12/12	Allan Cornish		/usr/src/sys/i8086/drv/al.c
 * Added 3rd argument to alpoll() to support non-blocking poll.
 *
 * 86/11/24	Allan Cornish		/usr/src/sys/i8086/drv/al.c
 * The new tty struct raw input and output buffers are now used.
 * Moved alstart() to alx.c/alxstart().
 * Replaced altime() by alxcycle().
 *
 * 86/11/19	Allan Cornish		/usr/src/sys/i8086/drv/al.c
 * Added support for non-blocking read/write, and System V.3 compatible polls.
 * alintr() now uses defer() rather than timeout() to delay call to altime().
 * Increased raw input buffer size from 48 to 64 bytes.
 *
 * 86/07/27	Allan Cornish		/usr/src/sys/i8086/drv/al.c
 * Made alload() disable interrupts, and verify hardware existence.
 * Revised to use ins8250.h header file rather than wd8250.h.
 *
 * 85/06/27	Allan Cornish		/usr/src/sys/i8086/drv/al.c
 * Made alintr() recognize received XOFF characters immediately,
 * rather than deferring recognization through timeout() to altime().
 * This is necessary to avoid input buffer overflow in some printers.
 */
#include <coherent.h>
#include <i8086.h>
#include <con.h>
#include <errno.h>
#include <stat.h>
#include <tty.h>
#include <uproc.h>
#include <clist.h>
#include <ins8250.h>
#include <sched.h>

/*
 * This driver can be compiled to drive any possible
 * async port by appropriate definitions of:
 *	ALPORT	the io port address
 *	ALINT	the interrupt level
 *	ALNAME	the xxcon name
 *	ALMAJ	the major device number
 * Common code for the different ports is handled by alx.c
 */

#ifdef ALCOM1		/* COM1 definitions */
#define ALPORT	0x3F8		/* Base of com1 port */
#define ALINT	4		/* Interrupt level of com1 port */
#define	ALNAME	a0con		/* CON name of com1 port */
#define ALMAJ	5		/* Major number of com1 port */
#endif

#ifdef ALCOM2		/* COM2 definitions */
#define ALPORT	0x2F8		/* Base of com2 port */
#define ALINT	3		/* Interrupt level of com2 port */
#define ALNAME	a1con		/* CON name of com2 port */
#define ALMAJ	6		/* Major number of com2 port */
#endif

#ifdef ALCOM3		/* COM3 definitions */
#define ALPORT	0x2F0		/* Base of com3 port */
#define ALINT	2		/* Interrupt level of com3 port */
#define ALNAME	a2con		/* CON name of com3 port */
#define ALMAJ	3		/* Major number of com3 port */
#endif

/*
 * Functions.
 */
int	alxopen();
int	alxclose();
int	alxioctl();
int	alxtimer();
int	alxparam();
int	alxcycle();
int	alxstart();
int	alxbreak();

int	alintr();
int	alopen();
int	alclose();
int	alread();
int	alwrite();
int	alioctl();
int	alload();
int	alunload();
int	alpoll();
int	nulldev();
int	nonedev();

/*
 * Configuration table.
 */
CON ALNAME ={
	DFCHR|DFPOL,			/* Flags */
	ALMAJ,				/* Major index */
	alopen,				/* Open */
	alclose,			/* Close */
	nulldev,			/* Block */
	alread,				/* Read */
	alwrite,			/* Write */
	alioctl,			/* Ioctl */
	nulldev,			/* Powerfail */
	alxtimer,			/* Timeout */
	alload,				/* Load */
	alunload,			/* Unload */
	alpoll				/* Poll */
};

/*
 * Terminal structure.
 */
static TTY	altty = { {0}, {0}, ALPORT, alxstart, alxparam, B9600, B9600 };

static
alload()
{
	register int s;
	static int init;
	extern int albaud[];

	s = sphi();
	if ( init == 0 ) {
		outb(ALPORT+IER, 0);	    /* disable port interrupts */
		++init;
		if ( inb(ALPORT+IER) == 0 ) {
			outb(ALPORT+MCR, MC_OUT2);  /* hangup port */
			outb(ALPORT+LCR, LC_DLAB);
			outb(ALPORT+DLL, albaud[B9600] );
			outb(ALPORT+DLH, albaud[B9600] >> 8 );
			outb(ALPORT+LCR, LC_CS8 );
			setivec(ALINT, alintr);     /* set interrupt vector */
		}
	}
	spl( s );
}

static
alunload()
{
	clrivec( ALINT );			/* release interrupt vector */
	outb(ALPORT+IER, 0);			/* disable port interrupts */
	outb(ALPORT+MCR, MC_OUT2);		/* hangup port */
	timeout( &altty.t_rawtim, 0, NULL, 0 );	/* cancel cyclic timer */
}

static
alopen(dev, mode)
dev_t	dev;
int	mode;
{
	alload();
	alxcycle( &altty );
	alxopen( dev, mode, &altty );
}

static
alclose(dev, mode)
dev_t	dev;
int	mode;
{
	register int s;

	if (--altty.t_open == 0) {	/* Last open */
		s = sphi();
		alxclose( dev, mode, &altty );
		spl(s);
	}
}

static
alread(dev, iop)
dev_t	dev;
IO	*iop;
{
	ttread(&altty, iop, 0);
}

static
alwrite(dev, iop)
dev_t	dev;
register IO	*iop;
{
	register int c;

	/*
	 * Treat user writes through tty driver.
	 */
	if ( iop->io_seg != IOSYS ) {
		ttwrite( &altty, iop, 0 );
		return;
	}

	/*
	 * Treat kernel writes by blocking on transmit buffer.
	 */
	while ( (c = iogetc(iop)) >= 0 ) {

		/*
		 * Wait until transmit buffer is empty.
		 * Check twice to prevent critical race with interrupt handler.
		 */
		for (;;) {
			if ( inb(ALPORT+LSR) & LS_TxRDY )
				if ( inb(ALPORT+LSR) & LS_TxRDY )
					break;
		}

		/*
		 * Output the next character.
		 */
		outb( ALPORT+DREG, c );
	}
}

static
alioctl(dev, com, vec)
dev_t	dev;
struct sgttyb *vec;
{
	alxioctl(dev, com, vec, &altty);
}

static
alpoll( dev, ev, msec )
dev_t dev;
int ev;
int msec;
{
	return ttpoll( &altty, ev, msec );
}

static
alintr()
{
	alxintr( &altty );
}
