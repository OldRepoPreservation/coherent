/*
 * process.c
 * nroff/Troff.
 * Formatting.
 */

#include <ctype.h>
#include <ascii.h>
#include "roff.h"

#define	CHYPHEN	'-'			/* hyphenation character */
#define	MAXTRNB	256			/* Maximum transparent buffer sz. */

/*
 * Make sure there is space to add another command.
 */
chkcode()
{
	if (nlinptr >= &linebuf[LINSIZE])
		panic("line buffer overflow");
}

/*
 * Add a character given the font number and width.
 */
addchar(f, w)
{
	chkcode();
	nlinptr->c_arg.c_code = f & 0xFF;
	nlinptr->c_arg.c_char = 0;
	nlinptr++->c_arg.c_move = w;
}

/*
 * Add a directive which takes an integer argument.
 */
addidir(d, i)
{
	chkcode();
	nlinptr->l_arg.c_code = d;
	nlinptr++->l_arg.c_iarg = i;
}

/*
 * Add a transparent buffer directive (dag).
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
	REG *rp;
	int lastc, n;
	register int c, w;
	char charbuf[CBFSIZE], name[2];

	c = '\n';
	for (;;) {
		lastc = c;
		c = getf(1);
	next:
		switch (c) {
		case EOF:		/* End of file */
			dprintd(DBGFILE|DBGPROC, ".end of file\n");
			return;
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
			hormove(mws);
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
			{
				char *tb, *tp;
				int i=0;
				int j=MAXTRNB;
				if ((tb = malloc(j)) == NULL) {
				    j /= 2;
				    if ((tb = malloc(j)) == NULL) {
					j /= 2;
					tb = nalloc(j);
				    }
				}
				tp = tb;
				while ((*tp++ = getl(0)) != '\n') {
				    *tp = '\0';
				    if (++i >= MAXTRNB) {
					*tp = '\0';
					addtrab(tb);
					tb = nalloc(j);
					tp = tb;
					i = 0;
				    }
				}
				if (*(tp - 2) == '\\') {
					tp -= 2;
				}
				*tp = '\0';
				if(NULL == (tb = realloc(tb, i+1)))
					nalloc(i + 1); /* should force crash */
				addtrab(tb);
			}
			c = '\n';	/* Last character was eol... */
			continue;
		case EHYP:		/* Potential hyphenation break */
			dprintd(DBGPROC, ".hyphen break\n");
			addidir(DHYPH, 0);
			continue;
		case ECHR:		/* Special character indicator */
			dprintd(DBGPROC, ".special char\n");
			printu("special char indicator");	/* $$TO_DO$$ */
			getf(0);
			getf(0);
			continue;
		case EBRA:		/* Bracket building function */
			dprintd(DBGPROC, ".bracket building\n");
			printu("bracket building");		/* $$TO_DO$$ */
			continue;
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
			n = 0;
			if (scandel(charbuf, CBFSIZE))
				n = number(charbuf, SMUNIT, SDUNIT, 0, 0, 0);
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
			n = 0;
			if (scandel(charbuf, CBFSIZE))
				n = number(charbuf, 1L, 1L, 0, 0, 0);
			dprint2(DBGPROC, ".horiz line %d\n", n);
			if (n < 0) {
				hormove(n);
				n = -n;
			}
			c = fontype;			/* dag */
			devfont(ufn);			/* dag */
			w = fonwidt['_'];
			w = unit(swdmul*w*psz, swddiv);	/* I guess */
			if (n < w) {
				hormove(- ((w-n)/2));
				addchar('_', n + w/2);
				nlindir++;
				nlinsiz += n + w/2;
				devfont(c);		/* dag */
				continue;
			}
			if (n % w != 0) {
				addchar('_', n%w);
				nlindir++;
				nlinsiz += n%w;
			}
			n /= w;
			while (n-- != 0) {
				addchar('_', w);
				nlindir++;
				nlinsiz += w;
			}
			devfont(c);			/* dag */
			continue;
		case EVLF:		/* Vertical line drawing function */
			dprintd(DBGPROC, ".vertical line\n");
			printu("vertical line drawing");	/* $$TO_DO$$ */
			continue;
		case EOVS:		/* Overstrike */
			dprintd(DBGPROC, ".overstrike\n");
			printu("overstrike");			/* $$TO_DO$$ */
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
			n = 0;
			if (scandel(charbuf, CBFSIZE))
				n = number(charbuf, SMUNIT, SDUNIT, 0, 1, 0);
			dprint2(DBGPROC, ".local vert move %d\n", n);
			vermove(n);
			continue;
		case EXLS:		/* Extra line spacing */
			n = 0;
			if (scandel(charbuf, CBFSIZE))
				n = number(charbuf, SMUNIT, SDUNIT, 0, 1, 0);
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
			printu("zero width character");		/* $$TO_DO$$ */
			continue;
		case ECOD:		/* Processed text */
			dprintd(DBGPROC, ".processed text\n");
			c = diverse();
			lastc = '\n';
			goto next;
		case ELDR:
			dprintd(DBGPROC, ".leader character\n");
			wordbreak(DHMOV);
			movetab(ldc);
			continue;
		case '\t':
		case ETAB:
			dprintd(DBGPROC, ".tab\n");
			if ((llinptr->l_arg.c_code == -7) || (llinptr->l_arg.c_code == -1))
				if (spcnt <
					((ctabptr + 1)->t_pos - ctabptr->t_pos))
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
				llinptr->l_arg.c_iarg += 2*mws;
				nlinsiz += 2*mws;
				break;
			default:
				llinptr->l_arg.c_iarg += mws;
				nlinsiz += mws;
			}
			if (fil==0 || cec) {
				spcnt = 0;
				linebreak();
			}
			if (cec)
				--cec;
			if (ulc) {
				if (--ulc == 0)
					setfont(ufp, 1);
			}
			if (inpltrc)
				if (--inpltrc == 0)
					execute(inptrap);
			continue;
		case ' ':
			if (lastc == '\n')
				setbreak();
			else
				wordbreak(DPADC);

			if(is_varspace(fontype) || fil) {
				llinptr->l_arg.c_iarg += mws;
				nlinsiz += mws;
				spcnt += mws;
				continue;
			}
		case EDWS:		/* Digit width space */
			dprintd(DBGPROC, ".digit width space\n");
			w = fonwidt['0'];
			hormove(unit(swdmul*w*psz, swddiv));
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
 * Vertical move code.
 */
vermove(n)
int	n;
{
	addidir(DVMOV, n);
	basline += n;
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
	w = fonwidt[f];				/* raw width */
	dprint3(DBGCHAR, "** char ='%c', width = %d\n", f, w);
	w = unit(swdmul*w*psz, swddiv);		/* scaled width */
	if (csz != 0 && csz != w) {
		/*
		 * Constant width desired.
		 * Adjust previous character width so this character
		 * ends up centered in the constant width space.
		 */
		w = (csz-w) / 2;		/* previous char width adjust */
		cp = llinptr - 1;
		if (cp >= linebuf && ifcchar(cp->c_arg.c_code))
			cp->c_arg.c_move += w;
		else
			hormove(w);
		w = csz - w;			/* current char width adjust */
	}
	addchar(f, w);				/* output a character */
	nlindir++;
	nlinsiz += w;
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
	int c, lastcode;

	lastcode = codeval.l_arg.c_code;	/* by cef */
	for (c = ECOD;  c == ECOD;  c = getf(1)) {
		code = codeval.l_arg.c_code;
		arg = codeval.l_arg.c_iarg;
		switch (code) {
		case DSPAR:
			if (arg==0) {
				break;	/* discard empty newline codes */
			} else if (fil) {
				wordbreak(DPADC);
				padspace(lastcode);
			} else {
				register int	cvls, clsp;
	
				wordbreak(DNULL);
				cvls = vls, clsp = lsp;
				vls = 0, lsp = 1;
				linebreak();
				spcnt = 0;
				vls = cvls, lsp = clsp;
				sspace(arg);
			}
			if (inpltrc && --inpltrc == 0)
				execute(inptrap);
			break;
		case DPADC:
			wordbreak(DPADC);
			if (fil) {
				padspace(lastcode);
			} else {
				llinptr->l_arg.c_iarg += arg;
				nlinsiz += arg;
			}
			break;
		case DHYPC:
			if (fil) {
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
				nlinsiz += arg;
				addidir(code, arg);
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
			devfont(arg);
			break;
		case DPSZE:
			devpsze(arg);
			break;
		default:
			nlindir++;
			nlinsiz += codeval.c_arg.c_move;
			addchar(code, codeval.c_arg.c_move);
			break;
		}
		lastcode = code;
	}
	return c;
}

padspace(lastcode)
int	lastcode;
{
	switch (lastcode) {
	case '!':
	case '?':
	case '.':
		llinptr->l_arg.c_iarg += 2*mws;
		nlinsiz += 2*mws;
		break;
	default:
		llinptr->l_arg.c_iarg += mws;
		nlinsiz += mws;
	}
}

/*
 * End the current line and left justify it.
 */
setbreak()
{
	register int cfil;

	if (nbrflag)
		return;
	wordbreak(DNULL);
	cfil = fil;
	fil = 0;
	linebreak();
	fil = cfil;
}

/*
 * End the current word.  The given directive type is added onto
 * the end of the line.
 */
wordbreak(dir) int dir;
{
	int n, s, d;

	if (nlindir == llindir)
		return;
	if (llinptr == linebuf)
		setwork();
	else {
		if (fil!=0 && nlinsiz>lln) {
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
	int hflag, b1, b2, h, d, s, n;
	register CODE *cp;
	register int c;
	register char *hp;

	hyphen(wp=llinptr+1, nlinptr);
	h = fonwidt[CHYPHEN];
	h = unit(swdmul*h*psz, swddiv);
	b1 = nlinsiz - lln;
	b2 = b1 + h;
	d = 0;
	s = 0;
	n = 0;
	cp = nlinptr;
	hp = &hyphbuf[nlinptr-wp];
	for (;;) {
		if (--cp < wp)
			return (0);
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
		if (ifcchar(c)) {
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
		addchar(CHYPHEN, h);
		llinptr = nlinptr;
		llinsiz += h;
		llindir++;
	}
	*dp = d;
	*sp = s;
	*np = n;
	return (1);
}

/*
 * End the current line.
 * This must be called after calling wordbreak.
 */
linebreak()
{
	if (llindir == 0)
		return;
	movetab(EOF);
	justify();
	if (mrch != '\0' && llinsiz != 0) {
		int w;
		int slsiz = nlinsiz;
		int sldir = nlindir;
		CODE *slptr = nlinptr;
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
	a_reg = posexls;
	setline();
	lspace(vls+posexls);
	nrnlreg->n_reg.r_nval = mdivp->d_rpos;
	sspace((lsp-1)*vls);
}

/*
 * Justify the current line.
 */
justify()
{
	register CODE *cp, *cp0;
	int t;
	register int n, r;

	n = cec  ?  CJUS  :  (fil==0||adm==0) ? LJUS : adj;
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
 * Tab to the next tab stop.  The intermediate space is filled
 * with the character c.  If the character passed is an EOF,
 * then it is being called at the end of a line to finish up
 * the final tab stop.  This must be called right after calling
 * wordbreak.
 */
movetab(c)
{
	register TAB *tp;
	int n, w, d2;
	register int d, d1;
	register int pos;

	tp = ctabptr;
	pos = tp->t_pos + tin;		/* relative to indent */
	switch (tp->t_jus) {
	case LJUS:
		d = pos - tlinsiz;
		/*
		tlinptr->l_arg.c_iarg -= d;
		nlinsiz = llinsiz -= d;
		*/
		break;
	case CJUS:
		d = pos - (llinsiz+tlinsiz)/2;
		break;
	case RJUS:
		d = pos - llinsiz;
		break;
	case NJUS:
		llinptr->l_arg.c_iarg += mws;
		return;
	}
	if (ltabchr == 0) {			/* Blank tabs	*/
		tlinptr->l_arg.c_iarg += d;
		llinptr->l_arg.c_iarg -= llinptr->l_arg.c_csp;
		llinptr->l_arg.c_csp = 0;
	} else {					/* Leader dots... */
		if ((w = ldrspc) == 0) {
			w = fonwidt[ltabchr];
			w = unit(swdmul*w*psz, swddiv);
		}
		if ((n=nlinptr-(tlinptr+1)) > WORSIZE)
			panic("word buffer overflow");
		copystr(wordbuf, tlinptr+1, sizeof (CODE), n);
		nlinptr = tlinptr + 1;
		addidir(DFONT, ldf);			/* dag */
		if ((d2=(d1=tlinsiz)%w) != 0) {
			d1 += w-d2;
			tlinptr->l_arg.c_iarg += w-d2;
		}
		d2 = tlinsiz + d;
		while ((d1+=w) <= d2)
			addchar(ltabchr, w);
		addidir(DHMOV, d2-(d1-w));
		addidir(DFONT, fontype);		/* dag */
		if (nlinptr+n >= &linebuf[LINSIZE])
			panic("line buffer overflow");
		copystr(nlinptr, wordbuf, sizeof (CODE), n);
		llinptr = nlinptr += n;
		--llinptr;
	}
	tlinsiz = nlinsiz = llinsiz += d;
	tlinptr = llinptr;
	if (c == EOF)
		return;
	while ((++tp)->t_jus!=NJUS && tp->t_pos+tin <= llinsiz)
		;
	/*
	if (tp->t_jus == LJUS) {
		tlinptr->l_arg.c_iarg += d = tp->t_pos+tin - tlinsiz;
		nlinsiz = llinsiz += d;
	}
	*/
	ctabptr = tp;
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
	ctabptr = tab;
	ltabchr = '\0';
	basline = 0;
	preexls = 0;
	posexls = 0;
	addidir(DHMOV, 0);
}

/* end of process.c */
