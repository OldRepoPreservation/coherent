/*
 * env.c
 * Nroff/Troff.
 * Environments.
 */

#include "roff.h"

/*
 * Initialize the current environment.
 */
setenvr()
{
	register int inc, i, n;

	hypf = 0;
	setline();
	preexls = 0;
	posexls = 0;
	setfont("R", 1);
	devpsze(unit(10*SMPOIN, SDPOIN));
	devvlsp(psz+10);		/* default leading is 11 on 10 */
	tab[0].t_pos = 0;
	tab[0].t_jus = LJUS;
	inc = n = unit(8*SMINCH, 10*SDINCH);
	for (i=1; i<TABSIZE-1; i++) {
		env.e_tab[i].t_pos = n;
		env.e_tab[i].t_jus = LJUS;
		n += inc;
	}
	tab[TABSIZE-1].t_pos = 0;
	tab[TABSIZE-1].t_jus = NJUS;
	lln = (lflag) ? unit(9*SMINCH, SDINCH) : unit(13*SMINCH, 2*SDINCH);
	ind = 0;
	tin = 0;
	tif = 0;
	fil = 1;
	adm = 1;
	adj = FJUS;
	cec = 0;
	ulc = 0;
	uft[0] = 'I';
	uft[1] = '\0';
	ufn = font_number(uft, NULL);
	mws = (ntroff == NROFF) ? unit(SMENSP, SDENSP) : unit(SMEMSP * 12, SDEMSP * 36);
	lsp = 1;
	hyp = 1;
	tln = lln;
	mar = mws;
	csz = 0;
	lgm = 0;
	lnn = 0;
	lmn = 0;
	lns = 0;
	lni = 0;
	nnc = 0;
	inpltrc = 0;
	inptrap[0] = '\0';
	oldfon[0] = fon[0];
	oldfon[1] = fon[1];
	oldpsz = psz;
	oldvls = vls;
	oldlln = lln;
	oldind = ind;
	oldmws = mws;
	oldlsp = lsp;
	oldtln = tln;
	oldmar = mar;
	ccc = '.';
	nbc = '\'';
	tbc = '\0';
	ldc = '.';
	ldrspc = 0;
	oldrspc = ldrspc;
	hic = EHYP;
	tpc = '%';
	mrc = '\0';
	mrch = '\0';
}

/* end of env.c */
