/*
 * fonts.c
 * Nroff/Troff.
 * Font handling.
 * Defines default fonts and supports loading of font width tables.
 */

#include <ctype.h>
#include <canon.h>
#include "roff.h"

int nfonts = NDFONTS;			/* number of fonts available	*/

/*
 * Map user font names to font numbers.
 * Several troff font names can map to the same font number.
 * This defines the built-in font names, the user can add more using .lf.
 */
FTB fontab[NFNAMES] ={
	{ 'R', '\0', TRMED },
	{ 'I', '\0', TRITL },
	{ 'B', '\0', TRBLD },
	{ 'T', 'R',  TRMED },
	{ 'T', 'I',  TRITL },
	{ 'T', 'B',  TRBLD }
};

/*
 * Font width table pointers, indexed by font number.
 * The order of built-in entries corresponds to the indices in "fonts.h".
 * The .lf request adds additional entries to this table.
 */
FWTAB	*fwptab[NFONTS]	= {
	&fwtab[0], &fwtab[1], &fwtab[2]
};

/* Built-in font width tables. */
FWTAB	fwtab[NDFONTS] = {
	/* 0 */
	{
		/* File tr100rpn.usp */
		"Times 10.00 point",
		"Times-Roman",	/* PostScript name */
		F_PCL,		/* flags	*/
		0,		/* fonttype	*/
		0,		/* orientation	*/
		1,		/* spacing	*/
		21,		/* symbol_set	*/
		44,		/* pitch	*/
		100,		/* pointsize	*/
		0,		/* style	*/
		0,		/* weight	*/
		5,		/* typeface	*/
		4, 165,		/* mul, div	*/
		/* Movement table: first = 33, last = 127 */
		{
			  0,  0,  0,  0,  0,  0,  0,  0,	/* 0x00-0x07 */
			  0,  0,  0,  0,  0,  0,  0,  0,	/* 0x08-0x0F */
			  0,  0,  0,  0,  0,  0,  0,  0,	/* 0x10-0x17 */
			  0,  0,  0,  0,  0,  0,  0,  0,	/* 0x18-0x1F */
			  0, 13, 13, 32, 20, 39, 34, 10,	/* 0x20-0x27 */
			 16, 16, 20, 34, 10, 14, 10, 21,	/* 0x28-0x2F */
			 20, 20, 20, 20, 20, 20, 20, 20,	/* 0x30-0x37 */
			 20, 20, 11, 11, 34, 34, 34, 20,	/* 0x38-0x3F */
			 40, 31, 28, 29, 33, 29, 25, 33,	/* 0x40-0x47 */
			 34, 15, 18, 30, 26, 39, 31, 33,	/* 0x48-0x4F */
			 25, 33, 31, 22, 27, 34, 31, 40,	/* 0x50-0x57 */
			 31, 31, 29, 13, 21, 13, 20, 20,	/* 0x58-0x5F */
			 10, 20, 22, 18, 22, 20, 13, 20,	/* 0x60-0x67 */
			 22, 11, 11, 21, 11, 34, 22, 22,	/* 0x68-0x6F */
			 22, 22, 16, 16, 13, 22, 18, 28,	/* 0x70-0x77 */
			 18, 18, 18, 21, 20, 21, 34, 42,	/* 0x78-0x7F */
			  0,  0,  0,  0,  0,  0,  0,  0,	/* 0x80-0x87 */
			  0,  0,  0,  0,  0,  0,  0,  0,	/* 0x88-0x8F */
			  0,  0,  0,  0,  0,  0,  0,  0,	/* 0x90-0x97 */
			  0,  0,  0,  0,  0,  0,  0,  0,	/* 0x98-0x9F */
			  0,  0,  0,  0,  0,  0,  0,  0,	/* 0xA0-0xA7 */
			  0,  0,  0,  0,  0,  0,  0,  0,	/* 0xA8-0xAF */
			  0,  0,  0,  0,  0,  0,  0,  0,	/* 0xB0-0xB7 */
			  0,  0,  0,  0,  0,  0,  0,  0,	/* 0xB8-0xBF */
			  0,  0,  0,  0,  0,  0,  0,  0,	/* 0xC0-0xC7 */
			  0,  0,  0,  0,  0,  0,  0,  0,	/* 0xC8-0xCF */
			  0,  0,  0,  0,  0,  0,  0,  0,	/* 0xD0-0xD7 */
			  0,  0,  0,  0,  0,  0,  0,  0,	/* 0xD8-0xDF */
			  0,  0,  0,  0,  0,  0,  0,  0,	/* 0xE0-0xE7 */
			  0,  0,  0,  0,  0,  0,  0,  0,	/* 0xE8-0xEF */
			  0,  0,  0,  0,  0,  0,  0,  0,	/* 0xF0-0xF7 */
			  0,  0,  0,  0,  0,  0,  0,  0 	/* 0xF8-0xFF */
		}
	},
	/* 1 */
	{
		/* File tr100ipn.usp */
		"Times 10.00 point italic",
		"Times-Italic",	/* PostScript name */
		F_PCL,		/* flags	*/
		0,		/* fonttype	*/
		0,		/* orientation	*/
		1,		/* spacing	*/
		21,		/* symbol_set	*/
		44,		/* pitch	*/
		100,		/* pointsize	*/
		1,		/* style	*/
		0,		/* weight	*/
		5,		/* typeface	*/
		4, 165,		/* mul, div	*/
		/* Movement table: first = 33, last = 127 */
		{
			  0,  0,  0,  0,  0,  0,  0,  0,	/* 0x00-0x07 */
			  0,  0,  0,  0,  0,  0,  0,  0,	/* 0x08-0x0F */
			  0,  0,  0,  0,  0,  0,  0,  0,	/* 0x10-0x17 */
			  0,  0,  0,  0,  0,  0,  0,  0,	/* 0x18-0x1F */
			  0, 12, 14, 32, 20, 40, 29, 10,	/* 0x20-0x27 */
			 16, 16, 20, 34, 10, 13, 10, 21,	/* 0x28-0x2F */
			 20, 20, 20, 20, 20, 20, 20, 20,	/* 0x30-0x37 */
			 20, 20, 11, 11, 34, 34, 34, 19,	/* 0x38-0x3F */
			 41, 29, 27, 28, 31, 28, 25, 33,	/* 0x40-0x47 */
			 32, 15, 16, 28, 27, 36, 31, 32,	/* 0x48-0x4F */
			 25, 32, 28, 23, 26, 30, 27, 36,	/* 0x50-0x57 */
			 28, 25, 28, 17, 21, 17, 20, 20,	/* 0x58-0x5F */
			 10, 21, 21, 18, 22, 18, 12, 17,	/* 0x60-0x67 */
			 22, 11, 11, 20, 11, 34, 22, 22,	/* 0x68-0x6F */
			 21, 21, 13, 16, 11, 22, 18, 27,	/* 0x70-0x77 */
			 17, 17, 16, 21, 20, 21, 34, 42,	/* 0x78-0x7F */
			  0,  0,  0,  0,  0,  0,  0,  0,	/* 0x80-0x87 */
			  0,  0,  0,  0,  0,  0,  0,  0,	/* 0x88-0x8F */
			  0,  0,  0,  0,  0,  0,  0,  0,	/* 0x90-0x97 */
			  0,  0,  0,  0,  0,  0,  0,  0,	/* 0x98-0x9F */
			  0,  0,  0,  0,  0,  0,  0,  0,	/* 0xA0-0xA7 */
			  0,  0,  0,  0,  0,  0,  0,  0,	/* 0xA8-0xAF */
			  0,  0,  0,  0,  0,  0,  0,  0,	/* 0xB0-0xB7 */
			  0,  0,  0,  0,  0,  0,  0,  0,	/* 0xB8-0xBF */
			  0,  0,  0,  0,  0,  0,  0,  0,	/* 0xC0-0xC7 */
			  0,  0,  0,  0,  0,  0,  0,  0,	/* 0xC8-0xCF */
			  0,  0,  0,  0,  0,  0,  0,  0,	/* 0xD0-0xD7 */
			  0,  0,  0,  0,  0,  0,  0,  0,	/* 0xD8-0xDF */
			  0,  0,  0,  0,  0,  0,  0,  0,	/* 0xE0-0xE7 */
			  0,  0,  0,  0,  0,  0,  0,  0,	/* 0xE8-0xEF */
			  0,  0,  0,  0,  0,  0,  0,  0,	/* 0xF0-0xF7 */
			  0,  0,  0,  0,  0,  0,  0,  0 	/* 0xF8-0xFF */
		}
	},
	/* 2 */
	{
		/* File tr100bpn.usp */
		"Times 10.00 point bold",
		"Times-Bold",	/* PostScript name */
		F_PCL,		/* flags	*/
		0,		/* fonttype	*/
		0,		/* orientation	*/
		1,		/* spacing	*/
		21,		/* symbol_set	*/
		44,		/* pitch	*/
		100,		/* pointsize	*/
		0,		/* style	*/
		3,		/* weight	*/
		5,		/* typeface	*/
		4, 165,		/* mul, div	*/
		/* Movement table: first = 33, last = 127 */
		{
			  0,  0,  0,  0,  0,  0,  0,  0,	/* 0x00-0x07 */
			  0,  0,  0,  0,  0,  0,  0,  0,	/* 0x08-0x0F */
			  0,  0,  0,  0,  0,  0,  0,  0,	/* 0x10-0x17 */
			  0,  0,  0,  0,  0,  0,  0,  0,	/* 0x18-0x1F */
			  0, 13, 14, 32, 20, 32, 32, 10,	/* 0x20-0x27 */
			 16, 16, 20, 34, 10, 13, 10, 21,	/* 0x28-0x2F */
			 20, 20, 20, 20, 20, 20, 20, 20,	/* 0x30-0x37 */
			 20, 20, 11, 11, 34, 34, 34, 20,	/* 0x38-0x3F */
			 40, 27, 27, 29, 30, 27, 25, 31,	/* 0x40-0x47 */
			 32, 16, 20, 31, 26, 39, 30, 32,	/* 0x48-0x4F */
			 25, 32, 29, 24, 26, 30, 26, 38,	/* 0x50-0x57 */
			 29, 26, 27, 18, 21, 18, 20, 20,	/* 0x58-0x5F */
			 10, 21, 22, 18, 23, 18, 13, 20,	/* 0x60-0x67 */
			 23, 12, 13, 22, 12, 34, 23, 21,	/* 0x68-0x6F */
			 23, 22, 18, 18, 14, 23, 19, 26,	/* 0x70-0x77 */
			 19, 18, 18, 21, 20, 21, 34, 42,	/* 0x78-0x7F */
			  0,  0,  0,  0,  0,  0,  0,  0,	/* 0x80-0x87 */
			  0,  0,  0,  0,  0,  0,  0,  0,	/* 0x88-0x8F */
			  0,  0,  0,  0,  0,  0,  0,  0,	/* 0x90-0x97 */
			  0,  0,  0,  0,  0,  0,  0,  0,	/* 0x98-0x9F */
			  0,  0,  0,  0,  0,  0,  0,  0,	/* 0xA0-0xA7 */
			  0,  0,  0,  0,  0,  0,  0,  0,	/* 0xA8-0xAF */
			  0,  0,  0,  0,  0,  0,  0,  0,	/* 0xB0-0xB7 */
			  0,  0,  0,  0,  0,  0,  0,  0,	/* 0xB8-0xBF */
			  0,  0,  0,  0,  0,  0,  0,  0,	/* 0xC0-0xC7 */
			  0,  0,  0,  0,  0,  0,  0,  0,	/* 0xC8-0xCF */
			  0,  0,  0,  0,  0,  0,  0,  0,	/* 0xD0-0xD7 */
			  0,  0,  0,  0,  0,  0,  0,  0,	/* 0xD8-0xDF */
			  0,  0,  0,  0,  0,  0,  0,  0,	/* 0xE0-0xE7 */
			  0,  0,  0,  0,  0,  0,  0,  0,	/* 0xE8-0xEF */
			  0,  0,  0,  0,  0,  0,  0,  0,	/* 0xF0-0xF7 */
			  0,  0,  0,  0,  0,  0,  0,  0 	/* 0xF8-0xFF */
		}
	}
};

static int errflag;			/* font table read error flag */

/* Read a canonical int from file. */
int
get_int(fp) register FILE *fp;
{
	int i;

	if (fread(&i, sizeof i, 1, fp) != 1)
		errflag = 1;
	canint(i);
	return i;
}

/*
 * Read a NUL-terminated string from file.
 * Return a pointer to a newly allocated copy of it.
 * Die if it is too long.
 */
#define	NBUF	256
char *
get_str(fp) register FILE *fp;
{
	static char buf[NBUF];
	register char *s;
	register int c;

	for (s = buf; s < &buf[NBUF]; ) {
		c = fgetc(fp);
		*s++ = c;
		if (c == '\0')
			break;
		else if (c == EOF) {
			errflag = 1;
			return NULL;
		}
	}
	if (s == &buf[NBUF]) {
		errflag = 1;
		return;
	}
	s = nalloc(strlen(buf) + 1);
	strcpy(s, buf);
	return s;
}

void
load_font(s, file) char *s, *file;
{
	register FWTAB *p;
	register FILE *fp;
	int i, new, newflag;

	if ((fp = fopen(file, "rb")) == NULL) {
		/* Not found, look in default fwt directory. */
		sprintf(miscbuf, "%s%s", (pflag) ? FWTPS : FWTPCL, file);
		if ((fp = fopen(miscbuf, "rb")) == NULL) {
			printe(".lf: cannot open file \"%s\"", file);
			return;
		}
	}
	if ((new = font_num(s)) == -1) {
		/* Font does not already exist, allocate new FWTAB entry. */
		if (nfonts >= NFONTS) {
			printe(".lf: cannot load more than %d fonts", NFONTS);
			return;
		}
		newflag = 1;
		new = nfonts;			/* assign new font number */
		fwptab[new] = nalloc(sizeof(FWTAB)); /* allocate new FWTAB */
	} else
		newflag = 0;
	errflag = 0;
	p = fwptab[new];
	p->f_descr = get_str(fp);
	p->f_PSname = get_str(fp);
	p->f_flags = get_int(fp);
	p->f_fonttype = get_int(fp);
	p->f_orientation = get_int(fp);
	p->f_spacing = get_int(fp);
	p->f_symset = get_int(fp);
	p->f_pitch = get_int(fp);
	p->f_psz = get_int(fp);
	p->f_style= get_int(fp);
	p->f_weight = get_int(fp);
	p->f_face = get_int(fp);
	p->f_num = get_int(fp);
	p->f_den = get_int(fp);
	for (i = 0; i < NWIDTH; i++)
		p->f_width[i] = fgetc(fp);
	i = fgetc(fp);				/* should return EOF */
	fclose(fp);
	if (i != EOF || errflag) {
		printe(".lf: %s, file \"%s\"",
			(i != EOF) ? "bad font width table" : "read error",
			file);
		if (newflag)
			free(p);
		return;
	} else if ((p->f_flags & F_PCL) == 0 && !pflag)
		printe(".lf: \"%s\" is not a PCL font width table", file);
	else if ((p->f_flags & F_PS) == 0 && pflag)
		printe(".lf: \"%s\" is not a PostScript font width table", file);
	fpsz[new] = p->f_psz;			/* enter pointsize in env */
	fcsz[new] = 0;				/* enter const char size in env */
	assign_font(s, new);			/* and assign desired name */
	if (newflag)
		++nfonts;
}

/* end of fonts.c */
