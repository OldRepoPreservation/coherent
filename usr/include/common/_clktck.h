#ifndef	__COMMON__CLKTCK_H__
#define	__COMMON__CLKTCK_H__

/*
 * This internal header file is intended as the sole point of definition for
 * the ISO C constant "CLOCKS_PER_SEC" and the related POSIX.1 constant
 * CLK_TCK.
 */

#include <common/feature.h>

#if	__BORLANDC__

#define	CLK_TCK			18.2
#define	CLOCKS_PER_SEC	18.2

#elif defined (__GNUC__)

#define	CLK_TCK			18.2
#define	CLOCKS_PER_SEC	18.2

#elif	__COHERENT__

#define	CLK_TCK			100
#define	CLOCKS_PER_SEC	100

#else

# error The clock rate is not known for this system

#endif


#endif	/* ! defined (__COMMON__CLKTCK_H__) */
