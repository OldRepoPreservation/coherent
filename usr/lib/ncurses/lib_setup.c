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
 *	setupterm(termname, filedes, errret)
 *
 *	Find and read the appropriate object file for the terminal
 *	Make cur_term point to the structure.
 *	Turn off the XTABS bit in the tty structure if it was on
 *	If XTABS was on, remove the tab and backtab capabilities.
 *
 *  $Log:	RCS/lib_setup.v $
 * Revision 2.2  91/04/20  21:45:19  munk
 * Usage of register variables
 * Made the large arrays static
 *
 * Revision 2.1  82/10/25  14:49:09  pavel
 * Added Copyright Notice
 * 
 * Revision 2.0  82/10/24  15:17:46  pavel
 * Beta-one Test Release
 * 
 * Revision 1.5  82/08/23  22:30:35  pavel
 * The REAL Alpha-one Release Version
 * 
 * Revision 1.4  82/08/20  15:12:24  pavel
 * Fixed use of un-initialised cur_term.
 * 
 * Revision 1.3  82/08/19  19:22:09  pavel
 * Alpha Test Release One
 * 
 * Revision 1.2  82/08/19  19:11:28  pavel
 * Alpha Test Release One
 * 
 * Revision 1.1  82/08/12  18:45:07  pavel
 * Initial revision
 * 
 *
 */

#ifndef COHERENT
static char RCSid[] =
	"$Header:   RCS/lib_setup.v  Revision 2.2  91/04/20  21:45:19  munk   Exp$";
#endif

#include <stdio.h>
#include "curses.h"
#include "curses.priv.h"
#include "term.h"

#define ret_error(code, fmt, arg)	if (errret)\
					{\
					    *errret = code;\
					    return(code);\
					}\
					else\
					{\
					    fprintf(stderr, fmt, arg);\
					    exit(1);\
					}


char ttytype[NAMESIZE];

int	_tracing;
struct screen *SP = 0;

setupterm(termname, filedes, errret)
char	*termname;
int	filedes;
int	*errret;
{
	static char	filename1[1024];
	static char	filename2[1024];
	char		*directory = SRCDIR;
	char		*malloc(), *getenv();
	char		*terminfo;
	struct term	*term_ptr;

#ifdef TRACE
	_init_trace();
	if (_tracing)
	    _tracef("setupterm(%s,%d,%o) called", termname, filedes, errret);
#endif

	if (termname == NULL)
	{
	    termname = getenv("TERM");
	    if (termname == NULL)
		ret_error(-1, "TERM environment variable not set.\n", "");
	}

	if (cur_term == 0)
	    term_ptr = &_first_term;
	else
	{
	    term_ptr = (struct term *) malloc(sizeof(struct term));

	    if (term_ptr == NULL)
		ret_error(-1, "Not enough memory to create terminal structure.\n", "");
	}

	if ((terminfo = getenv("TERMINFO")) != NULL)
	    directory = terminfo;

	sprintf(filename1, "%s/%c/%s", directory, termname[0], termname);
	sprintf(filename2, "%s/%c/%s", SRCDIR, termname[0], termname);

	if (read_entry(filename1, term_ptr) < 0
				    &&  read_entry(filename2, term_ptr) < 0)
	    ret_error(0, "'%s': Unknown terminal type.\n", termname);

	if (command_character  &&  getenv("CC"))
	    do_prototype();

	cur_term = term_ptr;
	strncpy(ttytype, cur_term->term_names, NAMESIZE - 1);
	ttytype[NAMESIZE - 1] = '\0';
	cur_term->Filedes = filedes;
	gtty(filedes, &cur_term->Ottyb);
	if (cur_term->Ottyb.sg_flags & XTABS)
	    tab = back_tab = NULL;

	cur_term->Nttyb = cur_term->Ottyb;
	cur_term->Nttyb.sg_flags &= ~XTABS;

	fixterm();

	if (errret)
	    *errret = 1;
	
	return(1);
}



/*
**	do_prototype()
**
**	Take the real command character out of the CC environment variable
**	and substitute it in for the prototype given in 'command_character'.
**
*/

static
do_prototype()
{
    	register int	i, j;
	char		CC;
	char		proto;

	CC = *getenv("CC");
	proto = *command_character;

	for (i=0; i < STRCOUNT; i++)
	{
	    j = 0;
	    while (cur_term->Strings[i][j])
	    {
		if (cur_term->Strings[i][j] == proto)
		    cur_term->Strings[i][j] = CC;
		j++;
	    }
	}
}
