/*
 *  alarm.c
 *
 *  A modest support for SIGALRM catching.
 */

#include "alarm.h"
#include "dcp.h"

int timedout;		/* flag for receipt of SIGALRM signal	*/

int alarmclk()
{
	printmsg(M_DEBUG, "SIGALRM received");
	timedout = 1;
	RESETALRM();
}
