/*
 *	Definitions and declarations for make - Created due to the offended 
 *	sensitivities of all MWC, 1-2-85
 */

#include <stdio.h>
#include <sys/types.h>
#include <time.h>
#include <ar.h>
#include <sys/stat.h>
#include <canon.h>
#include <path.h>

/*
 *	Make exit codes.
 */

#define	ALLOK	0	/* all ok, if -q option then all uptodate */
#define	ERROR	1	/* something went wrong */
#define	NOTUTD	2	/* with -q option, something is not uptodate */


#define	TRUE	(0 == 0)
#define	FALSE	(0 != 0)
#define	EOS	0200
#define	NUL	'\0'

/*
 * types
 */
#define	T_UNKNOWN	0
#define	T_NODETAIL	1
#define	T_DETAIL	2
#define	T_DONE		3

#define	NBACKUP	2048
#define	NMACRONAME	48
#define	NTOKBUF	100

#define	Streq(a,b)	(strcmp(a,b) == 0)

#define	REL	1	/* lseek argument for relative position */
#define	READ	0	/* open argument for reading */

extern char *getenv();
extern char *strcat();
extern char *strcpy();
extern char *strncpy();
extern char *malloc();
extern char *realloc();
extern char *index();
extern char *rindex();

#if MSDOS || GEMDOS
#define PATHSEP	'\\'
#define MACROFILE "mmacros"
#define ACTIONFILE  "mactions"
#endif
#if COHERENT
#define PATHSEP	'/'
#define MACROFILE "makemacros"
#define ACTIONFILE "makeactions"
#endif

