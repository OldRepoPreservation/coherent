/*
 *  exec.c
 *
 *  Support functions to execute UUCP programs in the background.
 */

#include <stdio.h>

#define CICO	"/usr/lib/uucp/uucico"
#define XQT	"/usr/lib/uucp/uuxqt"
#define CICO_CMD	"-s"


exec_cico(cmdsite)
char *cmdsite[40];

{
char site[43];

	strcpy(site,CICO_CMD);
	strcat(site,cmdsite);
	if ( fork() == 0) {
		execl(CICO, "memeCICO", "-r1", site, NULL);
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
