/*
 * die.c -- Get information out from a very young kernel which probably
 * can't do printf()'s.
 */

#include <sys/coherent.h>
#include <sys/mmu.h>

#define COLOR	((char *) (0x0000B0000 + ctob(SBASE-PBASE)))
#define MONO	((char *) (0x0000B8000 + ctob(SBASE-PBASE)))
#define PAGING	(0x80000000)	/* Paging bit in cr0, set if paging is on.  */
static int chirp_off;

/*
 * void _chirp(char c, off);
 * Put character 'c' directly in video memory at offset 'off';
 *
 * This routine must not use any static variables, since the .data
 * segment is not necessarily available yet.
 */
void
_chirp(c, off)
	char c;
{
	if (0 == (read_cr0() & PAGING)) {
		*(COLOR + off) = c;
		*(MONO + off) = c;
	} else {
		*((char *) (ctob(VIDEOa) + off)) = c;
		*((char *) (ctob(VIDEOb) + off)) = c;
	}
} /* _chirp() */

/*
 * void chirp(char c);
 * Put character 'c' directly in the first character of video memory;
 *
 * This routine must not use any static variables, since the .data
 * segment is not necessarily available yet.
 */
void
chirp(c)
	char c;
{
	_chirp(c, 158);
} /* chirp() */


/*
 * void mchirp(char *str);
 * Put string 'str' directly in the next character of video memory;
 * Note that calls to chirp and dchirp do not effect what mchirp considers
 *      to be the next character.
 *
 * This routine uses a ds variable, so it must not be used until the .data
 * segment is available (this currently happens in the middle of mchinit).
 */
void
strchirp(str)
	char *str;
{
	char c;
	
	while (c = *str++) {
		_chirp(c, chirp_off);
		chirp_off += 2;
	}
} /* strchirp() */

/*
 * void mchirp(char c);
 * Put character 'c' directly in the next character of video memory;
 * If c == 0 reset the "next" character to be the first character.
 * Note that calls to chirp and dchirp do not effect what mchirp considers
 *      to be the next character.
 *
 * This routine uses a static variable, so it must not be used until the .data
 * segment is available (this currently happens in the middle of mchinit).
 */
void
mchirp(c)
	char c;
{
	if ('\0' != c) {
		_chirp(c, chirp_off);
		chirp_off += 2;
	}
	else
		chirp_off = 0;
} /* mchirp() */

/*
 * void dchirp(char c, charpos);
 * Put character 'c' directly in the 'charpos' character of video memory;
 *
 * This routine must not use any static variables, since the .data
 * segment is not necessarily available yet.
 */
void
dchirp(c, charpos)
	char c;
{
	_chirp(c, charpos<<1);
} /* dchirp() */

/*
 * void die(char c);
 * Put character 'c' directly in video memory, and then halt.
 */
void
die(c)
	char c;
{
	_chirp(c, 0);
	for (;;) {
		halt();
	}
}
