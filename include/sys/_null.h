/* (-lgl
 * 	COHERENT Version 4.0.2
 * 	Copyright (c) 1982, 1992 by Mark Williams Company.
 * 	All rights reserved. May not be copied without permission.
 -lgl) */

#ifndef	__SYS__NULL_H__
#define	__SYS__NULL_H__

/*
 * Canonical definition for NULL.
 */

#ifdef	__cplusplus

# define	NULL		0

#elif	defined (COHERENT)

# define	NULL		((char *) 0)

#else

# error	The correct type for NULL is not known for your system.

#endif

#endif	/* ! defined (__SYS__NULL_H__) */
