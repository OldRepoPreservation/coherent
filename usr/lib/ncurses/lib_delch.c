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
**	lib_delch.c
**
**	The routine wdelch().
**
** $Log:	RCS/lib_delch.v $
 * Revision 2.2  91/04/20  18:13:33  munk
 * Usage of register variables
 *
 * Revision 2.1  82/10/25  14:46:52  pavel
 * Added Copyright Notice
 * 
 * Revision 2.0  82/10/24  15:20:47  pavel
 * Beta-one Test Release
 * 
**
*/

#ifndef COHERENT
static char RCSid[] =
	"$Header:   RCS/lib_delch.v  Revision 2.2  91/04/20  18:13:33  munk   Exp$";
#endif

#include "curses.h"
#include "curses.priv.h"
#include "term.h"

wdelch(win)
register WINDOW	*win;
{
	register chtype	*temp1, *temp2;
	chtype		*end;

#ifdef TRACE
	if (_tracing)
	    _tracef("wdelch(%o) called", win);
#endif

	end = &win->_line[win->_cury][win->_maxx];
	temp2 = &win->_line[win->_cury][win->_curx + 1];
	temp1 = temp2 - 1;

	while (temp1 < end)
	    *temp1++ = *temp2++;

	*temp1 = ' ' | win->_attrs;

	win->_lastchar[win->_cury] = win->_maxx;

	if (win->_firstchar[win->_cury] == _NOCHANGE
				   || win->_firstchar[win->_cury] > win->_curx)
	    win->_firstchar[win->_cury] = win->_curx;

	if (delete_character)
	    win->_numchngd += 1;
	else
	    win->_numchngd += win->_maxx - win->_curx + 1;
}
