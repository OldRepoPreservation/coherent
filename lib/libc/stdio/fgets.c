/*
 * libc/stdio/fgets.c
 * Coherent Standard I/O Library.
 * Read a string from input file pointer, leaving the trailing '\n'.
 */

#include <stdio.h>

char *
fgets(is, lim, ifp) char *is; register int lim; FILE *ifp;
{
	register int	c;
	register char	*s;

	s = is;
	while (--lim > 0 && (c = getc(ifp)) != EOF)
		if ((*s++ = c) == '\n')
			break;
	if (c == EOF && s == is)
		return NULL;		/* ANSI says leave *s unchanged */
	*s = '\0';			/* else NUL-terminate */
	return is;
}

/* end of libc/stdio/fgets.c */
