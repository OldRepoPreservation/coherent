/*
 * dcpunix.c
 *
 * Coherent/Unix/Minix support for dcp
 * Copyright 1989 (c) by Peter S. Housel.
 * Changes Copyright (c) 1989-1991 by Mark Williams Company.
 */

#include <stdio.h>

#include <signal.h>
#include "dial.h"
#include "dcp.h"
#include "alarm.h"

#if SGTTY
#include <sgtty.h>
#elif TERMIO
#include <termio.h>
#endif

int swritefd;		/* fd for serial write */
int sreadfd;		/* fd for serial read */

swrite(data, num)
char *data; 
int num;
{
	return( write(swritefd, data, num) );
}

#define	MINTIMEOUT	2

int sread(data, num, timeout)
char *data; 
int num, timeout;
{
	int ret;

	SETALRM( (timeout>MINTIMEOUT) ? timeout: MINTIMEOUT );
	ret = read(sreadfd, data, num);
	CLRALRM();

#if 0
	printmsg(M_DATA, "sread: {%s}", visbuf(data, num));
#endif
	return( (ret>0) ? ret: 0 );
}

int sread2(data, num)
char *data; 
int num;
{
	int retval = read(sreadfd, data, num);

#if 0
	printmsg(M_DATA, "sread2: {%s}", visbuf(data, num));
#endif
	return(retval);
}

/*
 *  Coherent support for setting the line parameters.
 *
 *  initline()  --  Used for uucico SLAVE mode.
 *	Sets the serial file descriptors: sreadfd and swritefd.
 *	Returns (1) for success, (0) for failure.
 *
 *  fixline()
 *	Fixes the line to RAW for uucico MASTER mode.
 */

int initline()
{
#if SGTTY
	struct sgttyb ttyb;

	sreadfd = 0;	/* standard input */
	swritefd = 1;	/* standard output */
	ioctl(sreadfd, TIOCHPCL);
	gtty(sreadfd, &ttyb);	/* set raw mode */
	ttyb.sg_flags |= (RAW|TANDEM);
	/* ttyb.sg_flags &= ~(XTABS | EVENP | ODDP | CRMOD | ECHO | CBREAK);*/
	ttyb.sg_flags &= ~(CRMOD | ECHO);
	stty(sreadfd, &ttyb);

#elif TERMIO
	struct termio tio;

	sreadfd = 0;			/* standard input */
	swritefd = 1;			/* standard output */
	ioctl(sreadfd, TCGETA, &tio);
	tio.c_iflag = 0;
	tio.c_oflag = 0;
	tio.c_cflag &= ~(CSIZE|PARENB);
	tio.c_cflag |= (HUPCL|CS8);
	tio.c_lflag = 0;
	ioctl(sreadfd, TCSETA, &tio);
#endif
	return(1);
}

fixline()
{
#if SGTTY
	struct sgttyb ttyb;

	gtty(sreadfd, &ttyb);
	ttyb.sg_flags |= (RAW|TANDEM);
	stty(sreadfd, &ttyb);

#elif TERMIO
	struct termio tio;

	ioctl(sreadfd, TCGETA, &tio);
#if 0
	printmsg(M_LOG, "tio.c_iflag = 0x%04x", tio.c_iflag);
#endif
	tio.c_iflag = 0;
	ioctl(sreadfd, TCSETA, &tio);
#endif
}


/*
 *  Coherent support for dialing and connecting with a modem device.
 *  Used for uucico MASTER mode.
 *
 *  dcpdial(dev, speed, tel)  char *dev, *speed, *tel;
 *	Initiates the call, utilizing the modemcap dial package,
 *	and sets the serial file descriptors: sreadfd and swritefd.
 *	Returns (1) for success, and (0) for failure.
 *
 *  dcpundial()
 *	Closes the serial file descriptors set up with dcpdial().
 */

static CALL call;		/* dial(3) structure, see "dial.h"	*/

int dcpdial(dev, speed, tel)
char *dev, *speed, *tel;
{
	char	*cp;

	call.baud = atoi(speed);
	call.line = dev;
	call.telno = tel;

	printmsg(M_CALL, "Trying to connect at speed %d", call.baud);
	if (tel != NULL)
		printmsg(M_CALL, "Calling phone# %s", call.telno);
	if ((sreadfd = swritefd = dial(&call)) < 0) {
		plog(M_CALL, "Dial failed, %s {%d}", _merr_list[-merrno],
			processid);
 		while ((cp = index(modembuf, '\r')) != NULL)
 			*cp = ' ';
 		while ((cp = index(modembuf, '\n')) != NULL)
 			*cp = ' ';
 		plog(M_CALL, "Modem says %s", modembuf);
		dcpundial();
		return( 0 );
	}
	return( 1 );
}

dcpundial()
{
	if (swritefd > 2)
		hangup(swritefd);
	else {
#if 0
		ioctl(swritefd, TIOCHPCL);
#endif
		close(swritefd);
	}

#if 0
	plog(M_CALL, "dcpundial(%d)", sreadfd);
	if (sreadfd > 2)
		hangup(sreadfd);
	else {
		ioctl(sreadfd, TIOCHPCL);
		close(sreadfd);
	}
#endif
}

sendbrk()
{}
