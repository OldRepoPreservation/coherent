/* (-lgl
 * 	COHERENT 386 Device Driver Kit release 2.0
 * 	Copyright (c) 1982, 1992 by Mark Williams Company.
 * 	All rights reserved. May not be copied without permission.
 -lgl) */

/*
 * limits.h
 * C numerical limits header.
 * Draft Proposed ANSI C Standard, 12/7/88 draft.
 * Sections 2.2.4.2, 4.1.4.
 *	2's complement arithmetic
 *	char	8 bits, sign-extended
 *	short	16 bits
 *	int	16 bits for i8086, 32 bits for i386
 *	long	32 bits
 */

#ifndef	__LIMITS_H__
#define	__LIMITS_H__

#include <common/feature.h>
#include <common/_limits.h>

#define	CHAR_BIT		__CHAR_BIT
#define	UCHAR_MAX		__UCHAR_MAX
#define	CHAR_MAX		__CHAR_MAX
#define	CHAR_MIN		__CHAR_MIN
#define	SCHAR_MAX		__SCHAR_MAX
#define	SCHAR_MIN		__CSHAR_MIN

#define	USHRT_MAX		__USHRT_MAX
#define	SHRT_MAX		__SHRT_MAX
#define	SHRT_MIN		__SHRT_MIN

#define	UINT_MAX		__UINT_MAX
#define	INT_MAX			__INT_MAX
#define	INT_MIN			__INT_MIN

#define	ULONG_MAX		__ULONG_MAX
#define	LONG_MAX		__LONG_MAX
#define	LONG_MIN		__LONG_MIN

#define	MB_LEN_MAX		1

#endif	/* ! defined (__LIMITS_H__) */
