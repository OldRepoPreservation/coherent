/*
 *  getseq.c
 *
 *  Support for sequence numbering in UUCP
 */

#include <stdio.h>
#include "dcp.h"

#define SPOOLSQD	"/usr/spool/uucp/.Sequence"
#define	MAXLOCKTRY	5
#define	SLEEPTIME	5

/*
 *  Gets the next sequence number (with locking) from the sequence
 *  subdirectory for the given system.  This returns a number "seq"
 *  with  0 <= seq < 10000
 */

getseq(system)
char *system;
{
	char	locknm[SITESIG+3];
	char	seqfn[CTLFLEN];
	char	buf[10];
	int	seq;
	FILE	*sfp;

	sprintf(locknm, "%.*s.S", SITESIG, system);
	sprintf(seqfn, "%s/%.*s", SPOOLSQD, SITESIG, system);
	seq = 0;
	while ( lockit(locknm) < 0 ) {
		sleep(SLEEPTIME);
		if (++seq > MAXLOCKTRY)
			fatal("Lock File timeout on: %s", seqfn);
	}

	if ( (sfp=fopen(seqfn, "r+")) == NULL ) {
		if ( (sfp=fopen(seqfn, "w+")) == NULL ) {
			lockrm(locknm);
			fatal("Error using sequence file %s", seqfn);
		}
		seq = 1;
	} else {
		fgets(buf, sizeof buf, sfp);
		seq = (atoi(buf) + 1) % 10000;
		rewind(sfp);
	}

	fprintf(sfp, "%d\n", seq);
	fclose(sfp);
	lockrm(locknm);
	return( seq );
}
