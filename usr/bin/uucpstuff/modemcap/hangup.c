/*
 *  hangup.c
 */

#include "dcp.h"
#include "modemcap.h"

hangup (fd)
int	fd;
{
	if ( (HU == NULL) && (HC == 0) ) {
		undial(fd);
		return(0);
	}
	sleep (3);
	if (AT != (char *) 0) {
		write (fd, AT, strlen (AT));
		if (AD)
			sleep (AD);
	}
	if (HU) {
		if (CS) {
			write (fd, CS, strlen (CS));
		}
		write (fd, HU, strlen (HU));
		if (CE) {
			write (fd, CE, strlen (CE));
		}
		if (IS) {
			write (fd, IS, strlen (IS));
			if (ID)
				sleep (ID);
		}
		undial (fd);
		return (1);
	}
	undial(fd);
	ttyexit(fd);
	return(1);
}
