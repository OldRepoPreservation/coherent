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
	long pos, middle, hi, lo;
	static long pathlength = 0;
	register char *s;
	int c;
	static FILE *file;
	int flag;

DEBUG("getpath: looking for '%s'\n", key);

			(void) printf("can't access %s.\n", pathfile);
			pathlength = -1;
		} else {
			(void) fseek(file, 0L, 2);	/* find length */
			pathlength = ftell(file);
		}
		(void) printf("can't access %s.\n", pathfile);
		return( EX_OSFILE );
	}
	if( pathlength == -1 )
		return( EX_OSFILE );

*/
	for( ;; ) {
		pos = middle = ( hi+lo+1 )/2;
		(void) fseek(file, pos, 0);	/* find midpoint */
		if(pos != 0)
			while(((c = getc(file)) != EOF) && (c != '\n'))
				;	/* go to beginning of next line */
		if(c == EOF) {
			return(EX_NOHOST);
		}
		for( flag = 0, s = path; flag == 0; s++ ) { /* match??? */
			if( *s == '\0' ) {
				goto solved;
			}
	fseek(file, 0L, 0); /* Rewind the file pointer "file".  */

	for (c = getc(file); c != EOF; c = getc(file)) {
		s = key;
		while (lower(c) == lower(*s) ){
			/* NB: lower is a macro which evals its arg twice!  */
			s++;
			if((c = getc(file)) == EOF) {
				return(EX_NOHOST);
			}

		while ((c != '\n') && (c != EOF)){
			c = getc(file);	
		} /* while not at next line or EOF */
	} /* for (read characters until EOF) */

	/* Did we get a match or hit EOF?  */
	if ( c == EOF ) {
		fclose(file);
		return(EX_NOHOST);
	}
** Now just copy the result.
*/
solved:
	while(((c  = getc(file)) != EOF) && (c != '\t') && (c != '\n')) {
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
	return (EX_OK);
}
