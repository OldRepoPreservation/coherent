/*
 * File: sem1.c
 *
 * Purpose: This module provides System V compatible semaphore operations.
 *
 * $Log$
 */

/*
 * ----------------------------------------------------------------------
 * Includes.
 */
#include <sys/coherent.h>
#include <sys/sched.h>
#if 0
#include <sys/proc.h>
#endif
#include <sys/types.h>
#include <sys/uproc.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/con.h>
#include <sys/sem.h>

/*
 * ----------------------------------------------------------------------
 * Definitions.
 *	Constants.
 *	Macros with argument lists.
 *	Typedefs.
 *	Enums.
 */

/*
 * ----------------------------------------------------------------------
 * Functions.
 *	Import Functions.
 *	Export Functions.
 *	Local Functions.
 */
int	iSemPerm();	/* Check permissions */
int	iSanityCheck();	/* Sanity check */
int	iSemInit();	/* Init semaphores */
void	vSemAdj();	/* Set sem adjust */
/*
 * ----------------------------------------------------------------------
 * Global Data.
 *	Import Variables.
 *	Export Variables.
 *	Local Variables.
 */
/* Patchable values */
int	SEMMNI = 10;	/* max # of the semaphore sets, systemwide */
int	SEMMNS = 60;	/* max # of semaphores, systemwide */
int	SEMMSL = 25;	/* max # of semaphores per set */
int	SEMVMX = 32767;	/* max value of any semaphore */
	
struct semid_ds	*semids = NULL;	/* Array of semaphore sets */
int		iSemTotal = 0;	/* Total number of semaphores, systemwide */
GATE		gSemGate;	/* Semaphore gate. */
unsigned short	usSEM_R = 0444;	/* Permission definition for read */
unsigned short	usSEM_A = 0222;  /* and alter */

/*
 * ----------------------------------------------------------------------
 * Code.
 */

/*
 * Semget gets set of semaphores. Returns semaphore set id on suuccess,
 * or sets u.u_error on error.
 */
usemget(skey, nsems, semflg)
key_t 	skey;		/* Semaphore key */
int 	nsems,		/* # of semaphores in the set */
	semflg;		/* Permission flag */
{
	register struct semid_ds	*semidp;	/* Semaphore set */
	register struct sem 		*semp;		/* Semaphores */
	struct semid_ds			*freeidp = 0;	/* Oldest free set */

	if (iSanityCheck(nsems))
		return;

	/* Allocate memmory on the first semget. This memory (~300 bytes)
	 * will stay alloc up to the next reboot. The alloced unused memory
	 * is smaller than code that will allow to manage it more sophisticated.
	 * Allocaton is used to allow patchability of the semaphores. 
	 */
	if (semids == NULL) {
		if (iSemInit()) {
			u.u_error = ENOSPC;
			return;
		}
	}
	lock(gSemGate);			/* Lock semaphore operations */

	/* Now we will go through all semaphores. */
	for (semidp = semids; semidp < semids + SEMMNI; semidp++) {
		/* If set is free look for the oldest */
		if ((semidp->sem_perm.mode & IPC_ALLOC) == 0) {
			if ((freeidp == 0) ||
			    (freeidp->sem_ctime > semidp->sem_ctime))
				freeidp = semidp;
			continue;
		}

		/* Check if a request was for the private set */
		if (skey == IPC_PRIVATE)
			continue;

		if (skey != semidp->sem_perm.key)
			continue; 
		/* Found */
		/* Exclusive set cannot be created */
		if ((semflg & IPC_CREAT) && (semflg & IPC_EXCL)) {
			unlock(gSemGate);
			u.u_error = EEXIST;
			return;
		}

		/* Check the permissions */
		if (iSemPerm(semidp, semflg)) {
			unlock(gSemGate);
			return;
		}

		/* Check the requested number of semaphores */
		if (semidp->sem_nsems < nsems) {
			unlock(gSemGate);
			u.u_error = EINVAL;
			return;
		}
		/* Semaphore set id number is the number of an array element */
		unlock(gSemGate);
		return semidp - semids;
	}

	/* Set does not exist. So, we have to creat it */

	/* Check the total number of semaphores */
	if ((iSemTotal + nsems > SEMMNS)) {
		unlock(gSemGate);
		u.u_error = ENOSPC;
		return;
	}

	/* Check if there is the request for creation */
	if (!(semflg & IPC_CREAT)) {
		unlock(gSemGate);
		u.u_error = ENOENT;
		return;
	}

	/* Out of system limits */
	if (freeidp == 0) {
		unlock(gSemGate);
		u.u_error = ENOSPC;
		return;
	}

	/* Now we have to creat a new set. freeidp points to the oldest free
	 * set which we will use.
	 */
	semidp = freeidp;
	/* Get space for semaphores */
	semidp->sem_base = kalloc(nsems * sizeof(struct sem));
	if (semidp->sem_base == 0) {
		unlock(gSemGate);
		u.u_error = ENOSPC;
		return;
	}

	/* Initialize created set */
	/* ipc_perm */
	semidp->sem_perm.cuid = semidp->sem_perm.uid = u.u_uid;
	semidp->sem_perm.cgid = semidp->sem_perm.gid = u.u_gid;
	semidp->sem_perm.mode = (semflg & 0777) | IPC_ALLOC;
	semidp->sem_perm.key  = skey;
	semidp->sem_perm.seq = semidp - semids;
	
	semidp->sem_nsems = nsems;
	semidp->sem_otime = 0;
	semidp->sem_ctime = timer.t_time;

	/* Increment number of semaphores in used */
	iSemTotal += nsems;

	/* Set values of the semaphores to 0.
	 * SVR3 does not do it and suggests set up sem struct using
	 * semctl() call. SVR4 manual says nothing about it.
	 */
	for (semp = semidp->sem_base; semp < semidp->sem_base + nsems; semp++)
		semp->semval = semp->sempid = semp->semncnt = semp->semzcnt = 0;

	unlock(gSemGate);
	return semidp - semids;
}

/*
 * Semctl provides semaphore control operation as specify by cmd.
 * On success return value depends on cmd, sets u.u_error on error.
 */
usemctl(semid, semnum, cmd, arg)
int	semid,		/* Semaphore set id */
	cmd;		/* Command */
int	semnum;		/* Semaphore # */
union semun {
	int		val;	/* Used for SETVAL only */
	struct semid_ds *buf;	/* Used for IPC_STAT and IPC_SET */
	unsigned short 	*array;	/* Used for IPC_GET_ALL and IPC_SETALL */
} arg;

{
	register struct semid_ds	*semidp;	/* Semaphore set */
	register struct sem		*semp;		/* Semaphore */
	int 				val;		/* Semaphore value */
	int				i;		/* Loopindex */
	unsigned short			*pusUserAr;	/* User array */

	if (iSanityCheck(semnum))
		return;

	/* Check if there was any successfull semget before.
	 * Problem may be if somebody does semctl() before semids was
	 * alloced.
	 */
	if (semids == 0) {
		u.u_error = EINVAL;
		return;
	}

	/* semid cannot be < 0 and more than systemwide limit */
	if (semid < 0 || semid >= SEMMNI) {
		u.u_error = EINVAL;
		return;
	}
	semidp = semids + semid;
	
	/* Check if the requested set is alloced */
	if ((semidp->sem_perm.mode & IPC_ALLOC) == 0) {
		u.u_error = EINVAL;
		return;
	}

	/* Check if semnum is a correct semaphore number.
	 * SV would do it only when there is request for a
	 * single semaphore value, as GETVAL or SETVAL.
	 */
	if (semnum >= semidp->sem_nsems) {
		u.u_error = EFBIG;
		return;
	}

	semp = semidp->sem_base + semnum;

	switch (cmd) {
	case GETVAL:		/* Return value of semval */
		if (iSemPerm(semidp, usSEM_R))	/* cannot read */
			return;
		return semp->semval;
	case SETVAL:	/* Set semval. Clear all semadj values on success. */
		if (iSemPerm(semidp, usSEM_A))		/* cannot alter */
			return;
		/* semval always >= 0 */
		if (arg.val > SEMVMX || arg.val < 0) {	/* illegal value */
			u.u_error = ERANGE;
			return;
		}
		/* Set semval and wakeup whatever should be */
		if (semp->semval = arg.val) {
			if (semp->semncnt)
				wakeup(&semp->semncnt);
		} else {
			if (semp->semzcnt)
				wakeup(&semp->semzcnt);
		}
		vSemAdj(semid, semnum, 0);
		return 0;
	case GETPID:		/* Return value of sempid */
		if (iSemPerm(semidp, usSEM_R))	/* cannot read */
			return;
		return semp->sempid;
	case GETNCNT:		/* Return value of semncnt */
		if (iSemPerm(semidp, usSEM_R))	/* cannot read */
			return;
		return semp->semncnt;
	case GETZCNT:		/* Return value of semzcnt */
		if (iSemPerm(semidp, usSEM_R))	/* cannot read */
			return;
		return semp->semzcnt;
	case GETALL:		/* Return semvals array */
		if (iSemPerm(semidp, usSEM_R))	/* cannot read */
			return;
		/* Copy all values to user array */
		semp  = semidp->sem_base;
		pusUserAr = arg.array;
		for (i = 0; i < semidp->sem_nsems; i++) {
			putusd(pusUserAr, semp->semval);
			if (u.u_error)
				return;
			semp++;
			pusUserAr++;
		}
		return 0;
	case SETALL:		/* Set semvals array */
		if (iSemPerm(semidp, SEM_A))	/* cannot alter */
			return;

		/* Set semvals accoding to the arg.array */
		semp  = semidp->sem_base;
		pusUserAr = arg.array;
		for (i = 0; i < semidp->sem_nsems; i++) {
			val = getusd(arg.array);
			if (u.u_error)
				return;
			if (val < 0 || val > SEMVMX) {
				u.u_error = ERANGE;
				return;
			}
			semp->semval = val;
			vSemAdj(semid, i, 0);
			pusUserAr++;
			semp++;
		}
		return 0;
	case IPC_STAT:
		if (iSemPerm(semidp, usSEM_R))	/* cannot read */
			return;
		kucopy(semidp, arg.buf, sizeof(struct semid_ds));
		return 0;
	case IPC_SET:
		if (iSemPerm(semidp, SEM_A))	/* cannot alter */
			return;
		semidp->sem_perm.uid   = getusd(&((arg.buf)->sem_perm.uid));
		semidp->sem_perm.gid   = getusd(&((arg.buf)->sem_perm.gid));
		semidp->sem_perm.mode  =
			(getusd(&((arg.buf)->sem_perm.mode))&0777) | IPC_ALLOC;
		return 0;
	case IPC_RMID:
		if ((u.u_uid != 0) && (u.u_uid != semidp->sem_perm.uid)
				&& u.u_uid != semidp->sem_perm.cuid) {
			u.u_error = EPERM;
			return;
		}

		/* We have to wake up all waiting proccesses  */
		for (semp = semidp->sem_base; semp < semidp->sem_base +
				semidp->sem_nsems; semp++) {
			if (semp->semncnt)
				wakeup(&semp->semncnt);
			if (semp->semzcnt)
				wakeup(&semp->semzcnt);
		}
		iSemTotal -= semidp->sem_nsems;
		semidp->sem_perm.mode = 0;
		kfree(semidp->sem_base);
		return 0;
	default:
		u.u_error = EINVAL;
		return;
	}
}

/*
 * Semop - Semaphore Operations.
 */
usemop(iSemId, pstSops, uNsops)
int		iSemId;		/* Semaphore identifier */
struct sembuf	*pstSops;	/* Array of sem. operations */
unsigned 	uNsops;		/* # of elements in the array */
{
	register struct semid_ds	*rpstSemSet;	/* Semaphore set */
	register struct sem 		*rpstSem;	/* Semaphore */
	struct sembuf 			*pstSemBuf;	/* Operations */
	unsigned short 			usSemNum, 	/* Semaphore number */
					usSemFlg;
	short 				sSemOper; 	/* Operation */
	int				i;		/* Loop */

	/* Check if semids was alloced */
	if (semids == 0) {
		u.u_error = EINVAL;
		return;
	}
	/* iSemId cannot be < 0 and more than systemwide limit */
	if (iSemId < 0 || iSemId >= SEMMNI) {
		u.u_error = EINVAL;
		return;
	}
	if (!useracc(pstSops, sizeof(struct sembuf) * uNsops, 0) || uNsops<1) {
		u.u_error = EFAULT;
		return;
	}
	
	rpstSemSet = semids + iSemId;	/* Requested set */

	if ((rpstSemSet->sem_perm.mode & IPC_ALLOC) == 0) {
		u.u_error = EINVAL;
		return -1;
	}
TRY_AGAIN:	/* Repeat the semaphore set */

	/* Lock semaphore system */
	lock(gSemGate); 

	/* do semaphore ops  */
	for (i = 0, pstSemBuf = pstSops; i < uNsops; i++, pstSemBuf++) {
		usSemNum = getusd(&(pstSemBuf->sem_num));
		sSemOper  = getusd(&(pstSemBuf->sem_op));
		usSemFlg = getusd(&(pstSemBuf->sem_flg));

		if ((u.u_error != 0) || (usSemNum >= rpstSemSet->sem_nsems)) {
			/* We have falure here. undo all previous 
			 * operations.
			 */
			semundo(rpstSemSet->sem_base, pstSops, i);
			/* If u_error was not set it means that sem_number 
			 * is bad. So, set error to EFBIG.
			 */
			if (u.u_error == 0)
				u.u_error = EFBIG;
			unlock(gSemGate); 
			return;
		}

		/* Go to proper semaphore */
		rpstSem = rpstSemSet->sem_base + usSemNum;
	
		/* We can have 3 different cases: sSemOper < 0,
		 * sSemOper == 0, & sSemOper > 0.
		 */
		if (sSemOper < 0) {	/* want to decrement semval */
			/* We do not have to undo anythig, if we cannot alter*/	
			if (iSemPerm(rpstSemSet, SEM_A)) { /* cannot alter */
				unlock(gSemGate); 
				return;
			}

			/* If we can decrement semval, do it. If
			 * semval becomes 0 wakeup all processes
			 * waiting for semval==0.
			 */
			if (rpstSem->semval >= -sSemOper) {
				if (!(rpstSem->semval += sSemOper))
		 			if (rpstSem->semzcnt)
						wakeup(&rpstSem->semzcnt);
				continue;
			}
			/* Can't decrement. */
			semundo(rpstSemSet->sem_base, pstSops, i);
			if (usSemFlg & IPC_NOWAIT) {
				/* Adjust???*/
				if (u.u_error == 0)
					u.u_error = EAGAIN;
				unlock(gSemGate); 
				return;
			} else {/* Go to sleep */
				/* Adjust???*/
				unlock(gSemGate); 
				if (semwait(iSemId, &rpstSem->semncnt) < 0) {
					return;
				}
				/* Now we can retry semaphore set */
					goto TRY_AGAIN;
			}
		} 
		if (sSemOper == 0) {
			if (iSemPerm(rpstSemSet, SEM_R)) { /* cannot read */
				unlock(gSemGate); 
				return;
			}
			if (rpstSem->semval == 0) 
				continue;

			/* Semaphore value isn't 0. Undo all previous
			 * operations.
			 */
			semundo(rpstSemSet->sem_base, pstSops, i);

			if (usSemFlg & IPC_NOWAIT) {
				if (u.u_error == 0)
					u.u_error = EAGAIN;
				unlock(gSemGate); 
				return;
			}

			unlock(gSemGate); 
			if (semwait(iSemId, &rpstSem->semzcnt) < 0) {
				return;
			}
			goto TRY_AGAIN;
		} 
		if (sSemOper > 0) {
			if (iSemPerm(rpstSemSet, SEM_A)) { /* cannot alter */
				unlock(gSemGate); 
				return;
			}
			rpstSem->semval += sSemOper;
			/* Wake up waiting processes */
			if (rpstSem->semncnt)
				wakeup(&rpstSem->semncnt);
			/* Adjust???*/
		}
	}
	rpstSemSet->sem_otime = timer.t_time;	/* adjust operation time */
	unlock(gSemGate);
	return 0;				/* return last prev semval */
}

/*
 * Wait for an event.
 */
semwait(iSemId, usSleepEvent)
int		iSemId;		/* Semaphore set id */
unsigned short	*usSleepEvent;	/* Could be semcnt or semzcnt */
{
	register struct	semid_ds	*rpstSemSet;	/* Semaphore set */

	(*usSleepEvent)++;
	
	rpstSemSet = semids + iSemId;
	
 	x_sleep(usSleepEvent, pritty, slpriSigCatch, "semwait");

	if (!(rpstSemSet->sem_perm.mode & IPC_ALLOC)) {	/* Semaphore gone */
		u.u_error = EIDRM;
		return -1;
	}
	(*usSleepEvent)--;

	if (SELF->p_ssig && nondsig()) {	/* Signal received */
		u.u_error = EINTR;
		return -1;
	}
	return 0;
}
	
/*
 * Undo a Semaphore Operation.
 */
semundo(pstSem, pstSemOp, iUndo)
struct sem	*pstSem;	/* Pointer to the first of the semaphores */
struct sembuf	*pstSemOp;	/* Pointer to the undo operation */
int		iUndo;		/* Number of semaphores to undo */
{
	register struct sem	*rpstSem;	/* */
	register struct sembuf	*rpstBuf;	/* */
	register int		i;		/* Loop index */
	unsigned short		usSemNum;	/* Semaphore number */
	short			sSemOper;	/* Value to undo */

	rpstSem = pstSem;
	rpstBuf = pstSemOp;
	for (i = 0; i < iUndo; i++) {
		usSemNum = getusd( &(rpstBuf->sem_num) );
		sSemOper  = getusd( &(rpstBuf->sem_op) );
		rpstSem->semval -= sSemOper;
		rpstBuf++;
		rpstSem++;
	}		
}

/*
 * Check permissions of the semaphore set.
 * Return 0 on success, -1 and set errno on error.
 */
int iSemPerm(pstSemId, iSemFlg)
struct semid_ds	*pstSemId;	/* Pointer to the semaphor set */
int		iSemFlg;	/* Requested permissions */
{
	int	iSemMode;

	/* Check if resource is alloced */
	if ((pstSemId->sem_perm.mode & IPC_ALLOC) == 0) {
		u.u_error = EINVAL;
		return -1;
	}
		
	/* We need 9 lower order bits. There is a question what we have to do
	 * if someone sets an execute bits on. At this point we just ignore 
	 * them.
	 */
	iSemMode = iSemFlg & 0666;

	/* For superuser or if mode is 0 */
	if (u.u_uid == 0 || !iSemMode) 
		return 0;

	/* For owner or creator */
	if (u.u_uid == pstSemId->sem_perm.uid || u.u_uid 
						== pstSemId->sem_perm.cuid) {
		if ((iSemMode & pstSemId->sem_perm.mode) & 0600)
			return 0;
		else {
			u.u_error = EACCES;
			return -1;
		}
	}
	/* For group */		
	if (u.u_gid == pstSemId->sem_perm.gid 
					|| u.u_gid == pstSemId->sem_perm.cgid) {
		if ((iSemMode & pstSemId->sem_perm.mode) & 060)
			return 0;
		else {
			u.u_error = EACCES;
			return -1;
		}
	}
	/* For the rest of the world */
	if ((iSemMode & pstSemId->sem_perm.mode) & 06) 
		return 0;
	else {
		u.u_error = EACCES;
		return -1;
	}
	/* We should never come here */
	u.u_error = EACCES;
	return -1;
}

/*
 * Allocate and clear space for semapohore sets
 * Return 0 on success, -1 and set errno on error.
 */
iSemInit()
{
	unsigned	uSize;		/* Size of the alloc memmory */

	uSize = sizeof(struct semid_ds) * SEMMNI;

	if ((semids = (struct semid_ds *) kalloc(uSize)) == NULL)
		return -1;
	memset((char *) semids, 0, uSize);
	return 0;
}

/*
 * Check if semaphore number is a valid number.
 */
iSanityCheck(iSemNumber)
int	iSemNumber;	/* Requested number of the semaphores in the set */
{
	/* Just to be on safe side */
	if (u.u_error)
		return -1;

	/* Check if we are inside system limits. */
	if (iSemNumber >= SEMMSL || iSemNumber < 0) {
		u.u_error = EINVAL;
		return -1;
	}
	return 0;
}

/*
 * Set semaphore adjust values.
 */
void	vSemAdj(iSemId, iSemNum, iValue)
int	iSemId;		/* Semaphore set id */
int	iSemNum;	/* Semaphore number */
int	iValue;		/* Adjust value */
{
/* Code should be written */
}
