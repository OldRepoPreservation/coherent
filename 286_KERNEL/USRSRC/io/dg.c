/*
 * dg - device driver for Digiboard PC/Xe intelligent multiport controller
 *
 */

/*
 * Definitions.
 *
 */
#define	DG_RAM_LENGTH	0x10000L
#define DG_MEMORY_SEG	0xF000		/* dual-port ram base on FEP side */
#define DG_BIOS_FILE	"/drv/xabios.bin"
#define DG_FEPOS_FILE	"/drv/xafep.bin"
#define DG_BIOS_ADDR	0xF800
#define DG_FEPOS_ADDR	0x2000
#define DG_BIOS_LOADER	0x80		/* minor number to write to BIOS */
#define DG_FEPOS_LOADER	0x40		/* minor number to write to FEPOS */
#define DG_BIOS_LENGTH	0x800		/* PC/Xe BIOS is 2k bytes */
#define DG_BIOS_CONFIRM	0x0C00		/* look for "GD" here */
#define DG_FEP_CONFIRM	0x0D20		/* look for "OS" here */
#define DG_BIOS_REQ	0x0C40		/* Start FEP BIOS requests here */
#define BIOS_GOOD	('G' + ('D'<<8))	/* "GD" */
#define FEPOS_GOOD	('O' + ('S'<<8))	/* "OS" */

#define CSTART		0x400		/* start addr of command queue */
#define NPORT		0x0C22		/* addr of # of ports */
#define CIN		0x0D10		/* addr for command in pointer */
#define COUT		0x0D12		/* addr for command out pointer */
#define ISTART		0x800		/* start addr of event queue */
#define EIN		0x0D18		/* addr for event in pointer */
#define EOUT		0x0D1A		/* addr for event out pointer */
#define INTERVAL	0x0E04		/* addr for ticks between irpts */
#define EVENT_LEN	4		/* bytes per FEP event */
#define COMMAND_LEN	4		/* bytes per FEP command */

/*
 * Includes.
 */
#include "coherent.h" 
#include <sys/io.h>		/* IO */
#include <sys/sched.h>		/* [CIS]VPAUSE */
#include <sys/uproc.h>		/* u.u_error */
#include <sys/proc.h>		/* wakeup();
				   includes sys/types.h - faddr_t, paddr_t
				   and sys/timeout.h - TIM
				   needs coherent.h for KERNEL */
#include <sys/con.h>		/* CON */
#include <sys/stat.h>		/* minor(dev) */
#include <devices.h>		/* device major numbers, including PE_MAJOR */
#include <errno.h>

/*
 * Export Functions.
 */

/*
 * Export variables - these may be patched in order to configure the driver.
 */
long	DG_RAM = 0xF0000L;	/* segment for 64k of dual-port RAM */
int	DG_IOB = 0x200;		/* address of i/o byte for controller */
int	DG_INT = 12;		/* IRQ number for board's interrupt */

/*
 * Import Functions
 */
int	nulldev();
int	nonedev();

/*
 * Local functions.
 */
static dgload();
static dgunload();
static dgopen();
static dgclose();
static dgread();
static dgwrite();
static void dgdelay();
static dg_start_timing();
static dg_stop_timing();
static int dginit2();
static int dginit3();
static void dgintr();
 
/*
 * Local variables.
 */
static faddr_t	dg_ram_fp;	/* (far *) to access screen */
static paddr_t	dg_ram_base;	/* physical address of screen base */
static TIM	delay_tim;	/* needed for calls to timeout() */
static TIM	timeout_tim;	/* needed for calls to timeout() */
static int	dg_expired;	/* TRUE after local timeout */
static int	bios_loading;	/* TRUE if minor device for BIOS load open */
static int	fepos_loading;	/* TRUE if minor device for FEPOS load open */
static int	load_byte_ct;	/* place-holder for BIOS/FEPOS load */
static int	board_ready;	/* TRUE when all board initialization done */
static int	dg_bios_wait;	/* TRUE if waiting for BIOS to be loaded */
static int	dg_fepos_wait;	/* TRUE if waiting for FEPOS to be loaded */
static int	nport;		/* number of ports on the Digiboard */

/*
 * Configuration table - another export variable.
 */
CON dgcon ={
	DFCHR,				/* Flags */
	PE_MAJOR,			/* Major index */
	dgopen,				/* Open */
	dgclose,			/* Close */
	nulldev,			/* Block */
	dgread,				/* Read */
	dgwrite,			/* Write */
	nulldev,			/* Ioctl */
	nulldev,			/* Powerfail */
	nulldev,			/* Timeout */
	dgload,				/* Load */
	dgunload,			/* Unload */
	nulldev				/* Poll */
};

/*
 * Load Routine.
 */
static dgload()
{
	char v;

	/*
	 * Allocate a selector to map onto the dual-port RAM.  ptov() will
	 * return the first available selector of the 8,192 possible.
	 */
	dg_ram_base = (paddr_t)DG_RAM << 4;
	dg_ram_fp = ptov(dg_ram_base, (fsize_t)DG_RAM_LENGTH);
	
	/*
	 * Reset the board and wait.
	 * Actual delay is a tick = 0.01 sec; only need 1msec.
	 */
	outb(DG_IOB, 0x04);
	dgdelay(1);
	
	/*
	 * Read board ID.
	 */
	v = inb(DG_IOB);
	if ((v & 0x01) == 0x01) {
		printf("Error - board type is PC/Xi\n");
		return;
	} else {
		outb(DG_IOB, 0x05);	/* hold FEP reset */
		v = inb(DG_IOB);
		if ((v & 0x01) == 0x01) {
			printf("Error - board type is PC/Xm\n");
			return;
		} else
			printf("Digiboard PC/Xe ID found\n");
	}
	
	/*
	 * Board Reset.
	 */
	outb(DG_IOB, 0x04);	/* reset board */
	dg_start_timing(100);	/* start 1-second timer */
	while ((inb(DG_IOB) & 0x0E) != 0x04) {
		if (dg_expired) {
			printf("Error - Digiboard failed to reset\n");
			return;
		}
		dgdelay(10);
	}
	outb(DG_IOB, 0x06);	/* enable memory */
	printf("PC/Xe passed reset\n");

	/*
	 * Minimal test of PC/Xe's 64k of dual-ported RAM.
	 */
	sfword(dg_ram_fp, 0xA55A);		/* store a "far" word */
	sfword(dg_ram_fp + 2, 0x3CC3);
	sfword(dg_ram_fp + 0xFFFC, 0xA55A);
	sfword(dg_ram_fp + 0xFFFE, 0x3CC3);
	if (ffword(dg_ram_fp) != 0xA55A		/* fetch a "far" word */
	||  ffword(dg_ram_fp + 2) != 0x3CC3
	||  ffword(dg_ram_fp + 0xFFFC) != 0xA55A
	||  ffword(dg_ram_fp + 0xFFFE) != 0x3CC3) {
		printf("Error - Digiboard failed memory test\n");
		return;
	} else
		printf("PC/Xe passed memory test\n");
		
	/*
	 * Load and execute the PC/Xe BIOS
	 */
	outb(DG_IOB, 0x06);	/* enable memory */
	dg_bios_wait = 1;
	printf("PC/Xe waiting for BIOS load\n");
}

static dgunload()
{
	if (board_ready) {
		board_ready = 0;
		/*
		 * Turn off and unhook interrupts from FEPOS
		 */
#if 0		 
		clrivec(DG_INT);
#else
	clrivec(03);	
	clrivec(04);	
	clrivec(05);	
	clrivec(07);	
	clrivec(10);	
	clrivec(11);	
	clrivec(12);	
	clrivec(15);	
#endif	
	}

	/*
	 * We have to free up the selector now that we're done using it.
	 */
	vrelse(dg_ram_fp);
}

/*
 * Open Routine.
 */
static dgopen( dev, mode )
dev_t dev;
{
	/*
	 * If minor number has 128's bit set to 1, this is an attempt to
	 * transfer the BIOS to the FEP.
	 */
	if (minor(dev) & DG_BIOS_LOADER) {
		if (bios_loading) {
			u.u_error = EDBUSY;
			return;
		} else {
			/*
			 * Only allow BIOS xfer if we are waiting for it.
			 */
			if (dg_bios_wait) {
				bios_loading = 1;
				load_byte_ct = 0;
			} else {
				u.u_error = EIO;
				return;
			}
		}
	/*
	 * If minor number has 64's bit set to 1, this is an attempt to
	 * transfer the FEPOS to the FEP.
	 */
	} else if (minor(dev) & DG_FEPOS_LOADER) {
		if (fepos_loading) {
			u.u_error = EDBUSY;
			return;
		} else {
			/*
			 * Only allow FEPOS xfer if we are waiting for it.
			 */
			if (dg_fepos_wait) {
				fepos_loading = 1;
				load_byte_ct = 0;
			} else {
				u.u_error = EIO;
				return;
			}
		}
	} else {
	}
}

/*
 * Close Routine.
 */
static dgclose( dev )
dev_t dev;
{
	/*
	 * If minor number has 128's bit set to 1, this is an attempt to
	 * transfer the BIOS to the FEP.
	 */
	if (minor(dev) & DG_BIOS_LOADER) {
		bios_loading = 0;
		/*
		 * Only set dg_fepos_wait if enough bytes got written.
		 */
		if (load_byte_ct == DG_BIOS_LENGTH) {
			/*
			 * After BIOS is xferred, try to finish
			 * PC/Xe initialization.
			 */
			if (dginit2()) {
				dg_bios_wait = 0;
				dg_fepos_wait = 1;
				printf("PC/Xe waiting for FEPOS load\n");
			}
		}
	/*
	 * If minor number has 64's bit set to 1, this is an attempt to
	 * transfer the FEPOS to the FEP.
	 */
	} else if (minor(dev) & DG_FEPOS_LOADER) {
		fepos_loading = 0;
		/*
		 * After BIOS is xferred, try to finish
		 * PC/Xe initialization.
		 */
		if (dginit3()) {
			dg_fepos_wait = 0;
			board_ready = 1;
			printf("PC/Xe ready for use\n");
		}
	} else {
	}
}

/*
 * Read Routine.
 */
static dgread( dev, iop )
dev_t dev;
register IO * iop;
{
#if 0
	static int offset;
	int c;
	/*
	 * Read a character code from video RAM
	 * Start reading RAM just after where previous read ended
	 *
	 * Note that "offset" is the value of the displacement into
	 * the screen RAM. Any expression which results in a value
	 * which is less than DG_RAM_LENGTH is OK here.
	 */
	while(iop->io_ioc) {
		c = ffbyte(dg_ram_fp + offset); /* fetch a "far" byte */
		if(ioputc(c, iop) == -1)
			break;
		offset += 2;
		offset %= DG_RAM_LENGTH;
	}
#endif	
}

/*
 * Write Routine.
 */
static dgwrite( dev, iop )
dev_t dev;
register IO * iop;
{
	/*
	 * If minor number has 128's bit set to 1, this is an attempt to
	 * transfer the BIOS to the FEP.
	 */
	if (minor(dev) & DG_BIOS_LOADER) {
		int c;

		while ((c = iogetc(iop)) >= 0 && load_byte_ct < DG_BIOS_LENGTH) {
			sfbyte(dg_ram_fp + DG_BIOS_ADDR + load_byte_ct, c);
			load_byte_ct++;
		}
	/*
	 * If minor number has 64's bit set to 1, this is an attempt to
	 * transfer the FEPOS to the FEP.
	 */
	} else if (minor(dev) & DG_FEPOS_LOADER) {
		int c;

		while ((c = iogetc(iop)) >= 0) {
			sfbyte(dg_ram_fp + DG_FEPOS_ADDR + load_byte_ct, c);
			load_byte_ct++;
		}
	} else {
	}
}

/*
 * Delay for some number of clock ticks.
 * 286/386 kernel ticks are at 100Hz
 * Use kernel function sleep(), which is NOT the system call by that name.
 */
static void dgdelay(ticks)
int ticks;
{
	timeout(&delay_tim, ticks, wakeup, (int)&delay_tim);
	sleep((char *)&delay_tim, CVPAUSE, IVPAUSE, SVPAUSE);
}

/*
 * Start a timeout for some number of ticks.
 * Caller knows timer has expired when "dg_expired" goes to 1.
 *
 * Sample invocation:
 *	dg_start_timing(n);
 *	while (check for desired event fails) {
 *		if (dg_expired) {
 *			...failure stuff..
 *			break;
 *		}
 *		dgdelay(m); <= needed to allow kernel to update timers
 *	}
 */
static dg_start_timing(ticks)
int ticks;
{
	dg_expired = 0;
	timeout(&timeout_tim, ticks, dg_stop_timing, 1);
}

/*
 * Stub function called only by dg_start_timing()
 */
static dg_stop_timing(flagval)
int flagval;
{
	dg_expired = flagval;
}

/*
 * Second part of PC/Xe initialization - done after the BIOS has been
 * written to dual-ported RAM.
 */
static int dginit2()
{
	/*
	 * Execute FEP BIOS
	 */
	sfword(dg_ram_fp + DG_BIOS_CONFIRM, 0);	/* clear confirm word */
	outb(DG_IOB, 0x02);			/* Release reset */
	dg_start_timing(1000);			/* start 10-second timer */

	while (ffword(dg_ram_fp + DG_BIOS_CONFIRM) != BIOS_GOOD) {
		if (dg_expired) {
			printf("Error - Digiboard BIOS won't start\n");
			return 0;
		}
		dgdelay(10);
	}
	printf("PC/Xe BIOS started\n");
	
	return 1;
}

/*
 * Third part of PC/Xe initialization - done after the FEPOS has been
 * written to dual-ported RAM.
 */
static int dginit3()
{
	int cmd;

	/*
	 * Ask FEP BIOS to move FEPOS into host memory.
	 */
	sfword(dg_ram_fp + DG_BIOS_REQ, 0x0002);
	sfword(dg_ram_fp + DG_BIOS_REQ+2, DG_MEMORY_SEG+0x200);
	sfword(dg_ram_fp + DG_BIOS_REQ+4, 0x0000);
	sfword(dg_ram_fp + DG_BIOS_REQ+6, 0x0200);
	sfword(dg_ram_fp + DG_BIOS_REQ+8, 0x0000);
	sfword(dg_ram_fp + DG_BIOS_REQ+0xA, 0x2000);
	outb(DG_IOB, 0x0A);			/* Toggle interrupt */
	outb(DG_IOB, 0x02);
	dg_start_timing(100);			/* start 1-second timer */
	while (ffword(dg_ram_fp + DG_BIOS_REQ) != 0) {
		if (dg_expired) {
			printf("Error - Digiboard FEPOS move failed\n");
			return;
		}
		dgdelay(10);
	}
	printf("PC/Xe FEPOS relocated to host RAM\n");

	/*
	 * Execute FEPOS
	 */
	sfword(dg_ram_fp + DG_BIOS_REQ, 0x0001);
	sfword(dg_ram_fp + DG_BIOS_REQ+2, 0x0200);
	sfword(dg_ram_fp + DG_BIOS_REQ+4, 0x0004);
	sfword(dg_ram_fp + DG_FEP_CONFIRM, 0);	/* clear confirm word */
	outb(DG_IOB, 0x0A);			/* Toggle interrupt */
	outb(DG_IOB, 0x02);
	dg_start_timing(500);			/* start 5-second timer */
	while (ffword(dg_ram_fp + DG_FEP_CONFIRM) != FEPOS_GOOD) {
		if (dg_expired) {
			printf("Error - Digiboard FEPOS won't start\n");
			printf("Failure code (%x)\n",
				ffword(dg_ram_fp + DG_BIOS_REQ));
			return 0;
		}
		dgdelay(10);
	}
	printf("PC/Xe FEPOS started\n");
	nport = ffbyte(dg_ram_fp+NPORT);
	printf("Board is PC/%de\n", nport);

	/*
	 * Enable and test interrupts from FEP
	 */

printf("about to setivec\n");
#if 0
	setivec(DG_INT, dgintr);
#else
	setivec(03,dgintr);	
	setivec(04,dgintr);	
	setivec(05,dgintr);	
	setivec(07,dgintr);	
	setivec(10,dgintr);	
	setivec(11,dgintr);	
	setivec(12,dgintr);	
	setivec(15,dgintr);	
#endif	
printf("about to enable host irq's\n");	
	sfword(dg_ram_fp+INTERVAL, 1);		/* request host interrupts */
printf("getting command pointer\n");	
	cmd = ffword(dg_ram_fp+CIN);		/* get command pointer */
printf("about to send invalid command\n");	
	sfword(dg_ram_fp+CSTART+cmd, 0xA1FF);	/* send an invalid command */
	sfword(dg_ram_fp+CSTART+cmd+2, 0xC3B2);
	sfword(dg_ram_fp+CIN, (cmd+4)&0x3ff);	/* update command pointer */
printf("invalid command sent\n");	
	dgdelay(100);				/* wait 1 second */
	sfword(dg_ram_fp+INTERVAL, 0);		/* stop host interrupts */
#if 0
{ int t;
	sfword(dg_ram_fp+INTERVAL, 1);		/* request host interrupts */
	t = ffword(dg_ram_fp+0x0D18);		/* get command pointer */
	sfword(dg_ram_fp+0x400+t, 0xA1FF);	/* send an invalid command */
	sfword(dg_ram_fp+0x402+t, 0xC3B2);
	sfword(dg_ram_fp+0xD18, (t+4)&0x3ff);	/* update command pointer */
	dgdelay(1000);				/* wait 10 seconds */
	sfword(dg_ram_fp+INTERVAL, 0);		/* stop host interrupts */
	dgintr();
}	
#endif
	return 1;
}

/*
 * Interrupt handler.
 *
 * No specific action is needed to clear the interrupt
 * as it was a pulse sent from the FEPOS to host's IRQ line.
 */
static void dgintr()
{
	int cin, cout, ein, eout;
	char event[EVENT_LEN];
	int i,j=0;
#define ILIMIT 5	

	printf("Interrupt handler called\n");
	cin = ffword(dg_ram_fp+CIN);
	cout = ffword(dg_ram_fp+COUT);
	ein = ffword(dg_ram_fp+EIN);
	eout = ffword(dg_ram_fp+EOUT);
	printf("cin=%x cout=%x ein=%x eout=%x\n",cin,cout,ein,eout);

	while (ffword(dg_ram_fp+EIN) != ffword(dg_ram_fp+EOUT)&&j < ILIMIT) {
		eout = ffword(dg_ram_fp+EOUT);
		for (i = 0; i < EVENT_LEN; i++)
			event[i] = ffbyte(dg_ram_fp+ISTART+eout+i);
		printf("%x %x %x %x\n", event[0],event[1],event[2],event[3]);	
		sfword(dg_ram_fp+EOUT, (eout+4)&0x3ff);
		j++;
	}
}
