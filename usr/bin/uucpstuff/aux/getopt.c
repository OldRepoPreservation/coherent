/*
 *  getopt.c
 *
 *  Working source for libc function getopt().
 */

#include <stdio.h>

extern char *index();

int	optind = 1;		/* argv index to be processed NEXT	*/
int	optopt = '\0';		/* Parsed option character value	*/
char	*optarg;		/* Parsed option argument pointer	*/

getopt(argc, argv, opts)
int argc;
char **argv, *opts;
{
	static int offset = 1;
	register char ch;
	register char *cp;

	optopt = '\0';
	if ( offset == 1 ) {
		if ( (optind >= argc) || (argv[optind][0] != '-') ||
		     (argv[optind][1] == '\0') )
			return(EOF);
		else if ( !strcmp(argv[optind], "--") ) {
			optind++;
			return(EOF);
		}
	}

	optopt = ch = argv[optind][offset];
	if ( (ch == ':') || ((cp=index(opts, ch)) == NULL) ) {
		if ( argv[optind][++offset] == '\0' ) {
			optind++;
			offset = 1;
		}
		return('?');
	}

	if( *(cp+1) == ':' ) {
		if ( argv[optind][offset+1] != '\0' ) {
			optarg = &argv[optind++][offset+1];
		} else if ( ++optind >= argc ) {
			optarg = NULL;
			offset = 1;
			return('?');
		} else
			optarg = argv[optind++];
		offset = 1;
	} else {
		if ( argv[optind][++offset] == '\0' ) {
			offset = 1;
			optind++;
		}
		optarg = NULL;
	}
	return(ch);
}
