/*
 *  exec.c
 *
 *  Support functions to execute UUCP programs in the background.
 */

#include <stdio.h>

#define CICO	"/usr/lib/uucp/uucico"
#define XQT	"/usr/lib/uucp/uuxqt"

exec_cico()
{
	if ( fork() == 0) {
		execl(CICO, "memeCICO", "-r1", "-sany", NULL);
		exit(0);
	}
	return(0);
}


exec_xqt()
{
	if ( fork() == 0) {
		execl(XQT, "memeXQT", NULL);
		exit(0);
	}
	return(0);
}
