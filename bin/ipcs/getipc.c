/*
 * Module read all information about ipc.
 */
#include <errno.h>
#include "ipcs.h"

/*
 * Get ipc and format ipc data
 */
int get_data()
{
	if(mflag){
		get_shmid_stats();
	}

	if(sflag){
		get_sem_stats();
	}

	if(qflag){
		get_msg_stats();
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


/* get_msg_stats() is basically a loop that is used to build an
 * array of pointers to msqid_ds structs.
 */

get_msg_stats()
{

int x, y;
struct msqid_ds dummy_msqid[OBSCENE_VALUE];
int found[OBSCENE_VALUE];

	/* initialize the variable that we will check in the print routines
	 * to see if a given msqid was indeed found in this function.
	 * We are also initializing another variable which will prevent from
	 * later performing unnecessary msgctl()s.
	 */

	for(y = 0 ; y < NMSQID ; y++){
		found[y] = 0;
		valid_msqid[y] = 0;
	}

	total_msgs = 0;

	/* Here's the scoop: Coherent 4.0 numbers it's msqid sequence
	 * numbers in steps of 256 (the identifier for the first msqid
	 * is 0, the next is 256, then 512...) When a given queue has
	 * been removed with a msgctl(id, IPC_RMID,o) command, its sequence
	 * number is bumped by a value of 1. When this sequence number
	 * is about to be increased into the next msqid sequence number,
	 * the driver (is supposed to, according to Vlad) drops this number
	 * back to its starting point. Example: if the first message queue
	 * (sequence or id number of 0 when first initialized) is used,
	 * removed and used again 255 times, it will not be increased to
	 * 256 the 256th time someone RMID's it, it will be reduced back
	 * to zero. (I don't believe that this actuall works yet, but I'll
	 * get around to testing it...
	 */

	for (x = 0; x <256 ; x++){
		for(y = 0; y< NMSQID; y++){

			/* our possible msqid is ((y*256) + x). If we
			 * find that a message queue is in use with this
			 * id, we set 2 flags. The first flag (found[y]) tells
			 * us that we needn't repeat the msgctl test which
			 * follows. The second flag will be used to tell us in
			 * the print function that this is a valid message
			 * queue to print information on.
			 */

			if(!found[y]){
				if(-1 != msgctl( ((y*256) + x), IPC_STAT, &dummy_msqid[y])){
					msqbuf[y] = &dummy_msqid[y];
					found[y] = 1;
					valid_msqid[y] = 1;
					total_msgs++;
				}
			}
		}
	}
}
