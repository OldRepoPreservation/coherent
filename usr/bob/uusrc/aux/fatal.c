/*
 *  fatal.c
 *
 *  Just a simple little fatal error routine.
 */

#include "dcp.h"

fatal(x)
{
	printmsg(M_FATAL, "%r", &x);
	exit(1);
}

nonfatal(x)
{
	printmsg(M_LOG, "%r", &x);
}
