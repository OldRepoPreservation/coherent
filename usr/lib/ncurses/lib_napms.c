/*********************************************************************
*                         COPYRIGHT NOTICE                           *
**********************************************************************
*        This software is copyright (C) 1992 by Udo Munk             *
*                                                                    *
*        Permission is granted to reproduce and distribute           *
*        this file by any means so long as no fee is charged         *
*        above a nominal handling fee and so long as this            *
*        notice is always included in the copies.                    *
*                                                                    *
*        Other rights are reserved except as explicitly granted      *
*        by written permission of the author.                        *
*                Udo Munk					     *
*                Oberstr. 21					     *
*                4040 Neuss 1					     *
*                Germany					     *
*                                                                    *
*                udo@umunk.GUN.de				     *
*********************************************************************/

/*
**	lib_napms.c
**
**	Sleep for some millisecounds
**
** $Log:	RCS/lib_napms.v $
 * Revision 1.0  92/11/26  21:47:38  munk
 * Initial version
 * 
**
*/

#ifdef RCSHDR
static char RCSid[] =
	"$Header:   RCS/lib_napms.v  Revision 1.0  92/11/26  21:47:38  munk  Exp$";
#endif

#include <poll.h>

int napms(timeout)
int timeout;
{
	int		n;
	struct pollfd	fda[1];
	
	if (timeout < 1)
		return(0);
	fda[0].fd = -1;
	n = poll((struct pollfd*) &fda[0], 1, timeout);
	return(n < 0 ? n : timeout); 
}
