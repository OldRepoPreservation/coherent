#ifndef	__COMMON__TRICKS_H__
#define	__COMMON__TRICKS_H__

/*
 * This file contains macro-definitions for a number of frequently-
 * rediscovered fundamental tricks from the field of radix-2 arithmetic
 * and the properties of Standard C.
 *
 * These include: discovering whether a number is a power of two, finding
 * the number of the least-significant set bit in an integer, finding the
 * number of the most-significant set bit in an integer, positive-integer
 * division that rounds upward, and so forth.
 *
 * The majority of the hacks given here are only defined on unsigned
 * arguments. Wherever appropriate, factors such as 0U or 1U are used to
 * encourage the system to coerce the result to an unsigned type. However,
 * while using 0UL or 1UL would guarantee this, we use 0U and 1U to allow
 * the operation to potentially be performed in as narrow an arithmetic
 * mode as possible.
 *
 * We have some special motives for using integral operand conversions in
 * this way rather than casts; not only can we be polymorphic, but we gain
 * the even greater advantage of keeping the expansions as integral constant
 * expressions suitable for evaluation in #if-expressions.
 *
 * In order to be flexible in the way we deal with the non-polymorphic
 * macros, we need data like that in <limits.h>. We get this information
 * from <common/_limits.h> so that we don't export stuff into the user
 * namespace.
 */ 

#include <common/feature.h>
#include <common/_limits.h>

/*
 * Since many of these hacks only work for special operand ranges, we allow
 * users the ability to selectively enable paranoid debugging; if a macro with
 * the name _TRICKS_ASSERT () is defined, we call it up with an argument that
 * is a predicate we insist on being true; don't forget that like the real-
 * life assert () in <assert.h>, this must evaluate to a void expression!
 */

#ifndef	_TRICKS_ASSERT
# define	_TRICKS_ASSERT(pred)	((void) 0)
#endif


/*
 * Many versions of Coherent 'cc' will refuse 0, 6 as a constant expression;
 * with this compiler, you lose assertions.
 */

#if	__COHERENT__
# define	__DANGER_WILL_ROBINSON(p)
#else
# define	__DANGER_WILL_ROBINSON(p)	_TRICKS_ASSERT(p),
#endif


/*
 * Is a number a power of two? This macro determines this, and should work
 * for numbers of any unsigned type. Users are cautioned to avoid passing
 * values of signed type to this macro, because then the result may depend
 * on the underlying machine representation of negative numbers. While twos-
 * complement machines are very common, they are not universal!
 */

#define	__IS_POWER_OF_TWO(an_integer)	\
		(((an_integer) - 1U) ^ (an_integer) == 0)


/*
 * Divide, with any non-zero remainder causing the result to be rounded to
 * the next highest integer, as opposed to the default unsigned rounding
 * mode of round-towards-zero. Users are cautioned that this technique does
 * not apply to signed integers, because implementations have the freedom to
 * use a rounding mode there that suits the properties of their signed-
 * integer representation.
 *
 * Furthermore, users should be aware that this technique can fail in a most
 * disastrous manner if (numerator - 1U + denominator) overflows the chosen
 * representation. In this case, users should use div () or ldiv () and
 * inspect the remainder to perform the rounding.
 */

#define	__DIVIDE_ROUNDUP(numerator,denominator) \
	(__DANGER_WILL_ROBINSON ((numerator) - 1U + (denominator)  >= (numerator)) \
	 ((numerator) - 1U + (denominator)) / (denominator))


/*
 * Locate the least-significant bit set within an integer, assuming that one
 * exists. The existence test has been left out because it is trivial and it
 * may be more efficently coded elsewhere... the other problem is the choice
 * of what value to return in this case. Since these macros are aimed at
 * speed, we punt this problem up to the caller.
 *
 * Since these macros involve lots of fixed constants, we provide a version
 * parameterised for each unsigned type.
 *
 * Note that for the portable versions, extra bits outside the defined size
 * are not considered; be warned that this is not part of the specification,
 * so our paranoia checks look to see that the extra bits are all zero, so
 * that maximum freedom is given to the assembly-language versions to be
 * fast.
 */

#define	__LEAST_BIT_8(bit_mask)	\
	(__DANGER_WILL_ROBINSON (((bit_mask) & 0xFFU) != 0) \
	 __DANGER_WILL_ROBINSON (((bit_mask) & ~ 0xFFU) == 0) \
	 (((bit_mask) & 0x0FU) == 0 ? 4 : 0) + \
	 (((bit_mask) & 0x33U) == 0 ? 2 : 0) + \
	 (((bit_mask) & 0x55U) == 0 ? 1 : 0))

#define	__LEAST_BIT_16(bit_mask)	\
	(__DANGER_WILL_ROBINSON (((bit_mask) & 0xFFFFU) != 0) \
	 __DANGER_WILL_ROBINSON (((bit_mask) & ~ 0xFFFFU) == 0) \
	 (((bit_mask) & 0x00FFU) == 0 ? 8 : 0) + \
	 (((bit_mask) & 0x0F0FU) == 0 ? 4 : 0) + \
	 (((bit_mask) & 0x3333U) == 0 ? 2 : 0) + \
	 (((bit_mask) & 0x5555U) == 0 ? 1 : 0))

#define	__LEAST_BIT_32(bit_mask)	\
	(__DANGER_WILL_ROBINSON ((bit_mask) != 0) \
	 (((bit_mask) & 0x0000FFFFUL) == 0 ? 16 : 0) + \
	 (((bit_mask) & 0x00FF00FFUL) == 0 ? 8 : 0) + \
	 (((bit_mask) & 0x0F0F0F0FUL) == 0 ? 4 : 0) + \
	 (((bit_mask) & 0x33333333UL) == 0 ? 2 : 0) + \
	 (((bit_mask) & 0x55555555UL) == 0 ? 1 : 0))
  
#define	__MOST_BIT_8(bit_mask)	\
	(__DANGER_WILL_ROBINSON (((bit_mask) & 0xFFU) != 0) \
	 __DANGER_WILL_ROBINSON (((bit_mask) & ~ 0xFFU) == 0) \
	 (((bit_mask) & 0xF0U) != 0 ? 4 : 0) + \
	 (((bit_mask) & 0xCCU) != 0 ? 2 : 0) + \
	 (((bit_mask) & 0xAAU) != 0 ? 1 : 0))

#define	__MOST_BIT_16(bit_mask)	\
	(__DANGER_WILL_ROBINSON (((bit_mask) & 0xFFFFU) != 0) \
	 __DANGER_WILL_ROBINSON (((bit_mask) & ~ 0xFFFFU) == 0) \
	 (((bit_mask) & 0xFF00U) != 0 ? 8 : 0) + \
	 (((bit_mask) & 0xF0F0U) != 0 ? 4 : 0) + \
	 (((bit_mask) & 0xCCCCU) != 0 ? 2 : 0) + \
	 (((bit_mask) & 0xAAAAU) != 0 ? 1 : 0))

#define	__MOST_BIT_32(bit_mask)	\
	(__DANGER_WILL_ROBINSON ((bit_mask) != 0) \
	 (((bit_mask) & 0xFFFF0000UL) != 0 ? 16 : 0) + \
	 (((bit_mask) & 0xFF00FF00UL) != 0 ? 8 : 0) + \
	 (((bit_mask) & 0xF0F0F0F0UL) != 0 ? 4 : 0) + \
	 (((bit_mask) & 0xCCCCCCCCUL) != 0 ? 2 : 0) + \
	 (((bit_mask) & 0xAAAAAAAAUL) != 0 ? 1 : 0))

#if	__GNUC__ && (defined (i386) || _I386)

/*
 * For the speed-obsessed, here are in-line versions for GCC on Intel i386/
 * i486 processors.
 */

#if	__CHAR_BIT != 8 || __SHRT_BIT != 16 || __LONG_BIT != 32
# error	Do you *really* have an i386/i486 system?
#endif

#include <common/ccompat.h>
#include <common/xdebug.h>
#include <common/__types.h>

__LOCAL__ __INLINE__ __uint_t (__LEAST_BIT_8) (__ulong_t _bit_mask) {
	int		_result;
	_TRICKS_ASSERT ((_bit_mask & 0xFFU) != 0); 
	_TRICKS_ASSERT ((_bit_mask & ~ 0xFFU) == 0); 
	__NON_ISO (asm) volatile ("bsf %1,%0" : "=r" (_result) :
				  "g" (_bit_mask));
	return _result;
}

__LOCAL__ __INLINE__ __uint_t (__LEAST_BIT_16) (__ulong_t _bit_mask) {
	int		_result;
	_TRICKS_ASSERT ((_bit_mask & 0xFFFFU) != 0);
	_TRICKS_ASSERT ((_bit_mask & ~ 0xFFFFU) == 0);
	__NON_ISO (asm) volatile ("bsf %1,%0" : "=r" (_result) :
				  "g" (_bit_mask));
	return _result;
}

__LOCAL__ __INLINE__ __uint_t (__LEAST_BIT_32) (__ulong_t _bit_mask) {
	int		_result;
	_TRICKS_ASSERT (_bit_mask != 0);
	__NON_ISO (asm) volatile ("bsf %1,%0" : "=r" (_result) :
				  "g" (_bit_mask));
	return _result;
}

__LOCAL__ __INLINE__ __uint_t (__MOST_BIT_8) (__ulong_t _bit_mask) {
	int		_result;
	_TRICKS_ASSERT ((_bit_mask & 0xFFU) != 0);
	_TRICKS_ASSERT ((_bit_mask & ~ 0xFFU) == 0);
	__NON_ISO (asm) volatile ("bsr %1,%0" : "=r" (_result) :
				  "g" (_bit_mask));
	return _result;
}

__LOCAL__ __INLINE__ __uint_t (__MOST_BIT_16) (__ulong_t _bit_mask) {
	int		_result;
	_TRICKS_ASSERT ((_bit_mask & 0xFFFFU) != 0);
	_TRICKS_ASSERT ((_bit_mask & ~ 0xFFFFU) == 0);
	__NON_ISO (asm) volatile ("bsr %1,%0" : "=r" (_result) :
				  "g" (_bit_mask));
	return _result;
}

__LOCAL__ __INLINE__ __uint_t (__MOST_BIT_32) (__ulong_t _bit_mask) {
	int		_result;
	_TRICKS_ASSERT (_bit_mask != 0);
	__NON_ISO (asm) volatile ("bsr %1,%0" : "=r" (_result) :
				  "g" (_bit_mask));
	return _result;
}

/*
 * Make the portable versions go away... we leave them around up to this point
 * for those people that want to check that the fast versions really work :-)
 */

#undef	__LEAST_BIT_8
#undef	__LEAST_BIT_16
#undef	__LEAST_BIT_32
#undef	__MOST_BIT_8
#undef	__MOST_BIT_16
#undef	__MOST_BIT_32

#endif	/* __GNUC__ && defined (i386) */


#if	__CHAR_BIT == 8
# define	__LEAST_BIT_UCHAR(bit_mask)	__LEAST_BIT_8(bit_mask)
# define	__MOST_BIT_UCHAR(bit_mask)	__MOST_BIT_8(bit_mask)
#else
# error	Expecting 8 bits/char!
#endif

#if	__SHRT_BIT == 16
# define	__LEAST_BIT_USHRT(bit_mask)	__LEAST_BIT_16(bit_mask)
# define	__MOST_BIT_USHRT(bit_mask)	__MOST_BIT_16(bit_mask)
#elif	__SHRT_BIT == 32
# define	__LEAST_BIT_USHRT(bit_mask)	__LEAST_BIT_32(bit_mask)
# define	__MOST_BIT_USHRT(bit_mask)	__MOST_BIT_32(bit_mask)
#else
# error	Expecting 16 or 32 bits per short!
#endif

#if	__INT_BIT == 16
# define	__LEAST_BIT_UINT(bit_mask)	__LEAST_BIT_16(bit_mask)
# define	__MOST_BIT_UINT(bit_mask)	__MOST_BIT_16(bit_mask)
#elif	__INT_BIT == 32
# define	__LEAST_BIT_UINT(bit_mask)	__LEAST_BIT_32(bit_mask)
# define	__MOST_BIT_UINT(bit_mask)	__MOST_BIT_32(bit_mask)
#else
# error	Expecting 16 or 32 bits per integer!
#endif

#if	__LONG_BIT == 16
# define	__LEAST_BIT_ULONG(bit_mask)	__LEAST_BIT_16(bit_mask)
# define	__MOST_BIT_ULONG(bit_mask)	__MOST_BIT_16(bit_mask)
#elif	__LONG_BIT == 32
# define	__LEAST_BIT_ULONG(bit_mask)	__LEAST_BIT_32(bit_mask)
# define	__MOST_BIT_ULONG(bit_mask)	__MOST_BIT_32(bit_mask)
#else
# error	Expecting 16 or 32 bits per long!
#endif

/*
 * This is not really a trick, it's too obvious. However, it is so frequently
 * used that we keep it around.
 */

#define	__ARRAY_LENGTH(array_type_or_object) \
		(sizeof (array_type_or_object) / \
			 sizeof ((array_type_or_object) [0]))

#endif	/* ! defined (__COMMON__TRICKS_H__) */

