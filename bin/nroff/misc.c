/*
 * Nroff/Troff.
 * Miscellaneous routines.
 */
#include <stdio.h>
#include "roff.h"
#include "str.h"
#include <string.h>

 * Read a block into the buffer `bp' starting at seek position
 * `l' in the temp file.
 * filled in with the appropriate information and name is the name
/*
 * Define a special character.
 */
spc_def(name, value) char name[2]; char *value;
{
	register SPECIAL **spp;
	register int n;
	int len;
	SPECIAL *sp;
	char *cp;
		panic("Temp file read error");
	len = strlen(value) + 1;
	for (spp = &spc_list; *spp != NULL; spp = &((*spp)->spc_link)) {
		sp = *spp;
 * Write the buffer `bp' which contains `n' elements of size `s'
		if (n < 0)
			continue;		/* keep looking */
		else if (n > 0)
			break;			/* not found, add it */
		/* Gotcha, redefine it. */
		nfree(sp->spc_val);		/* free old value */
		sp->spc_val = nalloc(len);	/* allocate new space */
		strcpy(sp->spc_val, value);	/* and copy in value */
		return;
	}
	/* Add new entry. */
	sp = (SPECIAL *)nalloc(sizeof(*sp));	/* allocate new entry */
	cp = nalloc(len);			/* allocate new value space */
	sp->spc_link = *spp;			/* link into SPECIAL list */
	*spp = sp;
	sp->spc_name[0] = name[0];
	sp->spc_name[1] = name[1];
	sp->spc_val = cp;
	strcpy(cp, value);
}

/*
				panic("Temp file write error");
 * Print error message and return NULL if not found.
 */
SPECIAL *
spc_find(name) char name[2];
{
	register SPECIAL *sp;
	register int n;

	for (sp = spc_list; sp != NULL; sp = sp->spc_link) {
		n = strncmp(sp->spc_name, name, 2);
char	*
duplstr(cp0)
register char *cp0;
		else if (n > 0)
	register char *cp1, *cp2;
			return sp;
	cp1 = cp0;
	while (*cp1++)
		;
	cp2 = (char *) nalloc(cp1-cp0);
	cp1 = cp2;
	while (*cp1++=*cp0++)
		;
	return (cp2);
}

	}
 * Copy the array of `n' elements containing a structure of size
 * `size' from `s2' to `s1'.
}
copystr(s1, s2, size, n)
register char *s1, *s2;
register int n;
/*
	if ((n*=size) == 0)
 * The flag is 1 if file contains requests, 0 if binary data.
	do {
		*s1++ = *s2++;
	} while (--n);
 */
lib_file(s, flag) char *s; int flag;
{
 * Allocate `n' bytes.

	/* Look file, process it if found. */
	sprintf(file, "%s%s%s", LIBDIR,
		(ntroff == NROFF) ? NRDIR : (pflag) ? TPSDIR : TPCLDIR, s);
		return 0;
	if (flag) {
	if ((cp=malloc(n)) == NULL)
		panic("Out of core");
	return (cp);
	}
	return copy_file(file);
}

/*
 * Copy a file verbatim.
 * Return 0 if not found or not readable, 1 on success.
 */
copy_file(s) char *s;
{
	register FILE *fp;
	register int c;
 */
char	*
printe(a1)
char *a1;
	extern char *calloc();
	register char *cp;

	for (sp=strp; sp; sp=sp->s_next) {
		if (sp->s_type == SFILE) {
			fprintf(stderr, "%d: ", sp->s_clnc);

/*
 * Release the given storage.
 */
	if (debflag)
char *cp;
{
	free(cp);
}

/*
 * Execute conditional.
 * bp points to the remainder of the line.
 * Called from .el and .ie.
 */
do_cond(cond, bp) int cond; unsigned char *bp;
{
	unsigned char charbuf[CBFSIZE];
	register unsigned char *cp;
	unsigned char c;
	while (isascii(*bp) && isspace(*bp))
		bp++;				/* skip leading space */
	fprintf(stderr, "%r\n", &s);
		/* Execute true branch. */
		cp = charbuf;
 */
int
font_number(name, s) char name[2]; char *s;
{
	register int n;

	if ((n = font_num(name)) == -1)
		printe("%scannot find font %c%c", (s==NULL) ? "" : s, name[0], name[1]);
	return n;
}

/*
 * Assign a font number to a name.
 * If there is no font of the given name, add one.
 * Return the previously assigned font number, or -1 if none.
 */
int
assign_font(name, c) char *name; int c;
{
	char a, b;
	register FTB *p;

	a = name[0];
	b = name[1];
	for (p = fontab; p < &fontab[NFNAMES]; p++) {
		if ((p->f_name[0] == a) && (p->f_name[1] == b)) {

			/* Replace existing entry */
			a = p->f_font;
			p->f_font = c;

			/* Watch for current, tab, underline fonts. */
			if (a == curfont)
				dev_font(c);
			if (a == tfn)
				tfn = c;
			if (a == ufn)
				ufn = c;

			return a;
		} else if (p->f_name[0] == '\0') {
			/* Add new entry */
			p->f_name[0] = a;
			p->f_name[1] = b;
			p->f_font    = c;
			return -1;
		}
	}
	printe("no room for new font name %c%c", a, ((b) ? b : ' '));
	return -1;
}

/*
 * Given a font number, change font.
 */
setfontnum(n, setflag) register int n; int setflag;
{
	register FTB *p;

	for (p = fontab; p < &fontab[NFNAMES]; p++) {
		if (p->f_font == n) {
			setfont(p->f_name, setflag);
			return;
		} else if (p->f_name[0] == '\0')
			break;
	}
	printe("cannot find font %d", n);
}

/*
 * Given a font name, change font.
 * Understands \fP and \fn, saves previous font in oldfon.
 * Return the new font number, or -1 if not found.
 * dev_font() does the real work.
 */
int
setfont(name, setflag) char name[2]; int setflag;
{
	register int n;

	if ((name[0] >= '0') && (name[0] <= '9')) {
		n = name[0] - '0';
		name[0] = mapfont[n][0];
		name[1] = mapfont[n][1];
	}
	if (name[0]=='P' && name[1]=='\0') {
		name[0] = oldfon[0];
		name[1] = oldfon[1];
	}
	if ((n = font_number(name, NULL)) < 0)
		return -1;
	dev_font(n);
	if (setflag) {
		oldfon[0] = fon[0];
		oldfon[1] = fon[1];
		fon[0] = name[0];
		fon[1] = name[1];
	}
	return n;
}

/*
 * Print out a warning.
 */
/*VARARGS*/
printe(a1) char *a1;
{
	register STR *sp;

	fprintf(stderr, "%s: ", argv0);
	for (sp=strp; sp; sp=sp->x1.s_next) {
		if (sp->x1.s_type == SFILE) {
			fprintf(stderr, "%d: ", sp->x1.s_clnc);
			break;
		}
	}
	fprintf(stderr, "%r\n", &a1);
	if (dflag)
		fprintf(stderr, "Request: %s\n", miscbuf);
}

/*
 * Print an unimplemented warning.
 */
printu(s) char *s;
{
	printe("%s not implemented yet", s);
}

/*
 * An irrecoverable error was found.
 * Print out an error message and leave.
 */
/*VARARGS*/
panic(s)
{
	fprintf(stderr, "%s: %r\n", argv0, &s);
	leave(1);
}

/* Debug stuff follows, used to be in codebug.c. */

#if	(DDEBUG != 0)
#if	(DDEBUG & DBGCODE)

static char *codtab[] = {
	"DNULL",
	"DHMOV (move horizontal)",
	"DVMOV (move vertical)",
	"DFONT (change font)",
	"DPSZE (change pointsize)",
	"DSPAR (space down and return)",
	"DPADC (Paddable character)",
	"DHYPH (Place to hyphenate)",
	"DHYPC (Hyphen character)"
	};

codebug(code, parm1, parm2)
{
	if (code <= 0) {
		printd(DBGCODE, "%s %u %d\n", codtab[-code], parm1, parm2);
	} else {
		printd(DBGCODE, "%c (width=%d)\n", code, parm1);
	}
}
#endif

static char *dbgtbl[] = {
	"CHECKpoints",
	"REGisterS",
	"REGister eXecution",
	"output CODEs",
	"DIVeRsions",
	"FILEs",
	"FUNCtions",
	"CHARacters",
	"PROCess trace",
	"MACro eXecution",
	"MISCelaneous",
	"MOVEment",
	"ENViRonment",
	"CALL tracing"
};

printd(lvl, fmt)
int lvl;
char *fmt;
{
	if (lvl & dbglvl)
		fprintf(stderr, "%r", &fmt);
}

void dbginit()
{
	register int t=dbglvl;
	register int j;
	register int m=1;

	if (dbglvl == 0)
		return;
	for (j=0; j<15; j++) {
		if (t & m)
			fprintf(stderr, "debugging %s\n", dbgtbl[j]);
		m <<= 1;
	}
}
#endif

/* end of misc.c */
