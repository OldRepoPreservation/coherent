/* ipcrm: remove message queue, shared memory segment or semaphore set
 * 	  given a queue identifier or key used to create the queue, segment
 *	  or set.
 */

#include <stdio.h>
#include <errno.h>
#include <sys/ipc.h>

#define FALSE 0
#define TRUE 1

/* flags */
short m,s,q,M,S,Q;

#define FALSE 0
#define TRUE 1
#define SIZE 9

/* functions */
int getid();		/* given a key, returns queue id */
int rmid();		/* given queue identifier, remove it */
int usage();		/* usage message */


main(argc, argv)
int argc;
char *argv;
{
	extern char *optarg;
	extern int optind;
	char flag;
	int midentifier;
	int sidentifier;
	int qidentifier;
	int Midentifier;
	int Sidentifier;
	int Qidentifier;

	while( (flag = getopt(argc, argv, "m:M:s:S:q:Q:")) != EOF){
		switch(flag){
				case 'm':
					m = TRUE;
					if((optarg == NULL) || (optarg == EOF)){
						printf("ipcs: missing identifier or key\n");
						exit(1);
					}
					midentifier = atoi(optarg);
					break;
				case 'M':
					M = TRUE;
					if((optarg == NULL) || (optarg == EOF)){
						printf("ipcs: missing identifier or key\n");
						exit(1);
					}
					Midentifier = atoi(optarg);
					break;
				case 'q':
					q = TRUE;
					if((optarg == NULL) || (optarg == EOF)){
						printf("ipcs: missing identifier or key\n");
						exit(1);
					}
					qidentifier = atoi(optarg);
					break;
				case 'Q':
					Q = TRUE;
					if((optarg == NULL) || (optarg == EOF)){
						printf("ipcs: missing identifier or key\n");
						exit(1);
					}
					Qidentifier = atoi(optarg);
					break;
				case 's':
					s = TRUE;
					if((optarg == NULL) || (optarg == EOF)){
						printf("ipcs: missing identifier or key\n");
						exit(1);
					}
					sidentifier = atoi(optarg);
					break;
				case 'S':
					S = TRUE;
					if((optarg == NULL) || (optarg == EOF)){
						printf("ipcs: missing identifier or key\n");
						exit(1);
					}
					Sidentifier = atoi(optarg);
					break;
				default:
					usage();
					exit(1);
		}
	}

	/* did the user specify an option at all? */
	if( (m+M+q+Q+s+S) == 0 ){
		usage();
		exit(1);
	}

	/* walk through our identifier flags and delete a shared mem segment, 
	 * message queue or semaphore set if the flag is set. 
	 */

	if(m)
		rmid('m', midentifier);
	if(q)
		rmid('q', qidentifier);
	if(s)
		rmid('s', sidentifier);


	/* walk through our KEY flags and get the identifier associated
	 * with the key. Then call rmid() to rid ourselves of the message
	 * queue, shared mem seg or semaphore set. Getid will return an integer,
	 * so we don't need to convert the identifier as we do above.
	 */

	if(M){
		midentifier = getid('m', Midentifier);
		rmid('m', midentifier);
	}

	if(Q){
		qidentifier = getid('q', Qidentifier);
		rmid('q',qidentifier);
	}

	if(S){
		sidentifier = getid('s', Sidentifier);
		rmid('s',sidentifier);
	}
}


/* rmid(): take a flag which tells us to remove a message queue, shared memory
 *	   segment or semaphore set and an identifier and perform the proper
 *	   command (msgctl, shmctl or semctl) with IPC_RMID to destroy the
 *	   allocated queue, segment or set.
 */

rmid(flag,identifier)
char flag;
int identifier;
{
	errno = 0;
	switch(flag){
		case 'm':
			shmctl(identifier, IPC_RMID, 0);			
			if(errno){
				printf("ipcrm (shared memory) error: %s\n",sys_errlist[errno]);
				exit(1);
			}
			break;
		case 'q':
			msgctl(identifier, IPC_RMID, 0);
			if(errno){
				printf("ipcrm (message queue) error: %s\n",sys_errlist[errno]);
				exit(1);
			}
			break;
		case 's':
			semctl(identifier, IPC_RMID, 0);
			if(errno){
				printf("ipcrm (semaphore set) error: %s\n",sys_errlist[errno]);
				exit(1);
			}
			break;
	}
}


/* getid():	Given a flag indicating that we are working on a shared memory
 *		segment, message queue or semaphore set and a key which 
 *		presumeably created it, get the id and return it to the
 *		calling function.
 */

getid(flag, key)
char flag;
int key;
{
	int ret;	/* queue identifier returned by msgget, semget, shmget */
	errno = 0;
	switch(flag){
		case 'm':
			ret = shmget(key,0000);
			if(errno){
				printf("ipcrm (shared memory) error: %s\n",sys_errlist[errno]);
				exit(1);
			}
			return(ret);
			break;
		case 'q':
			ret = msgget(key,0000);
			if(errno){
				printf("ipcrm (message queue) error: %s\n",sys_errlist[errno]);
				exit(1);
			}
			return(ret);
			break;
		case 's':
			ret = semget(key,0000);
			if(errno){
				printf("ipcrm (semaphore) error: %s\n",sys_errlist[errno]);
				exit(1);
			}
			return(ret);
			break;
	}
}


/* usage message */
usage()
{
	printf("Usage: ipcs [option] [identifier or key]\n");
	printf("options:  m  Remove shared mem. segment with following queue id\n");
	printf("          M  Remove shared mem. segment created with following key\n");
	printf("          q  Remove message queue with following queue id.\n");
	printf("          Q  Remove message queue created with following key.\n");
	printf("          s  Remove semaphore set with following id.\n");
	printf("          S  Remove semaphore set created with following key.\n");
}
