/*
 * libc/stdio/printf.c
 * C standard i/o library.
 * printf(), fprint(), sprintf().
 * Not ANSI compatible!
 * Non-portable things:
 * 1) alignment of arguments is assumed to be completely contiguous.
 * 2) the smallest number is assumed to negate to itself.
 * 3) All of long, int, char*, and double are assumed to
 *    be held in an exact number of ints.
 */

#include <stdio.h>
#include <sys/mdata.h>
#include <ctype.h>
#include <string.h>

/* Avoid calling the new style toupper() function on MSDOS and GEMDOS */
#ifdef _toupper
#define toupper(c) _toupper(c)
#endif

/* External. */
extern	char	*_dtefg();

/*
 * NXBUF is the size of the buffer for a single conversion item.
 * ANSI requires at least 509 characters for a single conversion;
 * note that e.g. "%f" of 1E300 requires a 300+ character buffer.
 * NIBUF and NLBUF must be long enough to hold a converted int and long;
 * a 32-bit value converts to maximum of 11 octal digits + NUL.
 */
#define	NXBUF	512		/* xprintf() buffer size */
#define	NIBUF	12		/* printi() buffer size */
#define	NLBUF	12		/* printl() buffer size */

#define	NULLFMT	"{NULL}"

union	alltypes {
	char	c;
	int	i;
	unsigned u;
	long	l;
	double	d;
	char	*s;
};

#define	bump(p,s)	(p += sizeof(s)/sizeof(int))

/* Forward. */
char	*printi();
char	*printl();

/*
 * Formatted print to standard output.
 */
printf(args) union alltypes args;
{
	xprintf(stdout, &args);
}

/*
 * Formatted print to a specific file.
 */
fprintf(fp, args) FILE *fp; union alltypes args;
{
	xprintf(fp, &args);
}

/*
 * Formatted print into given string.
 * Handcrafted file structure created for putc.
 */
sprintf(sp, args) char *sp; union alltypes args;
{
	FILE	file;

	_stropen(sp, -MAXINT-1, &file);
	xprintf(&file, &args);
	putc('\0', &file);
}

static
xprintf(fp, argp) FILE *fp; union alltypes *argp;
{
	register char *cbp;
	int *iap;
	register c;
	char *s;
	char *cbs;
	int adj, pad;
	int prec;
	int fwidth;
	int pwidth;
	int isnumeric;
	register char *fmt;
	union alltypes elem;
	char cbuf[NXBUF];

	iap = (int *)argp;
	fmt = *(char **)iap;
	bump(iap, char*);
	for (;;) {
		while((c = *fmt++) != '%') {
			if (c == '\0')
				return;		/* end of format string, done */
			putc(c, fp);		/* copy non-conversion char */
		}
		pad = ' ';
		fwidth = -1;
		prec = -1;
		c = *fmt++;
		if (c == '-') {
			adj = 1;
			c = *fmt++;
		} else
			adj = 0;
		if (c == '0') {
			pad = '0';
			c = *fmt++;
		}
		if (c == '*') {
			if ((fwidth = *iap++) < 0) {
				adj = 1;
				fwidth = -fwidth;
			}
			c = *fmt++;
		} else
			for (fwidth = 0; c>='0' && c<='9'; c = *fmt++)
				fwidth = fwidth*10 + c-'0';
		if (c == '.') {
			c = *fmt++;
			if (c == '*') {
				prec = *iap++;
				c = *fmt++;
			} else
				for (prec=0; c>='0' && c<='9'; c=*fmt++)
					prec = prec*10 + c-'0';
		}
		if (c == 'l') {
			c = *fmt++;
			if (c=='d' || c=='o' || c=='u' || c=='x')
				c = toupper(c);
		}
		cbp = cbs = cbuf;
		isnumeric = 1;
		switch (c) {

		case 'd':
			elem.i = *iap++;
			if (elem.i < 0) {
				elem.i = -elem.i;
				*cbp++ = '-';
			}
			cbp = printi(cbp, elem.i, 10);
			break;

		case 'u':
			cbp = printi(cbp, *iap++, 10);
			break;
	
		case 'o':
			cbp = printi(cbp, *iap++, 8);
			break;

		case 'x':
			cbp = printi(cbp, *iap++, 16);
			break;

		case 'D':
			elem.l = *(long *)iap;
			bump(iap, long);
			if (elem.l < 0) {
				elem.l = -elem.l;
				*cbp++ = '-';
			}
			cbp = printl(cbp, elem.l, 10);
			break;

		case 'U':
			cbp = printl(cbp, *(long *)iap, 10);
			bump(iap, long);
			break;

		case 'O':
			cbp = printl(cbp, *(long *)iap, 8);
			bump(iap, long);
			break;

		case 'X':
			cbp = printl(cbp, *(long *)iap, 16);
			bump(iap, long);
			break;

		case 'e':
		case 'f':
		case 'g':
			cbp = _dtefg(c, iap, prec, cbp);
			bump(iap, double);
			break;

		case 's':
			isnumeric = 0;
			if ((s = *(char **)iap) == NULL)
				s = NULLFMT;
			bump(iap, char*);
			cbp = cbs = s;
			while (*cbp++ != '\0')
				if (prec >= 0 && cbp-s > prec)
					break;
			cbp--;
			break;
	
		case 'c':
			isnumeric = 0;
			*cbp++ = *iap++;
			break;
	
		case 'r':
			xprintf(fp, *(char ***)iap);
			bump(iap, char**);
			break;
	
		default:
			putc(c, fp);
			continue;
		}
		if ((pwidth = fwidth + cbs-cbp) < 0)
			pwidth = 0;
		if (!adj) {
			if (isnumeric && pad == '0' && *cbs == '-')
				putc(*cbs++, fp);
			while (pwidth-- != 0)
				putc(pad, fp);
		}
		while (cbs < cbp)
			putc(*cbs++, fp);
		if (adj)
			while (pwidth-- != 0)
				putc(pad, fp);
	}
}

static readonly char digits[] = {
	'0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
	'A', 'B', 'C', 'D', 'E', 'F'
};

/*
 * Convert unsigned integer n to ASCII in base b.
 * Store it through cp and return a pointer past the end.
 */
static
char *
printi(cp, n, b) register char *cp; register unsigned int n; register int b;
{
	register unsigned int a;
	register char *ep;
	char pbuf[NIBUF];

	ep = &pbuf[NIBUF-1];
	*ep = '\0';
	for ( ; (a = n/b) != 0; n = a)
		*--ep = digits[n%b];
	*cp++ = digits[n];
	while (*ep)
		*cp++ = *ep++;
	return cp;
}

/*
 * Convert unsigned long integer n to ASCII in base b.
 * Store it through cp and return a pointer past the end.
 */
static
char *
printl(cp, n, b) register char *cp; register unsigned long n; register int b;
{
	register unsigned long a;
	register char *ep;
	char pbuf[NLBUF];

	ep = &pbuf[NLBUF-1];
	*ep = '\0';
	for ( ; (a = n/b) != 0; n = a)
		*--ep = digits[n%b];
	*cp++ = digits[n];
	while (*ep)
		*cp++ = *ep++;
	return cp;
}

/* end of libc/stdio/printf.c */
