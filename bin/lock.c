/*
 *  lock.c
 *
 *  Provide a locking mechanism for UUCP
 */

#include <stdio.h>
#include <access.h>

#ifdef UUCP
#include "dcp.h"
#else
#define SPOOLDIR	"/usr/spool/uucp"
#define LOCKSIG		9	/* Significant Chars of Lockable Resources.  */
#define LOKFLEN		64	/* Max Length of UUCP Lock File Name.	     */
#endif /* UUCP */

#define LOCKDIR	SPOOLDIR
#define	LOCKPRE	"LCK.."
#define PIDLEN	6	/* Maximum length of string representing a PID.  */

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
	char pidstring[PIDLEN];

	sprintf(lockfn, "%s/%s%.*s", LOCKDIR, LOCKPRE, LOCKSIG, resource);
	if ( (access(lockfn, AEXISTS) == 0) ||
	     ((lockfd=creat(lockfn, 0644)) == -1) ) {
#ifdef UUCP
		printmsg(M_DEBUG, "Can't lock: %s", lockfn);
#endif /* UUCP */
		return( -1 );
	}
#ifdef UUCP
	printmsg(M_DEBUG, "Just created lock: %s", lockfn);
#endif /* UUCP */
	sprintf(pidstring, "%d", getpid());
	write(lockfd, pidstring, PIDLEN);
	close(lockfd);
	return( 0 );
}

/*
 *  lockrm(resource)  char *resource;
 *
 *  Simply remove the lock on the given resource.
 *  Returns (-1) if not locked or error in unlocking.
 *          ( 0) if all ok, resource lock removed.
 *
 *
 * Open the lock file for read operations to try to read the PID
 * stored in the file. If the open fails, abort. If the read fails, 
 * abort. If the read PID does not match our PID, abort. We will only
 * remove the lock if our PID matches the PID written to the file.
*/

lockrm(resource)
char *resource;
{
	FILE *lockfd;	/* pointer to file to read */
	char gotpid[PIDLEN];	/* integer value of the PID that should be stored
			 * in the lock file pointed to by *lockfd.
			*/
	char lockfn[LOKFLEN];
	if ( resource == NULL )
		return( 0 );
	sprintf(lockfn, "%s/%s%.*s", LOCKDIR, LOCKPRE, LOCKSIG, resource);


	/* open the lock file for read, abort on failure */
	if( (lockfd = (fopen(lockfn, "r"))) == NULL){
#ifdef UUCP
		printmsg(M_DEBUG, "Error opening lock file for PID verify");
		plog(M_CALL, "Error opening lock file for PID verify");
#endif /* UUCP */
		return(-1);
	}


	/* read the contents of the file. Abort if empty */
	if ( fgets(gotpid, PIDLEN, lockfd) == NULL){
#ifdef UUCP
		printmsg(M_DEBUG, "Lockrm: Error reading lock file for PID verify");
		plog(M_CALL, "Lockrm: Error reading lock file for PID verify");
#endif /* UUCP */
		return(-1);
	}

	if (atoi(gotpid) != getpid()){
#ifdef UUCP
		printmsg(M_DEBUG, "Lockrm: PID verify failed. PID read was %s.", 
			gotpid);
		plog(M_CALL, "Lockrm: PID verify failed. PID read was %s", gotpid);
#endif /* UUCP */
		return(-1);
	}else{
#ifdef UUCP
		printmsg(M_DEBUG, "Lockrm: PID verify successful, removing lock.");
		plog(M_CALL, "Lockrm: PID verify successful, removing lock.");
#endif /* UUCP */

		if ( unlink(lockfn) < 0 ) {
#ifdef UUCP
			printmsg(M_DEBUG, "Lockrm: Error unlocking: %s", lockfn);
			plog(M_CALL, "Lockrm: Error unlocking: %s", lockfn);
#endif /* UUCP */
			return( -1 );
		}
#ifdef UUCP
		printmsg(M_DEBUG, "Just unlocked: %s", lockfn);
#endif /* UUCP */
		return( 0 );
	}
}


/*
 *  locknrm(resource, pid)  char *resource;
 *
 *  Remove the lock on the given resource, using pid as the process id to
 *  look for.
 *
 *  Returns (-1) if not locked or error in unlocking.
 *          ( 0) if all ok, resource lock removed.
 *
 *  Open the lock file for read operations to try to read the PID
 *  stored in the file. If the open fails, abort. If the read fails, 
 *  abort. If the read PID does not match our PID, abort. We will only
 *  remove the lock if the passed matches the PID written to the file.
 */

locknrm(resource, pid)
	char *resource;
	int pid;
{
	FILE *lockfd;	/* pointer to file to read */
	char gotpid[PIDLEN];	/* integer value of the PID that should be stored
			 	 * in the lock file pointed to by *lockfd.
				 */
	char lockfn[LOKFLEN];
	if ( resource == NULL )
		return( 0 );
	sprintf(lockfn, "%s/%s%.*s", LOCKDIR, LOCKPRE, LOCKSIG, resource);


	/* open the lock file for read, abort on failure */
	if( (lockfd = (fopen(lockfn, "r"))) == NULL){
#ifdef UUCP
		printmsg(M_DEBUG, "Error opening lock file for PID verify");
		plog(M_CALL, "Error opening lock file for PID verify");
#endif /* UUCP */
		return(-1);
	}


	/* read the contents of the file. Abort if empty */
	if ( fgets(gotpid, PIDLEN, lockfd) == NULL) {
#ifdef UUCP
		printmsg(M_DEBUG, "Lockrm: Error reading lock file for PID verify");
		plog(M_CALL, "Lockrm: Error reading lock file for PID verify");
#endif /* UUCP */
		return(-1);
	}

	if (atoi(gotpid) != pid){
#ifdef UUCP
		printmsg(M_DEBUG, "Lockrm: PID verify failed. PID read was %s.", 
			gotpid);
		plog(M_CALL, "Lockrm: PID verify failed. PID read was %s", gotpid);
#endif /* UUCP */
		return(-1);
	}else{
#ifdef UUCP
		printmsg(M_DEBUG, "Lockrm: PID verify successful, removing lock.");
		plog(M_CALL, "Lockrm: PID verify successful, removing lock.");
#endif /* UUCP */

		if ( unlink(lockfn) < 0 ) {
#ifdef UUCP
			printmsg(M_DEBUG, "Lockrm: Error unlocking: %s", lockfn);
			plog(M_CALL, "Lockrm: Error unlocking: %s", lockfn);
#endif /* UUCP */
			return( -1 );
		}
#ifdef UUCP
		printmsg(M_DEBUG, "Just unlocked: %s", lockfn);
#endif /* UUCP */
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

