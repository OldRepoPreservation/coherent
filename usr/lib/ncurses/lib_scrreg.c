/*********************************************************************
*                         COPYRIGHT NOTICE                           *
**********************************************************************
*        This software is copyright (C) 1982 by Pavel Curtis         *
*                                                                    *
*        Permission is granted to reproduce and distribute           *
*        this file by any means so long as no fee is charged         *
*        above a nominal handling fee and so long as this            *
*        notice is always included in the copies.                    *
*                                                                    *
*        Other rights are reserved except as explicitly granted      *
*        by written permission of the author.                        *
*                Pavel Curtis                                        *
*                Computer Science Dept.                              *
*                405 Upson Hall                                      *
*                Cornell University                                  *
*                Ithaca, NY 14853                                    *
*                                                                    *
*                Ph- (607) 256-4934                                  *
*                                                                    *
*                Pavel.Cornell@Udel-Relay   (ARPAnet)                *
*                decvax!cornell!pavel       (UUCPnet)                *
*********************************************************************/

/*
**	lib_scrreg.c
**
**	The routine wsetscrreg().
**
** $Log:	RCS/lib_scrreg.v $
 * Revision 2.2  91/04/20  21:42:05  munk
 * Usage of register variables
 *
 * Revision 2.1  82/10/25  14:49:00  pavel
 * Added Copyright Notice
 * 
 * Revision 2.0  82/10/25  13:49:29  pavel
 * Beta-one Test Release
 * 
**
*/

#ifndef COHERENT
static char RCSid[] =
	"$Header:   RCS/lib_scrreg.v  Revision 2.2  91/04/20  21:42:05  munk   Exp$";
#endif

#include "curses.h"
#include "curses.priv.h"


wsetscrreg(win, top, bottom)
register WINDOW	*win;
int		top, bottom;
{
#ifdef TRACE
	if (_tracing)
	    _tracef("wsetscrreg(%o,%d,%d) called", win, top, bottom);
#endif

    	if (0 <= top  &&  top <= win->_cury  &&
		win->_cury <= bottom  &&  bottom <= win->_maxy)
	{
	    win->_regtop = top;
	    win->_regbottom = bottom;

	    return(OK);
	}
	else
	    return(ERR);
}
