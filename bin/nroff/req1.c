/*
 * Nroff/Troff.
 * Requests (a-m).
 */
#include <stdio.h>
#include <ctype.h>
#include "roff.h"
#include "code.h"
#include "div.h"
#include "env.h"
#include "esc.h"
#include "reg.h"
#include "str.h"
#include "codebug.h"

/*
 * User abort.
 */
req_ab()
{
	register char *bp;

	bp = nextarg(miscbuf, NULL, 0);
	if (*bp != '\0')
		fprintf(stderr, "%s\n", bp);
	else
		fprintf(stderr, "User Abort\n");
	leave(1);
}

/*
 * Turn adjust mode on and set adjust type.
 */
req_ad(argc, argv)
char *argv[];
{
	adm = 1;
	if (argc < 2)
		return;
	switch (argv[1][0]) {
	case 'l':
		adj = LJUS;
		return;
	case 'c':
		adj = CJUS;
		return;
	case 'r':
		adj = RJUS;
		return;
	case 'b':
	case 'u':
		adj = FJUS;
		return;
	default:
		printe("Bad adjustment type");
		return;
	}
}

/*
 * Assign format.
 */
req_af(argc, argv)
char *argv[];
{
	register REG *rp;
	int n;
	char name[2];
	register char *p, c;

	argname(argv[1], name);
	rp = getnreg(name);
	if (index("iIaA", c=argv[2][0])) {
		rp->r_form = c;
		return;
	}
	if (isascii(c) && isdigit(c)) {
		n = '0';
		p = &argv[2][1];
		while (isascii(c=*p++) && isdigit(*p))
			;
		if (p-argv[2] > 9) {
			printe("Field with too large");
			return;
		}
		rp->r_form = '0' + p-argv[2];
	}
}

/*
 * Append to macro.
 */
req_am(argc, argv)
char *argv[];
{
	register REG *rp;
	register MAC *mp;
	char name[2];

	argname(argv[1], name);
	if ((rp=findreg(name, RTEXT)) == NULL) {
		rp = makereg(name, RTEXT);
		mp = &rp->r_macd;
	} else {
		for (mp=&rp->r_macd; mp->m_next; mp=mp->m_next)
			;
		mp->m_next = nalloc(sizeof (*mp));
		mp = mp->m_next;
	}
	deftext(mp, argv[2]);
}

/*
 * Append to string.
 */
req_as(argc, argv)
char *argv[];
{
	register REG *rp;
	register MAC *mp;
	char name[2];
	register char *cp;

	argname(argv[1], name);
	if ((rp=findreg(name, RTEXT)) == NULL) {
		rp = makereg(name, RTEXT);
		mp = &rp->r_macd;
	} else {
		for (mp=&rp->r_macd; mp->m_next; mp=mp->m_next)
			;
		mp->m_next = nalloc(sizeof *mp);
		mp = mp->m_next;
	}
	cp = nalloc(strlen(argv[2]) + 1);
	strcpy(cp, argv[2]);
	mp->m_next = NULL;
	mp->m_type = MTEXT;
	mp->m_core = cp;
}

/*
 * Begin page.
 */
req_bp(argc, argv)
char *argv[];
{
	setbreak();
	if (argc >= 2) {
		npn = number(argv[1], SMUNIT, SDUNIT, 0, 0, 0);
		pspace();
		return;
	}
	if (nsm == 0)
		pspace();
}

/*
 * Break.
 */
req_br()
{
	setbreak();
}

/*
 * Set nobreak control character.
 */
req_c2(argc, argv)
char *argv[];
{
	nbc = argc>1 ? argv[1][0] : '\'';
}

/*
 * Set control character.
 */
req_cc(argc, argv)
char *argv[];
{
	ccc = argc>1 ? argv[1][0] : '.';
}

/*
 * Centre all text.
 */
req_ce(argc, argv)
char *argv[];
{
	setbreak();
	cec = number(argv[1], SMUNIT, SDUNIT, 0, 0, 1);
}

/*
 * Change trap location.
 */
req_ch(argc, argv)
char *argv[];
{
	register TPL **tpp, *tp;
	register DIV *dp;
	int rpos, apos;
	char name[2];

	dp = mdivp;
	argname(argv[1], name);
	for (tpp=&dp->d_stpl; tp=*tpp; tpp=&tp->t_next) {
		if (name[0]==tp->t_name[0] && name[1]==tp->t_name[1]) {
			if (dp->d_trap == tp)
				dp->d_trap = tp->t_next;
			if (dp->d_ctpp == tp)
				dp->d_ctpp = tp->t_next;
			*tpp = tp->t_next;
			nfree(tp);
			break;
		}
	}
	if (argc >= 3) {
		rpos = number(argv[2], SMVLSP, SDVLSP, 0, 0, 0);
		apos = rpos>=0 ? rpos : pgl+rpos;
		for (tpp=&dp->d_stpl; tp=*tpp; tpp=&tp->t_next) {
			if (apos <= tp->t_apos)
				break;
		}
		tp = nalloc(sizeof (TPL));
		tp->t_rpos = rpos;
		tp->t_apos = apos;
		tp->t_name[0] = name[0];
		tp->t_name[1] = name[1];
		tp->t_next = *tpp;
		*tpp = tp;
		if (dp->d_trap==tp->t_next && apos>=0)
			dp->d_trap = tp;
		if (dp->d_ctpp==tp->t_next && apos>=dp->d_rpos)
			dp->d_ctpp = tp;
	}
}

/*
 * Set constant character space mode.
 * Note that the second argument (font) is ignored.
 */
req_cs(argc, argv)
{
	printe(".cs not implimented yet");
/*
	register int ems;

	ems = number(argv[3], SMPOIN, SDPOIN, 0, 0, unit(SMEMSP, SDEMSP));
	csz = number(argv[2], (long)ems, (long)1, 0, 0, 0);
*/
}

/*
 * Continous underline.
 */
req_cu(argc, argv)
char *argv[];
{
	ulc = INFINITY;
}

/*
 * Divert and append output to macro.
 */
req_da(argc, argv)
char *argv[];
{
	register REG *rp;
	register MAC *mp;
	char name[2];

	if (argc < 2) {
		enddivn();
		return;
	}
	argname(argv[1], name);
	newdivn(name);
	if ((rp=findreg(name, RTEXT)) == NULL) {
		rp = makereg(name, RTEXT);
		cdivp->d_seek = tmpseek;
		mp = &rp->r_macd;
		mp->m_next = NULL;
	} else {
		cdivp->d_maxh = rp->r_maxh;
		cdivp->d_maxw = rp->r_maxw;
		for (mp=&rp->r_macd; mp->m_next; mp=mp->m_next)
			;
	}
	mp->m_type = MDIVN;
	mp->m_size = 0;
	mp->m_core = NULL;
	mp->m_seek = tmpseek;
	cdivp->d_macp = mp;
}

/*
 * Define a special character.
 * Added by steve 4/16/91.
req_de(argc, argv)
char *argv[];
req_dc(argc, argv) int argc; char *argv[];
{
	char name[2];

	argname(argv[1], name);
	spc_def(name, (argc < 3) ? "" : argv[2]);
	deftext(&rp->r_macd, argv[2]);

/*
 * Define a macro.
 */
req_de(argc, argv) int argc; char *argv[];
req_di(argc, argv)
char *argv[];
	register REG *rp;
	char name[2];

	argname(argv[1], name);
	rp = makereg(name, RTEXT);
	deftext(&rp->t_reg.r_macd, argv[2]);
}

/*
 * Divert output to macro.
 */
	cdivp->d_macp = &rp->r_macd;
{
	rp->r_macd.m_next = NULL;
	rp->r_macd.m_type = MDIVN;
	rp->r_macd.m_size = 0;
	rp->r_macd.m_core = NULL;
	rp->r_macd.m_seek = tmpseek;
		return;
	}
	argname(argv[1], name);
	newdivn(name);
	rp = makereg(name, RTEXT);
req_ds(argc, argv)
char *argv[];
	cdivp->d_seek = tmpseek;
	rp->t_reg.r_macd.t_div.m_next = NULL;
	rp->t_reg.r_macd.t_div.m_type = MDIVN;
	rp->t_reg.r_macd.t_div.m_size = 0;
	rp->t_reg.r_macd.t_div.m_core = NULL;
	rp->t_reg.r_macd.t_div.m_seek = tmpseek;
}

/*
	rp->r_macd.m_next = NULL;
	rp->r_macd.m_type = MTEXT;
	rp->r_macd.m_core = cp;
{
	register REG *rp;
	char name[2];
 * Set a diversion trap.	(.dt)	$$TO_DO$$

req_dt(argc, argv)
char *argv[];
	rp = makereg(name, RTEXT);
	cp = nalloc(strlen(argv[2]) + 1);
	strcpy(cp, argv[2]);
	rp->t_reg.r_macd.t_div.m_next = NULL;
	rp->t_reg.r_macd.t_div.m_type = MTEXT;
	rp->t_reg.r_macd.t_div.m_core = cp;
}
req_ec(argc, argv)
char *argv[];
/*
 * Set a diversion trap.			(.dt)	$$TO_DO$$
 */
req_dt(argc, argv) int argc; char *argv[];
{
	printu(".dt");
}
req_el(argc, argv)
char *argv[];
/*
	char charbuf[CBFSIZE], c;
	register char *bp, *cp;

	bp = nextarg(miscbuf, NULL, 0);
	if (!lastcon) {
		cp = charbuf;
		if (*bp == EBEG)
			bp++;
		while (c=*bp++) {
			if (cp < &charbuf[CBFSIZE-2])
				*cp++ = c;
		}
		*cp++ = '\n';
		*cp++ = '\0';
		cp = duplstr(charbuf);
		adscore(cp);
		strp->s_srel = cp;
	} else {
		if (*bp == EBEG) {
			ifeflag = 1;
			while (getf(0) != EEND)
				;
			ifeflag = 0;
			while (getf(0) != '\n')
				;
		}
{
}

/*
 * Else part of if-else.
 */
req_em(argc, argv)
char *argv[];
{
	if (iestackx < 0) {
		printe(".el without .ie");
		return;
	}
	do_cond(!iestack[iestackx--], nextarg(miscbuf, NULL, 0));
}

/*
 * Set end macro.
 */
req_em(argc, argv) int argc; char *argv[];
{
 * Change enviroments.
}
req_ev(argc, argv)
char *argv[];
/*
 * Turn off escape mechanism.
 */
req_eo()
{
	esc = '\0';
			printe("Cannot pop enviroment");

/*
 * Change environments.
 */
req_ev(argc, argv) int argc; char *argv[];
		new = number(argv[1], SMUNIT, SDUNIT, 0, 0, 0);
	register int old, new;
		if (new<0 || new>=ENVSIZE) {
			printe("Enviroment does not exist");
		dprintd(DBGENVR, "pop environment\n");
		if (envs == 0) {
		if (envs >= EVSSIZE) {
			printe("Enviroments stacked too deeply");
		}
		old = envstak[envs];
		new = envstak[--envs];
	} else {
		new = numb(argv[1], SMUNIT, SDUNIT);
		dprint2(DBGENVR, "push environment %d\n", new);
	lseek(fileno(tmp), (long) old * sizeof (ENV), 0);
	if (write(fileno(tmp), &env, sizeof (env)) != sizeof (env))
		panic("Cannot write enviroment");
			printe("environment does not exist");
			return;
		}
		if (envs >= ENVSTACK) {
			printe("environments stacked too deeply");
			return;
		lseek(fileno(tmp), (long) new * sizeof (ENV), 0);
		if (read(fileno(tmp), &env, sizeof (env)) != sizeof (env))
			panic("Cannot read enviroment");
		addidir(DFONT, fontype);
		addidir(DPSZE, psz);
		old = envstak[envs];
		envstak[++envs] = new;
	}
	dprint2(DBGENVR|DBGFILE, "writing environment %d\n", old);
	envsave(old);
	if (envinit[new] == 0) {
		dprint2(DBGENVR, "initializing env %d\n", new);
		envset();
		envinit[new] = 1;
		setfont("R", 1);
	} else {
		dprint2(DBGENVR|DBGFILE, "reading environment %d\n", new);
	printu(".fc");
}

/*
 * Display a list of fonts to standard error.
	fil = 1;
 */
req_fd()
{
	font_display();
}

/*
 * Turn on fill mode.
 */
req_fi()
{
 * Define font at position
	fill = 1;
req_fp(argc, argv)
char *argv[];

	register n;
 * Flush.
 */
/*
	if ((n <= 8) && (n >= 1))
		mapfont[n] = argv[2][0];
	else
		printe("Font position out of range");
 */
}
req_fp(argc, argv) int argc; char *argv[];
{
	register int n;
req_ft(argc, argv)
char *argv[];
	n = argv[1][0] - '0';
	if ((1 <= n) && (n <= 9)) {
		mapfont[n][0] = argv[2][0];
		mapfont[n][1] = argv[2][1];
	} else
		printe("font position out of range");

}
{
	printu(".hw");
req_ie(argc, argv)
char *argv[];

	lastcon = req_if(argc, argv);
{
	printu(".hy");
}

 * This returns the condition and is called from `req_ie'.
 * If part of if-else.
 */
req_ie(argc, argv) int argc; char *argv[];
{
	char charbuf[CBFSIZE], endc;
		printe(".ie nested more than %d levels", IESTACKSIZE);
	register unsigned char *bp;
	register char *cp;
		iestack[++iestackx] = req_if(argc, argv);
	bp = nextarg(miscbuf, NULL, 0);
	not = 0;
 * If (conditional execution of command).
 * This returns the condition and is called from 'req_ie'.
 */
	}
	register int c;
	case 'e':	/* Current page number is even. */

	bp = (unsigned char *)nextarg(miscbuf, NULL, 0);
	case 'n':	/* Formatter is Nroff. */
	/* Look for leading '!'. */
	if (*bp == '!') {
	case 'o':	/* Current page number is odd. */
		not = 1;
	} else
	case 't':	/* Formatter is Troff. */

	/* Look for built-ins. */
	switch (*bp++) {
		--bp;
		if (isascii(*bp) && isdigit(*bp)) {
			bp = nextarg(bp, charbuf, CBFSIZE);
			con = number(charbuf, SMUNIT, SDUNIT, 0, 0, 0) > 0;
		break;
	case 'n':			/* Formatter is Nroff. */
		if ((endc=*bp) == '\0') {
			con = 1;
		break;
		}
		bp++;
		cp = charbuf;
		while ((c=*bp++) != endc) {
			if (c == '\0') {
				--bp;
				break;
			}
			if (cp < &charbuf[CBFSIZE-1])
				*cp++ = c;
		}
		*cp++ = '\0';
		cp = charbuf;
		con = 1;
		while ((c=*bp++) != endc) {
			if (c == '\0') {
				--bp;
				break;
			}
			if (c == *cp)
				cp++;
			else
				con = 0;
		}
			bp = (unsigned char *)nextarg(bp, charbuf, CBFSIZE);
	while (isascii(*bp) && isspace(*bp))
		bp++;
			con = numb(charbuf, SMUNIT, SDUNIT) != 0;
			break;
	if (con) {
		cp = charbuf;
		if (*bp == EBEG)
			bp++;
		while (c=*bp++) {
			if (cp < &charbuf[CBFSIZE-2])
				*cp++ = c;
		}
		*cp++ = '\n';
		*cp++ = '\0';
		cp = duplstr(charbuf);
		adscore(cp);
		strp->s_srel = cp;
	} else {
		if (*bp++ == EBEG) {
			while (*bp != '\0') if (*bp++ == EEND)
				return (con);
			ifeflag = 1;
			while (getf(0) != EEND)
				;
			ifeflag = 0;
			while (getf(0) != '\n')
				;
		}
	}
	return (con);
		/* String comparison. */
		con = 0;
		if (c == '\0')
			break;
		cp1 = ++bp;			/* start of first string */
req_ig(argc, argv)
char *argv[];
			break;
		cp2 = ++cp;			/* start of second string */
		if ((cp = index(cp2, c)) == NULL)
			break;
		bp = cp + 1;			/* bp points after third c */
		if (cp - cp2 != (cp2-1) - cp1)
			break;			/* lengths differ, unequal */
req_in(argc, argv)
char *argv[];
	}
	register int n;

	n = ind;
	ind = number(argv[1], SMEMSP, SDEMSP, ind, 0, oldind);
	oldind = n;
		con = !con;
	do_cond(con, bp);
	return con;
}

/*
 * Ignore.
req_it(argc, argv)
char *argv[];
req_ig(argc, argv) int argc; char *argv[];
{
	deftext(NULL, argv[1]);
}

	inpltrc = number(argv[1], SMUNIT, SDUNIT, 0, 0, 0);
 * Set indent.
 */
req_in(argc, argv) int argc; char *argv[];
{
{
	ldc = (argc < 1) ? '\0' : argv[1][0];
req_lg(argc, argv)
char *argv[];

	lgm = number(argv[1], SMUNIT, SDUNIT, 0, 0, 0) > 0;
 * Load a font width table.
 * !V7.
 * Added by steve 12/12/90.
 */
req_lf(argc, argv) int argc; char *argv[];
req_ll(argc, argv)
char *argv[];
	if (argc != 3) {
	register int n;
	}
	n = lln;
	lln = number(argv[1], SMEMSP, SDEMSP, lln, 0, oldlln);
	oldlln = n;
}

	load_font(argv[1], argv[2]);
}

req_ls(argc, argv)
char *argv[];
 * Set ligature mode.
	register int n;
{
	n = lsp;
	lsp = number(argv[1], SMUNIT, SDUNIT, lsp, 0, oldlsp);
	oldlsp = n;
}

	lgm = numb(argv[1], SMUNIT, SDUNIT) > 0;
}

req_lt(argc, argv)
char *argv[];
 * Set line length.
	register int n;
{
	n = tln;
	tln = number(argv[1], SMEMSP, SDEMSP, tln, 0, oldtln);
	oldtln = n;
}

	setval(&lln, &oldlln, argv[1], SMEMSP, SDEMSP);
}

req_mc(argc, argv)
char *argv[];
 * Set line spacing.
	mrc = argv[1][0];
	mar = number(argv[2], SMEMSP, SDEMSP, mar, 0, 0);
	setval(&tln, &oldtln, argv[1], SMEMSP, SDEMSP);
}

/*
 * Set margin character.
req_mk(argc, argv)
char *argv[];
req_mc(argc, argv) int argc; char *argv[];
{
	if (argc < 2) {
		mrc = '\0';
	} else {
		mrc = argv[1][0];
#ifdef	mfn
		/* Margin font number (if required later...) */
		mfn = curfont;
#endif
	}
		rp->r_incr = 1;
		rp->r_form = '1';
}
	rp->r_nval = cdivp->d_rpos;
/*
req_mk(argc, argv) int argc; char *argv[];
{
	register REG *rp;
	char name[2];

	if (argc == 1) {
		cdivp->d_mk = cdivp->d_rpos;
		return;
	}
	argname(argv[1], name);
	if ((rp=findreg(name, RNUMR)) == NULL) {
		rp = makereg(name, RNUMR);
		rp->n_reg.r_incr = 1;
		rp->n_reg.r_form = '1';
	}
	rp->n_reg.r_nval = cdivp->d_rpos;
}

/* end of req1.c */
