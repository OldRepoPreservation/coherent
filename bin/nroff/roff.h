/*
 * roff.h
 * Nroff/Troff.
 * Header file.
 */

#include <stdio.h>
#include "code.h"
#include "codebug.h"
#include "div.h"
#include "env.h"
#include "escapes.h"
#include "fonts.h"
#include "hyphen.h"
#include "reg.h"
#include "request.h"
#include "str.h"

#define	VERSION	"3.0"

/* Buffer sizes. */
#define	ABFSIZE	128			/* Size of argument buffer	*/
#define	ASCSIZE	128			/* Size of ASCII character set	*/
#define	CBFSIZE	128			/* Size of char buffer		*/
#ifdef	GEMDOS
#define	DBFSIZE	128			/* Size of disk buffer		*/
#else
#define	DBFSIZE	128			/* Size of disk buffer		*/
#endif
#define	MSCSIZE	256			/* Size of miscellaneous buffer	*/

/* Types of justification. */
#define	NJUS	0			/* None (marks end of table)	*/
#define	LJUS	1			/* Left justification		*/
#define	CJUS	2			/* Centre justification		*/
#define	RJUS	3			/* Right justification		*/
#define	FJUS	4			/* Filled justification		*/

/*
 * Unit measures.
 * These use constants defined as globals in tty.c (nroff) or output.c (troff).
 * The values are different for nroff and troff,
 * but the same unlinked object works for both.
 * Changing these to different manifest constants with a conditional
 * compilation switch would require different unlinked objects.
 */
#define	SMCENT	((long) 50*sinmul)	/* Centimetre (mul)		*/
#define	SDCENT	((long) 127*sindiv)	/* Centimetre (div)		*/
#define	SMEMSP	((long) psz*semmul)	/* Em space (mul)		*/
#define	SDEMSP	((long) semdiv)		/* Em space (div)		*/
#define	SMENSP	((long) psz*senmul)	/* En space (mul)		*/
#define	SDENSP	((long) sendiv)		/* En space (div)		*/
#define	SMHRES	((long) shrmul)		/* Horizontal resolution (mul)	*/
#define	SDHRES	((long) shrdiv)		/* Horizontal resolution (div)	*/
#define	SMINCH	((long) sinmul)		/* Inch (mul)			*/
#define	SDINCH	((long) sindiv)		/* Inch (div)			*/
#define	SMNARS	((long) psz*snrmul)	/* Narrow space (mul)		*/
#define	SDNARS	((long) snrdiv)		/* Narrow space (div)		*/
#define	SMPICA	((long) sinmul)		/* Pica (mul)			*/
#define	SDPICA	((long) 6*sindiv)	/* Pica (div)			*/
#define	SMPOIN	((long) sinmul)		/* Point (mul)			*/
#define	SDPOIN	((long) 72*sindiv)	/* Point (div)			*/
#define	SMUNIT	((long) 1)		/* Unit (mul)			*/
#define	SDUNIT	((long) 1)		/* Unit (div)			*/
#define	SMVEMS	((long) psz)		/* Vertical em space (mul)	*/
#define	SDVEMS	((long) 1)		/* Vertical em space  (div)	*/
#define	SMVLSP	((long) vls)		/* Line space (mul)		*/
#define	SDVLSP	((long) 1)		/* Line space (div)		*/
#define	SMVRES	((long) svrmul)		/* Vertical resolution (mul)	*/
#define	SDVRES	((long) svrdiv)		/* Vertical resolution (div)	*/

/* Miscellaneous. */
#define	NROFF		1		/* Program is nroff		*/
#define	TROFF		2		/* Program is troff		*/
#define	INFINITY	32767
#ifdef	MSDOS
#define	TMACDIR "\\bin\\"
#else
#ifdef	GEMDOS
#define	TMACDIR "\\bin\\"
#else
#define	TMACDIR	"/usr/lib/"		/* Library directory		*/
#endif
#endif

/* Roman numeral table. */
typedef struct {
	char	o_digit;		/* Offset for digit		*/
	char	o_state;		/* Next state			*/
} ROM;

/*
 * Device dependent variables,
 * defined in tty.c (nroff) or output.c (troff).
 */
extern	int	ntroff;			/* Program type			*/
extern	long	semmul;			/* Multiplier for em space	*/
extern	long	semdiv;			/* Divisor for em space		*/
extern	long	senmul;			/* Multiplier for en space	*/
extern	long	sendiv;			/* Divisor for en space		*/
extern	long	shrmul;			/* Horizontal resolution (mul)	*/
extern	long	shrdiv;			/* Horizontal resolution (div)	*/
extern	long	sinmul;			/* Multiplier for inch		*/
extern	long	sindiv;			/* Divisor for inch		*/
extern	long	snrmul;			/* Narrow space (mul)		*/
extern	long	snrdiv;			/* Narrow space (div)		*/
extern	long	svrmul;			/* Vertical resolution (mul)	*/
extern	long	svrdiv;			/* Vertical resolution (div)	*/

/* Global variables, defined in extern.c. */
extern	int	A_reg;			/* .A register			*/
extern	int	a_reg;			/* .a register			*/
extern	char	*argv0;			/* "nroff" or "troff"		*/
extern	int	byeflag;		/* True when exiting.		*/
extern	int	dbglvl;			/* Debug level.			*/
extern	int	dflag;			/* Debug flag			*/
extern	char	diskbuf[DBFSIZE];	/* Disk buffer for temp file	*/
extern	char	endtrap[2];		/* End macro name		*/
extern	char	esc;			/* Escape character		*/
extern	int	escflag;		/* Last character was escaped	*/
extern	int	ifeflag;		/* True in false conditional	*/
extern	int	lastcon;		/* Last condition of if/else	*/
extern	int	lflag;			/* Landscape mode		*/
extern	char	miscbuf[MSCSIZE];	/* Miscellaneous buffer		*/
extern	int	nbrflag;		/* Don't allow command to break	*/
extern unsigned	npn;			/* Next page number		*/
extern	int	n_reg;			/* .n register			*/
extern	int	oldpof;			/* Old page offset		*/
extern unsigned	pct;			/* Page counter			*/
extern	int	pflag;			/* Generate PostScript output	*/
extern unsigned	pgl;			/* Page length			*/
extern	int	pof;			/* Page offset			*/
extern	int	svs;			/* Saved space			*/
extern	FILE	*tmp;			/* Temp file pointer		*/
extern	unsigned long tmpseek;		/* Pointer into temp file	*/
extern	int	T_reg;			/* .T register			*/
extern	int	xflag;			/* Suppress page eject on exit	*/

/* Global tables, defined in tables.c. */
extern	char	esctab[];
extern	char	*mapfont[];
extern	REQ	reqtab[];
extern	ROM	romtab[];
extern	char	trantab[];

/* Functions. */
extern	STR	*allstr();
extern	char	*duplstr();
extern	REG	*findreg();
extern	void	font_display();
extern	REG	*getnreg();
extern	REG	*makereg();
extern	char	*nalloc();
extern	char	*nextarg();
extern	void	resetdev();

/* Library functions. */
extern	char	*index();
extern	void	free();
extern	long	lseek();
extern	char	*malloc();
extern	char	*realloc();
extern	char	*strcpy();

/* end of roff.h */
