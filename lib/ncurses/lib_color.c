/*********************************************************************
*                         COPYRIGHT NOTICE                           *
**********************************************************************
*        This software is copyright (C) 1993 by Udo Munk             *
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
**	lib_color.c
**
**	The color system of SV.3
**
** $Log:	RCS/lib_color.v $
 * Revision 1.0  93/02/16  17:47:46  munk
 * Initial version
 * 
**
*/

#ifdef RCSHDR
static char RCSid[] =
	"$Header:   RCS/lib_color.v  Revision 1.0  93/02/16  17:47:46  munk  Exp$";
#endif

#include "curses.h"
#include "term.h"

short __pairs__[64];

start_color()
{
	register int i;

	for (i = 0; i < 63; i++)
		__pairs__[i] = -1;

	COLORS = max_colors;
	COLOR_PAIRS = max_pairs;

	if ((COLORS > 0) && (COLOR_PAIRS > 0)) {
		color = TRUE;
		return(OK);
	} else {
		color = FALSE;
		return(ERR);
	}
}

has_colors()
{
	if ((max_colors > 0) && (max_pairs > 0))
		return(TRUE);
	else
		return(FALSE);
}

can_change_color()
{
	if (can_change)
		return(TRUE);
	else
		return(FALSE);
}

init_color(color, r, g, b)
int color, r, g, b;
{
	return(ERR);
}

color_content(color, r, g, b)
int color, *r, *g, *b;
{
	return(ERR);
}

init_pair(pair, f, b)
int pair, f, b;
{
	if (!color)
		return(ERR);
	if ((pair > 63) || (pair < 0))
		return(ERR);
	if (pair >= COLOR_PAIRS)
		return(ERR);
	if ((f >= COLORS) || (f < 0))
		return(ERR);
	if ((b >= COLORS) || (b < 0))
		return(ERR);


	return(OK);
}

pair_content(pair, f, b)
int pair, *f, *b;
{
}
