/*
 *  log.c
 *
 *  Implement various file logging needs.
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/dir.h>
#include <time.h>
#include "dcp.h"

#define	LOGSDIR		".Log"

static	FILE	*logfp = NULL;
static	char	logfn[LOGFLEN];

/*
 * open_the_logfile(subdir)  char *subdir;
 *
 * Opens a logfile in the given subdirectory below LOGDIR.  The log
 * file's name will be the global system name variable: rmtname.
 */

open_the_logfile(subdir)
char *subdir;
{
	if (logfp != NULL)
		fclose(logfp);

	sprintf(logfn, "%s/%s/%.*s/%s", SPOOLDIR, LOGSDIR, DIRSIZ,
							 subdir, rmtname);
	printmsg(M_DEBUG, "Opening Log File: %s", logfn);
	if ( (logfp=fopen(logfn, "a")) == NULL )
		fatal("Can't open log file: %s", logfn);
	fseek(logfp, 0L, 2);
}

close_logfile()
{
	if (logfp != NULL) {
		printmsg(M_DEBUG, "Closing Log File: %s", logfn);
		fclose(logfp);
	}
	logfp = NULL;
}

/*
 *  char *
 *  timestr()
 *
 *  Return a pointer to a static area that contains a string formatted
 *  with the current time suitable for UUCP Log file notations.
 */

char *
timestr()
{
	static char locbuf[20];
	struct tm local;
	time_t now;

	time(&now);
	local = *localtime(&now);
	sprintf(locbuf, "%02d/%02d %02d:%02d:%02d",
		local.tm_mon + 1, local.tm_mday,
		local.tm_hour, local.tm_min, local.tm_sec);
	return(&locbuf[0]);
}

/*
 *  plog(level, msg)
 *
 *  Log the message in the open log file with a timestamp, and send
 *  the message to printmsg() at the given level.
 */

plog(level, msg)
int level;
{
	char *ts = timestr();

	if ( logfp != NULL ) {
		fseek(logfp, 0L, 2);
		fprintf(logfp, "%s [%s] %r\n", ts, rmtname, &msg);
		fflush(logfp);
		if ( ferror(logfp) != 0 )
			fatal("Error writing logfile: %s", logfn);
	}

	if ( debuglevel && (level <= M_DEBUG) )
		printmsg(M_LOG, "%s [%s]: %r", ts,
			(logfp!=NULL) ? rmtname: "NOT OPEN", &msg);
}
