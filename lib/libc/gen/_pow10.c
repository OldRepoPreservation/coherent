/*
 * Compute 10**exp, as a double.
 * Called from dtefg.c.
 * This assumes that the exponent is in the range [-511,511].
 */
double
_pow10(exp)
register int	exp;
{
	register double	d;

	static readonly double _powtab[] = {
		1e0,	1e1,	1e2,	1e3,	1e4,	1e5,
		1e6,	1e7,	1e8,	1e9,	1e10,	1e11,
		1e12,	1e13,	1e14,	1e15 };

	if (exp < 0) {
		return (1.0 / _pow10(-exp));
	}
	d = 1.0;
	if (exp >= 256) {
		exp -= 256;
		d *= 1e256;
	}
	if (exp >= 128) {
		exp -= 128;
		d *= 1e128;
	}
	if (exp >= 64) {
		exp -= 64;
		d *= 1e64;
	}
	if (exp >= 32) {
		exp -= 32;
		d *= 1e32;
	}
	if (exp >= 16) {
		exp -= 16;
		d *= 1e16;
	}
	d *= _powtab[exp];
	return (d);
/*
	return (d * _powtab[ exp]);
*/
}
