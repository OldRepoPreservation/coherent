/*
 * Fatal error with message.
 */
#include <scn.h>

fatal(s)
char *s;
{
	closeUp();
	fprintf(stderr, "\nFatal: %r\n", &s);
	exit(1);
}
