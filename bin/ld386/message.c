/*
 * ld/message.c
 * Print messages of various origins.
 * And misc functions.
 */

#include "data.h"
#include <path.h>

extern char *memset();

void
message(args)
char * args;
{
	errCount++;
	printf("ld: %r\n", &args);
}

void
w_message(args)
char * args;
{
	if (watch)
		printf("ld: %r\n", &args);
}

/*
 * Fatal error; print message and exit
 */
void
fatal(args)
char * args;
{
	printf("ld: %r\n", &args);
	exit(1);
}

/*
 * message plus command prototype
 */
void
help()
{
	static char document[] =
		"Usage ld\n"
		"\t-d\t\tDefine commons even if undefined symbols\n"
		"\t-e entry\tSet entry point\n"
		"\t-K\t\tCompile kernel\n"
		"\t-l lib\t\tUse library\n"
		"\t-o outfile\tSet output filename default is a.out\n"
		"\t-r\t\tRetain relocation information\n"
		"\t-s\t\tStrip symbol table\n"
		"\t-u sym\t\tUndefine sym\n"
		"\t-w\t\tWatch messages enabled\n"
		"\t-X\t\tDiscard local symbols beginning .L\n"
		"\t-x\t\tDiscard all local symbols\n";

	printf(document);
	exit(0);
}

void
usage()
{
	fprintf(stderr,
"usage: ld [-drswxX?] [-o out] [-e entry] [-u sym] file ... [-l lib] ...\n");
	exit(1);
}

/*
 * message with filename
 */
void
filemsg( fname, args )
char	*fname, *args;
{
	message( "file %s: %r", fname, &args );
}

/*
 * Message with module and file name
 */
void
modmsg(fname, mname, args)
char * fname;
char   mname[DIRSIZ];
char * args;
{
	if (mname[0] == 0)
		filemsg( fname, "%r", &args );

	else
		filemsg(fname, "module %.*s: %r",
			DIRSIZ, mname, &args );
}

/*
 * Message for module passed by pointer
 */
void
mpmsg(mp, args)
mod_t * mp;
char  * args;
{
	modmsg(mp->fname, mp->mname, "%r", &args);
}

void
mpfatal(mp, args)
mod_t * mp;
char  * args;
{
	modmsg(mp->fname, mp->mname, "%r", &args);
	exit(1);
}

/*
 * Message for symbol passed by pointer
 */
void
spmsg(sp, args)
sym_t	*sp;
char	*args;
{
	static char msg[] = "symbol %s: %r";

	if (sp->mod == NULL)
		message(msg, sp->name, &args);
	else
		mpmsg(sp->mod, msg, sp->name, &args);
}

/*
 * Get space or die.
 */
char *
alloc(n)
unsigned n;
{
	char	*tmp;

	if (NULL == (tmp = malloc(n)))
		fatal(memok() ?
			"Out of space" :
			"Internal error: arena corrupt");
	return (memset(tmp, '\0', n));
}

/*
 * find a file on a path in the environment, or a default path
 * with an access priveledge.
 *
 * example: pathn("helpfile", "LIBPATH", ",,\lib", "r");
 *
 * Returns full path name.
 */
char	*getenv(), *path(), *strchr();
char *
pathn(name, envpath, deflpath)
char *name, *envpath, *deflpath;
{
	register char *pathptr;

	if ((NULL == envpath) || (NULL == (pathptr = getenv(envpath))))
		pathptr = deflpath;

	if ((pathptr = path(pathptr, name, AREAD)) == NULL)
		return (NULL);
	return (pathptr);
}

/*
 * Get arguments off of argv[]. more flexable than getopt().
 * Allows optional names after arguments flagged with ! on optstring.
 * Non optional names are still flagged with : on optstring.
 * Returns non switch arguments as if they had a precedding switch of zero,
 * this permits interspersed switch and non switch arguments.
 */
char	*optarg;		/* Global argument pointer. */
int	optind = 1;		/* Global argv index. */
extern char	*strchr();

int
getargs(argc, argv, optstring)
int argc;
char *argv[];
char *optstring;
{
	register char c, d;
	register char *place;
	static char	*scan = NULL;	/* Private scan pointer. */

	for (optarg = NULL; scan == NULL || !*scan; scan++, optind++) {
		if (optind >= argc) {
			scan = NULL;
			return(EOF);
		}
		if (*(scan = argv[optind]) != '-') {
			optarg = scan;
			scan = NULL;
			optind++;
			return (0);
		}
	}

	if ((place = strchr(optstring, c = *scan++)) == NULL || 
	     (c == ':') || (c == '!')) {
		printf("%s: unknown option %c\n", argv[0], c);
		return('?');
	}

	if ((d = place[1]) == ':' || d == '!') {
		if (*scan || d == '!') {
			optarg = scan;
			scan = NULL;
		} else if (optind < argc)
			optarg = argv[optind++];
		else {
			printf("%s: %c argument missing\n",
				argv[0], c);
			return('?');
		}
	}

	return(c);
}
