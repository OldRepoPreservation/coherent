/* $Header: /v4a/i386/RCS/md.c,v 1.2 92/01/06 10:48:05 hal Exp $ */
/* (lgl-
 *	The information contained herein is a trade secret of Mark Williams
 *	Company, and  is confidential information.  It is provided  under a
 *	license agreement,  and may be  copied or disclosed  only under the
 *	terms of  that agreement.  Any  reproduction or disclosure  of this
 *	material without the express written authorization of Mark Williams
 *	Company or persuant to the license agreement is unlawful.
 *
 *	COHERENT Version 2.3.37
 *	Copyright (c) 1982, 1983, 1984.
 *	An unpublished work by Mark Williams Company, Chicago.
 *	All rights reserved.
 -lgl) */
/*
 * Coherent 386
 *	IBM PC
 * Machine dependent stuff.
 *
 */
#include <sys/coherent.h>
#include <sys/reg.h>
#include <sys/clist.h>
#include <errno.h>
#include <sys/inode.h>
#include <sys/proc.h>
#include <sys/seg.h>
#include <signal.h>
#include <sys/uproc.h>
#include <sys/buf.h>

/*
 * Set up a new process.
 */
msetusr(ip, sp)
vaddr_t sp;
vaddr_t ip;
{
	u.u_regl[EIP] = ip;
	u.u_regl[UESP] = sp;
}

/*
 * Set the given address in the user area to the given value if it is
 * okay to do so.
 */
msetuof(a, v)
register int a;
{
	register int *ap;

	ap = (int *)&u+a/sizeof(long);
	if (ap<&u.u_regl[GS] || a>&u.u_regl[SS])
		return (0);
	if (ap == &u.u_regl[ERR])		/* Protect trap id */
		return (0);
	*ap = v;
	return (1);
}


/*
 * Cause the next instruction to single step.
 */
msigsin()
{
	u.u_regl[EFL] |= MFTTB;
}

#if 0
/*
 * Idle kernel process.
 */
idle()
{
	for (;;) {
		disflag = 1;
		_idle();
	}
}
#endif

int nirqslave;
/*
 * Set an interrupt vector.
 * Make an entry in the "vecs" table, for
 * use by the assist. Make sure that the channel
 * on the 8259 is armed.
 * Note that interrupt vectors 2 and 9 are mapped into channel 9.
 */
setivec(level, fun)
register int	level;
int		(*fun)();
{
	register int	picm;
	extern	 int	(*vecs[])();
	extern	 int	vret();

	if ((level &= 0x0F) == 2)
		level = 9;
	if (level==0 || vecs[level]!=&vret) {
		u.u_error = EBUSY;
		return;
	}
	vecs[level] = fun;
	if ( level >= 8 ) {
		++nirqslave;
		picm = inb(SPICM);
		picm &= ~(0x01 << (level-8));
		outb(SPICM, picm);
		level = 2;
	}
	picm = inb(PICM);
	picm &= ~(0x01 << level);
	outb(PICM, picm);
}

/*
 * Clear an interrupt vector.
 */
clrivec(level)
register int	level;
{
	register int	picm;
	extern	 int	(*vecs[])();
	extern	 int	vret();

	if ((level &= 0x0F) == 2)
		level = 9;
	if (level == 0)
		panic("clrivec: level=%d", level);
	vecs[level] = &vret;
	if (level >= 8) {
		--nirqslave;
		picm = inb(SPICM);
		picm |= (0x01 << (level-8));
		outb(SPICM, picm);
		level = 2;
	}
	if ((level != 2) || (nirqslave == 0)) {
		picm = inb(PICM);
		picm |= (0x01 << level);
		outb(PICM, picm);
	}
}


/*
 * Convert an array of filesystem 3 byte
 * numbers to longs. This routine, unlike the old one,
 * is independent of the order of bytes in a long.
 * Bytes have 8 bits, though.
 */
l3tol(lp, cp, nl)
register long *lp;
register unsigned char *cp;
register unsigned nl;
{
	register long l;

	if (nl != 0) {
		do {
			l  = (long)cp[0] << 16;
			l |= (long)cp[1];
			l |= (long)cp[2] << 8;
			cp += 3;
			*lp++ = l;
		} while (--nl);
	}
}
/*
 * Convert an array of longs into an array
 * of filesystem 3 byte numbers. This routine, unlike
 * the old one, is independent of the order of bytes in
 * a long. Bytes have 8 bits.
 */
ltol3(cp, lp, nl)
register char *cp;
register long *lp;
register unsigned nl;
{
	register long l;

	if (nl != 0) {
		do {
			l = *lp++;
			cp[0] = l >> 16;
			cp[1] = l;
			cp[2] = l >> 8;
			cp += 3;
		} while (--nl);
	}
}

/*
 * Convert long to comp_t style number.
 * A comp_t contains 3 bits of base-8 exponent
 * and a 13-bit mantissa.  Only unsigned
 * numbers can be comp_t numbers.
 */

#include <sys/types.h>

#define	MAXMANT		017777		/* 2^13-1 = largest mantissa */

comp_t
ltoc(l)
long l;
{
	register int exp;

	if (l < 0)
		return (0);
	for (exp = 0; l > MAXMANT; exp++)
		l >>= 3;
	return ((exp<<13) | l);
}

