/*
 * env.h
 * Nroff/Troff.
 * Environment header file.
 */

/* Buffer sizes. */
#define	ENVSIZE	3			/* Number of environments	*/
#define	EVSSIZE	20			/* Pushdown stack for environments */
#define	LINSIZE	300			/* Size of line buffer		*/
#define	NFONTS	50			/* Max number of fonts		*/
#define	TABSIZE	20			/* Maximum number of tab stops	*/
#define	WORSIZE	200			/* Size of word buffer		*/

/*
 * Tab stop entry.
 */
typedef	struct {
	int	t_pos;			/* Tab position			*/
	int	t_jus;			/* Type of justification	*/
} TAB;

/*
 * Environments.
 */
typedef	struct {
	int	e_adj;			/* Adjust justification type	*/
	int	e_adm;			/* Adjust mode			*/
	int	e_basline;		/* Height above baseline	*/
	char	e_ccc;			/* Current control character	*/
	int	e_cec;			/* Count for center command	*/
	int	e_csz;			/* Constant character size	*/
	TAB	*e_ctabptr;		/* Current tab pointer		*/
	int	e_enbldn;		/* Enbolden by this amount	*/
	int	e_fcsz[NFONTS];		/* Font character size		*/
	int	e_fil;			/* Fill mode			*/
	char	e_fon[2];		/* Font name			*/
	int	e_fontype;		/* Current font			*/
	unsigned char	*e_fonwidt;	/* Current font width table	*/
	int	e_fpsz[NFONTS];		/* Font pointsizes		*/
	char	e_hic;			/* Hyphenation indicator character */
	int	e_hyp;			/* Hyphenation mode		*/
	int	e_hypf;			/* Hyphenation of last word flag */
	int	e_ind;			/* Indent			*/
	int	e_inpltrc;		/* Input line trap count	*/
	char	e_inptrap[2];		/* Input line trap		*/
	char	e_ldc;			/* Leader character		*/
	int	e_ldf;			/* Leader dot font???		*/
	int	e_ldrspc;		/* leader spacing		*/
	int	e_lgm;			/* Ligature mode		*/
	CODE	e_linebuf[LINSIZE];	/* Line buffer			*/
	int	e_llindir;		/* Last directive count		*/
	CODE	*e_llinptr;		/* Last line pointer		*/
	int	e_llinsiz;		/* Last line size		*/
	int	e_lln;			/* Line length			*/
	int	e_lmn;			/* Line number multiple		*/
	int	e_lni;			/* Line number indent		*/
	int	e_lnn;			/* Current line number		*/
	int	e_lns;			/* Line number seperation	*/
	int	e_lsp;			/* Line spacing			*/
	char	e_ltabchr;		/* Last tab character		*/
	int	e_mar;			/* Margin seperation		*/
	char	e_mrc;			/* Margin character		*/
	char	e_mrch;			/* Margin character here	*/
	int	e_mws;			/* Minimum word spacing		*/
	char	e_nbc;			/* No break character		*/
	int	e_nlindir;		/* New directive count		*/
	CODE	*e_nlinptr;		/* New line pointer		*/
	int	e_nlinsiz;		/* New line size		*/
	int	e_nnc;			/* Count for no number command	*/
	char	e_oldfon[2];		/* Last value of fon		*/
	int	e_oldind;		/* Last value of ind		*/
	int	e_oldldf;		/* Old leader dot font???	*/
	int	e_oldlln;		/* Last value of lln		*/
	int	e_oldlsp;		/* Last value of lsp		*/
	int	e_oldmar;		/* Last value of mar		*/
	int	e_oldmws;		/* Last value of mws		*/
	int	e_oldpsz;		/* Last value of psz		*/
	int	e_oldrspc;		/* old leader spacing		*/
	int	e_oldtln;		/* Last value of tln		*/
	int	e_oldvls;		/* Last value of vls		*/
	int	e_posexls;		/* Post extra line space	*/
	int	e_preexls;		/* Pre extra line space		*/
	int	e_psz;			/* Pointsize			*/
	int	e_spcnt;		/* Pre tab space		*/
	long	e_swddiv;		/* width table divider		*/
	long	e_swdmul;		/* width table multiplier	*/
	TAB	e_tab[TABSIZE];		/* Table of tabs		*/
	char	e_tbc;			/* Tab character		*/
	int	e_tif;			/* Temporary indent flag	*/
	int	e_tin;			/* Temporary indent		*/
	CODE	*e_tlinptr;		/* Tab line pointer		*/
	int	e_tlinsiz;		/* Tab line size		*/
	int	e_tln;			/* Title length			*/
	char	e_tpc;			/* Page character in title	*/
	int	e_ufn;			/* Underline font number	*/
	char	e_ufp[2];		/* Previous font (from underline) */
	char	e_uft[2];		/* Underline font		*/
	int	e_ulc;			/* Count for underline command	*/
	int	e_vls;			/* Vertical line spacing	*/
	CODE	e_wordbuf[WORSIZE];	/* Word buffer			*/
} ENV;

/*
 * Variables in current environment.
 */
#define	adj	env.e_adj		/* Adjust justification type	*/
#define	adm	env.e_adm		/* Adjust mode			*/
#define	basline	env.e_basline		/* Height above baseline	*/
#define	ccc	env.e_ccc		/* Current control character	*/
#define	cec	env.e_cec		/* Count for center command	*/
#define	csz	env.e_csz		/* Constant character size	*/
#define	ctabptr	env.e_ctabptr		/* Current tab pointer		*/
#define	enbldn	env.e_enbldn		/* Enbolden by this amount	*/
#define	fcsz	env.e_fcsz		/* Font character size		*/
#define	fil	env.e_fil		/* Fill mode			*/
#define	fon	env.e_fon		/* Font name			*/
#define	fontype	env.e_fontype		/* Current font			*/
#define	fonwidt	env.e_fonwidt		/* Current font width table	*/
#define	fpsz	env.e_fpsz		/* Font pointsizes		*/
#define	hic	env.e_hic		/* Hyphenation indicator character */
#define	hyp	env.e_hyp		/* Hyphenation mode		*/
#define	hypf	env.e_hypf		/* Hyphenation of last word flag */
#define	ind	env.e_ind		/* Indent			*/
#define	inpltrc	env.e_inpltrc		/* Input line trap count	*/
#define	inptrap	env.e_inptrap		/* Input line trap		*/
#define	ldc	env.e_ldc		/* Leader character		*/
#define	ldf	env.e_ldf		/* Leader dot font???		*/
#define	ldrspc	env.e_ldrspc		/* leader dot spacing		*/
#define	lgm	env.e_lgm		/* Ligature mode		*/
#define	linebuf	env.e_linebuf		/* Line buffer			*/
#define	llindir	env.e_llindir		/* Last directive count		*/
#define	llinptr	env.e_llinptr		/* Last line pointer		*/
#define	llinsiz	env.e_llinsiz		/* Last line size		*/
#define	lln	env.e_lln		/* Line length			*/
#define	lmn	env.e_lmn		/* Line number multiple		*/
#define	lni	env.e_lni		/* Line number indent		*/
#define	lnn	env.e_lnn		/* Current line number		*/
#define	lns	env.e_lns		/* Line number seperation	*/
#define	lsp	env.e_lsp		/* Line spacing			*/
#define	ltabchr	env.e_ltabchr		/* Last tab character		*/
#define	mar	env.e_mar		/* Margin seperation		*/
#define	mrc	env.e_mrc		/* Margin character		*/
#define	mrch	env.e_mrch		/* Margin character here	*/
#define	mws	env.e_mws		/* Minimum word spacing		*/
#define	nbc	env.e_nbc		/* No break character		*/
#define	nlindir	env.e_nlindir		/* Current directive count	*/
#define	nlinptr	env.e_nlinptr		/* New line pointer		*/
#define	nlinsiz	env.e_nlinsiz		/* New line size		*/
#define	nnc	env.e_nnc		/* Count for no number command	*/
#define	oldfon	env.e_oldfon		/* Last value of fon		*/
#define	oldind	env.e_oldind		/* Last value of ind		*/
#define	oldldf	env.e_oldldf		/* Old Leader dot font???	*/
#define	oldlln	env.e_oldlln		/* Last value of lln		*/
#define	oldlsp	env.e_oldlsp		/* Last value of lsp		*/
#define	oldmar	env.e_oldmar		/* Last value of mar		*/
#define	oldmws	env.e_oldmws		/* Last value of mws		*/
#define	oldpsz	env.e_oldpsz		/* Last value of psz		*/
#define	oldrspc	env.e_oldrspc		/* old leader dot spacing	*/
#define	oldtln	env.e_oldtln		/* Last value of tln		*/
#define	oldvls	env.e_oldvls		/* Last value of vls		*/
#define	posexls	env.e_posexls		/* Post extra line space	*/
#define	preexls	env.e_preexls		/* Pre extra line space		*/
#define	psz	env.e_psz		/* Pointsize			*/
#define	spcnt	env.e_spcnt		/* Pre tab space		*/
#define	swddiv	env.e_swddiv		/* width table divider		*/
#define	swdmul	env.e_swdmul		/* width table multiplier	*/
#define	tab	env.e_tab		/* Table of tabs		*/
#define	tbc	env.e_tbc		/* Tab character		*/
#define	tif	env.e_tif		/* Temporary indent flag	*/
#define	tin	env.e_tin		/* Temporary indent		*/
#define	tlinptr	env.e_tlinptr		/* Tab line pointer		*/
#define	tlinsiz	env.e_tlinsiz		/* Tab line size		*/
#define	tln	env.e_tln		/* Title length			*/
#define	tpc	env.e_tpc		/* Page character in title	*/
#define	ufn	env.e_ufn		/* Underline font number	*/
#define	ufp	env.e_ufp		/* Previous font (from underline) */
#define	uft	env.e_uft		/* Underline font		*/
#define	ulc	env.e_ulc		/* Count for underline command	*/
#define	vls	env.e_vls		/* Vertical line spacing	*/
#define	wordbuf	env.e_wordbuf		/* Word buffer			*/

/*
 * Global variables.
 */
extern	ENV	env;			/* Current environment		*/
extern	int	envinit[ENVSIZE];	/* If initialized		*/
extern	int	envs;			/* Environment stack index	*/
extern	int	envstak[EVSSIZE];	/* Environment stack		*/

/* end of env.h */
