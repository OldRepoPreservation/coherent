/*
 * libc/stdio/fread.c
 * Standard i/o library.
 * Read nitems of size from file fp to bp.
 */

#include <stdio.h>

int
fread(bp, size, nitems, fp)
register char	*bp;
unsigned int	size;
unsigned int	nitems;
register FILE	*fp;
{
	unsigned int	nb;
	register int	c;

	nb = size * nitems;
	if (fp->_ff&_FUNGOT) {
		*bp++ = (*fp->_gt)(fp);
		nb--;
	}
	if (fp->_bp!=NULL || !(fp->_ff&_FSTBUF))
		for (; nb && (c=getc(fp))!=EOF; nb--)
			*bp++ = c;
	else if ((c=read(fileno(fp), bp, nb)) > 0)
		nb -= c;
	else if (c == 0)
		fp->_ff |= _FEOF;
	else
		fp->_ff |= _FERR;
	/* Adjust seek after partial read. */
	if (nb != 0 && nb % size != 0)
		fseek(fp, (long)(nb % size - size), SEEK_CUR);
	return ((size*nitems-nb)/size);
}

/* end of libc/stdio/fread.c */
