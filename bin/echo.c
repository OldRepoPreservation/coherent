/*
 * echo.c
 * 2/14/91
 * Print arguments.
 * Usage: echo [ -n ] [ argument ... ]
 * Option: -n	Do not print terminal newline
 * Recognizes the following C-like escape conventions
 * (quote args to avoid conflicts with the shell's use of '\\'):
 *	\b	backspace
 *	\c	print line without new-line (same as -n option)
 *	\f	form-feed
 *	\n	new-line
 *	\r	carriage return
 *	\t	tab
 *	\v	vertical tab
 *	\\	backslash
 *	\0octal	where 'octal' represents the octal value of an ASCII character
 * Does not use stdio.
 */

/* Globals. */
char	buf[512];		/* working buffer			*/
char	*bufp	= &buf[0];	/* pointer to current offset in buf	*/
char	lastch	= '\n';		/* last output char (if non-zero)	*/

extern char *getoct();

main(argc, argv) int argc; register char **argv;
{
	/* Handle -n option. */
	if (strcmp(*++argv, "-n") == 0) {
		lastch = 0;
		++argv;
	}

	/*
	 * Process each argument string into buffer.
	 * Separate each argument with a blank.
	 */
	while (*argv) {
		echos(*argv);
		if (*++argv)
			echos(" ");
	}

	/* Flush the buffer. */
	if (*bufp = lastch)
		++bufp;
	if (bufp > buf)
		write(1, buf, bufp - buf);
	exit(0);
}

/*
 * Transfer string 'sp' to output buffer at offset specified by 'bufp'.
 * If buffer becomes full during transfer, write buffer to stdout,
 * and wrap 'bufp' back to beginning of buffer.
 */
echos(sp) register char *sp;
{
	register char *dp;

 	/*
	 * Copy argument to buffer.
	 */
	for (dp = bufp; *sp ; ++sp) {
		if (*sp == '\\') {
			switch (*++sp) {
			case '\0':		break;
			case 'b': *dp = '\b';	break;
			case 'c': lastch = 0;	continue;
			case 'f': *dp = '\f';	break;
			case 'n': *dp = '\n';	break;
			case 'r': *dp = '\r';	break;
			case 't': *dp = '\t';	break;
			case 'v': *dp = '\v';	break;
			case '0': sp  = getoct(++sp, dp);	break;
			default : *dp = *sp;	break;
			}
		} else
			*dp = *sp;

	 	/*
		 * Empty buffer to standard output if full.
		 */
		if (++dp == &buf[sizeof buf]) {
			write(1, buf, sizeof buf);
			dp = buf;
		}
	}
	bufp = dp;
}

/*
 * Read octal value from string (up to 3 octal digits).
 * Store result through supplied pointer, return pointer to last octal digit.
 */
char *
getoct(s, dp) register char *s; char *dp;
{
	register int val, i;

	for (val = i = 0; *s >= '0' && *s <= '7' && i < 3; ++s, ++i)
		val = val * 8 + *s - '0';	/* octal value accumulation */
	*dp = val;
	return --s;
}

/* end of echo.c */
