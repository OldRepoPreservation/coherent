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
 *	raw.c
 *
 *	Routines:
 *		raw()
 *		echo()
 *		nl()
 *		cbreak()
 *		crmode()
 *		noraw()
 *		noecho()
 *		nonl()
 *		nocbreak()
 *		nocrmode()
 *
 *	cbreak() == crmode()
 *	nocbreak() == crmode()
 *
 *  $Log:	RCS/lib_raw.v $
 * Revision 2.1  82/10/25  14:48:42  pavel
 * Added Copyright Notice
 * 
 * Revision 2.0  82/10/24  15:17:40  pavel
 * Beta-one Test Release
 * 
 * Revision 1.3  82/08/23  22:30:32  pavel
 * The REAL Alpha-one Release Version
 * 
 * Revision 1.2  82/08/19  19:11:25  pavel
 * Alpha Test Release One
 * 
 * Revision 1.1  82/08/12  18:43:18  pavel
 * Initial revision
 * 
 *
 */

#ifndef COHERENT
static char RCSid[] =
	"$Header:   RCS/lib_raw.v  Revision 2.1  82/10/25  14:48:42  pavel  Exp$";
#endif

#include "curses.h"
#include "curses.priv.h"
#include "term.h"




raw()
{
#ifdef TRACE
	if (_tracing)
	    _tracef("raw() called");
#endif

    cur_term->Nttyb.sg_flags |= RAW;
    stty(cur_term->Filedes, &cur_term->Nttyb);

    SP->_raw = TRUE;
    SP->_nlmapping = TRUE;
}


cbreak()
{
#ifdef TRACE
	if (_tracing)
	    _tracef("cbreak() called");
#endif

    cur_term->Nttyb.sg_flags |= CBREAK;
    stty(cur_term->Filedes, &cur_term->Nttyb);

    SP->_cbreak = TRUE;
}

crmode()
{
#ifdef TRACE
	if (_tracing)
	    _tracef("crmode() called");
#endif

    cbreak();
}


echo()
{
#ifdef TRACE
	if (_tracing)
	    _tracef("echo() called");
#endif

    cur_term->Nttyb.sg_flags |= ECHO;
    stty(cur_term->Filedes, &cur_term->Nttyb);

    SP->_echo = TRUE;
}


nl()
{
#ifdef TRACE
	if (_tracing)
	    _tracef("nl() called");
#endif

    cur_term->Nttyb.sg_flags |= CRMOD;
    stty(cur_term->Filedes, &cur_term->Nttyb);

    SP->_nl = TRUE;
    SP->_nlmapping = ! SP->_raw;
}


noraw()
{
#ifdef TRACE
	if (_tracing)
	    _tracef("noraw() called");
#endif

    cur_term->Nttyb.sg_flags &= ~RAW;
    stty(cur_term->Filedes, &cur_term->Nttyb);

    SP->_raw = FALSE;
    SP->_nlmapping = SP->_nl;
}


nocbreak()
{
#ifdef TRACE
	if (_tracing)
	    _tracef("nocbreak() called");
#endif

    cur_term->Nttyb.sg_flags &= ~CBREAK;
    stty(cur_term->Filedes, &cur_term->Nttyb);

    SP->_cbreak = FALSE;
}

nocrmode()
{
#ifdef TRACE
	if (_tracing)
	    _tracef("nocrmode() called");
#endif

    nocbreak();
}


noecho()
{
#ifdef TRACE
	if (_tracing)
	    _tracef("noecho() called");
#endif

    cur_term->Nttyb.sg_flags &= ~ECHO;
    stty(cur_term->Filedes, &cur_term->Nttyb);

    SP->_echo = FALSE;
}


nonl()
{
#ifdef TRACE
	if (_tracing)
	    _tracef("nonl() called");
#endif

    cur_term->Nttyb.sg_flags &= ~CRMOD;
    stty(cur_term->Filedes, &cur_term->Nttyb);

    SP->_nl = FALSE;
    SP->_nlmapping = FALSE;
}
