/*
 * uucpname.c 
 *
 * |uucpname()| returns a pointer to the local host's UUCP nodename.
 * There are several possible means of determining this, depending
 * on the operating system version. For now, this version just reads
 * one line from the |NODENAME| file, which is usually either "/etc/cpu"
 * or "/etc/uucpname".
 */

#include <stdio.h>
#include "dcp.h"

#define NODENAME	"/etc/uucpname"		/* File with UUCP nodename */

char *
uucpname()
{
	register FILE *fp;
	register char *tmp;
	static char uuname[SITELEN+1];

	if ( (fp=fopen(NODENAME, "r")) == NULL )
		return( "" );
	fgets(uuname, SITELEN, fp);
	fclose(fp);

	tmp = &uuname[strlen(uuname) - 1];
	if ( *tmp == '\n' )
		*tmp = '\0';
	return( &uuname[0] );
}
