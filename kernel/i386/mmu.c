/*
 * MMU dependent code for Coherent 386
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
#include <sys/alloc.h>
#include <l.out.h>
#include <ieeefp.h>

/* These defines belong somewhere else:  */
#define LOMEM	0x15	/* CMOS address of size in K of memory below 1MB.  */
#define EXTMEM	0x17	/* CMOS address of size in K of memory above 1MB.  */
#define ONE_K	1024
#define ONE_MEG	1048576
#define USE_NDATA	1
#define SUNLOAD		0

/*
 * DMA will not work to memory above 16M, so limit the amount of memory
 * above 1M to 15M.  A much cleverer scheme should be implemented.
 */
int HACK_LIMIT = (15*ONE_MEG);

/*
 * For 0 < i < 64, buddysize[i] is log(base 2) of nearest power of two
 * which is greater than or equal to i.
 */
char	buddysize[64] = {
	-1, 0, 1, 2, 2, 3, 3, 3,
	3, 4, 4, 4, 4, 4, 4, 4,
	4, 5, 5, 5, 5, 5, 5, 5,
	5, 5, 5, 5, 5, 5, 5, 5,
	5, 6, 6, 6, 6, 6, 6, 6,
	6, 6, 6, 6, 6, 6, 6, 6,
	6, 6, 6, 6, 6, 6, 6, 6,
	6, 6, 6, 6, 6, 6, 6, 6 };

#define	min(a, b)	((a) < (b) ? (a) : (b))

/*
 * Functions.
 *	Import.
 *	Export.
 *	Local.
 */
void		areacheck();
void		areafree();
void		areainit();
BLOCKLIST *	arealloc();
int		areasize();
cseg_t *	c_alloc();
cseg_t *	c_extend(); 
void		c_free();
int		c_grow();
int		countsize();
void		doload();
void		i8086();
void		idtinit();
void		init_phy_seg();
void		mchinit();
void		msigend();
void		msigstart();
SR		*loaded();
unsigned int	read16_cmos();
void		segload();
void		sunload();
void		unload();
void		valloc();

#define		zero_fill(from, len)	memset(from, 0, len)

/*
 * "load" a handle "hp"  to a segment into the space tree for a process
 */
void
doload(srp)
register SR	*srp;
{
	register int	n;
	register	cseg_t *pp;
	register int	base1, flags;
	register int	akey;

	pp = srp->sr_segp->s_vmem;
	flags = srp->sr_segp->s_flags;
	base1 = btocrd(srp->sr_base);
	n = btoc(srp->sr_size);

	/*
	 * we load all pages
	 */
	switch (flags&(SFSYST|SFTEXT)) {
	case SFTEXT:	akey = SEG_RO;  break;
	case SFSYST:	akey = SEG_SRW; break;
	default:	akey = SEG_RW;  break;
	}

	do
		ptable1_v[base1++] = *pp++ | akey;
	while (--n);
	mmuupd();
}


/*
 * unload a handle key "key" to a segment from the MMU hardware
 */
void
unload(srp)
register SR *srp;
{
	register int	n, base1;

	base1 = btocrd(srp->sr_base);
	
	n = btoc(srp->sr_size);
	do {
		ptable1_v[base1++] = SEG_ILL;
	} while (--n);
	mmuupd();
}

/*
 * Allocate 'clicks_wanted' clicks of core space.
 * Returns physical segment descriptor if success, else NULL.
 * The physical segment descriptor is a table of page table entries
 * suitable for insertion into a page table.
 */
cseg_t *
c_alloc(clicks_wanted)
unsigned	clicks_wanted;
{
	unsigned	pno;
	cseg_t *pp;
	register cseg_t *qp;

	/* Do we have enough free phsycial clicks for this request?  */
	if (clicks_wanted > allocno())
		goto no_c_alloc;

	/* Allocate some space for the table to return.  */
	if ((pp = (cseg_t *)arealloc(clicks_wanted)) == 0)
		goto no_c_alloc;
	qp = pp;

	/* fill in entries in the requested table */
	do {
		pno = *--sysmem.pfree;
		if (!pvalid(pno))
			panic("c_alloc");
		*qp++ = (clickseg(pno) & ~SEG_BITS) | SEG_PRE;
	} while (--clicks_wanted);
	return pp;

no_c_alloc:
	return 0;
}

/*
 * free core space
 * sz - allocated size
 */
void
c_free(pp, size)
cseg_t	*pp;
unsigned	size;
{
	unsigned	pno;
	register cseg_t *qp;
	register int	sz;
	SR		*srp;

#if SUNLOAD
	sunload(pp);
#else
	if (srp = loaded(pp)) {
		unload(srp);
		srp->sr_segp = 0;
	}
#endif
	sz = size;
	if (&sysmem.pfree[sz] > sysmem.efree)
		panic("c_free - nalloc");
	qp = pp;
	do {
		pno = segclick(*qp++);
		if (!pvalid(pno))
			panic("c_free");
		*sysmem.pfree++ = pno;
	} while (--sz);
	areafree((BLOCKLIST *)pp, size);
}

cseg_t *
c_extend(pp, osz) 
register cseg_t *pp;
{
	register	cseg_t *pp1;
	register unsigned	pno;
	register int	i;
	SR		*srp;

	if (sysmem.pfree < &sysmem.tfree[1])
		goto no_c_extend;

#if SUNLOAD
	sunload(pp);
#else
	if (srp = loaded(pp)) {
		unload(srp);
		srp->sr_segp = 0;
	}
#endif

	/*
	 * If the old size was a power of 2, it has used up an entire
	 * buddy area, so we will need to allocate more space.
	 */
	if (IS_POW2(osz)) {
		if ((pp1 = (cseg_t*) arealloc(osz+1))==0)
			goto no_c_extend;
		for (i=0; i < osz; i++)
			pp1[i] = pp[i];
		areafree(pp, osz);
		pp = pp1;
	}

	for (i=osz; --i >= 0; )
		pp[i+1] = pp[i];

	pno = *--sysmem.pfree;
	if (!pvalid(pno))
		panic("c_extend");
	pp[0] = clickseg(pno) | SEG_RW;
	return pp;

no_c_extend:
	return 0;
}

/*
 * Given segment size in bytes, estimate total space needed
 * to keep track of the segment (I think - hws).
 *
 * return value is num_bytes plus some overhead...
 */
int
countsize(num_bytes)
int num_bytes;
{
	int ret;

	if (num_bytes <= NBPC/sizeof(long))
		ret = num_bytes+1;
	else
		ret = num_bytes
		  + ((num_bytes + NBPC/sizeof(long) - 1) >> BPC1SHIFT) + 1;
	return ret;
}

/*
 * buddy allocation 
 */

/*
 * allocate a segment descriptor area;
 */
void
areafree(sp, sz)
BLOCKLIST *sp;
int sz;
{
	register int	n;	/* adresse du buddy, taille du reste */
	register int	ix, nx;
	register	BLOCKLIST *buddy;

	areacheck(2, sp);
	ix = sp - sysmem.u.budtab;
	n = areasize(sz);
	do {
		nx = BUDDY(ix, n);
		if (sysmem.budfree[nx>>WSHIFT] & 1<<(nx&(WCOUNT-1))) {
			buddy = sysmem.u.budtab + nx; 
			if (buddy->kval != n)
				break;
			sysmem.budfree[nx>>WSHIFT] &= ~ (1<<(nx & (WCOUNT-1)));
			DELETE2(buddy);
			if (nx < ix) 
				ix = nx;
		} else
			break;
	} while (++n < NBUDDY);
	sysmem.budfree[ix>>WSHIFT] |= 1 << (ix & (WCOUNT-1));
	buddy = sysmem.u.budtab + ix;
	INSERT2(BLOCKLIST, buddy, &sysmem.bfree[n]);
	buddy->kval = n;
	areacheck(3, buddy);
}

/*
 * arealloc()
 *
 * Given size in "clicks" of a segment to manage,
 * return pointer to an array of enough descriptors.
 * If not enough free descriptors available, return 0.
 */
BLOCKLIST *
arealloc(clicks)
register int clicks;
{
	register	BLOCKLIST *sp;
	register	BLOCKLIST *p, *q;
	register int	size;
	BLOCKLIST	*rsp;
	register int	nx;

	areacheck(0, 0);
	size = areasize(clicks);
	/*
	 * 1. Find little end, bloc p, free >= size
	 */
	for (q = p = sysmem.bfree + size;p->forw == p; size++, p++)
		if (p >= sysmem.bfree + NBUDDY - 1) {
			return(0);	/* y en a pas */
		}

	rsp = p->forw;
	DELETE2(rsp);
	nx = rsp - sysmem.u.budtab;
	sysmem.budfree[nx>>WSHIFT] &= ~(1 << (nx & (WCOUNT-1)));
	size = 1<<size;
	sp = rsp + size; /* buddy address */
	while (p-- != q) {
		/*
		 * 2.1 The block is too big, uncouple & free buddy
		 */
		sp -= (size >>= 1);
		nx = sp - sysmem.u.budtab;
		sysmem.budfree[nx>>WSHIFT] |= 1 << (nx & (WCOUNT-1));
		INSERT2(BLOCKLIST, sp, p);
		sp->kval = p - sysmem.bfree;
	}
	areacheck(1, rsp);
	return rsp;
}

void
areainit(n)
{
	extern char __end[];
	register int i;

	for (i=0; i < (1<<(NBUDDY-WSHIFT)); i++)
		sysmem.budfree[i] = 0;
	for (i=0; i<NBUDDY; i++)
		INIT2(&sysmem.bfree[i]);
	sysmem.u.budtab = (BLOCKLIST *)__end;
	n /= sizeof(BLOCKLIST);
	if (n > (1 << NBUDDY))
		panic("areainit");
	for (i=0; i<n; i++)
		areafree(&sysmem.u.budtab[i], sizeof(BLOCKLIST)/sizeof(long));
}

/*
 * areasize()
 *
 * Do a log(base 2) calculation on n.
 * If n is zero, return -1.
 *
 * Else, consider the nearest power of two which is greater than or
 * equal to n
 *	p/2 < n <= p
 * Then set p = 4 * (2**x).  Note BLKSZ is 2.
 * Return max(x,0).
 *
 * If n is too large (more than 3F00), we will go beyond the limits of
 * table buddysize[].
 *
 * In practice, n is the total number of clicks needed in a segment,
 * and the return value will be used to access a buddy system list.
 */
int
areasize(n)
register unsigned int	n;
{
	register int m;
#ifdef FROTZ
	int ret, oldn = n;
#endif

	if (n > 0x3F00)
		panic("areasize");

	n = (n + (1 << BLKSZ) - 1) >> BLKSZ;
	m = n & 0x3F;
#ifdef FROTZ
	if ((n >>= 6) == 0)
		ret = buddysize[m];
	else {
		int index;

		index = n;
		if (m)
			index++;
		ret = buddysize[index] + 6;
	}
	return ret;
#else
	if ((n >>= 6) == 0)
		return buddysize[m];
	return buddysize[n + ((m!=0)?1:0)] + 6;
#endif
}

#define	MAXBUDDY	2048
#define	CHECK(p) ((p>=&sysmem.bfree[0] && p<&sysmem.bfree[NBUDDY]) || \
		(p>=sysmem.u.budtab && p<&sysmem.u.budtab[1<<NBUDDY]))
void
areacheck(flag, sp)
register	BLOCKLIST *sp;
{
	register	BLOCKLIST *next, *start;
	register int i, nx;

	if (sp) {
		if (&sysmem.u.budtab[sp-sysmem.u.budtab] != sp)
		  printf("*check* %d %x %x\n", flag, sp, sysmem.u.budtab);
	}
		
	for (i=0; i<NBUDDY; i++) {
		start = next = &sysmem.bfree[i];
		do {
			next = next->forw;
			if (!CHECK(next))
				printf("next = %x (%d)\n", next, flag);
			if (next->back != start)
				printf("%x->forw->back != %x\n", next, start);
			if (next != &sysmem.bfree[i]) {
				if (next->kval != i)
					printf("bad kval %x, %d (%d)\n",
						next, next->kval, flag);
				nx = next - sysmem.u.budtab;
				if ((sysmem.budfree[nx>>WSHIFT] & (1 << (nx & (WCOUNT-1)))) == 0)
					printf("in bfree but not budfree %x (%d)\n", next, flag);
			}
			start = next;
		} while (next != &sysmem.bfree[i]);
	}
}

#undef	ptable1_v
#undef	ptable0_v
#define	ptable0_v	((long *)(&stext[ctob(-1)]))

int total_clicks;	/* How many clicks did we start with?  */

void
mchinit()
{
	extern char __end[], __end_data[], stext[], __end_text[], sdata[];
	int lo;		/* Number of bytes of physical memory below 640K.  */
	int hi;		/* Number of bytes of physical memory above 1M.  */
	register char *pe; 
	register int zero = 0;
	register int	i;
	register	long *ptable1_v;
	register unsigned short	base;
	int	sysseg, codeseg, stackseg, ramseg, ptable1;
	int	ptoff;	/* An offset into ptable0_v[]  */
#if USE_NDATA
	int	dataseg[NDATA];
#else
	int	dataseg;
#endif
	int	nalloc, diff;
	extern char	digtab[];
	static	SEG	uinit;

	/*
	 * 1.
	 *   a. Relocate the data on a page boundary (4K bytes) the
	 *      bootstrap relocates it on a paragraph boundary (16 bytes)
	 *
	 *   b. Verify that the data has been relocated correctly 
	 */
	pe = __end_data;						/* 1.a */
	i = (((unsigned)__end_text+15) & ~15) - (unsigned)sdata;
	do {
		pe--;
		pe[0] = pe[i];
	} while (pe != sdata);					/* 1.b */

	/*
	 * Can now access the .data segment from C.
	 * If not, next loop will hang the kernel.
	 */
	CHIRP('A');
	while (digtab[0]!='0');
	CHIRP('*');

	/*
	 * 2. Zero the bss
	 *    Zero the level 0 page directory
	 */ 
	pe = __end_data;
	do
		*pe++ = zero;
	while (pe != __end);

	pe = (char *) ptable0_v;
	do
		*pe++ = zero;
	while (pe != stext);

	CHIRP('2');

	/*
	 * 3. Calculate total system memory in taking
	 *    into account the space used by the system and the page
	 *    descriptors, the interrupt stack, and the refresh work area
	 *
	 * a. initialize allocation area and adjust system size
	 *	to take allocation area and free page area into account
	 */
	sysmem.lo = btoc((unsigned)__end) - SBASE + PBASE;

	/*
	 * lo is the size in bytes of memory between the end of the kernel
	 *	and the end of memory below 640K.
	 * hi is the size in bytes of memory over 1 Megabyte (Extended memory).
	 *
	 * Round the sizes from the CMOS down to the next click.  This
	 * compensates for systems where the CMOS reports sizes that are
	 * not multiples of 4K.
	 */
	lo = ctob(read16_cmos(LOMEM) >> 2) - ctob(sysmem.lo);
	hi = ctob(read16_cmos(EXTMEM) >> 2);

	T_PIGGY( 0x400, {
		strchirp(" cmos lo: ");
		print16(read16_cmos(LOMEM));
		strchirp(" cmos hi: ");
		print16(read16_cmos(EXTMEM));
		strchirp(" lo: ");
		print32(lo);
		strchirp(" hi: ");
		print32(hi);
	} );

	/*
	 * Sometimes, we die horribly if there is too much memory.
	 * Artificially limit hi to HACK_LIMIT.
	 */
	if (hi > HACK_LIMIT)
		hi = HACK_LIMIT;

	CHIRP('z');
	zero_fill(ctob(sysmem.lo+SBASE-PBASE), lo);
	CHIRP('Z');
	zero_fill(ONE_MEG+ctob(SBASE-PBASE), hi);
	CHIRP('Y');
	
	/* Record the total memory for later use.  */
	total_mem = ctob(sysmem.lo) + lo + hi;

	nalloc = (lo+hi) / (sizeof(short) + SPLASH*sizeof(long) + NBPC);
	/*
	 * ASSERT:
	 * For the moment we want only to assure that the
	 * BUDDY arena and the stack of free pages will fit below
	 * 640K.
	 */
#define SIZEOF_BUDDY ( (unsigned)SPLASH*nalloc*sizeof(long) )
#define SIZEOF_FREE_PAGES ( ( btoc(hi) + btoc(lo) )* sizeof(short) )
	T_PIGGY( 0x800, {
		if ( SIZEOF_BUDDY + SIZEOF_FREE_PAGES >= lo ) {
			strchirp("Too much memory");
			panic("Too much memory");
		}
	} );


	/*
	 * Initialize the buddy system arena.  This memory is used
	 * for the compressed page tables.
	 */
	areainit(SPLASH*nalloc*sizeof(long));

	/*
	 * Initialize the stack of free pages.
	 */
	sysmem.tfree = sysmem.pfree = 
		(unsigned short *)(__end + SPLASH*nalloc*sizeof(long));

	sysmem.hi = btoc(hi+1024*1024);
	base = sysmem.lo + (lo>>BPCSHIFT);
	diff = ((lo + hi) >> BPCSHIFT) - nalloc;
	sysmem.lo += diff;	
	sysmem.vaddre = ctob(sysmem.lo+SBASE-PBASE);
	/* include in system area pages for arena, free area */

	CHIRP('3');

	/*
	 * 4.
	 *  Free the memory from [end, 640) kilobytes
	 *  Free the memory from [1024, 16*1024) kilobytes
	 *
	 *  We are building a stack of free pages bounded below
	 *  by sysmem.tfree and above by sysmem.efree.  sysmem.pfree
	 *  is the top of the stack.  The stack grows upwards.
	 */
	total_clicks = 0;
	while (base > sysmem.lo) {
		*sysmem.pfree++ = --base;
		++total_clicks;
	}

	base = btoc(1024*1024);
	while (base < sysmem.hi) {
		*sysmem.pfree++ = base++;
		++total_clicks;
	}

	sysmem.efree = sysmem.pfree;

	T_PIGGY( 0x400, {
		strchirp("  sysmem.efree: ");
		print32(sysmem.efree);
		strchirp("  nalloc: ");
		print32(nalloc);
		strchirp("  total_clicks: ");
		print32(total_clicks);
		strchirp("  allocno(): ");
		print32(allocno());
	} );

	T_PIGGY( 0x800, {
		/*
		 * ASSERT:  The stack of free pages should end within a click
		 * of the lowest available memory.
		 */
		if ( (cseg_t *)ctob(sysmem.lo+SBASE-PBASE) < sysmem.efree ) {
			strchirp("sysmem.lo is too low: ");
			print32(ctob(sysmem.lo+SBASE-PBASE));
			strchirp("  sysmem.efree: ");
			print32(sysmem.efree);
			panic("sysmem.lo is too low");
		}

		if ( sysmem.efree < (cseg_t *)ctob(sysmem.lo+SBASE-PBASE - 1)){
			strchirp("sysmem.efree is too low: ");
			print32(sysmem.efree);
			strchirp("  sysmem.lo-1: ");
			print32(ctob(sysmem.lo+SBASE-PBASE - 1));
			panic("sysmem.efree is too low");
		}

		/*
		 * ASSERT:  There should be nalloc total_clicks.
		 */
		if ( nalloc != total_clicks ) {
			strchirp("nalloc != total_clicks: ");
			print32(nalloc);
			strchirp(" != ");
			print32(total_clicks);
			panic("nalloc != total_clicks ");
		}
	} );

	/*
	 * We may want to be able to stop at this point so we can see
	 * anything that has just been printed.
	 */
	T_PIGGY( 0x80, {
		for (;;) {
			/* DO NOTHING FOREVER */
		}
	} );

	CHIRP('4');

	/*
	 * 5. allocate page entries and initialize level 0 ^'s
	 * a. [ 00000000 .. 003FFFFF )		user code segment
	 * b. [ 00400000 .. 007FFFFF )		user data & bss
	 * c. [ 7FC00000 .. 7FFFFFFF )		user stack
	 *c.i.[ 80000000 .. 80FFFFFF )		ram disk
	 * d. [ FF800000 .. FFBFFFFF )		pointers to level 1 page table
	 * e. [ FFC00000 .. FFFFFFFF )		system process addresses
	 */
	codeseg = clickseg(*--sysmem.pfree);		/* 5.a */
	ptable0_v[0x000] = codeseg  | DIR_RW; 

#if USE_NDATA
	for (i = 0; i < NDATA; i++) {
		dataseg[i] = clickseg(*--sysmem.pfree);	/* 5.b */
		ptable0_v[0x001+i] = dataseg[i] | DIR_RW;
	}
#else
	dataseg = clickseg(*--sysmem.pfree);		/* 5.b */
	ptable0_v[0x001] = dataseg | DIR_RW;
#endif

	stackseg = clickseg(*--sysmem.pfree);		/* 5.c */
	ptable0_v[0x1FF] = stackseg  | DIR_RW; 

	ptable1 = clickseg(*--sysmem.pfree);		/* 5.d */
	ptable0_v[0x3FE] = ptable1 | DIR_RW; 

	sysseg = clickseg(*--sysmem.pfree);		/* 5.e */
	ptable0_v[0x3FF] = sysseg  | DIR_RW;

	CHIRP('5');

	/*
	 * 6. initialize  level 2 ^'s to [5.d]
	 */

	ptable1_v  = (long *)(ptable1 + ctob(SBASE-PBASE));
	ptable1_v[0x000] = codeseg | SEG_SRW;
#if USE_NDATA
	for (i = 0; i < NDATA; i++)
		ptable1_v[0x001+i] = dataseg[i] | SEG_SRW;
#else
	ptable1_v[0x001] = dataseg | SEG_SRW;
#endif
	ptable1_v[0x1FF] = stackseg| SEG_SRW;

	/*
	 * This ram disk stuff should go away once the scheme
	 * for allocating pieces of virtual memory space is in place.
	 */
	for (ptoff = 0x200; ptoff < 0x204; ++ptoff) {
		ramseg =  clickseg(*--sysmem.pfree);		/* 5.c.i */
		ptable0_v[ptoff] = ramseg  | DIR_RW; 
		ptable1_v[ptoff] = ramseg | SEG_SRW;
	}

	ptable1_v[0x3FF] = sysseg  | SEG_SRW;

	CHIRP('6');

	/*
	 * 7.
	 * b. map kernel code and data
	 * 	map ^ to:
	 * c. 	level 0 page table
	 * d. 	level 1 page table
	 * e. 	I/O segments (video RAM, ...) 
	 */ 

	ptable1_v  = (long *)(sysseg + ctob(SBASE-PBASE));	/* 7.b */
	for (i = PBASE; i <sysmem.lo; i++)
		ptable1_v[i-PBASE] = clickseg(i) | SEG_SRW;

	ptable1_v[0x3FE] = clickseg(PTABLE0_P) | SEG_SRW;	/* 7.c */
	ptable1_v[0x3FD] = ptable1 | SEG_SRW;			/* 7.d */

	init_phy_seg(ptable1_v, ROM-SBASE,   0x0000F0000);	/* 7.e. */
	init_phy_seg(ptable1_v, VIDEOa-SBASE,0x0000B0000);
	init_phy_seg(ptable1_v, VIDEOb-SBASE,0x0000B8000);

	CHIRP('7');

	/*
	 * 8. allocate and map U area
	 */

	uinit.s_flags = SFSYST|SFCORE;
	uinit.s_size = UPASIZE;
	uinit.s_vmem = c_alloc(btoc(UPASIZE));
	ptable1_v[0x3FF] = *uinit.s_vmem | SEG_SRW;
	procq.p_segp[SIUSERP] = &uinit;

	CHIRP('8');

	/*
	 * 9. make FFC00000 and 00002000 map to the same address
	 * to prevent the prefetch after the instruction turning on
	 * paging from causing a page fault
	 */
	ptable1_v  = (long *)(codeseg + ctob(SBASE-PBASE));
	ptable1_v[PBASE] = clickseg(PBASE) | SEG_SRW;

	CHIRP('9');

	/*
	 * 10. load page table base address into MMU
	 *	fix up the interrupt vectors
	 */
	mmuupd();
	CHIRP('U');
	idtinit();
	CHIRP('I');
}

typedef struct
{
	unsigned short	off_lo;
	unsigned short	seg;
	unsigned short	flags;
	unsigned short	off_hi;
} IDT;

/*
 * ldtinit()
 *
 * Fix up descriptors which are hard to create properly at compile/link time.
 * Apply to idt and ldt.
 *
 * Swap 16-bit words at descriptor+2, descriptor+6.
 */
void
idtinit()
{
	extern IDT	idt[], idtend[];
	extern IDT	ldt[], ldtend[];
	register IDT *ip;
	register unsigned short tmp;

	for (ip = idt; ip <idtend; ip++) {
		tmp = ip->off_hi;
		ip->off_hi = ip->seg;
		ip->seg = tmp;
	}

	for (ip = ldt; ip <ldtend; ip++) {
		tmp = ip->off_hi;
		ip->off_hi = ip->seg;
		ip->seg = tmp;
	}
}

void
init_phy_seg(ptable1_v, addr, base)
long	*ptable1_v;
{
	register int i;

	for (i=0; i<btoc(0x10000); i++) {
		ptable1_v[addr+i] = base | SEG_SRW; 
		base += NBPC;
	}
}

/*
 * Load up segmentation registers.
 */
SR	ugmtab[NUSEG];

void
segload()
{
	register int i;
	register	SR *start;

	/*
	 * 1. unprogram the currently active UGM user segments
	 *    reset ugmtab
	 */
	for (start = &ugmtab[1]; start < &ugmtab[NUSEG]; start++) {
		if (start->sr_segp)
			unload(start);
		start->sr_segp = 0;
	}

	/*
	 * 2. Load each segment in the p->p_region list into the MMU
	 *    Remember values in ugmtab.
	 */
	start = &ugmtab[1];
	for (i = 1; i < NUSEG; i++) {
		if (u.u_segl[i].sr_segp) {
			*start = u.u_segl[i];
			switch (i) {
			case SIPDATA:
				if (u.u_segl[SISTACK].sr_base)
					start->sr_size = min(start->sr_size,
						u.u_segl[SISTACK].sr_base-
						   u.u_segl[SISTACK].sr_size);
				break;
			case SISTACK:
				start->sr_base -= start->sr_size;
				break;
			}

			start->sr_segp = 0;
			if (SELF->p_segp[i]) {
				start->sr_segp = SELF->p_segp[i];
				doload(start);
			}
			start++;
		}
	}
}

#if !SUNLOAD
SR *
loaded(pp)
register cseg_t *pp;
{
	register SR	*start;

	for (start = ugmtab; start < ugmtab + NUSEG; start++) {
		if (start->sr_segp && start->sr_segp->s_vmem == pp) {
			return start;
		}
	}
	return 0;
}
#endif

/*XXX*/
MAKESR(r0stk, _r0stk);
extern int tss_sp0;

/*
 * General initialization
 */
void
i8086()
{
	unsigned	csize, isize, ssize, allsize;
	vaddr_t	base;
	unsigned int	calc_mem, boost;

	/*
	 * Allocate a click for ring 0 stack.
	 */
	r0stk.sr_size = NBPC;
	valloc(&r0stk);
	tss_sp0 = r0stk.sr_base + NBPC;

	/*
	 * calc_mem is used for autosizing buffer cache and kalloc pool.
	 * It is total_mem, limited below by 1 meg and above by 12 meg.
	 * The upper limit is a temporary move to allow booting on 16 Meg
	 * systems.
	 *
	 * "boost" is used in autosizing buffer cache and kalloc pool.
	 * It is the number of megabytes of calc_mem above 1 meg, i.e.,
	 * a number between 0 and 11.
	 */
#ifndef FOO
	if (total_mem < ONE_MEG)
		calc_mem = ONE_MEG;
	else if (total_mem > 12 * ONE_MEG)
		calc_mem = 12 * ONE_MEG;
	else
		calc_mem = total_mem;

	boost = (calc_mem - ONE_MEG) / ONE_MEG;
#endif

	/*
	 * If the number of cache buffers was not explicitly set (i.e., !0)
	 * then calculate the number of buffers using the simple heuristic:
	 *     128 minimum + 400 per MB of available RAM (i.e., after 1MB)
	 */
	if (NBUF == 0)
		NBUF = 128 + (400 * boost);

	/*
	 * If the amount of kalloc() space was not explicitly set (i.e., !0)
	 * then calculate using the simple heuristic:
	 *     64k minimum + 32k per MB of available RAM (i.e., after 1MB)
	 */
	if (ALLSIZE == 0)
		ALLSIZE = 65536 + (32768 * boost);

	blockp.sr_size = NBUF*BSIZE;
	valloc(&blockp);

	allocp.sr_size= allsize = NBUF*sizeof(BUF) + ALLSIZE;
#if USE_SLOT
	allocp.sr_size += ssize = NSLOT * (sizeof(int) + slotsz);
#else
	ssize = 0;
#endif
	allocp.sr_size += isize = NINODE* sizeof(INODE);
	allocp.sr_size += csize = NCLIST* sizeof(CLIST);
	valloc(&allocp);
	base = allocp.sr_base;
	allkp = setarena(base, allsize);
	base += allsize;
#if USE_SLOT
	slotp = (int *)base;
	base += ssize;
#endif
	inodep = (INODE*) base;
	base += isize;
	clistp = (paddr_t)base;
}

/*
 * Allocate srp->sr_size bytes of physical memory, and map it into
 * virtual memory space.  At the end, the struct at srp will describe
 * the new segment.
 */
void
valloc(srp)
SR	*srp;
{
	register int npage;

	/*
	 * If we've run out of virtual memory space, panic().
	 *
	 * A more graceful solution is needed, but valloc() does
	 * not provide a return value.
	 */
	if (sysmem.vaddre + srp->sr_size > MAX_VADDR) {
		panic("valloc: out of virtual memory space");
	}

	npage = btoc(srp->sr_size);

	srp->sr_base = sysmem.vaddre;
	srp->sr_segp->s_size = srp->sr_size;
	srp->sr_segp->s_vmem = c_alloc(npage);
	srp->sr_segp->s_flags = SFSYST|SFCORE;
	doload(srp);

	sysmem.vaddre += ctob(npage);
}

/*
 * See if the given process may fit in core.
 */
int
testcore(pp)
register PROC *pp;
{
	return 1;
}

/*
 * Calculate segmentation for a
 * new program. If there is a stack segment
 * present merge it into the data segment and
 * relocate the argument list.
 * Make sure that the changes are reflected in the u.u_segl array
 * which sproto sets up.
 */
int
mproto()
{
	return 1;
}

int
accdata(base, count)
unsigned	base, count;
{
	SR *srp;

	srp = &u.u_segl[SIPDATA];
	return base>=srp->sr_base && base+count <= srp->sr_base+srp->sr_size;
}

int
accstack(base, count)
unsigned	base;
{
	SR *srp;

	srp = &u.u_segl[SISTACK];
	return base>=srp->sr_base-srp->sr_size && base+count<=srp->sr_base;
}

int
acctext(base, count)
unsigned	base;
{
	SR *srp;

	srp = &u.u_segl[SISTEXT];
	return base>=srp->sr_base && base+count <= srp->sr_base+srp->sr_size;
}

void
msigstart(n, f)
{
	register int uesp;

	--n;
	if (SELF->p_dsig & (1<<n))
		SELF->p_hsig |= 1 << n;
	uesp = u.u_regl[UESP] - sizeof(struct _fpstackframe);
	putuwd(uesp + (SS+3) * sizeof(long), 0);
	kucopy(u.u_regl, uesp + 2*sizeof(long), (SS+1) * sizeof(long));
	putuwd(uesp+sizeof(long), n+1);
	putuwd(uesp, u.u_sigreturn);
	u.u_regl[EFL] &= ~MFTTB;
	u.u_regl[EIP] = f;
	u.u_regl[UESP] = uesp;
}

void
msigend(gs, fs, es, ds, edi, esi, ebp, esp, ebx, edx, ecx, eax, trapno, err,
	eip, cs, efl, uesp, ss)
{
	register int signo;
	register PROC *pp = SELF;

	u.u_regl = &gs;
	signo = getuwd(uesp-sizeof(long)) - 1; 
	ukcopy(uesp, u.u_regl, (SS+1) * sizeof(long));
	/*
	 * if the signal has been sigset
	 * simulate a sigrelse(signal)
	 */
	if (pp->p_hsig & 1<<signo) {
		pp->p_hsig &= ~(1 << signo);
		if (nondsig()) {
			T_PIGGY( 0x100, printf("actvsig(mmu)"); );
			actvsig();
		}
	}
}

int	evin, evout;
extern	trapcode;
EVENT	evtab[NEV];

EVENT *
evtrap()
{
	EVENT *evp;

	evp = &evtab[evin++ & (NEV-1)];
	evp->pid = SELF->p_pid;
	kkcopy(u.u_regl, evp->r, sizeof(evp->r));
	evp->err = 0;
	evp->res = 0;
	return evp;
}

evput(n)
{
	int	id;
	register EVENT *evp;
	if (evout < evin-n) 
		evout = evin-n;
	while (evout<evin) {
		evp = &evtab[evout++ & (NEV-1)];
		evprint(evp);
		if (evout==evin) { /* last one */			
			id = evp->r[ERR];
			if (id==14)
				printf("    cr2 = %x, ", regcr2());
			if (0x7B00 & (1<<id)) /* 8, 10, 11-14 */
				printf("code = %x\n", trapcode&0xFFFF);
/*			evdump(NBPS + (evp->r[EIP]&~0xFF));		*/
			evtrace(evp);
		}
	}
}

#define	NWDLINE	8

evdump(where)
unsigned	where;
{
	register int save;
	unsigned	i, j;

	save = setspace(SEG_386_KD);
	for (i = where; i<where+256; i+= NWDLINE*sizeof(long)) {
		printhex(i, 5);
		for (j = 0; j<NWDLINE*sizeof(long); j += sizeof(long)) {
			putchar(' ');
			printhex(getuwd(i + j), 8);
		}
		putchar('\n');
	}
	setspace(save);
}

printhex( v, max)
unsigned long v;
{
	register int i;

	for (i = max-1; i>=0; --i) 
		putchar(digtab[(v >> (i*4)) & 0xF]);
}

evprint(evp)
register EVENT *evp;
{
	int	id;
	register int	i;

	id = evp->r[ERR];
	printf("%d:%x: ", evp->pid, id);
	printf("<isb>p %x %x %x <abcd>x %x %x %x %x ",
		evp->r[EIP], evp->r[UESP], evp->r[EBP],
		evp->r[EAX], evp->r[EBX], evp->r[ECX], evp->r[EDX]);
	if (id==SISYS || id==SIOSYS) {
		printf("%x(", evp->func);
		for (i=1; i<=evp->a[0]; i++)
			printf("%x ", evp->a[i]);
		printf(") = ");
		if (evp->err)
			printf("[%d]", evp->err);
		else
			printf("%x", evp->res);
	}
	putchar('\n');
}

evtrace(evp)
EVENT *evp;
{
	register int	save;

	printf("trace: ");
	save = setspace(SEG_386_KD);

	switch (evp->r[CS]&0xFFFF) {
	case SEG_286_UII+R_USR:
	case SEG_286_UI+R_USR:
		evtrace16(evp->r[EBP], 0, ISP_286);
		break;
	case SEG_386_KI:
	case SEG_386_II:
		evtrace32(evp->r[EBP], &u, (char *)&u + UPASIZE - 1);
		break;
	case SEG_386_UI+R_USR:
		evtrace32(evp->r[EBP], ISP_386 - stob(1), ISP_386);
		break;
	default:
		printf("evp->r[CS] = %x\n", evp->r[CS]);
		panic("evtrace");
	}
	printf("\n");
	setspace(save);
}

#define	MAXCALL	4

typedef struct frame32
{	
	int		prev;
	int		ra;
	int		a[1];
} FRAME32;

evtrace32(fp, fmin, fmax)
FRAME32	*fp, *fmin, *fmax;
{
	register int i, where;

	printf("evtrace32: %x %x %x\n", fp, fmin, fmax);
	for (i = 0; fp>fmin && fp<fmax && i<MAXCALL; i++) {
		where = getuwd(&fp->ra);
		if (getubd(where - 5*sizeof(char)) == 0xe8) /*call*/
			where += getuwd(where-sizeof(long));
		else
			putchar('*');
		printf("%x(", where);
		printf("%x %x)", getuwd(&fp->a[0]), getuwd(&fp->a[1]));
		fp = (FRAME32*) getuwd(&fp->prev);
	}
}

typedef struct frame16
{	
	unsigned short prev;
	unsigned short si, di; 	
	unsigned short ra;
	unsigned short a[1];
} FRAME16;

evtrace16(fp, fmin, fmax)
FRAME16 *fp, *fmin, *fmax;
{
	register int i, where;


	printf("evtrace16: %x %x %x\n", fp, fmin, fmax);
	for (i = 0; fp>fmin && fp<fmax && i<MAXCALL; i++) {
		where = (unsigned short) getuwd(&fp->ra);
		if (getubd(where+NBPS-3*sizeof(char)) == 0xe8) /*call*/
			where = (unsigned short) (where + 
				getuwd(where+NBPS-sizeof(short)));
		else
			putchar('*');
		printf("%x(", where);
		printf("%x %x)",
			(unsigned short) getuwd(&fp->a[0]),
			(unsigned short) getuwd(&fp->a[1]));
		fp = (FRAME16*) (unsigned short) getuwd(&fp->prev);
	}
}

/* Read a 16 byte number from the CMOS.  */
unsigned int
read16_cmos(addr)
unsigned int addr;
{
        unsigned char read_cmos();
	
	return((read_cmos(addr+1)<<8) + read_cmos(addr));
} /* read16_cmos() */

int
c_grow(sp, new_bytes)
SEG *sp;
int new_bytes;
{
	register int	i;
	register cseg_t *pp;
	int		new_clicks, pno, nsize, old_clicks;
	SR		*srp;

	T_PIGGY( 0x8000000, printf("c_grow(sp: %x, new: %x)", sp, new_bytes); );

	new_clicks = btoc(new_bytes);
	old_clicks = btoc(sp->s_size);

	if (new_clicks == old_clicks) {
		goto ok_c_grow;
	}

	if (new_clicks < old_clicks) {
		printf("%s:can't contract segment\n",u.u_comm);
		goto no_c_grow;
	}

	if (new_clicks - old_clicks > allocno()) {
		goto no_c_grow;
	}

	T_PIGGY( 0x8000000, printf("nc: %x, oc: %x,",new_clicks,old_clicks); );

	/*
	 * Allocate a new descriptor vector if necessary.
	 * pp is the element corresponding to the virtual address
	 * "0"(sr_base)
	 */
	pp = sp->s_vmem;
	nsize = areasize(new_clicks);
	if (nsize != areasize(old_clicks)
	  && !(pp = (cseg_t*)arealloc(new_clicks))) {
		T_PIGGY( 0x8000000,
			 printf("Can not allocate new descriptor."); );
		goto no_c_grow;
	}

	T_PIGGY( 0x8000000, printf("new pp: %x", pp); );

#if SUNLOAD
	sunload(sp->s_vmem);
#else
	if (0 != (srp = loaded(sp->s_vmem))) {
		T_PIGGY( 0x8000000, printf("unloading srp: %x, ", srp); );
		unload(srp);
		srp->sr_segp = 0;
	}
#endif

	/*
	 * Allocate new descriptors.
	 */
	T_PIGGY( 0x8000000, printf("new desc: ["); );
	for (i = old_clicks; i < new_clicks; i++) {
		pno = *--sysmem.pfree;
		pp[i] = clickseg(pno) | SEG_RW;
		T_PIGGY( 0x8000000, printf("%x, ", pp[i]); );
	}
	T_PIGGY( 0x8000000, printf("]"); );

	/*
	 * Copy unchanged descriptors and free old vector if necessary.
	 */
	if (pp != sp->s_vmem) {
		T_PIGGY( 0x8000000, printf("old desc: ["); );
		for (i = 0; i < old_clicks; i++) {
			pp[i] = sp->s_vmem[i];
			T_PIGGY( 0x8000000, printf("%x, ", pp[i]); );
		}
		T_PIGGY( 0x8000000, printf("]"); );
		areafree((BLOCKLIST*)sp->s_vmem, old_clicks);
	}

	sp->s_vmem = pp;

	/*
	 * clear the added clicks
	 *
	 * MAPIO macro - convert array of page descriptors, offset
	 *   into system global address.
	 */
	T_PIGGY( 0x8000000, printf("dmaclear(%x, %x, 0)", 
				ctob(new_clicks - old_clicks),
				MAPIO(sp->s_vmem, ctob(old_clicks))
			    );
	); /* T_PIGGY() */

	dmaclear(ctob(new_clicks - old_clicks),
	  MAPIO(sp->s_vmem, ctob(old_clicks)), 0);

ok_c_grow:
	return 0;

no_c_grow:
	return -1;
}

#if SUNLOAD
void
sunload(pp)
register	cseg_t *pp;
{
	register	SR	*start;

	for (start = ugmtab; start < &ugmtab[NUSEG]; start++) {
		if (start->sr_segp && start->sr_segp->s_vmem==pp) {
			unload(start);
			start->sr_segp = 0;
		}
	}
}
#endif
