/*
 * output.c
 * troff
 * Back end to output device-dependent bits for troff.
 * Writes either PCL escape sequences for the Hewlett-Packard LaserJet
 * or (if pflag) PostScript.
 */

#include <ctype.h>
#include "roff.h"

extern	char	*fontname();
extern	char	*ndiv10();

/* HP LaserJet PCL. */
/* Printer initialization string:
 *	reset printer
 *	clear margins
 *	0 top margin
 *	move to horizontal 0
 *	move to vertical 0
 * Font selection happens later.
 */
#define	HPLJINIT	\
"\033E"\
"\0339"\
"\033&l0E"\
"\033&a0H"\
"\033&a0V"
#define	HPLJLINIT	"\033&l1O"	/* landscape orientation	*/
#define	HPLJEJECT	"\033&l0H"	/* page eject			*/
#define HPLJRESET	"\033E"		/* reset	 		*/

/* PostScript. */
#define	PSINIT	\
"/state save def\n"\
"/S { show } bind def\n"\
"/M { moveto } bind def\n"
#define	PSLINIT	"90 rotate\n0 -612 translate\n"
#define	PSEJECT	"\nshowpage\n"
#define	PSRESET	"\nstate restore\n"

/*
 * Device parameters.
 */
int	ntroff	=	TROFF;		/* Programme is TROFF type	*/

long	semmul	=	1;		/* Multiplier for em space	*/
long	semdiv	=	1;		/* Divisor for em space		*/

long	senmul	=	1;		/* Multiplier for en space	*/
long	sendiv	=	2;		/* Divisor for en space		*/

long	shrmul	=	12;		/* Horizontal resolution (mul)	*/
long	shrdiv	=	5;		/* Horizontal resolution (div)	*/

long	sinmul	=	720;		/* Multiplier for inch		*/
long	sindiv	=	1;		/* Divisor for inch		*/

long	snrmul	=	1;		/* Narrow space (mul)		*/
long	snrdiv	=	6;		/* Narrow space (div)		*/

long	svrmul	=	12;		/* Vertical resolution (mul)	*/
long	svrdiv	=	5;		/* Vertical resolution (div)	*/

/*
 * Local variables.
 */
static	int	hposd;			/* device horizontal postition	*/
static	int	vposd;			/* device vertical position	*/
static	int	inword;			/* in word flag for PostScript	*/

/*
 * Initialize the device.
 * Select the default font.
 */
devparm()
{
	register int i;

	for (i = 0; i < nfonts; i++) {
		fpsz[i] = fwptab[i]->f_psz;	/* init. env pointsize */
		fcsz[i] = 0;			/* init. env const. char size */
	}
	printf((pflag) ? PSINIT : HPLJINIT);
	if (lflag && !pflag)
		printf(HPLJLINIT);
	devfont(TRMED);
}

/*
 * Given a font number, change to the given font.
 */
devfont(n) register int n;
{
	register FWTAB	*fp;

	if (n >= nfonts)
		panic("bad font %d at devfont, nfonts=%d", n, nfonts);
	fontype = n;
	addidir(DFONT, n);
	fp = fwptab[n];
	csz = fcsz[n];
	swdmul = (long)fp->f_num;
	swddiv = (long)fp->f_den;
	fonwidt = fp->f_width;
	devpsze(fpsz[n]);
}

/*
 * Change to the given pointsize.
 * There are several different notions of pointsize:
 *	global psz (and oldpsz) in the environment,
 *	fpsz for each font in the environment,
 *	fp->f_psz for each font, and
 *	wpsz in flushl() (see comment below).
 * The font fp->f_psz is used to initialize fpsz in the environment
 * but is subsequently unused.
 * The environment fpsz[n] gives the current pointsize for each font.
 * The environment psz is the pointsize of the current input font,
 * wpsz is the pointsize of the current output font.
 * Changing pointsize with a .ps directive or \s escape
 * changes fpsz[] for each font.
 */
devpsze(n) register int n;
{
	oldpsz = psz;
	psz = n;
	addidir(DPSZE, n);
}

/*
 * Set a fixed pointsize for a font.
 */
dev_fz(n, s) register int n; char *s;
{
	fwptab[n]->f_flags |= F_FIXED;
	fpsz[n] = number(s, SMPOIN, SDPOIN, fpsz[n], 0, fpsz[n]);
	if (n == fontype)
		devpsze(fpsz[n]);
}

/*
 * Specify a constant size font.
 */
dev_cs(n, size) register int n, size;
{
	fcsz[n] = size;
	if (n == fontype)
		csz = size;
}

/*
 * Change all non-fixed font pointsizes to the given pointsize,
 * then change the current pointsize.
 */
newpsze(n) register int n;
{
	register int i;

	for (i = 0; i < nfonts; i++)
		if (((fwptab[i]->f_flags) & F_FIXED) == 0)
			fpsz[i] = n;
	devpsze(n);
}

/*
 * Change the vertical spacing.
 */
devvlsp(psize) int psize;
{
	vls = psize;
}

/*
 * Given a pointer to a buffer containing stream directives
 * and a pointer to the end of the buffer,
 * print the buffer.
 * The output writer maintains its own notion of current font and pointsize
 * because [nt]roff buffers output and then flushes at the end of a line;
 * the environment "fontype" is the current font for input stream processing,
 * the output writer "font" is the current output stream font.
 * The font change is implicit until a character is written in the new font.
 */
flushl(buffer, bufend) CODE *buffer, *bufend;
{
	register CODE	*cp;
	register int	next;
	int		i;
	char		*tp;
	static	FWTAB	*fp;		/* current font table entry	*/
	static	int	newpage = 1;	/* new page flag		*/
	static	int	hpost;		/* troff horizontal pos (u's)	*/
	static	int	vpost;		/* troff vertical pos (u's)	*/
	static	int	lastfont = -1;	/* current output font		*/
	static	int	font = -1;	/* current font			*/
	static	unsigned char *wtab;	/* current font width table	*/
	static	long	wnum;		/* current width numerator	*/
	static	long	wden;		/* current width denominator	*/
	static	int	wpsz;		/* current pointsize		*/

	for (cp = buffer; cp < bufend; cp++) {
		next = cp->l_arg.c_code;
		i = cp->l_arg.c_iarg;
		if (pflag && ifcdirc(next))
			endword();
#if	0
		fprintf(stderr, "output: %d arg=%d\n", next, i);
#endif
		switch (next) {
		case DNULL:			/* null code */
		case DHYPH:			/* place to hyphenate */
			continue;
		case DHMOV:			/* move horizontally */
		case DPADC:			/* paddable character */
			hpost += i;
			if (hpost < 0)
				hpost = 0;
			continue;
		case DVMOV:			/* move vertically */
			vpost += i;
			if (vpost < 0)
				vpost = 0;
			continue;
		case DFPOS:			/* fix position */
			move();
			continue;
		case DFONT:			/* change font */
			if (i == font)
				continue;	/* unchanged */
			if ((unsigned)i >= nfonts)
				panic("bad font %d, nfonts=%d", i, nfonts);
			font = i;
			fp = fwptab[font];
			wtab = fp->f_width;
			wpsz = fpsz[font];
			wnum = (long)fp->f_num * wpsz;
			wden = (long)fp->f_den;
			continue;
#if	0
		case DTRAN:			/* transparent character */
			putchar(i);
			continue;
#endif
		case DTRAB:			/* trans line (dag)	*/
			tp = cp->b_arg.c_bufp;
			while (*tp)
				outchar( *tp++ );
			free(cp->b_arg.c_bufp);
			continue;
		case DPSZE:			/* change  pointsize */
			if (wpsz != i) {
				wpsz = i;
				wnum = (long)fp->f_num * wpsz;
				if (pflag) {
					/* Mask off used bit to force rescaling */
					lastfont = -1;
					fp->f_flags &= ~F_USED;
				} else {
					/*
					 * PCL font scaling is untested!
					 * This is a guess...
					 */
					printf("\033(s%dV", wpsz);
				}
			}
			continue;
		case DSPAR:			/* space down and return */
			hposd = hpost = 0;
			vpost += i;
			if (vpost < 0)
				vpost = 0;
			else if (vpost >= pgl) {
				/* New page. */
				endpage();
				vpost %= pgl;
				vposd = vpost;
				newpage = 1;
			}
			continue;
		default:			/* print something */
			/* Start a new page. */
			if (newpage) {
				if (lflag && pflag)
					printf(PSLINIT);
				hposd = hpost;
				vposd = vpost;
				move();
				newpage = 0;
			}
			/*
			 * Check whether we are on a new page.
			 * Note, the laser goes to funny places when it
			 * crosses a page boundary.
			 */
			if (vpost >= pgl) {
				/* New page. */
				endpage();
				vpost %= pgl;
				vposd = vpost;
				hposd = hpost;
				newpage = 1;
			}
			/* Output move to new position if appropriate. */
			if (hpost != hposd || vpost != vposd) {
				hposd = hpost;
				vposd = vpost;
				move();
			}
			/* Change to different font. */
			if (lastfont != font)
				selectfont(lastfont=font, wpsz);
			if (next == DHYPC)
				next = '-';
			if (next < 0 || next >= NWIDTH)
				panic("bad directive %d", next);
			hpost += cp->c_arg.c_move;
			hposd += unit(wnum*wtab[next], wden);
			i = next;
			if (pflag && !inword)
				startword();
			outchar(next);
			if (enbldn != 0) {	/* dag's enbolden...	*/
				hposd -= unit(wnum*wtab[i], wden);
				vposd -= enbldn;
				move();
				putchar(next);
				hposd -= enbldn;
				move();
				putchar(next);
				vposd += enbldn;
				putchar(next);
				hposd += (unit(wnum*wtab[i], wden) + enbldn);
				move();
			}
		}
	}
}

/*
 * Print proper escape sequence to reset the laser printer so that the
 * last page is ejected.
 *
 */
void
resetdev()
{
	printf((pflag) ? PSRESET : HPLJRESET);
}

/*
 * List all the font names and descriptions in this version.
 */
void
font_display()
{
	register FTB *p;
	register int a, b;

	fprintf(stderr, "Fonts available in this version:\n");
	for (p = fontab; p < &fontab[NFNAMES]; p++) {
		if ((a = p->f_name[0]) == 0)
			break;
		if ((b = p->f_name[1]) == 0)
			b = ' ';
		fprintf(stderr," %c%c %s\n", a, b, fwptab[p->f_font]->f_descr);
	}
}

/*
 * Return true if font uses variable spacing.
 */
int
is_varspace(t) int t;
{
	return ((fwptab[t]->f_spacing) == 1);
}

/*
 * Move horizontally and/or vertically.
 * This used to shift PCL output right to avoid printing at left border;
 * that responsibility is now left to the user.
 */
move()
{
	static int vold = -1;

	if (pflag) {
		/* PostScript. */
		if (inword)
			endword();
		printf("\n%s", ndiv10(hposd));
		printf(" %s M", ndiv10(pgl - vposd));
	} else {
		/* PCL. */
		if (vposd == vold)
			printf("\033&a%dH", hposd);
		else
			printf("\033&a%dh%dV", hposd, vposd);
	}
	vold = vposd;
}

/*
 * Select a font.
 */
selectfont(font, ptsize) int font, ptsize;
{
	register FWTAB *fp;
	register char *s;

	fp = fwptab[font];
	if (!pflag) {
		/* Select font via PCL. */
		printf("\033(%d%c", fp->f_symset/32, fp->f_symset%32+64); /* symbol set */
		printf("\033(s%dp", fp->f_spacing);		/* spacing */
		if (fp->f_pitch != 0)
			printf("%sh", ndiv10(fp->f_pitch));	/* pitch */
		printf("%sv", ndiv10(fpsz[font]));		/* point size */
		printf("%ds", fp->f_style);			/* style */
		printf("%db", fp->f_weight);			/* stroke weight */
		printf("%dT", fp->f_face);			/* typeface */
		return;
	}
	/* Select font via PostScript. */
	s = fontname(font);
	endword();
	if ((fp->f_flags & F_USED) == 0) {
		fp->f_flags |= F_USED;
		printf("\n"
			"/font%s /%s findfont %d.%d scalefont def\n"
			"/f%s { font%s setfont } bind def\n",
			s, fp->f_PSname,
			ptsize / 10, ptsize % 10,
			s, s);
	}
	printf(" f%s", s);
}

/*
 * Return fontname associated with font number n.
 * Because the mapping is many->one, the user might have
 * specified the font with a different name.
 * The returned value points to a statically allocated buffer.
 */
char *
fontname(n) register int n;
{
	static char buf[3];
	register FTB *p;

	for (p = fontab; p < &fontab[NFNAMES]; p++) {
		if (p->f_font == n) {
			buf[0] = p->f_name[0];
			buf[1] = p->f_name[1];
			buf[2] = '\0';
			return buf;
		}
	}
	return NULL;
}

/*
 * Convert e.g. 123 to "12.3" without using floating point output.
 * Return a pointer to statically allocated buffer.
 */
char *
ndiv10(n) register int n;
{
	static char buf[10];

	if ((n % 10) == 0)
		sprintf(buf, "%d", n/10);
	else
		sprintf(buf, "%d.%d", n/10, n%10);
	return buf;
}

/*
 * Output a character.
 * If writing PostScript output, watch out for "()\\".
 */
outchar(n) register int n;
{
	if (pflag && (n == '(' || n == ')' || n == '\\'))
		putchar('\\');
	putchar(n);
}

startword()
{
	putchar(' ');
	putchar('(');
	inword = 1;
}

endword()
{
	if (inword) {
		printf(") S");
		inword = 0;
	}
}

endpage()
{
	if (pflag)
		endword();
	printf((pflag) ? PSEJECT : HPLJEJECT);
}

/* end of output.c */
