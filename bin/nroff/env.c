/*
 * env.c
 * Nroff/Troff.
 * Environments.
 */

#include "roff.h"

/*
 * Initialize the current environment.
 * The new environment inherits the values of all
 * environmental variables which are not initialized here:
 * this includes fcsz, fpsz, tbf, ufn, ufp.
 */
setenvr()
{
	register int inc, i, n;

	/* Set output line, default font, pointsize, vertical spacing. */
	tln = lln = (lflag) ? unit(9*SMINCH, SDINCH) : unit(13*SMINCH, 2*SDINCH);
	setline();
	setfont("R", 1);
	devpsze(unit(10*SMPOIN, SDPOIN));
	devvlsp(psz+10);		/* default leading is 11 on 10 */

	/* Set default tab stops. */
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

	/* Set other variables. */
	adj = FJUS;
	adm = 1;
	ccc = '.';
	cec = 0;
	csz = 0;
	enb = 0;
	fill = 1;
	hic = EHYP;
	hyp = 1;
	hypf = 0;
	ind = 0;
	inpltrc = 0;
	inptrap[0] = '\0';
	ldc = '.';
	lgm = 0;
	lmn = 0;
	lni = 0;
	lnn = 0;
	lns = 0;
	lsp = 1;
	mar = ssz = (ntroff == NROFF) ? unit(SMENSP, SDENSP)
				      : unit(SMEMSP * 12, SDEMSP * 36);
	mrc = '\0';
	mrch = '\0';
	nbc = '\'';
	nnc = 0;
	spcnt = 0;
	tbc = '\0';
	tbs = 0;
	tif = 0;
	tin = 0;
	tpc = '%';
	ulc = 0;
	oldfon[0] = fon[0];
	oldfon[1] = fon[1];
	oldind = ind;
	oldlln = lln;
	oldlsp = lsp;
	oldmar = mar;
	oldssz = ssz;
	oldpsz = psz;
	oldtln = tln;
	oldvls = vls;
}

/*
 * Save environment n.
 */
envsave(n) int n;
{
	lseek(fileno(tmp), (long) n * sizeof (ENV), 0);
	if (write(fileno(tmp), &env, sizeof (env)) != sizeof (env))
		panic("cannot write environment");
}

/*
 * Restore environment n.
 * Bug: in troff, restoring a saved environment does not set
 * fpsz[n] for a font n loaded in the new environment.
 */
envload(n) int n;
{
	lseek(fileno(tmp), (long) n * sizeof (ENV), 0);
	if (read(fileno(tmp), &env, sizeof (env)) != sizeof (env))
		panic("cannot read environment");
	devfont(fontype);
}

/* end of env.c */
