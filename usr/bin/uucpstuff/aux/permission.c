/*
 *  permission.c
 *
 *  Parse the UUCP Permissions file:  /usr/lib/uucp/Permissions
 */

#include <stdio.h>
#include "dcp.h"
#include "perm.h"

#define	LINELEN	(3 * BUFSIZ)

extern	char *index();
extern	char *strstr();
static	char linebuf[LINELEN];

/*
 *  The following arrays kw_txt[], default_val[], and kw_ptr[]
 *  are indexed via PERM_E, defined in perm.h
 */

static	char *kw_txt[] = { 
	"LOGNAME",
	"MACHINE",
	"REQUEST",
	"SENDFILES",
	"READ",
	"WRITE",
	"NOREAD",
	"NOWRITE",
	"CALLBACK",
	"COMMANDS",
	"VALIDATE",
	"MYNAME",
	"PUBDIR"
 };

static	char *default_val[] = { 
	LOGNAME_D,
	MACHINE_D,
	REQUEST_D,
	SENDFILES_D,
	READ_D,
	WRITE_D,
	NOREAD_D,
	NOWRITE_D,
	CALLBACK_D,
	COMMANDS_D,
	VALIDATE_D,
	MYNAME_D,
	PUBDIR_D
};

static	char *kw_ptr[PERMLAST_e];	/* Parsed fields from PERMS	*/
static	int perminit = 0;		/* Successful perm_get() call	*/

/*
 *  perm_get()
 *
 *  Parse the PERMS file using the "machine" and the "logname" as the
 *  index field.  Note: Exactly one of "machine" and "logname" will be
 *  non-NULL, and only that field will act as the index.
 *
 *  This function will set up the "kw_ptr[]" array which will serve as
 *  the list referenced by calls to perm_value().
 *
 *  This function will return successfully or else fatal error.
 */

char *defaultmsg = "Supplying default permissions for %s name: %s";

perm_get(machine, logname)
char *machine, *logname;
{
	FILE *fp;
	char *txtstr, *valstr;
	int val;
	PERM_E i;

	if ( (machine != NULL) && (logname != NULL) )
		fatal("Bad function call: perm_get");

	if ((fp=fopen(PERMS, "r")) == NULL)
		fatal("Can't open permissions file: %s", PERMS);

	if ( machine != NULL ) {
		txtstr = kw_txt[machine_e];
		valstr = machine;
	} else {
		txtstr = kw_txt[logname_e];
		valstr = logname;
	}
	
	kw_ptr[logname_e] = logname;
	kw_ptr[machine_e] = machine;
	for (i=request_e; i<PERMLAST_e; i++)
		kw_ptr[i] = default_val[i];

	while ( (val=getline(fp, linebuf, LINELEN)) != 0 ) {
		if ( val < 0 )
			fatal("Permissions line too long: {%s}", linebuf);
		if ( (val=perm_line(txtstr, valstr)) == 0 )
			continue;
		break;
	}
	fclose(fp);
	if ( val < 0 )
		fatal("Syntax error in Permissions Line: {%s}", linebuf);
	if ( val == 0 ) {
		if ( machine != NULL ) {
			if ( strcmp(machine, "OTHER") != 0 ) {
				perm_get("OTHER", NULL);
				plog(M_INFO, defaultmsg, "machine", machine);
			}
		} else
			plog(M_INFO, defaultmsg, "login", logname);
	}
	perminit = 1;
}

/*
 *  perm_line() searches through the string linebuf[] for a field matching
 *  "txtstr=valstr" (e.g. "MACHINE=mwc"  or  "LOGNAME=uucp").  If it finds
 *  a match, then it parses the entire string and sets the results in the
 *  array kw_ptr[].  Return values are (1) for match and parsing success,
 *  (0) for no match, and (-1) for syntax error in linebuf[].
 *
 *  Note, the syntax of linebuf[] is a string containing a SINGLE-blank
 *  separated set of "option=value" pairs.  The "value" section of any
 *  particular pair may contain colons (:) to separate multiple values.
 *  As such, if we call perm_line("MACHINE", "mwc")  where
 *  linebuf[] = "MACHINE=a:mwc:b", then this would be considered a match.
 */

static
perm_line(txtstr, valstr)
char *txtstr, *valstr;
{
	char *cp, ch;
	char *col, *spc, *eql;
	PERM_E i;
	int len = strlen(valstr);

	printmsg(M_DEBUG, "Permissions Line: {%s}", linebuf);

	if ( (cp=strstr(linebuf, txtstr)) == NULL )
		return(0);

	cp += strlen(txtstr);
	if ( *cp++ != '=' )
		return(-1);

	while ( (strncmp(cp, valstr, len) != 0) ||
	        (((ch=*(cp+len)) != ':') && (ch != ' ') && (ch != '\0')) )  {

		if ( ((col=index(cp, ':')) == NULL) ||
		     (((spc=index(cp, ' ')) != NULL) && (col > spc)) )
			return(0);
		cp = col+1;
	}

	for (cp=linebuf; (eql=index(cp, '=')) != NULL; cp=spc+1) {
		*eql++ = '\0';
		for (i=0; i<PERMLAST_e; i++)
			if ( strcmp(cp, kw_txt[i]) == 0 ) {
				kw_ptr[i] = eql;
				break;
			}
		if ( i >= PERMLAST_e )
			return(-1);
		if ( (spc=index(eql, ' ')) != NULL )
			*spc = '\0';
	}

	printmsg(M_INFO, "Matched %s = %s", txtstr, valstr);
	for (i=0; i<PERMLAST_e; i++)
		printmsg(M_INFO, "\t%-9s = %s", kw_txt[i],
					kw_ptr[i] != NULL ? kw_ptr[i]: "");
	return(1);
}	

/*
 *  perm_value() returns the field associated with the desired keyword
 *  as specified by the KEYWORD_E argument.  See "perm.h" for more info.
 */

char *
perm_value(val)
PERM_E val;
{
	if ( !perminit ) {
		printmsg(M_DEBUG, "perm_value() called too early.");
		return( NULL );
	}

	if ( val >= PERMLAST_e )
		fatal("perm_value arg too big: %d", val);

	printmsg(M_DEBUG, "perm_value( %s ) = %s", kw_txt[val],
				kw_ptr[val] != NULL ? kw_ptr[val]: "");
	return( kw_ptr[val] );
}
