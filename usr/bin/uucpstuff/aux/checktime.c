/*
 *  checktime.c
 *
 *  Check if it is a good time to place a call, according the the
 *  L.sys uucp database systems file.
 */

#include <stdio.h>
#include <sys/types.h>
#include <time.h>
#include <ctype.h>
#include "dcp.h"

/*
 * checktime()
 * Check if we may make a call at this time.  The argument is the
 * schedule time string from the uucp systems database L.sys file.
 * Returns 1 if ok to call, and 0 otherwise.
 */

static	char *retstr[] = {
			   "Not Now",
			   "Ok to call now"
			 };

checktime(timestr)
char *timestr;
{
	register char *sp, *cp;
	int timeok = 0;

	sp = timestr;
	while ( !timeok ) {
		if ( (cp=index(sp, ',')) != NULL )
			*cp = '\0';
		if ( forcecall ) {
			timeok = strncmp(sp, "Never", 5) ? 1: 0;
			break;
		}
		if ( cktm(sp) )
			timeok = 1;
		if ( cp == NULL )
			break;
		*cp++ = ',';
		sp = cp;
	}
	printmsg(M_CALL, "checktime(%s) = %s%s", timestr, retstr[timeok],
				forcecall ? " -- Forced": "");
	return( timeok );
}

char	*daycodes = "SuMoTuWeThFrSaWk";

static
cktm(onetime)
char *onetime;
{
	int	i, dayok, wday, curtime, stop, start;
	char	*cp, *sp;
	time_t	now;
	struct	tm *local;

	if ( (onetime==NULL) || (strncmp(onetime, "Never", 5)==0) )
		return( 0 );
	time(&now);
	local = localtime(&now);
	wday = local->tm_wday;
	cp = onetime;
	dayok = 0;

	while ( *cp && *(cp+1) ) {
		if (strncmp(cp, "Any", 3) == 0) {
			cp += 3;
			dayok = 1;
			break;
		}
		for (i=0; i<=7; i++) {
			if (strncmp(cp, &daycodes[2*i], 2) == 0) {
				if ( (i==wday) || 
				     ( (i==7) && (wday >=1) && (wday <=5) ) ) {
					dayok = 1;
					break;
				}
			}
		}
		cp += 2;
		if ( dayok )
			break;
	}

	if ( !dayok )
		return( 0 );

	for (; *cp && !isdigit(*cp); cp++) ;
	if ( *cp == '\0' )
		return( 1 );

	for (sp=cp; isdigit(*sp); sp++) ;
	if ( (sp==cp) || (*sp!='-') )
		return( 0 );
	*sp = '\0';
	start = atoi(cp);
	*sp++ = '-';
	for (cp=sp; isdigit(*cp); cp++) ;
	if ( (cp==sp) || (*cp!='\0') )
		return( 0 );
	stop = atoi(sp);

	if ( (start > 2400) || (stop > 2400) )
		return( 0 );

	curtime = local->tm_hour * 100 + local->tm_min;

	return( ((stop>=start) && (curtime>=start) && (curtime<=stop)) ||
		((stop<=start) && ((curtime>=start) || (curtime<=stop))) );
}
