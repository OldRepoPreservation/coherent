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
**	lib_newwin.c
**
**	The routines newwin(), subwin() and their dependent
**
** $Log:	RCS/lib_newwin.v $
 * Revision 2.2  91/04/20  19:31:13  munk
 * Usage of register variables
 *
 * Revision 2.1  82/10/25  14:48:18  pavel
 * Added Copyright Notice
 * 
 * Revision 2.0  82/10/25  13:47:18  pavel
 * Beta-one Test Release
 * 
**
*/

#include "term.h"
#include "curses.h"
#include "curses.priv.h"

short	*calloc();
#ifndef COHERENT
WINDOW	*malloc();
#endif

static WINDOW	*makenew();


WINDOW *
newwin(num_lines, num_columns, begy, begx)
int	num_lines, num_columns, begy, begx;
{
	register WINDOW	*win;
	chtype		*ptr;
	register int	i, j;

#ifdef TRACE
	if (_tracing)
	    _tracef("newwin(%d,%d,%d,%d) called", num_lines, num_columns, begy, begx);
#endif

	if (num_lines == 0)
	    num_lines = lines - begy;

	if (num_columns == 0)
	    num_columns = columns - begx;

	if ((win = makenew(num_lines, num_columns, begy, begx)) == ERR)
	    return(ERR);

	for (i = 0; i < num_lines; i++)
	{
	    if ((win->_line[i] = (chtype *) calloc(num_columns, sizeof(chtype)))
								      == NULL)
	    {
		for (j = 0; j < i; j++)
		    free(win->_line[j]);

		free(win->_firstchar);
		free(win->_lastchar);
		free(win->_line);
		free(win);

		return(ERR);
	    }
	    else
		for (ptr = win->_line[i]; ptr < win->_line[i] + num_columns; )
		    *ptr++ = ' ';
	}

#ifdef TRACE
	if (_tracing)
	    _tracef("\tnewwin: returned window is %o", win);
#endif

	return(win);
}



WINDOW *
subwin(orig, num_lines, num_columns, begy, begx)
WINDOW	*orig;
int	num_lines, num_columns, begy, begx;
{
	register WINDOW	*win;
	register int	i;
	int		j, k;

#ifdef TRACE
	if (_tracing)
	    _tracef("subwin(%d,%d,%d,%d) called", num_lines, num_columns, begy, begx);
#endif

	/*
	** make sure window fits inside the original one
	*/

	if (begy < orig->_begy || begx < orig->_begx
			|| begy + num_lines > orig->_maxy
			|| begx + num_columns > orig->_maxx)
	    return(ERR);

	if (num_lines == 0)
	    num_lines = orig->_maxy - orig->_begy - begy;

	if (num_columns == 0)
	    num_columns = orig->_maxx - orig->_begx - begx;

	if ((win = makenew(num_lines, num_columns, begy, begx)) == ERR)
	    return(ERR);

	j = orig->_begy + begy;
	k = orig->_begx + begx;

	for (i = 0; i < num_lines; i++)
	    win->_line[i] = &orig->_line[j++][k];

	win->_flags = _SUBWIN;

#ifdef TRACE
	if (_tracing)
	    _tracef("\tsubwin: returned window is %o", win);
#endif

	return(win);
}



static WINDOW *
makenew(num_lines, num_columns, begy, begx)
int	num_lines, num_columns, begy, begx;
{
	register int	i;
	register WINDOW	*win;

	if ((win = (WINDOW *) malloc(sizeof(WINDOW))) == NULL)
		return ERR;

	if ((win->_line = (chtype **) calloc(num_lines, sizeof (chtype *)))
								       == NULL)
	{
		free(win);

		return(ERR);
	}

	if ((win->_firstchar = calloc(num_lines, sizeof(short))) == NULL)
	{
		free(win);
		free(win->_line);

		return(ERR);
	}

	if ((win->_lastchar = calloc(num_lines, sizeof(short))) == NULL)
	{
		free(win);
		free(win->_line);
		free(win->_firstchar);

		return(ERR);
	}

	if ((win->_numchngd = calloc(num_lines, sizeof(short))) == NULL)
	{
		free(win);
		free(win->_line);
		free(win->_firstchar);
		free(win->_lastchar);

		return(ERR);
	}

	win->_curx       = 0;
	win->_cury       = 0;
	win->_maxy       = num_lines - 1;
	win->_maxx       = num_columns - 1;
	win->_begy       = begy;
	win->_begx       = begx;

	win->_flags      = 0;
	win->_attrs      = A_NORMAL;

	win->_clear      = (num_lines == lines  &&  num_columns == columns);
	win->_scroll     = FALSE;
	win->_leave      = FALSE;
	win->_use_keypad = FALSE;
	win->_use_meta   = FALSE;
	win->_nodelay    = FALSE;

	win->_regtop     = 0;
	win->_regbottom  = num_lines - 1;

	for (i = 0; i < num_lines; i++)
	{
	    win->_firstchar[i] = win->_lastchar[i] = _NOCHANGE;
	    win->_numchngd[i] = 0;
	}

	if (begx + num_columns == columns)
	{
		win->_flags |= _ENDLINE;

		if (begx == 0  &&  num_lines == lines  &&  begy == 0)
			win->_flags |= _FULLWIN;

		if (begy + num_lines == lines)
			win->_flags |= _SCROLLWIN;
	}

	return(win);
}
