/* $Header: /usr/src/lib/libcurses/RCS/beep.c,v 1.1 89/04/07 13:20:37 src Exp $
 *
 *	The  information  contained herein  is a trade secret  of INETCO
 *	Systems, and is confidential information.   It is provided under
 *	a license agreement,  and may be copied or disclosed  only under
 *	the terms of that agreement.   Any reproduction or disclosure of
 *	this  material  without  the express  written  authorization  of
 *	INETCO Systems or persuant to the license agreement is unlawful.
 *
 *	Copyright (c) 1989
 *	An unpublished work by INETCO Systems, Ltd.
 *	All rights reserved.
 */

#include <stdio.h>
#include "curses.ext"

beep()
{
	register char * s;

# ifdef	DEBUG
	fprintf( outf, "BEEP\n");
# endif

	/*
	 * Use audible bell if available, otherwise visible bell.
	 */
	if ( (s = bell) || (s = VB) || (s = "\007") )
		write( 1, s, strlen(s) );
}

flash()
{
	register char * s;

# ifdef	DEBUG
	fprintf( outf, "FLASH\n");
# endif

	/*
	 * Use visible bell if available, otherwise audible bell.
	 */
	if ( (s = VB) || (s = bell) || (s == "\007") )
		write( 1, s, strlen(s ) );
}
