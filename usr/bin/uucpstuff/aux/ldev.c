/*
 *  ldev.c
 *
 *  Parse the UUCP Devices file:  /usr/lib/uucp/L-devices
 */

#include <stdio.h>
#include <access.h>
#include "dcp.h"
#include "ldev.h"

#define	LINELEN	(2 * BUFSIZ)

extern	char *index();
static	char linebuf[LINELEN];

static	char *kw_txt[] = {		/* Indexed by LDEV_E		*/
	"Connection Type",
	"Local Line",
	"Remote Line",
	"Baud Rate",
	"Brand"
};

static	char *ldev_ptr[LDEVLAST_e];	/* Parsed fields from LDEV	*/

static	FILE *ldevfp = NULL;		/* Global LDEV file pointer	*/
static	int ldevinit = 0;		/* Successful call ldev_next()	*/

/*
 *  Parsing support routines for the LDEV uucp devices file.
 *
 *  ldev_open()  :	Open LDEV data file
 *  ldev_close() :	Close LDEV data file
 *  ldev_next()  :	Parse the next logical line from the LDEV file
 *  ldev_value() :	Return the parsed value from most recent ldev_next()
 */

ldev_open()
{
	if ( ldevfp != NULL )
		ldev_close();
	if ((ldevfp=fopen(LDEV, "r")) == NULL)
		fatal("Can't open Devices file: %s", LDEV);
}

ldev_close()
{
	if ( ldevfp != NULL )
		fclose(ldevfp);
	ldevfp = NULL;
}

/*
 *  Parses the 'next' entry from the LDEV file.  
 *  Return value is '1' for success, and '0' for eof.
 */

ldev_next()
{
	int val;
	LDEV_E i;

	if ( ldevfp == NULL )
		fatal("Bad function call: ldev_next");

	for (i=0; i<LDEVLAST_e; i++)
		ldev_ptr[i] = NULL;

	if ( (val=getline(ldevfp, linebuf, LINELEN)) <= 0 ) {
		if ( val < 0 )
			fatal("Devices line too long: {%s}", linebuf);
		return( 0 );
	}
	printmsg(M_DEBUG, "Devices Line: {%s}", linebuf);

	if ( !ldev_line() )
		fatal("Syntax error in Devices Line: {%s}", linebuf);

	if ( strcmp(ldev_ptr[type_e], "ACU") &&
	     strcmp(ldev_ptr[type_e], "DIR") )
		fatal("Unknown device type in Devices File: {%s}",
						 ldev_ptr[type_e]);

	printmsg(M_INFO, "Parsed Devices Entry:");
	for (i=0; i<LDEVLAST_e; i++)
		printmsg(M_INFO, "\t%-11s = %s", kw_txt[i],
				ldev_ptr[i] != NULL ? ldev_ptr[i]: "");
	ldevinit = 1;
	return( 1 );
}

/*
 *  ldev_line() parses the string linebuf[] into the LDEV_E fields as
 *  specified in "ldev.h" and fills the arrays ldev_ptr[].
 *  Return values are (0) for syntax error in linebuf[], or (1) for success.
 */

static
ldev_line()
{
	register char *cp;
	LDEV_E i;

	cp = &linebuf[0];
	for (i=type_e; i<=brand_e; i++) {
		ldev_ptr[i] = cp;
		if ( (cp=index(cp, ' ')) == NULL )
			return ( (i==brand_e) ? 1: 0 );
		*cp++ = '\0';
	}
	return ( *cp == '\0' );
}	

/*
 *  ldev_value() returns the field associated with the desired keyword
 *  as specified by the LDEV_E argument.  See "ldev.h" for more info.
 */

char *
ldev_value(val)
LDEV_E val;
{
	if ( !ldevinit ) {
		printmsg(M_DEBUG, "ldev_value() called too early");
		return( NULL );
	}

	if ( val >= LDEVLAST_e )
		fatal("ldev_value arg too big: %d", val);

	printmsg(M_DEBUG, "ldev_value( %s ) = %s", kw_txt[val],
				ldev_ptr[val] != NULL ? ldev_ptr[val]: "");
	return( ldev_ptr[val] );
}
