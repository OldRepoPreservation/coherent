/*
 * Coherent.
 *
 * $Log:	/usr/src/sys/coh/RCS/main.c,v $
 * Revision 1.2	88/06/29  12:00:29 	src
 * Real/Protected mode status now printed during boot sequence.
 * Three part serial numbers now supported, moved to optional fourth line.
 * 
 * Revision 1.1	88/03/24  16:13:58	src
 * Initial revision
 * 
 * 87/04/09	Allan Cornish		/usr/src/sys/coh/main.c
 * Serial numbers changed to support group.
 *
 * 87/01/05	Allan Cornish		/usr/src/sys/coh/main.c
 * Copyright notice revised to include 1987.
 */
#include <coherent.h>
#include <sys/proc.h>
#include <sys/seg.h>
#include <sys/uproc.h>

#ifndef VERSION		/* This should be specified at compile time */
#define VERSION	"..."
#endif

unsigned long	_entry = 0;		/* really the serial number */
unsigned long	__ = 0;			/* really the serial number also */

/*
 * Initialise various things.  When we return we will return to user mode.
 */
char version[] = VERSION;
char copyright[] = "\
Copyright (c) 1982, 1991 by Mark Williams Company\n\
";

main()
{
	register SEG *sp;
	extern int realmode;	/* real addressing mode - as2.s */

	u.u_error = 0;
	bufinit();
	cltinit();
	pcsinit();
	seginit();
	devinit();
	printf("\Mark Williams COHERENT Version %s - %s Mode (mem=%u Kbytes)\n",
		version, (realmode ? "Real" : "Protected"), msize );
	printf(copyright);

	if ( _entry ) {
		printf("Serial Number ");
		printf("%U\n", _entry);
	}

	/*
	 * Verify correct serial number
	 */
	if (_entry != __)
		panic("Verification error - call Mark Williams Company at 1-800-MARK-WMS\n");

	/*
	 * Turn on clock, start off processes, mount root device
	 * and return.
	 */
	batflag = 1;
	if ((sp=salloc((fsize_t)UPASIZE, SFNCLR|SFNSWP)) == NULL)
		panic("Cannot allocate user area");
	if ((iprocp=process(idle))==NULL || (eprocp=process(NULL))==NULL)
		panic("Cannot create process");
	eveinit(sp);
	fsminit();
}
