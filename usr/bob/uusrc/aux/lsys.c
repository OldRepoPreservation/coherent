/*
 *  lsys.c
 *
 *  Parse the UUCP Systems file:  /usr/lib/uucp/L.sys
 */

#include <stdio.h>
#include <access.h>
#include "dcp.h"
#include "lsys.h"

#define	LINELEN	(2 * BUFSIZ)

extern	char *index();
static	char linebuf[LINELEN];

static	char *kw_txt[] = {		/* Indexed by LSYS_E		*/
	"System Name",
	"Schedule",
	"Device",
	"Speed",
	"Phone"
};

static	char *lsys_ptr[LSYSLAST_e];	/* Parsed fields from LSYS	*/
static	char *expect[MAXCHAT];		/* Expect strings from LSYS	*/
static	char *send[MAXCHAT];		/* Send strings from LSYS	*/

static	FILE *lsysfp = NULL;		/* Global LSYS file pointer	*/
static	int numchat = -1;		/* Number of parsed chats	*/
static	int lsysinit = 0;		/* Successful call lsys_next()	*/

/*
 *  Parsing support routines for the LSYS uucp systems file.
 *
 *  lsys_access():	Return (1) if read access to LSYS, otherwise (0).
 *  lsys_open()  :	Open LSYS data file
 *  lsys_close() :	Close LSYS data file
 *  lsys_next()  :	Parse the next logical line from the LSYS file
 *  lsys_value() :	Return the parsed value from most recent lsys_next()
 *  lsys_expect():	Return Expect Strings from most recent lsys_next()
 *  lsys_send()  :	Return Send Strings from most recent lsys_next()
 */

lsys_access()
{
	return( !access(LSYS, AREAD) );
}

lsys_open()
{
	if ( lsysfp != NULL )
		lsys_close();
	if ((lsysfp=fopen(LSYS, "r")) == NULL)
		fatal("Can't open Systems file: %s", LSYS);
}

lsys_close()
{
	if ( lsysfp != NULL )
		fclose(lsysfp);
	lsysfp = NULL;
}

/*
 *  Parses the 'next' entry from the LSYS file.  
 *  Return value is '1' for success, and '0' for eof.
 */

lsys_next()
{
	int val, chat;
	LSYS_E i;

	if ( lsysfp == NULL )
		fatal("Bad function call: lsys_next");

	numchat = -1;
	for (i=0; i<LSYSLAST_e; i++)
		lsys_ptr[i] = NULL;

	if ( (val=getline(lsysfp, linebuf, LINELEN)) <= 0 ) {
		if ( val < 0 )
			fatal("Systems line too long: {%s}", linebuf);
		return( 0 );
	}
	printmsg(M_DEBUG, "Systems Line: {%s}", linebuf);

	if ( (numchat=lsys_line()) < 0 )
		fatal("Syntax error in Systems Line: {%s}", linebuf);

	printmsg(M_INFO, "Parsed Systems Entry:");
	for (i=0; i<LSYSLAST_e; i++)
		printmsg(M_INFO, "\t%-11s = %s", kw_txt[i],
				lsys_ptr[i] != NULL ? lsys_ptr[i]: "");
	for (chat=0; chat<numchat; chat++) {
		printmsg(M_INFO, "\t Expect[%02d] = %s", chat, expect[chat]);
		printmsg(M_INFO, "\t   Send[%02d] = %s", chat, send[chat]);
	}
	lsysinit = 1;
	return( 1 );
}

/*
 *  lsys_line() parses the string linebuf[] into the LSYS_E fields as
 *  specified in "lsys.h" and fills the arrays lsys_ptr[], expect[], and
 *  send[].  Return values are (-1) for syntax error in linebuf[], or
 *  for success, the number of chat pairs parsed.
 */

static
lsys_line()
{
	register char *cp, *spc, *dsh;
	int numdash, chat;
	LSYS_E i;

	cp = &linebuf[0];
	for (i=sys_e; i<=phone_e; i++) {
		lsys_ptr[i] = cp;
		if ( (cp=index(cp, ' ')) == NULL )
			return ( (i==phone_e) ? 0: -1 );
		*cp++ = '\0';
	}

	if ( strlen(lsys_ptr[sys_e]) > SITELEN )
		fatal("Systems Sitename too long: %s", lsys_ptr[sys_e]);

	chat = 0;
	while ( (spc=index(cp, ' ')) != NULL ) {
		*spc++ = '\0';
		for (dsh=cp, numdash=0; (dsh=index(dsh, '-')) != NULL;
					numdash++, dsh++) ;
		if ( (numdash % 2) != 0 )
			fatal("Systems chat script dash count error: {%s}", cp);
		expect[chat] = cp;
		send[chat++] = spc;
		if ( (cp=index(spc, ' ')) == NULL )
			break;
		*cp++ = '\0';
	}
	return ( ((cp==NULL) || (*cp=='\0')) ? chat: -1 );
}	

/*
 *  lsys_value() returns the field associated with the desired keyword
 *  as specified by the LSYS_E argument.  See "lsys.h" for more info.
 */

char *
lsys_value(val)
LSYS_E val;
{
	if ( !lsysinit ) {
		printmsg(M_DEBUG, "lsys_value() called too early");
		return( NULL );
	}

	if ( val >= LSYSLAST_e )
		fatal("lsys_value arg too big: %d", val);

	printmsg(M_DEBUG, "lsys_value( %s ) = %s", kw_txt[val],
				lsys_ptr[val] != NULL ? lsys_ptr[val]: "");
	return( lsys_ptr[val] );
}

char *
lsys_expect(i)
int i;
{
	register char *ret;

	if ( !lsysinit ) {
		printmsg(M_DEBUG, "lsys_expect() called too early");
		return( NULL );
	}

	if ( i < 0 ) 
		fatal("lsys_expect arg neg: %d", i);

	ret = ( i >= numchat ) ? NULL: expect[i];
	printmsg(M_DEBUG, "lsys_expect( %d ) = %s", i, (ret!=NULL) ? ret: "");
	return( ret );
}

char *
lsys_send(i)
int i;
{
	register char *ret;

	if ( !lsysinit ) {
		printmsg(M_DEBUG, "lsys_send() called too early");
		return( NULL );
	}

	if ( i < 0 ) 
		fatal("lsys_send arg neg: %d", i);

	ret = ( i >= numchat ) ? NULL: send[i];
	printmsg(M_DEBUG, "lsys_send( %d ) = %s", i, (ret!=NULL) ? ret: "");
	return( ret );
}
