/*
 * ld/message.c
 * Print messages of various origins.
 * And misc functions.
 */

#include "ld.h"
#include <errno.h>

extern char *memset();

void
message(args)
char * args;
{
	errCount++;
	printf("%s: %r\n", argv0, &args);
}

void
w_message(args)
char * args;
{
	if (watch)
		printf("%s: %r\n", argv0, &args);
}

/*
 * Fatal error; print message and exit
 */
fatal(args)
char *args;
{
	printf("%s: %r\n", argv0, &args);
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
filemsg(fname, args)
char	*fname, *args;
{
	message( "file '%s': %r", fname, &args );
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
		filemsg(fname, "%r", &args );

	else
		filemsg(fname, "module '%.*s': %r",
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

/*
 * Message for symbol passed by pointer
 */
void
spmsg(sp, args)
sym_t	*sp;
char	*args;
{
	static char msg[] = "symbol '%s' %r";
	char work[SYMNMLEN + 1], *name;

	name = symName(&(sp->sym), work);

	if (sp->mod == NULL)
		message(msg, name, &args);
	else
		mpmsg(sp->mod, msg, name, &args);
}

/*
 * Warning message for symbol passed by pointer
 */
void
spwarn(sp, args)
sym_t	*sp;
char	*args;
{
	static char msg[] = "warning: symbol '%s' %r";
	char work[SYMNMLEN + 1], *name;

	if (qflag)
		return;

	errCount--;	/* don't increment count */
	name = symName(&(sp->sym), work);

	if (sp->mod == NULL)
		message(msg, name, &args);
	else
		mpmsg(sp->mod, msg, name, &args);
}

/*
 * Open a file or die in the attempt.
 */
int
qopen(fn, type)
char *fn;
{
	int fd;

	if (type)
		fd = creat(fn, 0666);
	else
		fd = open(fn, 0);

	if (-1 == fd) {
		filemsg(fn, "Cannot open.");	/**/
		exit(1);
	}
	return (fd);
}
