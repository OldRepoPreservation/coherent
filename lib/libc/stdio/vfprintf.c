/*
 * libc/stdio/printf.c
 * C standard i/o library.
 * vfprintf()
 * Not ANSI compatible!
 */

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdarg.h>

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

/* Forward. */
char	*printi();
char	*printl();

int
vfprintf(fp, fmt, args) FILE *fp; register char *fmt; va_list args;
{
	register char *cbp;
	register int c;
	char *s;
	char *cbs;
	int i, base, adj, pad, prec, fwidth, pwidth, isnumeric;
	long l;
	va_list rargs;
	char cbuf[NXBUF];

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
			if ((fwidth = va_arg(args, int)) < 0) {
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
				prec = va_arg(args, int);
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
		base = 10;
		switch (c) {

		case 'd':
			i = va_arg(args, int);
			if (i < 0) {
				i = -i;
				*cbp++ = '-';
			}
			goto integer;
			break;

		case 'u':
signedint:
			i = va_arg(args, int);
integer:
			cbp = printi(cbp, i, base);
			break;
	
		case 'o':
			base = 8;
			goto signedint;
			break;

		case 'x':
			base = 16;
			goto signedint;
			break;

		case 'D':
			l = va_arg(args, long);
			if (l < 0) {
				l = -l;
				*cbp++ = '-';
			}
			goto longint;
			break;

		case 'U':
longuns:
			l = va_arg(args, long);
longint:
			cbp = printl(cbp, l, base);
			break;

		case 'O':
			base = 8;
			goto longuns;
			break;

		case 'X':
			base = 16;
			goto longuns;
			break;

		case 'e':
		case 'f':
		case 'g':
			cbp = _dtefg(c, (double *)args, prec, cbp);
			args = ((char *)args) + sizeof(double);
			break;

		case 's':
			isnumeric = 0;
			if ((s = va_arg(args, char *)) == NULL)
				s = NULLFMT;
			cbp = cbs = s;
			while (*cbp++ != '\0')
				if (prec >= 0 && cbp-s > prec)
					break;
			cbp--;
			break;
	
		case 'c':
			isnumeric = 0;
			*cbp++ = (unsigned char)va_arg(args, int);
			break;
	
		case 'r':
			rargs = va_arg(args, va_list);
			s = va_arg(rargs, char *);
			vfprintf(fp, s, rargs);
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
