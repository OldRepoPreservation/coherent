/*
 *  knowhost.c
 *
 *  Scan the Systems file (L.sys) to determine if a host name is known.
 */

#include "lsys.h"

/*
 * knowhost(host)  char *host;
 *
 * Returns:  (1)  if host is known.
 *	     (0)  otherwise.
 */

knowhost(host)
char *host;
{
	int found = 0;

	lsys_open();
	while ( !found && (lsys_next() > 0) )
		found = !strcmp(host, lsys_value(sys_e));
			
	lsys_close();
	return( found );
}
