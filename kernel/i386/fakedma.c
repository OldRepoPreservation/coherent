/*
 * these routines are written in C until the 386 compiler/assembler
 * are available
 *
 * Copyright (c) Ciaran O'Donnell, Bievres (FRANCE), 1991
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

#define	min(a, b)	((a) < (b) ? (a) : (b))


dmacopy(npage, from, to) 
long	npage;
cseg_t	*from, *to;
{
	int save = setspace(SEG_386_KD);

	while (npage--) {
		ptable1_v[WORK0] = *from++ | SEG_SRW;
		ptable1_v[WORK1] = *to++ | SEG_SRW;
		mmuupd();
		copyseg_d(ctob(1), ctob(WORK0), ctob(WORK1));
	}
	setspace(save);
}


/*
 * dmaclear()
 *
 * Given a byte count, a system absolute address, and a fill value,
 * write the fill value through the given range of memory.
 */
dmaclear(nbytes, to, fill)
long	nbytes, fill;
paddr_t	to;
{
	unsigned off;
	int	n;
	cseg_t *base;
	int save = setspace(SEG_386_KD);

	off = to & (NBPC-1);
	base = &sysmem.u.pbase[btocrd(to)];
	n = min(nbytes, NBPC-off);
	ptable1_v[WORK0] = *base++ | SEG_SRW;
	mmuupd();
	
	clearseg_d(n, ctob(WORK0)+off, fill);
	nbytes -= n;

	while (nbytes >= ctob(1)) {
		ptable1_v[WORK0] = *base++ | SEG_SRW;
		mmuupd();
		clearseg_d(ctob(1), ctob(WORK0), fill);
		nbytes -= ctob(1);
	}

	if (nbytes) {
		ptable1_v[WORK0] = *base++ | SEG_SRW;
		mmuupd();
		clearseg_d(nbytes, ctob(WORK0), fill);
	}
	setspace(save);
}

dmain(nbytes, to, vaddr)
long	nbytes;
paddr_t	to;
vaddr_t	vaddr;
{
	unsigned off;
	unsigned	n, n1;
	cseg_t* base;
	int save = setspace(SEG_386_KD);

	off = to & (NBPC-1);
	base = &sysmem.u.pbase[btocrd(to)];

	n = min(nbytes, NBPC-off);
	ptable1_v[WORK0] = *base++ | SEG_SRW;
	mmuupd();

	if (nbytes==n) {
		/*
		 * only one page
		 * n = min(n & (sizeof(long)-1), n)
		 * copy n bytes; nbytes -= n;
		 * copy (nbytes >> 2) long words; nbytes &= sizeof(long)-1
		 * copy nbytes bytes
		 */
		if (n >= sizeof(long))
			n &= sizeof(long)-1;
		if (n)
			copyseg_b(n, ctob(WORK0)+off, vaddr);
		off += n;
		vaddr += n;
		nbytes -= n;
		if (n = nbytes & ~(sizeof(long)-1)) {
			copyseg_d(n, ctob(WORK0)+off, vaddr);
			off += n;
			vaddr += n;
			nbytes -= n;
		}
	} else {
		/*
		 * more than one page
		 * copy n&3 bytes
		 * copy n >> 2 long words
		 * in the first page
		 */			
		if (n1 = n & 3)
			copyseg_b(n1, ctob(WORK0)+off, vaddr);
		off += n1;
		vaddr += n1;
		nbytes -= n1;
		if (n = n & ~(sizeof(long)-1)) {
			copyseg_d(n, ctob(WORK0)+off, vaddr);
			off += n;
			vaddr += n;
			nbytes -= n;
		}

		/*
		 * copy nbytes>>BPCSHIFT full pages
		 */
		while (nbytes >= ctob(1)) {
			ptable1_v[WORK0] = *base++ | SEG_SRW;
			mmuupd();
	
			copyseg_d(ctob(1), ctob(WORK0), vaddr);
			vaddr += ctob(1);
			nbytes -= ctob(1);
		}
		/*
		 * page n-1 (last one)
		 *
		 * copy nbytes>>2 long words
		 * copy nbytes & 3 bytes
		 */
		ptable1_v[WORK0] = *base++ | SEG_SRW;
		mmuupd();
	
		if (n = nbytes & ~(sizeof(long)-1)) {
			copyseg_d(n, ctob(WORK0), vaddr);
			vaddr += n;
			nbytes -= n;
		}
		if (nbytes)
			copyseg_b(nbytes, ctob(WORK0)+n, vaddr);
	}

	setspace(save);
}

dmaout(nbytes, to, vaddr)
long	nbytes;
paddr_t	to;
vaddr_t	vaddr;
{
	unsigned off;
	unsigned	n, n1;
	cseg_t *base;
	int save = setspace(SEG_386_KD);

	off = to & (NBPC-1);
	base = &sysmem.u.pbase[btocrd(to)];

	n = min(nbytes, NBPC-off);
	ptable1_v[WORK0] = *base++ | SEG_SRW;
	mmuupd();

	if (nbytes==n) {
		/*
		 * only one page
		 * n = min(n & (sizeof(long)-1), n)
		 * copy n bytes; nbytes -= n;
		 * copy (nbytes >> 2) long words; nbytes &= sizeof(long)-1
		 * copy nbytes bytes
		 */
		if (n1 = n & (sizeof(long)-1))
			copyseg_b(n1, vaddr, ctob(WORK0)+off);
		off += n1;
		vaddr += n1;
		nbytes -= n1;
		if (n = nbytes & ~(sizeof(long)-1)) {
			copyseg_d(n, vaddr, ctob(WORK0)+off);
			off += n;
			vaddr += n;
			nbytes -= n;
		}
	} else {
		/*
		 * more than one page
		 * copy n&3 bytes
		 * copy n >> 2 long words
		 * in the first page
		 */			
		if (n1 = n & (sizeof(long)-1))
			copyseg_b(n1, vaddr, ctob(WORK0)+off);
		off += n1;
		vaddr += n1;
		nbytes -= n1;
		if (n = n & ~(sizeof(long)-1)) {
			copyseg_d(n, vaddr, ctob(WORK0)+off);
			off += n;
			vaddr += n;
			nbytes -= n;
		}

		/*
		 * copy nbytes>>BPCSHIFT full pages
		 */
		while (nbytes >= ctob(1)) {
			ptable1_v[WORK0] = *base++ | SEG_SRW;
			mmuupd();
			copyseg_d(ctob(1), vaddr, ctob(WORK0));
			vaddr += ctob(1);
			nbytes -= ctob(1);
		}
		/*
		 * page n-1 (last one)
		 *
		 * copy nbytes>>2 long words
		 * copy nbytes & 3 bytes
		 */
		ptable1_v[WORK0] = *base++ | SEG_SRW;
		mmuupd();
	
		if (n = nbytes & ~(sizeof(long)-1)) {
			copyseg_d(n, vaddr, ctob(WORK0));
			vaddr += n;
			nbytes -= n;
		}
		if (nbytes)
			copyseg_b(nbytes, vaddr, ctob(WORK0)+off);
	}

	setspace(save);
}

dmaio2(nbytes, to, port)
long	nbytes, port;
paddr_t	to;
{
	unsigned off;
	int	n;
	cseg_t *base;
	int save = setspace(SEG_386_KD);

	off = to & (NBPC-1);
	base = &sysmem.u.pbase[btocrd(to)];

	n = min(nbytes, NBPC-off);
	ptable1_v[WORK0] = *base++ | SEG_SRW;
	mmuupd();
	
	io2seg(n, ctob(WORK0)+off, port);
	nbytes -= n;

	while (nbytes >= ctob(1)) {
		ptable1_v[WORK0] = *base++ | SEG_SRW;
		mmuupd();
		io2seg(ctob(1), ctob(WORK0), port);
		nbytes -= ctob(1);
	}

	if (nbytes) {
		ptable1_v[WORK0] = *base++ | SEG_SRW;
		mmuupd();
		io2seg(nbytes, ctob(WORK0), port);
	}
	setspace(save);
}

dma2io(nbytes, to, port)
long	nbytes, port;
paddr_t	to;
{
	unsigned off;
	int	n;
	cseg_t *base;
	int save = setspace(SEG_386_KD);

	off = to & (NBPC-1);
	base = &sysmem.u.pbase[btocrd(to)];

	n = min(nbytes, NBPC-off);
	ptable1_v[WORK0] = *base++ | SEG_SRW;
	mmuupd();
	
	seg2io(n, ctob(WORK0)+off, port);
	nbytes -= n;

	while (nbytes >= ctob(1)) {
		ptable1_v[WORK0] = *base++ | SEG_SRW;
		mmuupd();
		seg2io(ctob(1), ctob(WORK0), port);
		nbytes -= ctob(1);
	}

	if (nbytes) {
		ptable1_v[WORK0] = *base++ | SEG_SRW;
		mmuupd();
		seg2io(nbytes, ctob(WORK0), port);
	}
	setspace(save);
}

pxcopy(uo, v, n, space)
unsigned	uo;
char	*v;
register int n;
{
	cseg_t *base;
	register	int save, err;

	if (n > NBPC)
		return 0;

	if (space & SEG_VIRT) {
		space &= ~SEG_VIRT;
		base = &sysmem.u.pbase[btocrd(uo)];
		ptable1_v[WORK0] = *base++ | SEG_SRW;
		ptable1_v[WORK1] = *base++ | SEG_SRW;
	} else {
		ptable1_v[WORK0] = (uo&~(NBPC-1)) + SEG_SRW;
		ptable1_v[WORK1] = (uo&~(NBPC-1)) + NBPC + SEG_SRW;
	}
	mmuupd();
	save = setspace(space);

	err = ukcopy(ctob(WORK0) + (uo&(NBPC-1)), v, n);
	setspace(save);
	return err;
}

xpcopy(v, uo, n, space)
char	*v;
unsigned uo;
register int n;
{
	register cseg_t *base;
	register	int save, err;

	if (n > NBPC)
		return 0;

	if (space & SEG_VIRT) {
		space &= ~SEG_VIRT;
		base = &sysmem.u.pbase[btocrd(uo)];
		ptable1_v[WORK0] = *base++ | SEG_SRW;
		ptable1_v[WORK1] = *base++ | SEG_SRW;
	} else {
		ptable1_v[WORK0] = (uo&~(NBPC-1)) + SEG_SRW;
		ptable1_v[WORK1] = (uo&~(NBPC-1)) + NBPC + SEG_SRW;
	}
	mmuupd();
	save = setspace(space);

	err = kucopy(v, ctob(WORK0) + (uo&(NBPC-1)), n);
	setspace(save);
	return err;
}
