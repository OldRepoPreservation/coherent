/*
 * libc/stdio/gets.c
 * Standard I/O Library.
 * Get string from standard input, deleting trailing '\n'.
 */

#include <stdio.h>

char *
gets(is) register char *is;
{
	register char	*s;
	register int	c;

	s = is;
	while ((c = getchar()) != EOF && c != '\n')
		*s++ = c;
	if (c == EOF && s == is)
		return NULL;		/* ANSI says leave *s unchanged */
	*s = '\0';			/* else NUL-terminate */
	return is;
}

/* end of libc/stdio/gets.c */
