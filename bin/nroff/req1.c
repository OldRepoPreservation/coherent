/*
 * req1.c
 * Nroff/Troff.
 * Requests (a-m).
 */

#include <ctype.h>
#include "roff.h"

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
		fprintf(stderr, "User abort\n");
	leave(1);
}

/*
 * Turn adjust mode on and set adjust type.
 */
req_ad(argc, argv) int argc; char *argv[];
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
		printe("bad adjustment type");
		return;
	}
}

/*
 * Assign format.
 */
req_af(argc, argv) int argc; char *argv[];
{
	register REG *rp;
	int n;
	char name[2];
	register char *p, c;

	argname(argv[1], name);
	rp = getnreg(name);
	if (index("iIaA", c=argv[2][0])) {
		rp->n_reg.r_form = c;
		return;
	}
	if (isascii(c) && isdigit(c)) {
		n = '0';
		p = &argv[2][1];
		while (isascii(c = *p++) && isdigit(*p))
			;
		if (p-argv[2] > 9) {
			printe("field with too large");
			return;
		}
		rp->n_reg.r_form = '0' + p-argv[2];
	}
}

/*
 * Append to macro.
 */
req_am(argc, argv) int argc; char *argv[];
{
	register REG *rp;
	register MAC *mp;
	char name[2];

	argname(argv[1], name);
	if ((rp=findreg(name, RTEXT)) == NULL) {
		rp = makereg(name, RTEXT);
		mp = &rp->t_reg.r_macd;
	} else {
		for (mp = &rp->t_reg.r_macd;
		     mp->t_div.m_next;
		     mp=mp->t_div.m_next)
			;
		mp->t_div.m_next = (MAC *)nalloc(sizeof (*mp));
		mp = mp->t_div.m_next;
	}
	deftext(mp, argv[2]);
}

/*
 * Append to string.
 */
req_as(argc, argv) int argc; char *argv[];
{
	register REG *rp;
	register MAC *mp;
	char name[2];
	register char *cp;

	argname(argv[1], name);
	if ((rp=findreg(name, RTEXT)) == NULL) {
		rp = makereg(name, RTEXT);
		mp = &rp->t_reg.r_macd;
	} else {
		for (mp = &rp->t_reg.r_macd; mp->t_div.m_next; mp=mp->t_div.m_next)
			;
		mp->t_div.m_next = (MAC *)nalloc(sizeof *mp);
		mp = mp->t_div.m_next;
	}
	cp = nalloc(strlen(argv[2]) + 1);
	strcpy(cp, argv[2]);
	mp->t_div.m_next = NULL;
	mp->t_div.m_type = MTEXT;
	mp->t_div.m_core = cp;
}

/*
 * Embolden.					(.bd)	$$TO_DO$$
 */
req_bd(argc, argv) int argc; char *argv[];
{
	printu(".bd");
}

/*
 * Overstrike bold command.
 * !V7.
 */
req_bo(argc, argv) int argc; char *argv[];
{
	if (argc == 1)
		enbldn++;
	else {
		enbldn = atoi(argv[1]);
	}
}

/*
 * Begin page.
 */
req_bp(argc, argv) int argc; char *argv[];
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
req_c2(argc, argv) int argc; char *argv[];
{
	nbc = argc>1 ? argv[1][0] : '\'';
}

/*
 * Set control character.
 */
req_cc(argc, argv) int argc; char *argv[];
{
	ccc = argc>1 ? argv[1][0] : '.';
}

/*
 * Centre all text.
 */
req_ce(argc, argv) int argc; char *argv[];
{
	setbreak();
	cec = number(argv[1], SMUNIT, SDUNIT, 0, 0, 1);
}

/*
 * Change trap location.
 */
req_ch(argc, argv) int argc; char *argv[];
{
	register TPL **tpp, *tp;
	register DIV *dp;
	int rpos, apos;
	char name[2];

	dp = mdivp;
	argname(argv[1], name);
	for (tpp = &dp->d_stpl; tp = *tpp; tpp = &tp->t_next) {
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
		for (tpp = &dp->d_stpl; tp = *tpp; tpp = &tp->t_next) {
			if (apos <= tp->t_apos)
				break;
		}
		tp = (TPL *)nalloc(sizeof (TPL));
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
 * Copy input verbatim to output.
 * !V7.
 * Added by steve 12/21/90.
 */
req_co(argc, argv) int argc; char *argv[];
{
	char endmark[MSCSIZE];
	register char *cp;
	register int c;
	char *cp1;

	strcpy(endmark, (argc == 1) ? ".co" : argv[1]);
	cp = endmark;
	while ((c = getl(1)) != EOF) {
		if (c == *cp) {			/* match next endmark char */
			cp++;
			continue;
		} else if (c == '\n' && *cp == '\0')
			return;			/* matched endmark, done */
		for (cp1 = endmark; cp1 < cp; )
			putchar(*cp1++);	/* copy matched portion */
		putchar(c);			/* copy this character */
		cp = endmark;			/* try again from start */
	}
	printe(".co: unexpected EOF before %s", endmark);
}

/*
 * Set constant character space mode.
 */
req_cs(argc, argv) int argc; char *argv[];
{
	char name[2];
	register int n, ems;

	argname(argv[1], name);
	if ((n = font_number(name, ".fz: ")) < 0)
		return;
	ems = number(argv[3], SMPOIN, SDPOIN, 0, 0, unit(SMEMSP, SDEMSP));
	dev_cs(n, number(argv[2], (long)ems, 36L, 0, 0, 0));
}

/*
 * Continous underline.
 */
req_cu(argc, argv) int argc; char *argv[];
{
	ulc = INFINITY;
}

/*
 * Divert and append output to macro.
 */
req_da(argc, argv) int argc; char *argv[];
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
		mp = &rp->t_reg.r_macd;
		mp->t_div.m_next = NULL;
	} else {
		cdivp->d_maxh = rp->t_reg.r_maxh;
		cdivp->d_maxw = rp->t_reg.r_maxw;
		for (mp = &rp->t_reg.r_macd; mp->t_div.m_next; mp=mp->t_div.m_next)
			;
	}
	mp->t_div.m_type = MDIVN;
	mp->t_div.m_size = 0;
	mp->t_div.m_core = NULL;
	mp->t_div.m_seek = tmpseek;
	cdivp->d_macp = mp;
}

/*
 * Define a special character.
 * Added by steve 4/16/91.
 */
req_dc(argc, argv) int argc; char *argv[];
{
	char name[2];

	argname(argv[1], name);
	spc_def(name, (argc < 3) ? "" : argv[2]);
}

/*
 * Define a macro.
 */
req_de(argc, argv) int argc; char *argv[];
{
	register REG *rp;
	char name[2];

	argname(argv[1], name);
	rp = makereg(name, RTEXT);
	deftext(&rp->t_reg.r_macd, argv[2]);
}

/*
 * Divert output to macro.
 */
req_di(argc, argv) int argc; char *argv[];
{
	register REG *rp;
	char name[2];

	if (argc < 2) {
		enddivn();
		return;
	}
	argname(argv[1], name);
	newdivn(name);
	rp = makereg(name, RTEXT);
	cdivp->d_macp = &rp->t_reg.r_macd;
	cdivp->d_seek = tmpseek;
	rp->t_reg.r_macd.t_div.m_next = NULL;
	rp->t_reg.r_macd.t_div.m_type = MDIVN;
	rp->t_reg.r_macd.t_div.m_size = 0;
	rp->t_reg.r_macd.t_div.m_core = NULL;
	rp->t_reg.r_macd.t_div.m_seek = tmpseek;
}

/*
 * Define a string.
 */
req_ds(argc, argv) int argc; char *argv[];
{
	register REG *rp;
	char name[2];
	register char *cp;

	argname(argv[1], name);
	rp = makereg(name, RTEXT);
	cp = nalloc(strlen(argv[2]) + 1);
	strcpy(cp, argv[2]);
	rp->t_reg.r_macd.t_div.m_next = NULL;
	rp->t_reg.r_macd.t_div.m_type = MTEXT;
	rp->t_reg.r_macd.t_div.m_core = cp;
}

/*
 * Set a diversion trap.			(.dt)	$$TO_DO$$
 */
req_dt(argc, argv) int argc; char *argv[];
{
	printu(".dt");
}

/*
	char charbuf[CBFSIZE], c;
	register char *bp, *cp;

	bp = nextarg(miscbuf, NULL, 0);
	if (!lastcon) {
		cp = charbuf;
		if (*bp == EBEG)
			bp++;
		while (c = *bp++) {
			if (cp < &charbuf[CBFSIZE-2])
				*cp++ = c;
		}
		*cp++ = '\n';
		*cp++ = '\0';
		cp = duplstr(charbuf);
		adscore(cp);
		strp->x3.s_srel = cp;
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
req_el(argc, argv) int argc; char *argv[];
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
	argname(argv[1], endtrap);
}

/*
 * Turn off escape mechanism.
 */
req_eo()
{
	esc = '\0';
}

/*
 * Change environments.
 */
req_ev(argc, argv) int argc; char *argv[];
		new = number(argv[1], SMUNIT, SDUNIT, 0, 0, 0);
	register int old, new;
		if (new<0 || new>=ENVSIZE) {
	if (argc < 2) {
		dprintd(DBGENVR, "pop environment\n");
		if (envs == 0) {
		if (envs >= EVSSIZE) {
			return;
		}
		old = envstak[envs];
		new = envstak[--envs];
	} else {
		new = numb(argv[1], SMUNIT, SDUNIT);
		dprint2(DBGENVR, "push environment %d\n", new);
	lseek(fileno(tmp), (long) old * sizeof (ENV), 0);
	if (write(fileno(tmp), &env, sizeof (env)) != sizeof (env))
		panic("cannot write environment");
			printe("environment does not exist");
			return;
		}
		if (envs >= ENVSTACK) {
			printe("environments stacked too deeply");
			return;
		lseek(fileno(tmp), (long) new * sizeof (ENV), 0);
		if (read(fileno(tmp), &env, sizeof (env)) != sizeof (env))
			panic("cannot read environment");
		devfont(fontype);
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
		envload(new);
	}
}

/*
 * Exit from nroff.
 */
req_ex()
{
	leave(0);
}

/*
 * Flush the buffers (used for transparent flushing...)
 * !V7.
 */
req_fb()
{
	flushl(linebuf, llinptr);	/* Flush the buffer... */
	llinptr = linebuf;		/* Reset the buffer... */
}

/*
 * Set field delimiter and pad character.	(.fc)	$$TO_DO$$
 */
req_fc(argc, argv) int argc; char *argv[];
{
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
	setbreak();
	fill = 1;
}

/*
 * Flush.
 */
req_fl()
{
	setbreak();
}

/*
 * Define font at position.
 */
req_fp(argc, argv) int argc; char *argv[];
{
	register int n;

	n = argv[1][0] - '0';
	if ((1 <= n) && (n <= 9)) {
		mapfont[n][0] = argv[2][0];
		mapfont[n][1] = argv[2][1];
	} else
		printe("font position out of range");

}

/*
 * Set current font.
 */
req_ft(argc, argv) int argc; char *argv[];
{
	char name[2];

	argname(argv[1], name);
	setfont(name, 1);
}

/*
 * Force font size.
 */
req_fz(argc, argv) int argc; char *argv[];
{
	char name[2];
	register int n;

	argname(argv[1], name);
	if ((n = font_number(name, ".fz: ")) >= 0)
		dev_fz(n, argv[2]);
}

/*
 * Hyphenation mode.				(.hc)	$$TO_DO$$
 */
req_hc(argc, argv) int argc; char *argv[];
{
	printu(".hc");
}

/*
 * Hyphenation indicator character.		(.hw)	$$TO_DO$$
 */
req_hw(argc, argv) int argc; char *argv[];
{
	printu(".hw");
}

	lastcon = req_if(argc, argv);
{
	printu(".hy");
}

/*
 * If part of if-else.
 */
req_ie(argc, argv) int argc; char *argv[];
{
	char charbuf[CBFSIZE], endc;
		printe(".ie nested more than %d levels", IESTACKSIZE);
	register unsigned char *bp;
	register char *cp;
		iestack[++iestackx] = req_if(argc, argv);
}
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
	case 'l':
			con = number(charbuf, SMUNIT, SDUNIT, 0, 0, 0) > 0;
		break;
	case 'n':			/* Formatter is Nroff. */
		if ((endc = *bp) == '\0') {
			con = 1;
		break;
		}
		bp++;
		cp = charbuf;
		while ((c = *bp++) != endc) {
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
		while ((c = *bp++) != endc) {
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
		while (c = *bp++) {
			if (cp < &charbuf[CBFSIZE-2])
				*cp++ = c;
		}
		*cp++ = '\n';
		*cp++ = '\0';
		cp = duplstr(charbuf);
		adscore(cp);
		strp->x3.s_srel = cp;
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
		if ((cp = index(cp1, c)) == NULL)
			break;
		cp2 = ++cp;			/* start of second string */
		if ((cp = index(cp2, c)) == NULL)
			break;
		bp = cp + 1;			/* bp points after third c */
		if (cp - cp2 != (cp2-1) - cp1)
			break;			/* lengths differ, unequal */
		con = strncmp(cp1, cp2, cp - cp2) == 0;	/* compare strings */
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
 */
req_ig(argc, argv) int argc; char *argv[];
{
	deftext(NULL, argv[1]);
}

	inpltrc = number(argv[1], SMUNIT, SDUNIT, 0, 0, 0);
 * Set indent.
 */
req_in(argc, argv) int argc; char *argv[];
{
 * Leader repetition character.			(.lc)	$$TO_DO$$
	setbreak();
}

	printu(".lc");
/*
 * Set an input line count trap.
 */
req_it(argc, argv) int argc; char *argv[];
{
	if (argc < 3) {
		inpltrc = 0;
		return;
	}
	inpltrc = numb(argv[1], SMUNIT, SDUNIT);
	argname(argv[2], inptrap);
}

/*
 * Leader repetition character.
 */
req_lc(argc, argv) int argc; char *argv[];
{
	ldc = (argc < 1) ? '\0' : argv[1][0];
}

	lgm = number(argv[1], SMUNIT, SDUNIT, 0, 0, 0) > 0;
 * Load a font width table.
 * !V7.
 * Added by steve 12/12/90.
 */
req_lf(argc, argv) int argc; char *argv[];
{
	if (argc != 3) {
	register int n;
	}
	n = lln;
	lln = number(argv[1], SMEMSP, SDEMSP, lln, 0, oldlln);
	oldlln = n;
}

	load_font(argv[1], argv[2]);
}

/*
 * Set ligature mode.
	register int n;
{
	n = lsp;
	lsp = number(argv[1], SMUNIT, SDUNIT, lsp, 0, oldlsp);
	oldlsp = n;
}

	lgm = numb(argv[1], SMUNIT, SDUNIT) > 0;
}

/*
 * Set line length.
	register int n;
{
	n = tln;
	tln = number(argv[1], SMEMSP, SDEMSP, tln, 0, oldtln);
	oldtln = n;
}

	setval(&lln, &oldlln, argv[1], SMEMSP, SDEMSP);
}

/*
 * Set line spacing.
 */
req_ls(argc, argv) int argc; char *argv[];
{
	setval(&lsp, &oldlsp, argv[1], SMUNIT, SDUNIT);
}

/*
 * Set title length.
 */
req_lt(argc, argv) int argc; char *argv[];
{
	setval(&tln, &oldtln, argv[1], SMEMSP, SDEMSP);
}

/*
 * Set margin character.
 */
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
	if (argc > 2)
		mar = number(argv[2], SMEMSP, SDEMSP, mar, 0, 0);
}

/*
 * Mark vertical place internally in diversion or in number register.
 */
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
