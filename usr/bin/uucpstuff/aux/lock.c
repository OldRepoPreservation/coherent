/*
 *  lock.c
 *
 *  Provide a locking mechanism for UUCP
 */

#include <stdio.h>
#include <access.h>
#include "dcp.h"

#define LOCKDIR	SPOOLDIR
#define	LOCKPRE	"LCK.."

/*
 *  lockit(resource)  char *resource;
 *
 *  Lock the given resource.
 *  Returns (-1) if already locked or error in locking.
 *          ( 0) if all ok, resource locked.
 */

lockit(resource)
char *resource;
{
	int lockfd;
	char lockfn[LOKFLEN];

	sprintf(lockfn, "%s/%s%.*s", LOCKDIR, LOCKPRE, LOCKSIG, resource);
	if ( (access(lockfn, AEXISTS) == 0) ||
	     ((lockfd=creat(lockfn, 0)) == -1) ) {
		printmsg(M_DEBUG, "Can't lock: %s", lockfn);
		return( -1 );
	}
	printmsg(M_DEBUG, "Just locked: %s", lockfn);
	close(lockfd);
	return( 0 );
}

/*
 *  lockrm(resource)  char *resource;
 *
 *  Simply remove the lock on the given resource.
 *  Returns (-1) if not locked or error in unlocking.
 *          ( 0) if all ok, resource lock removed.
 */

lockrm(resource)
char *resource;
{
	char lockfn[LOKFLEN];

	if ( resource == NULL )
		return( 0 );
	sprintf(lockfn, "%s/%s%.*s", LOCKDIR, LOCKPRE, LOCKSIG, resource);
	if ( unlink(lockfn) < 0 ) {
		printmsg(M_DEBUG, "Error unlocking: %s", lockfn);
		return( -1 );
	} else {
		printmsg(M_DEBUG, "Just unlocked: %s", lockfn);
		return( 0 );
	}
}

/*
 *  lockexist(resource)  char *resource;
 *
 *  Test for existance of a lock on the given resource.
 *
 *  Returns:  (1)  Resource is locked.
 *	      (0)  Resource is not locked.
 */

lockexist(resource)
char	*resource;
{
	char lockfn[LOKFLEN];

	if ( resource == NULL )
		return(0);
	sprintf(lockfn, "%s/%s%.*s", LOCKDIR, LOCKPRE, LOCKSIG, resource);
	return( !access(lockfn, AEXISTS) );
}
