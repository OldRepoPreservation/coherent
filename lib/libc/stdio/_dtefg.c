/*
 * Floating point output conversion routines for 'printf'.
 * This source is conditionalized #if NDP to do 8087 conversion.
 */

#include <math.h>

#if NDP
#include <stdio.h>

/*
 * This table, indexed by the return value of the "_fxam" routine,
 * gives the string to print.
 * The number is converted if the entry is NULL.
 */
static readonly char *fxamsg[] = {
	"{+ Unnormal}",
	"{+ NAN}",
	"{- Unnormal}",
	"{- NAN}",
	NULL,
	"{+ Infinity}",
	NULL,
	"{- Infinity}",
	NULL,
	NULL,
	NULL,
	NULL,
	"{+ Denormal}",
	NULL,
	"{- Denormal}",
	NULL
};
#endif

/*
 * Convert a floating point number from binary
 * into 'e', 'f' or 'g' format ASCII.
 * The 'fmt' argument is the conversion type.
 * The 'w' argument is the precision.
 * The characters get put into the buffer 'bp'.
 * A pointer past the last character is returned.
 */
char *
_dtefg(fmt, d, w, bp)
double	d;
char	*bp;
{
	register char	*cp1, *cp2;
	int		dscale;
	int		minusf;
	char		*_dtoa();
	char		tbuf[64];

#if NDP
	if ((cp1=fxamsg[_fxam(d)]) != NULL) {
		cp2 = bp;
		while (*cp2++ = *cp1++)
			;
		return (cp2);
	}
#endif
	if (w < 0)
		w = 6;
	cp1 = _dtoa(fmt, d, w, &dscale, &minusf, tbuf);
	if (fmt == 'g') {
		int	nd;

		cp2 = cp1;
		while (*cp2)
			++cp2;
		while (cp2!=cp1 && cp2[-1]=='0')
			--cp2;
		nd = cp2-cp1;
		if (dscale < -3 || dscale > nd+5)
			fmt = 'e';
		else if (dscale >= nd)
			w = 0;				/* 'd' format */
	}
	cp2 = bp;
	if (minusf != 0)
		*cp2++ = '-';
	if (fmt == 'e') {
		*cp2++ = *cp1++;
		if (d != 0.0)
			--dscale;
		for (*cp2++ = '.'; w > 0; --w)
			*cp2++ = *cp1 ? *cp1++ : '0';
		*cp2++ = 'e';
		if (dscale >= 0)
			*cp2++ = '+';
		else {
			*cp2++ = '-';
			dscale = -dscale;
		}
		sprintf(cp2, "%u", dscale);
		cp2 += strlen(cp2);
	} else {
		if (dscale <= 0)
			*cp2++ = '0';
		else do
			*cp2++ = *cp1 ? *cp1++ : '0';
		while (--dscale);
		if (w != 0) {
			for (*cp2++ = '.'; w > 0; --w) {
				if (dscale++ < 0)
					*cp2++ = '0';
				else
					*cp2++ = *cp1 ? *cp1++ : '0';
			}
		}
	}
	return (cp2);
}

/*
 * Convert double to string of ASCII digits
 * rounded after precision digits behind the decimal point.
 * The decimal scale is stored indirectly through 'dscalep' and the sign
 * (nonzero if negative) is saved indirectly through 'minusfp'.
 */
char *
_dtoa(fmt, d, w, dscalep, minusfp, buf)
double	d;
int	w;
int	*dscalep;
int	*minusfp;
char	*buf;
{
	register char	*cp;
	register int	digit;
	register int	i;
	register int	dscale;
	int		ndigit;
	int		binexp;
	double		frexp();
	double		_pow10();

	if (d >= 0.0) {
		*minusfp = 0;
		if (d == 0.0) {
			*dscalep = 0;
			cp = buf;
			while (w-- != 0)
				*cp++ = '0';
			*cp = '\0';
			return (buf);
		}
	} else {
		*minusfp = 1;
		d = -d;
	}
	frexp(d, &binexp);
	dscale = binexp / LOG10B2;
	d /= _pow10(dscale);
	if (d >= 1.0)
		++dscale;
	else
		d *= 10.0;
	*dscalep = dscale;
	digit = (int) d;
	cp = buf;
	ndigit = w;
	if (fmt == 'f'
	 || fmt == 'g' && 0 <= dscale && dscale <= 5+w)
		ndigit = w+dscale;
	else
		ndigit = w+1;
	if (ndigit < 0)
		ndigit = 0;
	if (ndigit > L10P)
		ndigit = L10P;
	for (i=0; i<ndigit; ++i) {
		*cp++ = digit + '0';
		d = 10.0 * (d-digit);
		digit = (int) d;
	}
	*cp = '\0';
	if (digit < 5)
		return (buf);
	while (cp != buf) {
		--cp;
		if (++*cp <= '9')
			return (buf);
		*cp = '0';
	}
	*cp = '1';
	cp[ndigit] = '\0';
	++*dscalep;
	return (buf);
}
