/*
 * fonts.h
 * Nroff/Troff.
 * Font handling header.
 */

#define	NFNAMES	100			/* max number of font names	*/
#define	NDFONTS	3			/* number of default fonts	*/
#define	NWIDTH	256			/* number of width entries	*/

/*
 * The font table contains the user name and font index of each font.
 */
typedef struct {
	char f_name[2];
	int f_font;
} FTB;

/*
 * A font width table defines the widths of characters in a font.
 * The entries in f_width * f_psz * f_num / f_den should
 * be the widths in troff units; e.g., if the f_width table values are
 * in 300ths of an inch, then f_psz * f_num / f_den should be 12/5.
 * fwtable.c knows how to write one of these for PCL,
 * fwtableps.c knows how to write one of these for PostScript, and
 * fonts.c/load_font knows how to read one; they had better all agree.
 * fonts.c also defines the three default troff FWTABs.
 * Many of the members could be chars.
 */
typedef struct	fwtab {
	char		*f_descr;	/* descriptive name for font	*/
	char		*f_PSname;	/* PostScript font name		*/
	int		f_flags;	/* flag bits			*/
	int		f_fonttype;	/* font type			*/
	int		f_orientation;	/* portrait=0, landscape=1	*/
	int		f_spacing;	/* fixed=0, variable=1		*/
	int		f_symset;	/* symbol set			*/
	int		f_pitch;	/* pitch			*/
	int		f_psz;		/* point size (internal units)	*/
	int		f_style;	/* upright=0, italic=1		*/
	int		f_weight;	/* stroke weight		*/
	int		f_face;		/* typeface			*/
	int		f_num;		/* width table numerator	*/
	int		f_den;		/* width table denominator	*/
	unsigned char	f_width[NWIDTH]; /* width table			*/
} FWTAB;

/* FWTAB f_flags bits */
#define	F_USED		1		/* Font has been used		*/
#define	F_FIXED		2		/* Font has fixed pointsize	*/

/*
 * Indices for built-in fonts.
 * These correspond to the order of entries in fwptab[].
 * NDFONTS above gives the number of built-in fonts.
 */
#define	TRMED		0		/* Times-Roman medium upright	*/
#define	TRITL		1		/* Times-Roman medium italic	*/
#define	TRBLD		2		/* Times-Roman bold upright	*/

/* Globals in fonts.c. */
extern	FTB	fontab[];		/* Font table			*/
extern	FWTAB	fwtab[];		/* Builtin font width tables	*/
extern	FWTAB	*fwptab[];		/* Font width table pointers	*/
extern	int	nfonts;			/* Number of fonts		*/

/* end of fonts.h */
