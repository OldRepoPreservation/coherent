/*
 * Module read all information about ipc.
 */
#include <errno.h>
#include <coff.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/msg.h>
#include "ipcs.h"

extern char	*calloc();

struct msqid_ds	*msqbuf;

/*
 * Get ipc and format ipc data
 */
int get_data(fname, corefile)
char	*fname;	/* namelist */
char	*corefile;
{
	if (mflag) {
		get_shmid_stats();
	}

	if (sflag) {
		get_sem_stats();
	}

	if (qflag) {
		get_msg_stats(fname, corefile);
	}

	return 0;
}


/* 
 * get shared memory information.
 *
 */

int get_shmid_stats()
{

int id = NSHMID -1;	/* shared memory identifier number */

	total_shmids = 0;

	/* loop through all shared memory segments available to system.
	 * increment total_shmid counter when we find one.
	 */

	for (id = NSHMID -1; id >= 0 ; id --){
		if( -1 != (shmctl(id, IPC_STAT, &shmid[id]))){
			total_shmids++;
			valid_shmid[id] = 1;
		}else{
			valid_shmid[id] = 0;
		}			
	}
}

/*
 * get_sem_stats() contains a loop that performs semctls to IPC_STAT
 * allocated semaphores.
 *
 */

get_sem_stats()
{

int id;
	total_sems = 0;

	for (id = NSEMID -1 ; id >= 0 ; id--){
		if( -1 != (semctl(id, 0 , IPC_STAT, &semid[id]))){
			valid_semid[id] = 1;
			total_sems++;
		}else{
			valid_semid[id] = 0;
		}
	}
}

/* 
 * get_msg_stats() read msgq data from the corefile to msqbuf
 */
get_msg_stats(fname, corefile)
char	*fname;
char	*corefile;
{
	SYMENT 	sym;		/* The table of names to find */
	int	fd;		/* kernel file descriptor */
	long	msqoffset;	/* offset to message queue headers */

	/* Allocate space for message queues headers */
	if (NULL == (msqbuf = (struct msqbuf *)
	    calloc((unsigned) NMSQID, (unsigned) sizeof(struct msqid_ds)))) {
		perror("ipcs");
		exit(1);
	}
	sym._n._n_n._n_zeroes = 0;	/* stuff for coffnlist */
	sym.n_type = -1;

	/* "msqs" is internal kernel variable that points to message
	 * queues headers.
	 */
	strcpy(sym.n_name, "msqs");
	/* do lookups. coffnlist returns 0 on error. */
	if (!coffnlist(fname, &sym, NULL, 1)) {
		fprintf(stderr, "ipcs: error obtaining values from %s\n", 
									fname);
		exit(1);
	}

	/* Now we got addresses of the queues headers. So, we can go to corefile
	 * and read proper values. 
	 */
	if ((fd = open(corefile, O_RDONLY)) < 0) {
		fprintf(stderr, "ipcs: cannot open /dev/kmem\n");
		exit(1);
	}
	/* Get address of message queue data */
	lseek(fd, sym.n_value, 0);
	if (read(fd, &msqoffset, sizeof(int)) != sizeof(int)) {
		fprintf(stderr, "ipcs: error getting message queue offset\n");
		exit(1);
	}
	/* Before first request to msgget, msqs points to NULL.
	 * So we cannot and should not read. 
	 * After the first request to msgget, all queue headers are alloced.
	 * At this point we will have to use IPC_ALLOC bit.
	 */
	usemsqs = msqoffset ? 1 : 0;
	if (!usemsqs) {	/* There was no msq in use */
		free(msqbuf);
		return;
	}
	lseek(fd, msqoffset, 0);
	if (read(fd, msqbuf, sizeof(struct msqid_ds) * NMSQID) !=
 				sizeof(struct msqid_ds) * NMSQID) {
		fprintf(stderr, "ipcs: error getting message queue status\n");
		exit(1);
	}
}

