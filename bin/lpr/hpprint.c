#include	<stdio.h>
#include	<sgtty.h>
extern int printing;
extern FILE *lp;

char escbuff[64];

print( file)
char	*file;
{
	register FILE	*f;
	register int c;
	register int nraw = 0;
	int col = 0;
	struct sgttyb sg;

	f = fopen( file, "r");
	if (f == NULL)
		return (1);
	ioctl(fileno(lp), TIOCGETP, &sg);
	c = sg.sg_flags;
	sg.sg_flags &= ~(XTABS|CRMOD);
	ioctl(fileno(lp), TIOCSETP, &sg);
	sg.sg_flags = c;
	while ((c=getc(f)) != EOF && printing > 0) {
		if (nraw) {
			putc(c, lp);
			--nraw;
			continue;
		}
		switch (c) {
		case 033:
			switch (escape(f)) {
			case 'E':
				continue;
			case 'W':
				if (escbuff[1] == '*' && escbuff[2] == 'b')
					nraw = atoi(escbuff+3);
				continue;
			case 'X':
				if (escbuff[1] == '&' && escbuff[2] == 'p')
					nraw = atoi(escbuff+3);
				continue;
			}
			continue;
		case '\n':
			putc('\r', lp);
			col = 0;
			putc(c, lp);
			continue;
		case '\r':
			col = 0;
			putc(c, lp);
			continue;
		case '\t':
			do putc(' ', lp); while ((++col%8) != 0);
			continue;
		case '\b':
			if (col)
				col -= 1;
			putc(c, lp);
			continue;
		default:
			if (c >= ' ' && c < 0177)
				++col;
			putc(c, lp);
			continue;
		}
	}
	ioctl(fileno(lp), TIOCSETP, &sg);
	fclose( f);
	return (0);
}
/* Buffer up escape sequences */
escape(fp) register FILE *fp;
{
	register char *p;
	register int c;
	p = escbuff;
	*p++ = 033;
	putc(033, lp);
	if ((c = getc(fp)) != EOF && printing > 0) {
		putc(c, lp);
		*p++ = c;
		*p = 0;
		if (c == '=' || c == '9' || c == 'Y' || c == 'Z')
			return c;
		while ((c = getc(fp)) != EOF && printing > 0) {
			putc(c, lp);
			*p++ = c;
			*p = 0;
			if (c >= '@' && c <= 'Z')
				return c;
		}
	}
	return 0;
}
