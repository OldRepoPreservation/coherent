#ifndef	D_D_DEBUG_TROFF
/*
 * Debugging header for TROFF/NROFF
 */
#define	D_D_DEBUG_TROFF	1	/* Process this only once...	*/
#ifndef	DDEBUG
#define	DDEBUG	0		/* Default to a level of 0	*/
#endif

#define	DBGCHEK	1		/* Various check points	*/
#define	DBGREGS	2		/* Register creation	*/
#define	DBGREGX	4		/* Register access	*/
#define	DBGCODE	8		/* Output codes		*/
#define	DBGDIVR	16		/* Diversions and traps	*/
#define	DBGFILE	32		/* File access		*/
#define	DBGFUNC	64		/* Various functions	*/
#define	DBGCHAR 128		/* Characters		*/
#define	DBGPROC	256		/* "process" debugging	*/
#define	DBGMACX	512		/* Macro execution	*/
#define	DBGREQX 1024		/* request processing	*/
#define	DBGMISC	2048		/* Misc. things		*/
#define	DBGMOVE	4096		/* "pel" movement	*/
#define	DBGENVR	8192		/* Environment		*/
#define	DBGCALL	16384		/* Special call stuff	*/

#if	DDEBUG
#define	dprintd(a,b)	printd(a, b)		/* debug information	*/
#define	dprint2(a,b,c)	printd(a, b, c)		/* debug info with arg.	*/
#define	dprint3(a,b,c,d) printd(a, b, c, d)	/* debug info with args	*/
#else
#define	dprintd(a, b)				/* Not without debug!	*/
#define	dprint2(a, b, c)			/*			*/
#define	dprint3(a, b, c, d)			/*			*/
#endif

extern int dbglvl;				/* Debugging level.	*/
#endif
