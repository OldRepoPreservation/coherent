/*
 * tty.c
 * Nroff/Troff.
 * TTY driver.
 */

#include <ctype.h>
#include "roff.h"

/*
 * Device parameters.
 */
int	ntroff	=	NROFF;		/* Programme is NROFF type */
long	semmul	=	3;		/* Multiplier for em space */
long	semdiv	=	5;		/* Divisor for em space */
long	senmul	=	3;		/* Multiplier for en space */
long	sendiv	=	5;		/* Divisor for en space */
long	shrmul	=	12;		/* Horizontal resolution (mul) */
long	shrdiv	=	1;		/* Horizontal resolution (div) */
long	sinmul	=	120;		/* Multiplier for inch */
long	sindiv	=	1;		/* Divisor for inch */
long	snrmul	=	0;		/* Narrow space (mul) */
long	snrdiv	=	1;		/* Narrow space (div) */
long	svrmul	=	20;		/* Vertical resolution (mul) */
long	svrdiv	=	1;		/* Vertical resolution (div) */

/*
 * Map user fontnames to font numbers.
 */
FTB fontab[NFNAMES] ={
	{ 'R',  '\0', TRMED },
	{ 'I',  '\0', TRITL },
	{ 'B',  '\0', TRBLD }
};

/*
 * Width table.
 * Initialized in devparm().
 */
unsigned char widtab[NWIDTH];

/*
 * Set up the non-constant parameters that depend on a particular device.
 */
devparm()
{
	register int i;

	for (i = 0; i < NWIDTH; i++)
		widtab[i] = 12;		/* initialize font width table */
	fonwidt = widtab;		/* width table for all fonts */
	swdmul	= 1;			/* multiplier for width table */
	swddiv	= 20;			/* divisor for width table */
	devfont(TRMED);
	oldpsz = psz = unit(SMINCH, 6*SDINCH);
	vls = unit(SMINCH, 6*SDINCH);
}

/*
 * Given a font number, change to the given font.
 */
devfont(n) register int n;
{
	addidir(DFONT, fontype = n);
}

/*
 * Change the pointsize to the one specified.
 * A nop for nroff.
 */
devpsze(n) int n;
{
	/* psz initialized in devparm() */
}

/*
 * Change the vertical spacing.
 * A nop for nroff.
 */
devvlsp(n)
{
	/* vls initialized in devparm() */
}

/*
 * Given a pointer to a buffer containing stream directives
 * and a pointer to the end of the buffer, print the buffer
 * out.
 */
flushl(buffer, bufend)
CODE *buffer;
CODE *bufend;
{
	static int hpos;
	static int hres;
	static int vres;
	static int font;
	register CODE	*cp;
	register int	n;

#if	(DDEBUG & DBGFUNC)
	printd(DBGFUNC, "flushl: hpos=%d, hres=%d, vres=%d, font=%d\n",
		hpos, hres, vres, font);
#endif
	for (cp=buffer; cp<bufend; cp++) {
#if	(DDEBUG & DBGCODE)
		codebug(cp->l_arg.c_code, cp->l_arg.c_iarg, cp->c_csp);
#endif
#if	0
		fprintf(stderr, "output: %d arg=%d\n", cp->l_arg.c_code, cp->l_arg.c_iarg);
#endif
		switch (cp->l_arg.c_code) {
		case DNULL:
		case DHYPH:
			continue;
		case DHMOV:
		case DPADC:
			hres += cp->l_arg.c_iarg;
			if ((hpos+=cp->l_arg.c_iarg)<0) {
				hres -= hpos;
				hpos = 0;
			}
			continue;
		case DVMOV:
			vres += cp->l_arg.c_iarg;
			continue;
		case DFONT:
			font = cp->l_arg.c_iarg;
			continue;
		case DPSZE:
			continue;
		case DSPAR:
			hpos = hres = 0;
			vres += cp->l_arg.c_iarg;
			if (vres >= 0) {
				n = (vres+10) / 20;
				vres -= n*20;
				while (n--)
					putchar('\n');
			} else {
				putchar('\r');
				n = (-vres+9)/20;
				vres += n*20;
				while (n--)
					printf("\0337");
			}
			continue;
		default:
			if (vres >= 0) {
				vres += 5;
				n = (vres) / 20;
				while (n--)
					printf("\033B");
				if (vres%20/10)
					printf("\0339");
			} else {
				vres -= 5;
				n = (-vres)/20;
				while (n--)
					printf("\0337");
				if (-vres%20/10)
					printf("\0338");
			}
			vres = 0;
			if (hres >= 0) {
				n = (hres+6) / 12;
				hres -= n*12;
				while (n--)
					putchar(' ');
			} else {
				n = (-hres+5)/12;
				hres += n*12;
				while (n--)
					putchar('\b');
			}
			if (cp->l_arg.c_code==DHYPC)
				n = '-';
			else
				n = cp->l_arg.c_code;
			if (n < 0 || n >= NWIDTH)
				panic("bad directive %d", n);
			if ((font != TRMED)
			&&  (isascii(n))
			&&  (isupper(n) || islower(n) || isdigit(n)))
				switch (font) {
				case TRBLD:
					printf("%c\b", n);
					break;
				case TRITL:
#if	1
					printf("_\b");
#else
					n |= 0x80;
#endif
					break;
#if	0
				case HELV:
					printf("_\b");
					break;
#endif
				default:
					panic("bad font %d", font);
				}
			putchar(n);
			hres += cp->c_arg.c_move-12;
			hpos += cp->c_arg.c_move;
		}
	}
}

/*
 * Display available fonts.
 */
void
font_display()
{
	fprintf(stderr,
		"Fonts available in this version:\n R  Roman\n I  Italic\n B  Bold\n"
		);
}

/*
 * The following troff functions are nops for nroff.
 */
dev_cs(){}
dev_fz(){}
newpsze(){}
load_font(){}
void resetdev(){}

int
is_varspace(t) int t;
{
	return 0;
}

/* end of tty.c */
