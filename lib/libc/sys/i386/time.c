/*
 * C library -- time() function recoded in C for sm90.
 * Copyright (c) Bureau d'Etudes Ciaran O'Donnell,1987,1990,1991
 */

/*
 * tvec = time(tloc);
 */

#include "sys/types.h"

time_t time(tloc)
register long *tloc;
{
	register time_t tvec;
	time_t _time();

	if((tvec = _time()) != -1L && tloc)
		*tloc = tvec;

	return(tvec);
}

