/*
 * libm/sin.c
 * Evaluate the sine function.
 */

#include <math.h>

#if	EMU87
#include "emumath.h"
#endif

/*
 * To get a correct result for very small |x|,
 * the code below just returns x for |x| < THRESHOLD.
 * We can derive a theoretical value for THRESHOLD from the series:
 *	sin(x)  = x - x^3/3! + x^5/5! - ...
 * so if |x| < sqrt(6 * DBL_EPSILON) then x^3/3! < x * DBL_EPSILON
 * and the low-order terms must be insignificant.
 * The threshold value below, arrived at empirically, is somewhat larger;
 * it is for IEEE fp, the DECVAX value must be slightly different but...
 */
#define	THRESHOLD	2.1485600010223542e-8

double
sin(x) double x;
{
	if (fabs(x) < THRESHOLD)
		return x;
	return cos(x - PI/2.0);
}

/* end of sin.c */
