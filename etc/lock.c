/*
 *  lock.c
 *
 *  Provide a locking mechanism for UUCP
 */

#include <stdio.h>
#include <access.h>

#define LOCK	"/usr/spool/uucp/LCK.."

static	char	lockfile[80];

char *
lockit(resource)
char *resource;
{
	int lockfd;

	strcpy(lockfile, LOCK);
	strcat(lockfile, resource);
	if ( (access(lockfile, AEXISTS) == 0) ||
	     ((lockfd=creat(lockfile, 0)) == -1) )
		return( NULL );
	close(lockfd);
	return( lockfile );
}

/* Lock removal routine symetric with lockit() */
unlockit(resource)
char *resource;
{
	strcpy(lockfile, LOCK);
	strcat(lockfile, resource);
	lockrm(lockfile);
}

lockrm(the_lockfile)
char	*the_lockfile;
{
	if (*the_lockfile != '\0')
		unlink(the_lockfile);
}

/*
 *	locktry
 *	if the lock cannot be written, return 1
 */
int
lockexist(resource)
char	*resource;
{
	strcpy(lockfile, LOCK);
	strcat(lockfile, resource);
	if (access(lockfile, AEXISTS) == 0)
		return 1;
	return 0;
}
