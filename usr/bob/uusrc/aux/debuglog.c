/*
 *  debuglog.c
 *
 *  Implement the debuglevel relative output stuff.
 */

#include <stdio.h>
#include <sys/dir.h>
#include <ctype.h>
#include "dcp.h"

#define	LOGSDIR		".Log"

int	debuglevel = 0;		/* User specified Debug Level		*/
static	FILE *debugfp = NULL;
static	FILE *stdtty = NULL;
static	char debugfn[LOGFLEN];

/*
 * open_debug(subdir, flag)  char *subdir;  int flag;
 *
 * Sets the variable "debugfp" for the use of debug output.  It is set
 * to the upper cased filename of the given subdirectory of LOGSDIR.
 *
 * In addition, we set the variable "stdtty" to "stderr" if flag is not
 * set and we are pretty certain this program is being executed from a tty.
 */

open_debug(subdir, flag)
char *subdir;
int flag;
{
	static char updir[DIRSIZ+1];
	register char *cp;

	strncpy(updir, subdir, DIRSIZ);
	updir[DIRSIZ] = '\0';
	for (cp=&updir[0]; *cp; cp++)
		if ( islower(*cp) )
			*cp = toupper(*cp);

	sprintf(debugfn, "%s/%s/%.*s/%s", SPOOLDIR, LOGSDIR, DIRSIZ,
							subdir, updir);
	if ( (debugfp=fopen(debugfn, "a")) == NULL )
		fatal("Can't open debug log file: %s", debugfn);
	fseek(debugfp, 0L, 2);
	if ( !flag && ( isatty(fileno(stdin)) ||
			isatty(fileno(stdout)) ||
			isatty(fileno(stderr)) ) ) {
		stdtty = stderr;
	}
}

close_debug()
{
	if ( (debugfp!=stderr) && (debugfp!=NULL) ) {
		printmsg(M_DEBUG, "Closing Debug Log File: %s", debugfn);
		fclose(debugfp);
	}
	debugfp = NULL;
}

/*
 * |printmsg(level, args, ...)| prints an error or debugging message
 * into the system error log file.  If not remote, also print to standard
 * error.  All messages at levels less than or equal to the current
 * |debuglevel| are printed.
 */

printmsg(level, args)
register int level;
{
	register FILE *fp;

	if ( level <= debuglevel ) {
		fp = (debugfp != NULL) ? debugfp: stderr;
		fseek(fp, 0L, 2);
		fprintf(fp, "%r\n", &args);
		fflush(fp);
		if ( (stdtty!=NULL) && (fp!=stdtty) ) {
			fseek(stdtty, 0L, 2);
			fprintf(stdtty, "%r\n", &args);
			fflush(stdtty);
		}
	}
}
