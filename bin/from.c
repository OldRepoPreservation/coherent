/*
 * from - generate a list of numbers
 */

#include <stdio.h>

#define sign(x)	(x<0?-1:1)	/* returns sign of number */
#define	USAGE	"Usage: from number to number [ by number ]"

main(argc, argv)
char *argv[];
{
	int i;
	int start, end, incr;

	switch ( argc ) {

	case 4:
		incr = 1;
		break;

	case 6:
		if ( strcmp(argv[4], "by") )
			usage();
		else
			if ( (incr = numeric(argv[5])) == 0 )
				error("increment must be non-zero");
		break;

	default:
		usage();
		break;
	}
	if ( strcmp(argv[2], "to") )
		usage();
	start = numeric(argv[1]);
	end = numeric(argv[3]);
	if (start != end && sign(end-start) * sign(incr) < 0)
		error("increment has wrong sign");

	if ( incr > 0 )
		for ( i = start; i <= end; i += incr )
			printf("%d\n", i);
	else
		for ( i = start; i >= end; i += incr )
			printf("%d\n", i);
	exit(0);
}

/*
 * Return the value of a numeric arg;
 * otherwise, call usage().
 */
numeric(s)
register char *s;
{
	register int n;

	n = atoi(s);
	if ( *s == '-' )
		s++;
	for ( ; *s; s++)
		if (*s<'0' || *s>'9')
			usage();
	return(n);
}

error(x) char *x;
{
	fprintf(stderr, "from: %r\n", &x);
	exit(1);
}

usage()
{
	fprintf(stderr, "%s\n", USAGE);
	exit(1);
}
