/*
 * Print out received ipc data corresponding to print optoins.
 */
#include <stdio.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include "ipcs.h"

struct group *grp;
struct passwd *pstp;

struct msqid_ds	*msqbuf;	/* message queue data */
/* 
 * Print information about active message queues
 */
print_q() 
{
	int 	x;		/* counter */
	char 	date[30];	/* date string */

	printf("MESSAGE QUEUES:\n");

	/* Check if msq were in use */
	if (!usemsqs)
		return; 

	fputs("T\tID\tKEY\tMODE\tOWNER\tGROUP\t", stdout);

	if (cflag) 
		fputs("CREATOR\tCGROUP\t", stdout);

	if (oflag) 
		fputs("CBYTES\tQNUM\t", stdout);

	if (bflag)
		fputs("QBYTES\t", stdout);

	if (pflag)
		fputs("LSPID\tLRPID\t", stdout);

	if (tflag)
		fputs("STIME\tRTIME\tCTIME\t", stdout);

	puts("");

	for (x = 0 ; x < NMSQID; x++) {
		if (msqbuf[x].msg_perm.mode & IPC_ALLOC) {
			printf("q\t%d\t%d\t", msqbuf[x].msg_perm.seq,
					msqbuf[x].msg_perm.key);	
	
			/* print message queue mode, and any flag indicating
			 * that a process is waiting on a msgrcv() or
			 * msgsnd().
			 */	
			if (msqbuf[x].msg_perm.mode & MSG_RWAIT) {
				printf("R "); /* pid waiting for msgrcv() */
			} else {
				if (msqbuf[x].msg_perm.mode & MSG_WWAIT) {
					printf("S "); /* waiting for msgsnd() */
				} else {
					printf("- "); /* no flag set */
				}	

			}
			
			printf("0%o\t",	msqbuf[x].msg_perm.mode & 0777);

			/* get owner's name from /etc/passwd */
			if ((pstp = getpwuid(msqbuf[x].msg_perm.uid)) == NULL) {
				fprintf(stderr,"Error reading password file!\n");
				exit(1);
			}
			printf("%s\t",pstp->pw_name);
	
			/* get group name of owner */
			if ((grp = getgrgid(msqbuf[x].msg_perm.gid)) == NULL) {
				fprintf(stderr, "Error reading group file!\n");
				exit(1);
			}
			printf("%s\t",grp->gr_name);

			if (cflag) {
			/* get creator's name from /etc/passwd */
				if ((pstp = getpwuid(msqbuf[x].msg_perm.cuid)) == NULL){
					printf("Error reading password file!\n");
					exit(1);
				}
				printf("%s\t",pstp->pw_name);

				/* get group name of creator */
				if ((grp = getgrgid(msqbuf[x].msg_perm.cgid)) == NULL){
					printf("Error reading group file!\n");
					exit(1);
				}
				printf("%s\t",grp->gr_name);
			}

			/* current bytes & # of messages */
			if (oflag) 
				printf("%d\t%d\t", msqbuf[x].msg_cbytes,
					msqbuf[x].msg_qnum);

			/* max # bytes on queue */
			if (bflag)
				printf("%d\t",msqbuf[x].msg_qbytes);

			/* last send & receive processes */
			if (pflag) 
				printf("%d\t%d\t", msqbuf[x].msg_lspid,
							msqbuf[x].msg_lrpid);

			/* times of last send and receive and modification*/
			if (tflag) {
				sprintf(date,"%s", ctime(&msqbuf[x].msg_stime));
				printf("%.5s\t",date + 11);
				sprintf(date,"%s", ctime(&msqbuf[x].msg_rtime));
				printf("%.5s\t",date + 11);
				sprintf(date,"%s", ctime(&msqbuf[x].msg_ctime));
				printf("%.5s",date + 11);
			}
			printf("\n");
		}
	}
	printf("\n");
}
/* 
 * Print information about active shared memory segments
 */
print_m()
{
int x;		/* counter for print loop */
char date[30];	/* used to hold the date string */

	printf("SHARED MEMORY:\n");

	if(total_shmids){
		printf("T\tID\tKEY\tMODE\tOWNER\tGROUP\t");

		if(cflag){
			printf("CREATOR\tCGROUP\t");
		}
	
		if(tflag){
			printf("CTIME\t");
		}

		if(oflag){
			printf("NATTCH\t");
		}

		if(bflag){
			printf("SEGSZ\t");
		}

		if(pflag){
			printf("CPID\tLPID\t");
		}

		if(tflag){
			printf("ATIME\tDTIME");
		}

		printf("\n");


		for(x = NSHMID -1; x >= 0 ; x--){
			if(valid_shmid[x]){
				printf("m\t%d\t%d\t0%o\t", /* id, mode & key */
					x, shmid[x].shm_perm.key,
					shmid[x].shm_perm.mode & 0777);

			/* get owner's name from /etc/passwd */
				if((pstp = getpwuid(shmid[x].shm_perm.uid)) == NULL){
					printf("Error reading password file!\n");
					exit(1);
				}
				printf("%s\t",pstp->pw_name);
	
			/* get group name of owner */
	
				if((grp = getgrgid(shmid[x].shm_perm.gid)) == NULL){
					printf("Error reading group file!\n");
					exit(1);
				}
				printf("%s\t",grp->gr_name);

				if(cflag){
				/* get creator's name from /etc/passwd */
					if((pstp = getpwuid(shmid[x].shm_perm.cuid)) == NULL){
						printf("Error reading password file!\n");
						exit(1);
					}
					printf("%s\t",pstp->pw_name);
	
				/* get group name of creator */
					if((grp = getgrgid(shmid[x].shm_perm.cgid)) == NULL){
						printf("Error reading group file!\n");
						exit(1);
					}
					printf("%s\t",grp->gr_name);
				}

				if(tflag){	/* time */
					sprintf(date,"%s", ctime(&shmid[x].shm_ctime));
					printf("%.5s\t",date + 11);
				}

				if(oflag){	/* attached segments */
					printf("N/A\t");
				}
		
				if(bflag){	/* segment size */
					printf("%d\t",shmid[x].shm_segsz);
				}

				if(pflag){	/* processs id of last op */
					printf("%d\t%d\t",
						shmid[x].shm_cpid,
						shmid[x].shm_lpid);
				}

				if(tflag){ 	/* attatch/detach times */
					printf("N/A\tN/A");
				}
				printf("\n");
			}
		}
	}	
	printf("\n");
			
}

/* 
 * Print information about active semaphores
 */
print_s()
{
int x;		/* loop counter */
char date[30];	/* holds our date string */

	printf("SEMAPHORES:\n");

	if(total_sems){
	printf("T\tID\tKEY\tMODE\tOWNER\tGROUP\t");

		if(cflag){
			printf("CREATOR\tCGROUP\t");
		}

		if(bflag){
			printf("NSEMS\t");
		}

		if(tflag){
			printf("OTIME");
		}
		printf("\n");

		for(x = NSEMID -1; x >= 0 ; x-- ){
			if(valid_semid[x]){
 				printf("s\t%d\t%d\t0%o\t",x,  /* mode and key */
					semid[x].sem_perm.key,
					semid[x].sem_perm.mode & 0777);

			/* get owner's name from /etc/passwd */
				if((pstp = getpwuid(semid[x].sem_perm.uid)) == NULL){
					printf("Error reading password file!\n");
					exit(1);
				}
				printf("%s\t",pstp->pw_name);
	
			/* get group name of owner */
	
				if((grp = getgrgid(semid[x].sem_perm.gid)) == NULL){
					printf("Error reading group file!\n");
					exit(1);
				}
				printf("%s\t",grp->gr_name);

				if(cflag){
				/* get creator's name from /etc/passwd */
					if((pstp = getpwuid(semid[x].sem_perm.cuid)) == NULL){
						printf("Error reading password file!\n");
						exit(1);
					}
					printf("%s\t",pstp->pw_name);
	
				/* get group name of creator */
					if((grp = getgrgid(semid[x].sem_perm.cgid)) == NULL){
						printf("Error reading group file!\n");
						exit(1);
					}
					printf("%s\t",grp->gr_name);
				}

				if(bflag){	/* number of semaphore elements */	
					printf("%d\t",semid[x].sem_nsems);
				}

				if(tflag){	/* time of last semop */
					sprintf(date,"%s", ctime(&semid[x].sem_ctime));
					printf("%.5s",date + 11);
				}
			}
		}
	}
	printf("\n");	
}
