/* Copyright (c) Bureau d'Etudes Ciaran O'Donnell,1987,1990,1991 */
#define	_SHMAT	0
#define	_SHMDT	1
#define	_SHMCTL	2
#define	_SHMGET	3

int shmfd = -1;
static char shmdev[] = "/dev/shm";

shmat(shmid, shmaddr, shmflg)
{
	return _shmsys(_SHMAT, shmid, shmaddr, shmflg);
}

shmctl(shmid, cmd, buf)
{
	return _shmsys(_SHMCTL, shmid, cmd, buf);
}

shmdt(shmaddr)
{
	return _shmsys(_SHMDT, shmaddr);
}

shmget(key, size, shmflg)
{
	if(shmfd < 0){
		if ( (shmfd = open(shmdev,2)) < 0){
			perror( shmdev );
			return -1;
		}
	}

	return _shmsys(_SHMGET, key, size, shmflg);
}
