#ifndef	__COMMON_TRICKS_H__
#define	__COMMON_TRICKS_H__

/*
 * Export some of the definitions from <common/_tricks.h> into the user
 * namespace.
 */

#include <common/_tricks.h>

#define	IS_POWER_OF_TWO(i)	__IS_POWER_OF_TWO (i)
#define	DIVIDE_ROUNDUP(num,den)	__DIVIDE_ROUNDUP (num, den)
#define	LEAST_BIT_UCHAR(i)	__LEAST_BIT_UCHAR (i)
#define	LEAST_BIT_USHRT(i)	__LEAST_BIT_USHRT (i)
#define	LEAST_BIT_UINT(i)	__LEAST_BIT_UINT (i)
#define	LEAST_BIT_ULONG(i)	__LEAST_BIT_ULONG (i)
#define	ARRAY_LENGTH(a)		__ARRAY_LENGTH (a)

#endif	/* ! defined (__COMMON_TRICKS_H__) */

