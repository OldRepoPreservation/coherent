#ifndef lint
static char 	*sccsid="@(#)getpath.c	2.5 (smail) 9/15/87";
#endif

# include	<stdio.h>
# include	<sys/types.h>
# include	<ctype.h>
# include	"defs.h"

extern enum edebug debug;	/* how verbose we are 		*/ 
extern char *pathfile;		/* location of path database	*/

/*
**
** getpath(): look up key in ascii sorted path database.
**
*/

getpath( key, path , cost)
char *key;		/* what we are looking for */
char *path;		/* where the path results go */
int *cost;		/* where the cost results go */
{
	register char *s;
	int c;
	static FILE *file;

DEBUG("getpath: looking for '%s'\n", key);

	if((file = fopen(pathfile, "r")) == NULL) {
		(void) printf("can't access %s.\n", pathfile);
		return( EX_OSFILE );
	}

	/* Linear search for key "path" in file stream "file".  */
	fseek(file, 0L, 0); /* Rewind the file pointer "file".  */

	for (c = getc(file); c != EOF; c = getc(file)) {
		s = key;
		while (lower(c) == lower(*s) ){
			/* NB: lower is a macro which evals its arg twice!  */
			s++;
			if((c = getc(file)) == EOF) {
				fclose(file);
				return(EX_NOHOST);
			}
		} /* while (lower(c) == lower(*s++)) */

		if (*s == '\0') {
			if ((c == '\t') || (c == ' ')){
				break;
			} /* if found seperator character */
		} /* if key hit end of string */

		while ((c != '\n') && (c != EOF)){
			c = getc(file);	
		} /* while not at next line or EOF */
	} /* for (read characters until EOF) */

	/* Did we get a match or hit EOF?  */
	if ( c == EOF ) {
		fclose(file);
		return(EX_NOHOST);
	}

	while(((c = getc(file)) != EOF) && (c != '\t') && (c != '\n')) {
		*path++ = c;
	}
	*path = '\0';
/*
** See if the next field on the line is numeric.
** If so, use it as the cost for the route.
*/
	if(c == '\t') {
		int tcost = -1;
		while(((c = getc(file)) != EOF) && isdigit(c)) {
			if(tcost < 0) tcost = 0;
			tcost *= 10;
			tcost += c - '0';
		}
		if(tcost >= 0) *cost = tcost;
	}
	fclose(file);
	return (EX_OK);
}
