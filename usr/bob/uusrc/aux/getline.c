/*
 *  getline.c
 *
 *  Support the parsing of ASCII text files into logical lines.
 */

#include <stdio.h>

static	char linebuf[BUFSIZ];

/*
 * getline() reads from the specified file the next logical line of
 * text, and returns the number of characters in that logical line,
 * not counting the NUL terminator.  End of file is indicated by a
 * return value of zero. The syntax which is implemented is as follows:
 *
 *	A backslash (\) in a physical line, which is followed by white
 *	space and does NOT directly follow a backslash (\), indicates
 *	the logical line will continue with the next physical line in
 *	the file.  
 *
 *	A pound sign (#) in a physical line, which does not directly
 *	follow a backslash (\), indicates the logical line being built
 *	(if already begun) is complete and the remainder of the physical
 *	line is a comment.
 *
 *	All contiguous sequences of white space (' ' and '\t') are
 *	coalesced into a single space character.
 *
 *  If the logical line will exceed the maximum length specified, then
 *  the return value will be (-1).  In that case, the buffer will be
 *  filled with the full "maxlen" characters of the logical line which
 *  were read from the file (i.e. not NUL terminated).
 */

#define	insch(CH)	*cp++ = CH;  if (cp > eob) return( -1 )

int
getline(fp, buf, maxlen)
FILE *fp;
char *buf;
int maxlen;
{
	register char ch, *ptr;
	char *cp = buf;
	char *eob = buf + maxlen - 1;
	int eopl, eoll;

	eoll = 0;
	while ( (!eoll) && (fgets(linebuf, BUFSIZ, fp) != NULL) ) {
		eopl = 0;
		for (ptr=&linebuf[0]; (!eopl) && (ch=*ptr++);) {
			switch ( ch ) {
			case '#':
			case '\n':
				eopl = 1;
				eoll = (cp != buf);
				continue;
			case ' ':
			case '\t':
				if ( (cp == buf) || (*(cp-1) == ' ') )
					continue;
				insch(' ');
				continue;
			case '\\':
				switch ( ch = *ptr++ ) {
				case '\0':
				case '\t':
				case '\n':
				case ' ' :
					eopl = 1;  continue;
				}
				insch('\\');
			default:
				insch(ch);
				continue;
			}
		}
	}
	if ( *(cp-1) == ' ' )
		--cp;
	*cp = '\0';
	return( cp - buf );
}
