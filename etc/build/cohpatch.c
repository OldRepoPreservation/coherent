/*
 * cohpatch.c
 * 10/11/90
 * Uses common routines in build0.c: cc -o cohpatch cohpatch.c build0.c
 * Patch new copy of COHERENT for update installation.
 */

#include "build0.h"
#include <l.out.h>

extern	long	lseek();

#define	COH	"/coherent"
#define	COHNEW	"/coherent.new"
#define	COHOLD	"/coherent.old"
#define	KMEM	"/dev/kmem"
#define	NENTRIES	6

char	buf[256];		/* temporary buffer */
char	cmd[512];		/* command buffer */
int	kfd;			/* kernel file desciptor */
struct	nlist	nl[NENTRIES] = {
	{ "rootdev_", 0, 0 },
	{ "pipedev_", 0, 0 },
	{ "ronflag_", 0, 0 },
	{ "___", 0, 0 },
	{ "_entry_", 0, 0 },
	{ "", 0, 0 }
};
int	sizes[NENTRIES] = {		/* sizes of preceding, yuk */
		sizeof (unsigned int),
		sizeof (unsigned int),
		sizeof (unsigned int),
		sizeof (unsigned long),
		sizeof (unsigned long),
		0
};

extern	void	kread();

main(argc, argv) int argc; char *argv[];
{
	register int i;
	unsigned int uval;
	unsigned long ulval;

	argv0 = argv[0];
	if ((kfd = open(KMEM, 0)) < 0)
		fatal("cannot open \"%s\"", KMEM);
	sprintf(cmd, "/conf/patch %s ", COHNEW);
	nlist(COH, nl);
	for (i = 0; i < NENTRIES-1; i++) {
		if (nl[i].n_type == 0)
			fatal("cannot find symbol \"%s\"", nl[i].n_name);
		if (sizes[i] == sizeof (unsigned int)) {
			kread(nl[i].n_value, &uval, sizes[i]);
			sprintf(buf, "%s=%u ", nl[i].n_name, uval);
		} else if (sizes[i] == sizeof (unsigned long)) {
			kread(nl[i].n_value, &ulval, sizes[i]);
			sprintf(buf, "%s=%lu:l ", nl[i].n_name, ulval);
		}
		strcat(cmd, buf);
	}
	sys(cmd, S_FATAL);
	sprintf(cmd, "/bin/mv %s %s", COH, COHOLD);
	sys(cmd, S_FATAL);
	sprintf(cmd, "/bin/mv %s %s", COHNEW, COH);
	sys(cmd, S_FATAL);
	close(kfd);
	exit(0);
}

void
kread(useek, bp, n) unsigned int useek; char *bp; int n;
{
	if (lseek(kfd, (long)useek, 0) == -1L)
		fatal("seek error on \"%s\"", KMEM);
	if (read(kfd, bp, n) != n)
		fatal("read error on \"%s\"", KMEM);
}

/* end of cohpatch.c */
