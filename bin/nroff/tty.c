/*
 * Nroff/Troff.
 * TTY driver.
 */
#include <stdio.h>
#include <ctype.h>
#include "roff.h"
#include "code.h"
#include "char.h"
#include "env.h"
#include "font.h"
#include "codebug.h"

/*
 * Device parameters.
 */
int	ntroff	=	NROFF;		/* Programme is NROFF type */
long	sinmul	=	120;		/* Multiplier for inch */
long	sindiv	=	1;		/* Divisor for inch */
long	semmul	=	3;		/* Multiplier for em space */
long	semdiv	=	5;		/* Divisor for em space */
long	senmul	=	3;		/* Multiplier for en space */
long	sendiv	=	5;		/* Divisor for en space */
long	snrmul	=	0;		/* Narrow space (mul) */
long	snrdiv	=	1;		/* Narrow space (div) */
long	shrmul	=	12;		/* Horizontal resolution (mul) */
long	shrdiv	=	1;		/* Horizontal resolution (div) */
long	svrmul	=	20;		/* Vertical resolution (mul) */
long	svrdiv	=	1;		/* Vertical resolution (div) */

/*
 * For mapping user fonts to real fonts.
 */
FTB fontab[] ={
	{ 'R',  '\0', TRMED },
	{ 'I',  '\0', TRITL },
	{ 'B',  '\0', TRBLD },
	{ '\0' }
};

/* Some names for the fonts */
char *fnames[] = {
	"Medium",
	"Italic",
	"Bold",
	"Nothing"
};

/*
 * Table to convert from the internal character set to ASCII.
 */
char intasc[] ={
	  0,  '0',  '1',  '2',  '3',  '4',  '5',  '6',  '7',  '8',
	'9',  'A',  'B',  'C',  'D',  'E',  'F',  'G',  'H',  'I',
	'J',  'K',  'L',  'M',  'N',  'O',  'P',  'Q',  'R',  'S',
	'T',  'U',  'V',  'W',  'X',  'Y',  'Z',  'a',  'b',  'c',
	'd',  'e',  'f',  'g',  'h',  'i',  'j',  'k',  'l',  'm',
	'n',  'o',  'p',  'q',  'r',  's',  't',  'u',  'v',  'w',
	'x',  'y',  'z',  '!',  '"',  '#',  '$',  '%',  '&',  '(',
	')',  '*',  '+',  ',',  '-',  '.',  '/',  ':',  ';',  '<',
	'=',  '>',  '?',  '@',  '[', '\\',  ']',  '^',  '_',  '{',
	'|',  '}',  '~',  '`', '\'', '\'',  '`',  '^',  '-'
};

/*
 * Width table.
 */
unsigned char widtab[] ={
	  0,   12,   12,   12,   12,   12,   12,   12,   12,   12,
	 12,   12,   12,   12,   12,   12,   12,   12,   12,   12,
	 12,   12,   12,   12,   12,   12,   12,   12,   12,   12,
	 12,   12,   12,   12,   12,   12,   12,   12,   12,   12,
	 12,   12,   12,   12,   12,   12,   12,   12,   12,   12,
	 12,   12,   12,   12,   12,   12,   12,   12,   12,   12,
	 12,   12,   12,   12,   12,   12,   12,   12,   12,   12,
	 12,   12,   12,   12,   12,   12,   12,   12,   12,   12,
	 12,   12,   12,   12,   12,   12,   12,   12,   12,   12,
	 12,   12,   12,   12,   12,   12,   12,   12,   12
};

/*
 * Set up the non constant parameters that are dependent on a
 * particular device.  Namely pointsize and font.
 */
devparm()
{
	devfont(TRMED);
}

/*
 * Given a font, consisting of the font number, set the new font
 * to the one given.
 */
devfont(font)
{
	swdmul	= 1;		/* Multiplier for width tables */
	swddiv	= 20;		/* Divisor for width tables */
	fonwidt = widtab;
	newfont = font;
}

/*
 * Change the pointsize to the one specified.
 */
devpsze(n)
{
	psz = newpsz = unit(SMINCH, 6*SDINCH);
}

/*
 * Change the vertical spacing.
 */
devvlsp(n)
{
	vls = unit(SMINCH, 6*SDINCH);
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
	static int	hpos=0, hres=0, vres=0, font=0;
	register CODE	*cp;
	register int	n;
#if	(DDEBUG & DBGFUNC)
	printd(DBGFUNC, "flushl: hpos=%d, hres=%d, vres=%d, font=%d\n",
		hpos, hres, vres, font);
#endif
	for (cp=buffer; cp<bufend; cp++) {
#if	(DDEBUG & DBGCODE)
		codebug(cp->c_code, cp->c_iarg, cp->c_csp);
#endif
		switch (cp->c_code) {
		case DNULL:
		case DHYPH:
			continue;
		case DHMOV:
		case DPADC:
			hres += cp->c_iarg;
			if ((hpos+=cp->c_iarg)<0) {
				hres -= hpos;
				hpos = 0;
			}
			continue;
		case DVMOV:
			vres += cp->c_iarg;
			continue;
		case DFPOS:
			continue;
		case DFONT:
			font = cp->c_iarg;
			continue;
		case DPSZE:
			continue;
		case DTRAN:			/* trans char (dag)	*/
			putchar(cp->c_iarg);
			continue;
		case DTRAB:			/* trans line (dag)	*/
			{
				char *tp;
				tp = cp->c_bufp;
				while (*tp)
					putchar( *tp++ );
				free(cp->c_bufp);
				continue;
			}
		case DSPAR:
			hpos = hres = 0;
			vres += cp->c_iarg;
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
			if (cp->c_code==DHYPC)
				n = CMINUS;
			else
				n = cp->c_code;
			if (n<=0 || n>=sizeof intasc)
				panic("Bad directive %d", n);
			n = intasc[n];
			if ((font != TRMED)
			&&  (isascii(n))
			&&  (isupper(n) || islower(n) || isdigit(n)))
				switch (font) {
				case TRBLD:
					printf("%c\b", n);
					break;
				case TRITL:
					printf("_\b");
					break;
				default:
					panic("Bad font %d", font);
				}
			putchar(n);
			if (enbldn) {		/* Universal bolding.	*/
				int q;
				for (q=0; q<enbldn; q++) {
					printf("\b");
					putchar(n);
				}
			}
			hres += cp->c_move-12;
			hpos += cp->c_move;
		}
	}
}

/*
 *	Reset device before leaving. Unecessary in nroff.
 *
 */

void
resetdev()
{
	return;
}

/*
 * A bunch of dummy routines for TROFF compatibility
 */
assign_font()
{
	printe(".rf not in NROFF");
}

/*
 * Return false.
 */
int is_varspace()
{
	return (0==1);
}

int font_number()
{
	return 0;
}

font_display()
{
	register FTB *fntb = fontab;
	register char a, b;

	fprintf(stderr, "Fonts available in NROFF\n");
	while ((a=fntb->f_name[0]) != 0) {
		if ((b = fntb->f_name[1]) == 0)
			b = ' ';
		fprintf(stderr, " %c%c %s\n", a, b, fnames[fntb->f_font]);
		fntb++;
	}
}
