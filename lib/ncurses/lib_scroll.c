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
**	lib_scroll.c
**
**	The routine scroll().
**
** $Log:	RCS/lib_scroll.v $
 * Revision 2.2  91/04/20  21:40:21  munk
 * Usage of register variables
 *
 * Revision 2.1  82/10/25  14:48:54  pavel
 * Added Copyright Notice
 * 
 * Revision 2.0  82/10/25  13:49:22  pavel
 * Beta-one Test Release
 * 
**
*/

#ifndef COHERENT
static char RCSid[] =
	"$Header:   RCS/lib_scroll.v  Revision 2.2  91/04/20  21:40:21  munk   Exp$";
#endif

#include "curses.h"
#include "curses.priv.h"


scroll(win)
register WINDOW	*win;
{
	register int	i;
	chtype		*ptr, *temp;
	chtype		blank = ' ' | win->_attrs;

#ifdef TRACE
	if (_tracing)
	    _tracef("scroll(%o) called", win);
#endif

	if (! win->_scroll)
	    return;

	temp = win->_line[0];
	for (i = 0; i < win->_regbottom; i++)
	    win->_line[i] = win->_line[i+1];

	for (ptr = temp; ptr - temp <= win->_maxx; ptr++)
	    *ptr = blank;

	win->_line[win->_regbottom] = temp;

	win->_cury--;
}
