/*
 *  ckdir.c
 *
 *  Check for the existence of the given directory.  If not there, then
 *  try to "uumkdir" it.  Returns: 1 all ok, or 0 error.
 */

#include <stdio.h>
#include <sys/stat.h>

ckdir(dname)
char *dname;
{
	struct stat statbuf;
	char mkdircmd[BUFSIZ];

	if (stat(dname, &statbuf) == -1) {
		sprintf(mkdircmd, "/usr/lib/uucp/uumkdir -m 0755 -p %s\n",
							 dname);
		if (system(mkdircmd) != 0)
			return(0);
	} 
	return(1);
}

