/*
 * dcpmisc.c
 *
 * Miscellaneous Support Routines for uucico
 */

#include "dcp.h"

usage()
{
	fatal("\n\
Usage:  uucico [-xn] [-r0]		slave mode\n\
	uucico [-xn] [-r1] -{sS}host 	call host\n\
	uucico [-xn] [-r1] -{sS}all	call all known hosts\n\
	uucico [-xn] -r1		call uutouch queued hosts\n\
");
}

fatal(x)
{
	printmsg(M_LOG, "%r", &x);
	if ( lockexist(rmtname) )
		lockrm(rmtname);
	if ( lockttyexist(rdevname) ) {
		dcpundial();
		if(unlocktty(rdevname) == -1){
			printmsg(M_LOG,"fatal: could not remove lock file");
			plog(M_CALL,"fatal: could not remove lock file");
		}
	}
	close_logfile();
	exit(1);
}

catchhup()
{
	plog(M_LOG, "Call terminated by hangup.");
	terminatelevel++;
	abort_cico = 1;
}

catchquit()
{
	fatal("Call terminated by quit signal");
}

catchterm()
{
	fatal("Call terminated by local signal");
}

catchsegv()
{
	fatal("Segmentation violation--Call aborted");
}
