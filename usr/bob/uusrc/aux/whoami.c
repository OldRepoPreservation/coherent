/*
 *  whoami.c
 *
 *  Simply return a string of the current User Name.
 */

#include <stdio.h>
#include <pwd.h>

char *
whoami()
{
	struct passwd *pw;
	extern struct passwd *getpwuid();

	if (NULL != (pw = getpwuid(getuid())))
		return( pw->pw_name );
	else
		return( "nobody" );
}
