#ifndef	__COMMON__LIMITS_H__
#define	__COMMON__LIMITS_H__

/*
 * This header file is the internal equivalent of the ISO C Standard header
 * file <limits.h>. It defines many similar constants that are useful in
 * system headers for parameterising definitions based on such things as
 * the representation ranges of various types, but the names of the constants
 * are prefixed with underscores to stay in the implementation namespace.
 *
 * Rather than just stating the constants, we calculate then from a small
 * set of fundamental parameters. Doing it this way will give the right
 * results as long as your preprocessor and compiler understand that ISO C
 * type rules. If they don't, well, this will tell you that.
 *
 * Note that the "fundamental" constants can be determined at run-time in
 * a portable fashion, but since we need them for the preprocessor we must
 * depend on someone running such a program and supplying them.
 *
 * The fundamental parameters:
 *	__CHAR_BIT	The number of bits in a character. While most systems
 *			built in the 1990s have 8, others are possible.
 *	__SHORT_DIV	The factor by which to divide the number of bits in
 *			an integer by to give the number of bits in a short.
 *	__TWOSCOMP	This contains 1 if the representation is twos-
 *			complement for negative integers, 0 if we have a
 *			sign-magnitude or ones-complement machine.
 *	__CHAR_SIGNED	1 if characters are signed, undefined if they are
 *			unsigned.
 *
 * Parameters that can be inferred from the type system:
 *	__INT_BIT	The number of bits in an integer.
 *	__LONG_BIT	The number of bits in a long.
 */

#include <common/feature.h>

#if	__COHERENT__ || __BORLANDC__ || defined (GNUDOS)

# define	__CHAR_BIT	8
# define	__TWOSCOMP	1

#if	'\x80' < 0
# define	__CHAR_SIGNED	1
#endif

# if	__BORLANDC__
#  define	__SHORT_DIV	1
# else
#  define	__SHORT_DIV	2
# endif

#else

# error	The fundamental parameters of this system are not known.

#endif

#define	__TOP_BIT_INT(bytes)	(1U << (__CHAR_BIT * (bytes) - 1))
#define	__TOP_BIT_LONG(bytes)	(1UL << (__CHAR_BIT * (bytes) - 1))

#if	(~ 0U ^ (~ 0U / 2)) == __TOP_BIT_INT(1)
# define	__INT_BIT	(1 * __CHAR_BIT)
#elif	(~ 0U ^ (~ 0U / 2)) == __TOP_BIT_INT(2)
# define	__INT_BIT	(2 * __CHAR_BIT)
#elif	(~ 0U ^ (~ 0U / 2)) == __TOP_BIT_INT(4)
# define	__INT_BIT	(4 * __CHAR_BIT)
#elif	(~ 0U ^ (~ 0U / 2)) == __TOP_BIT_INT(8)
# define	__INT_BIT	(8 * __CHAR_BIT)
#else
# error	Cannot determine number of bits per int.
#endif

#if	(~ 0UL ^ (~ 0UL / 2)) == __TOP_BIT_LONG(1)
# define	__LONG_BIT	(1 * __CHAR_BIT)
#elif	(~ 0UL ^ (~ 0UL / 2)) == __TOP_BIT_LONG(2)
# define	__LONG_BIT	(2 * __CHAR_BIT)
#elif	(~ 0UL ^ (~ 0UL / 2)) == __TOP_BIT_LONG(4)
# define	__LONG_BIT	(4 * __CHAR_BIT)
#elif	(~ 0UL ^ (~ 0UL / 2)) == __TOP_BIT_LONG(8)
# define	__LONG_BIT	(8 * __CHAR_BIT)
#else
# error	Cannot determine number of bits per long.
#endif

#define	__SHRT_BIT	(__INT_BIT / __SHORT_DIV)

#define	__UCHAR_MAX	((1U << (__CHAR_BIT - 1)) + \
				 ((1U << (__CHAR_BIT - 1)) - 1))
#define	__SCHAR_MAX	((1 << (__CHAR_BIT - 2)) + \
				 ((1 << (__CHAR_BIT - 2)) - 1))
#define	__SCHAR_MIN	(- __SCHAR_MAX - __TWOSCOMP)

#if	__CHAR_SIGNED
# define	__CHAR_MAX	__SCHAR_MAX
# define	__CHAR_MIN	__SCHAR_MIN
#else
# define	__CHAR_MAX	__UCHAR_MAX
# define	__CHAR_MIN	0
#endif

#define	__USHRT_MAX	((1U << (__SHRT_BIT - 1)) + \
				 ((1U << (__SHRT_BIT - 1)) - 1))
#define	__SHRT_MAX	((1 << (__SHRT_BIT - 2)) + \
				 ((1 << (__SHRT_BIT - 2)) - 1))
#define	__SHRT_MIN	(- __SHRT_MAX - __TWOSCOMP)

#define	__UINT_MAX	((1U << (__INT_BIT - 1)) + \
				 ((1U << (__INT_BIT - 1)) - 1))
#define	__INT_MAX	((1 << (__INT_BIT - 2)) + \
				 ((1 << (__INT_BIT - 2)) - 1))
#define	__INT_MIN	(- __INT_MAX - __TWOSCOMP)

#define	__ULONG_MAX	((1U << (__LONG_BIT - 1)) + \
				 ((1U << (__LONG_BIT - 1)) - 1))
#define	__LONG_MAX	((1 << (__LONG_BIT - 2)) + \
				 ((1 << (__LONG_BIT - 2)) - 1))
#define	__LONG_MIN	(- __LONG_MAX - __TWOSCOMP)

#endif	/* ! defined (__COMMON__LIMITS_H__) */
