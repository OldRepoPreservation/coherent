/*
 * process.c
 * nroff/Troff.
 * Formatting.
 */

#include <ctype.h>
#include <ascii.h>
#include "roff.h"

#define	vermove(d)	addidir(DVMOV, (d))	/* vertical movement */

#define	CHYPHEN	'-'				/* hyphenation character */

/*
 * Make sure there is space to add another command.
 */
chkcode()
{
	if (nlinptr >= &linebuf[LINSIZE])
		panic("line buffer overflow");
}

/*
 * Add a character.
 * Bump the directive count and nlinsiz.
 */
addchar(f, w) int f; register int w;
{
	chkcode();
	nlinptr->c_arg.c_code = f & 0xFF;
	nlinptr++->c_arg.c_move = w;
	nlindir++;
	nlinsiz += w;
}

/*
 * Add a directive which takes an integer argument.
 */
addidir(d, i)
{
	chkcode();
	nlinptr->l_arg.c_code = d;
	nlinptr->l_arg.c_iarg = i;
	nlinptr++->l_arg.c_csp = 0;
}

/*
 * Add a transparent buffer directive.
 */
addtrab(bp) char *bp;
{
	chkcode();
	nlinptr->b_arg.c_code = DTRAB;
	nlinptr++->b_arg.c_bufp = bp;
}

/*
 * Process input, formatting text.
 */
process()
{
	register int c, w;
	REG *rp;
	int lastc, n;
	char charbuf[CBFSIZE], name[2];
	unsigned char *cp;

	c = '\n';
	for (;;) {
		lastc = c;
		c = getf(1);
	next:
		switch (c) {
		case EOF:		/* End of file */
			dprintd(DBGFILE|DBGPROC, ".end of file\n");
			return;
		case EBEG:		/* Open brace \{ */
		case EEND:		/* Close brace \} */
			continue;	/* ignore braces in plain text */
		case EESC:		/* Printable version of escape */
			dprintd(DBGPROC, ".printable escape\n");
			character(esc);
			continue;
		case EACA:		/* Acute accent */
			dprintd(DBGPROC, ".acute accent\n");
			character('\'');
			continue;
		case EGRA:		/* Grave accent */
			dprintd(DBGPROC, ".grave accent\n");
			character('`');
			continue;
		case EMIN:		/* Minus sign */
			dprintd(DBGPROC, ".minus sign\n");
			character('-');
			continue;
		case EUNP:		/* Unpaddable space */
			dprintd(DBGPROC, ".unpaddable space\n");
			hormove(ssz);
			continue;
		case EM06:		/* 1/6 em (narrow space) */
			dprintd(DBGPROC, ".narrow space\n");
			hormove(unit(SMNARS, SDNARS));
			continue;
		case EM12:		/* 1/12 em (half narrow space) */
			dprintd(DBGPROC, ".half narrow space\n");
			hormove(unit(SMNARS, SDNARS*2));
			continue;
		case ENOP:		/* Zero width character */
			dprintd(DBGPROC, ".zero width character\n");
			continue;
		case ETLI:		/* Transparent line indicator */
			dprintd(DBGPROC, ".transparent line indicator\n");
			for (cp = charbuf; ; c = *cp++ = getl(0)) {
				if (c == '\n' || cp == &charbuf[CBFSIZE-1]) {
					*cp = '\0';
					addtrab(duplstr(charbuf));
					if (c == '\n')
						break;
					cp = charbuf;
				}
			}
			continue;
		case EHYP:		/* Potential hyphenation break */
			dprintd(DBGPROC, ".hyphen break\n");
			addidir(DHYPH, 0);
			continue;
		case EBRA:		/* Bracket building function */
			dprintd(DBGPROC, ".bracket building\n");
#if	1
			printu("bracket building");		/* $$TO_DO$$ */
			continue;
#else
			/*
			 * The following does not work with special characters
			 * which include font escape sequences.
			 */
			scandel(charbuf, CBFSIZE);
			for (w = 0, cp = charbuf; *cp; cp++) {
				n = width(trantab[*cp]);
				if (n > w)
					w = n;			/* max width */
			}
			n = cp - charbuf;			/* count */
			vermove(unit(-SMVEMS * (n - 1), SDVEMS * 2));
			for (cp = charbuf; *cp; cp++) {
				addchar(trantab[*cp], 0);
				vermove(unit(SMVEMS, SDVEMS));
			}
			vermove(unit(-SMVEMS * (n + 1), SDVEMS * 2));
			hormove(w);
			continue;
#endif
		case EINT:		/* Interrupt text processing */
			dprintd(DBGPROC, ".interrupt proc\n");
			if ((c=getf(0)) != '\n')
				goto next;
			continue;
		case EVNF:		/* 1/2 em vertical motion */
			dprintd(DBGPROC, ".half vertical\n");
			vermove(unit(SMVEMS, SDVEMS*2));
			continue;
		case EFON:		/* Change font */
			if ((c=getf(0)) != '(') {
				name[0] = c;
				name[1] = '\0';
			} else {
				name[0] = getf(0);
				name[1] = getf(0);
			}
			dprint3(DBGPROC, ".font change to %c%c\n", name[0], name[1]);
			setfont(name, 1);
			continue;
		case EHMT:		/* Local horizontal motion */
			n = scandel(charbuf, CBFSIZE) ? numb(charbuf, SMUNIT, SDUNIT) : 0;
			dprint2(DBGPROC, ".local horiz motion = %d\n", n);
			hormove(n);
			continue;
		case EMAR:		/* Mark horizontal input place */
			name[0] = c = getf(0);
			name[1] = '\0';
			dprint3(DBGPROC, ".mark horizontal input in %c at %d\n", c, nlinsiz);
			rp = getnreg(name);
			rp->n_reg.r_nval = nlinsiz;
			continue;
		case EHLF:		/* Horizontal line drawing function */
			/*
			 * This still does not handle line drawing with
			 * characters other than under-bar.
			 */
			n = scandel(charbuf, CBFSIZE) ? numb(charbuf, 1L, 1L) : 0;
			dprint2(DBGPROC, ".horiz line %d\n", n);
			if (n == 0)
				continue;
			else if (n < 0) {
				hormove(n);
				n = -n;
			}
			c = curfont;
			dev_font(ufn);
			w = width('_');
			if (n < w) {
				/* Line length less than '_' width. */
				hormove(- ((w-n)/2));
				addchar('_', n + w/2);
				dev_font(c);
				continue;
			}
			if (n % w != 0)
				addchar('_', n%w);
			n /= w;
			while (n-- != 0)
				addchar('_', w);
			dev_font(c);
			continue;
		case EVLF:		/* Vertical line drawing function */
			dprintd(DBGPROC, ".vertical line\n");
#if	1
			printu("vertical line drawing");	/* $$TO_DO$$ */
			continue;
#else
			n = scandel(charbuf, CBFSIZE) ? numb(charbuf, 1L, 1L) : 0;
			dprint2(DBGPROC, ".vert line %d\n", n);
			if (n == 0)
				continue;
			else if (n < 0) {
				vermove(n);
				n = -n;
			}
			c = curfont;
			dev_font(ufn);
			w = unit(SMVEMS, SDVEMS);
			if (n < w) {
				/* Line length less than '_' width. */
				vermove(-((w-n)/2));
				addchar('|', 0);
				vermove(((w-n)/2));
				dev_font(c);
				continue;
			}
			if (n % w != 0) {
				addchar('|', 0);
				vermove(n%w);
			}
			n /= w;
			while (n-- != 0) {
				addchar('|', 0);
				vermove(w);
			}
			dev_font(c);
			continue;
#endif
		case EOVS:		/* Overstrike */
			dprintd(DBGPROC, ".overstrike\n");
			scandel(charbuf, CBFSIZE);
			for (w = 0, cp = charbuf; *cp; cp++) {
				n = width(trantab[*cp]);
				if (n > w)
					w = n;			/* max width */
			}
			for (cp = charbuf; *cp; cp++) {
				n = width(trantab[*cp]);
				hormove((w-n)/2);
				addchar(trantab[*cp], 0);	/* centered */
				hormove((n-w)/2);
			}
			hormove(w);
			continue;
		case ESPR:		/* Break and spread output line */
			dprintd(DBGPROC, ".break and spread\n");
			wordbreak(DNULL);
			linebreak();
			continue;
		case EVRM:		/* Reverse 1 em vertically */
			dprintd(DBGPROC, ".reverse vertical\n");
			vermove(unit(-SMVEMS, SDVEMS));
			continue;
		case EPSZ:		/* Change pointsize */
			dprintd(DBGPROC, ".pointsize change\n");
			if (scandel(charbuf, CBFSIZE)) {
				n = number(charbuf, SMPOIN, SDPOIN, psz, 0, oldpsz);
				if (n == 0)
					n = oldpsz;
				newpsze(n);
			}
			continue;
		case EVRN:		/* Reverse 1 en vertically */
			dprintd(DBGPROC, ".reverse 1 en vert\n");
			vermove(unit(-SMVEMS, SDVEMS*2));
			continue;
		case EVMT:		/* Local vertical motion */
			n = scandel(charbuf, CBFSIZE) ? number(charbuf, SMUNIT, SDUNIT, 0, 1, 0) : 0;
			dprint2(DBGPROC, ".local vert move %d\n", n);
			vermove(n);
			continue;
		case EXLS:		/* Extra line spacing */
			n = scandel(charbuf, CBFSIZE) ? number(charbuf, SMUNIT, SDUNIT, 0, 1, 0) : 0;
			dprint2(DBGPROC, ".extra line space %d\n", n);
			if (n < 0) {
				if (-n > preexls)
					preexls = -n;
			} else {
				if (n > posexls)
					posexls = n;
			}
			continue;
		case EZWD:		/* Print character with zero width */
			dprintd(DBGPROC, ".print with zero width\n");
			addchar(trantab[getf(0)], 0);
			continue;
		case ECOD:		/* Processed text */
			dprintd(DBGPROC, ".processed text\n");
			c = diverse();
			lastc = '\n';
			goto next;
		case '\001':
		case ELDR:
			dprintd(DBGPROC, ".leader character\n");
			wordbreak(DHMOV);
			movetab(ldc);
			continue;
		case '\t':
		case ETAB:
			dprintd(DBGPROC, ".tab\n");
			if (spcnt != 0
			 && (llinptr->l_arg.c_code == DHYPH
			  || llinptr->l_arg.c_code == DHMOV))
				if (spcnt < ((tabptr + 1)->t_pos - tabptr->t_pos))
					llinptr->l_arg.c_csp = spcnt;
			spcnt = 0;
			wordbreak(DHMOV);
			movetab(tbc);
			continue;
		case '\n':
			dprintd(DBGPROC, ".newline\n");
			wordbreak(DPADC);
			switch (lastc) {
			case '\n':
				setbreak();
				nlindir++;
				setbreak();
				break;
			case '.':
			case '!':
			case '?':
				pad(2 * ssz);
				break;
			default:
				pad(ssz);
			}
			if (fill==0 || cec) {
				spcnt = 0;
				linebreak();
			}
			if (cec)
				--cec;
			if (ulc) {
				if (--ulc == 0)
					setfontnum(ufp, 1);
			}
			if (inpltrc) {
				if (--inpltrc == 0)
					execute(inptrap);
			}
			continue;
		case ' ':
			if (lastc == '\n')
				setbreak();
			else
				wordbreak(DPADC);

			if (fill || varsp) {
				pad(ssz);
				spcnt += ssz;
				continue;
			}
		case EDWS:		/* Digit width space */
			dprintd(DBGPROC, ".digit width space\n");
			hormove(width('0'));
			continue;
		default:
			if (lastc == '\n') {
				if (c == ccc) {
					request();
					c = '\n';
					continue;
				} else if (c == nbc) {
					nbrflag = 1;
					request();
					nbrflag = 0;
					c = '\n';
					continue;
				}
			}
#if	0
			/* Ignore non-ASCII characters. */
			if (!isascii(c))
				continue;
#endif
			character(trantab[c]);
		}
	}
}

/*
 * Horizontal move code.
 */
hormove(n)
int	n;
{
	addidir(DHMOV, n);
	nlinsiz += n;
}

/*
 * Character code.
 */
character(f) register int f;
{
	register int w;
	CODE *cp;

	/* Should this complain about 0-width characters?  Silent for now. */
	w = width(f);				/* scaled width */
	dprint3(DBGCHAR, "** char ='%c', width = %d\n", f, w);
	if (csz != 0 && csz != w) {
		/*
		 * Constant width desired.
		 * Adjust previous character width so this character
		 * ends up centered in the constant width space.
		 */
		w = (csz-w) / 2;		/* previous char width adjust */
		cp = llinptr - 1;
		if (cp >= linebuf && is_char(cp->c_arg.c_code))
			cp->c_arg.c_move += w;
		else
			hormove(w);
		w = csz - w;			/* current char width adjust */
	}
	addchar(f, w);				/* output a character */
}

/*
 * Read text from a diversion.
 * Whoever thought up the way diversions work in NROFF/TROFF
 * should have done something else.
 */
int
diverse()
{
	register int code, arg;
	register int	cvls, clsp;
	int c, lastcode;
	char *tp;

	lastcode = ' ';
	for (c = ECOD;  c == ECOD;  c = getf(1)) {
		code = codeval.l_arg.c_code;
		arg = codeval.l_arg.c_iarg;
		switch (code) {
		case DSPAR:
			if (arg==0) {
				break;		/* discard empty newline codes */
			} else if (fill) {
				wordbreak(DPADC);
				padspace(lastcode);
			} else {
				wordbreak(DNULL);
				cvls = vls;
				clsp = lsp;
				vls = 0;
				lsp = 1;
				linebreak();
				spcnt = 0;
				vls = cvls;
				lsp = clsp;
				sspace(arg);
			}
			if (inpltrc && --inpltrc == 0)
				execute(inptrap);
			break;
		case DPADC:
			wordbreak(DPADC);
			if (fill) {
				padspace(lastcode);
			} else
				pad(arg);
			break;
		case DHYPC:
			if (fill) {
				register int	c;
	
				addidir(DHYPH, 0);
				while ((c=getf(0))==ECOD
				 && codeval.l_arg.c_code==DSPAR
				 && codeval.l_arg.c_iarg==0 )
					;
				if (c!=ECOD || codeval.l_arg.c_code!=DSPAR)
					panic("cannot dehyphenate");
			} else {
				nlindir++;
				addidir(code, arg);
				nlinsiz += arg;
			}
			break;
		case DHMOV:
			nlinsiz += arg;
		case DNULL:
		case DVMOV:
		case DHYPH:
			addidir(code, arg);
			break;
		case DFONT:
			dev_font(arg);
			break;
		case DPSZE:
			dev_ps(arg);
			break;
		case DTRAB:
			tp = codeval.b_arg.c_bufp;
			adscore(tp);
			strp->x3.s_srel = tp;
			break;
		default:
			addchar(code, codeval.c_arg.c_move);
			break;
		}
		lastcode = code;
	}
	return c;
}

pad(n) register int n;
{
	llinptr->l_arg.c_iarg += n;
	nlinsiz += n;
}

padspace(lastcode) int lastcode;
{
	switch (lastcode) {
	case '!':
	case '?':
	case '.':
		pad(2 * ssz);
		break;
	default:
		pad(ssz);
		break;
	}
}

/*
 * End the current line and left justify it.
 */
setbreak()
{
	register int savfill;

#if	0
	fprintf(stderr, "setbreak()\n");
#endif
	if (nbrflag)
		return;
	wordbreak(DNULL);
	savfill = fill;
	fill = 0;
	linebreak();
	fill = savfill;
}

/*
 * End the current word.  The given directive type is added onto
 * the end of the line.
 */
wordbreak(dir) int dir;
{
	int n, s, d;

#if	0
	fprintf(stderr, "wordbreak(%d)\n", dir);
#endif
	if (nlindir == llindir)
		return;
	if (llinptr == linebuf)
		setwork();
	else {
		if (fill!=0 && nlinsiz>lln) {
			n = nlinptr - (llinptr+1);
			s = nlinsiz - llinsiz - llinptr->l_arg.c_iarg;
			d = nlindir - llindir;
			if (hyp==0 || (hypf = fitword(&n, &s, &d))==0)
				copystr(wordbuf, llinptr+1, sizeof (CODE), n);
			if (n > WORSIZE)
				panic("word buffer overflow");
			linebreak();
			hypf = 0;
			copystr(nlinptr, wordbuf, sizeof (CODE), n);
			nlinptr += n;
			nlinsiz += s;
			nlindir += d;
			setwork();
		}
	}
	if (mrc != '\0')		/* Added by dag	*/
		mrch = mrc;
	llinptr = nlinptr;
	llinsiz = nlinsiz;
	llindir = nlindir;
	addidir(dir, 0);
}

/*
 * Set up working parameters for the line.
 */
setwork()
{
	if (tif)
		tif = 0;
	else
		tin = ind;
	linebuf[0].l_arg.c_iarg += tin;
	llinsiz += tin;
	nlinsiz += tin;
	tlinsiz += tin;
}

/*
 * Try to hyphenate and fit the last word in a line.
 * This routine is really part of the routine 'wordbreak'.
 * The arguments are pointers to variables in 'wordbreak'.
 */
fitword(np, sp, dp) int *dp; int *sp; int *np;
{
	CODE *wp;
	int hflag, b1, b2, h, d, s, n, sav;
	register CODE *cp;
	register int c;
	register char *hp;

	hyphen(wp=llinptr+1, nlinptr);
	h = width(CHYPHEN);
	b1 = nlinsiz - lln;
	b2 = b1 + h;
	d = 0;
	s = 0;
	n = 0;
	cp = nlinptr;
	hp = &hyphbuf[nlinptr-wp];
	for (;;) {
		if (--cp < wp)
			return 0;
		c = cp->l_arg.c_code;
		if (cp>wp && c==CHYPHEN && s>=b1) {
			hflag = 0;
			break;
		}
		if (*--hp) {
			if (s >= b2) {
				hflag = 1;
				break;
			}
		}
		if (is_char(c)) {
			d++;
			s += cp->c_arg.c_move;
			continue;
		}
		if (c==DHMOV || c==DPADC) {
			s += cp->l_arg.c_iarg;
			continue;
		}
	}
	n = nlinptr - ++cp;
	copystr(wordbuf, cp, sizeof (CODE), n);
	llinptr = cp;
	llinsiz = nlinsiz - s;
	llindir = nlindir - d;
	if (hflag) {
		nlinptr = llinptr;
		sav = nlinsiz;
		addchar(CHYPHEN, h);
		nlinsiz = sav;
		llinptr = nlinptr;
		llinsiz += h;
		llindir = nlindir;
	}
	*dp = d;
	*sp = s;
	*np = n;
	return 1;
}

/*
 * End the current line.
 * This must be called after calling wordbreak.
 */
linebreak()
{
	int slsiz, sldir;
	CODE *slptr;

#if	0
	fprintf(stderr, "linebreak() llindir=%d\n", llindir);
#endif
	if (llindir == 0) {
		if (nlinptr != llinptr)
			lineflush();
		return;
	}
	movetab(EOF);
	justify();
	if (mrch != '\0' && llinsiz != 0) {

		/* Output margin character at mar from right margin. */
		slsiz = nlinsiz;
		sldir = nlindir;
		slptr = nlinptr;
		nlinsiz = llinsiz;
		nlinptr = llinptr;
		nlindir = llindir;
#if	0
		/* Debug print-out to stderr */
		fprintf(stderr, "margin char %c %d %d %d %d\n",
			mrch, mar, lln, llinsiz, (lln-llinsiz) + mar);
#endif
		hormove((lln - llinsiz) + mar);
		character(mrch);
		llinsiz = nlinsiz;
		llindir = nlindir;
		llinptr = nlinptr;
		nlinsiz = slsiz;
		nlindir = sldir;
		nlinptr = slptr;
		mrch = '\0';
	}
	if (llinsiz > cdivp->d_maxw)
		cdivp->d_maxw = llinsiz;
	sspace(preexls);
	spcnt = 0;
	if (cdivp == mdivp) {
		n_reg = llinsiz - linebuf[0].l_arg.c_iarg;
		linebuf[0].l_arg.c_iarg += pof;
		flushl(linebuf, llinptr);
		nsm = 0;
	} else
		flushd(linebuf, llinptr);
	if (nlinptr != llinptr)
		lineflush();
	a_reg = posexls;
	setline();
	lspace(vls+posexls);
	nrnlreg->n_reg.r_nval = mdivp->d_rpos;
	sspace((lsp-1)*vls);
}

/*
 * The unflushed part of a buffered line can contain significant directives,
 * notably pointsize and font changes.
 * This flushes directives which would otherwise get ignored.
 * There must be a better way...
 */
lineflush()
{
	register CODE *cp;

	for (cp = llinptr; cp < nlinptr; cp++) {
		switch(cp->l_arg.c_code) {
		case DFONT:	break;
		case DPSZE:	break;
		default:	continue;
		}
		if (cdivp == mdivp)
			flushl(cp, cp+1);
		else
			flushd(cp, cp+1);
	}
}

/*
 * Justify the current line.
 */
justify()
{
	register CODE *cp, *cp0;
	int t;
	register int n, r;

	n = cec  ?  CJUS  :  (fill==0||adm==0) ? LJUS : adj;
	switch (n) {
	case LJUS:
		n = 0;
		break;
	case CJUS:
		n = (lln-llinsiz) / 2;
		break;
	case RJUS:
		n = lln - llinsiz;
		break;
	case FJUS:
		r = 0;
		/*
		 * Walk backward through the line looking for horizontal move.
		 * If found, do not pad characters to its left.
		 */
		for (cp0 = llinptr-1; cp0 > linebuf; cp0--)
			 if (cp0->l_arg.c_code == DHMOV)
				break;
		for (cp = cp0; cp < llinptr; cp++)
			if (cp->l_arg.c_code == DPADC)
				r++;		/* paddable character count */
		if (r == 0)
			return;			/* no paddable characters */
		n = lln - llinsiz;		/* padding required */
		t = n%r;			/* padding remainder */
		n = 1 + n/r;			/* starting pad amount */
		r = t;				/* to do before decrementing */
		for (cp = cp0; cp < llinptr; cp++) {
			if (cp->l_arg.c_code != DPADC)
				continue;
			if (r-- == 0)
				--n;		/* decrement padding amount */
			cp->l_arg.c_iarg += n;	/* pad by n */
		}
		llinsiz = lln;			/* dag for mrc/mrch */
		return;
	}
	llinsiz += n;				/* dag for mrc/mrch */
	linebuf[0].l_arg.c_iarg += n;
}

/*
 * Tab to the next tab stop, filling intermediate space with character c.
 * If the character passed is an EOF, this is being called
 * at the end of a line to finish up the final tab stop.
 * This must be called right after calling wordbreak().
 */
movetab(c)
{
	register TAB *tp;
	int n, w, d2, savfont;
	register int d, d1;
	register int pos;

#if	0
	fprintf(stderr, "movetab(%d) tab=%d ltabchr=%d\n", c, tabptr-tab, ltabchr);
#endif
	tp = tabptr;
	pos = tp->t_pos + tin;		/* relative to indent */
	switch (tp->t_jus) {
	case LJUS:
		d = pos - tlinsiz;
		break;
	case CJUS:
		d = pos - (llinsiz+tlinsiz)/2;
		break;
	case RJUS:
		d = pos - llinsiz;
		break;
	case NJUS:
		llinptr->l_arg.c_iarg += ssz;
		return;
	}
	if (ltabchr == 0) {
		/* Blank tabs. */
		tlinptr->l_arg.c_iarg += d;
		if (llinptr->l_arg.c_csp != 0) {
			/* Remove trailing space padding from last char. */
			llinptr->l_arg.c_iarg -= llinptr->l_arg.c_csp;
			llinsiz -= llinptr->l_arg.c_csp;			
			llinptr->l_arg.c_csp = 0;
		}
	} else {
		/* Leader dots or nonblank tabs. */
		if ((n = nlinptr - (tlinptr+1)) > WORSIZE)
			panic("word buffer overflow");
		copystr(wordbuf, tlinptr+1, sizeof (CODE), n);	/* save */
		nlinptr = tlinptr + 1;
		savfont = curfont;
		dev_font(tfn);				/* tab font */
		if ((w = tbs) == 0)
			w = width(ltabchr);		/* tab char width */
		if ((d2 = (d1 = tlinsiz) % w) != 0) {
			/* Futz start position to tab char width multiple. */
			d1 += w-d2;
			tlinptr->l_arg.c_iarg += w-d2;
		}
		d2 = tlinsiz + d;			/* end position */
		while ((d1 += w) <= d2)
			addchar(ltabchr, w);		/* write a tab char */
		addidir(DHMOV, d2-(d1-w));		/* skip remaining space */
		dev_font(savfont);			/* restore font */
		if (nlinptr+n >= &linebuf[LINSIZE])
			panic("line buffer overflow");
		copystr(nlinptr, wordbuf, sizeof (CODE), n);	/* restore */
		llinptr = nlinptr += n;
		--llinptr;
	}
	tlinsiz = nlinsiz = llinsiz += d;
	tlinptr = llinptr;
	if (c == EOF)
		return;
	while ((++tp)->t_jus != NJUS && tp->t_pos+tin <= llinsiz)
		;
	tabptr = tp;
	ltabchr = c;
}

/*
 * Initialize line data for a new line.
 */
setline()
{
	llinptr = nlinptr = tlinptr = linebuf;
	llinsiz = nlinsiz = tlinsiz = 0;
	llindir = nlindir = 0;
	tabptr = tab;
	ltabchr = '\0';
	preexls = posexls = 0;
	addidir(DHMOV, 0);
}

/* end of process.c */
