/*
 * Nroff/Troff.
 * Formatting.
 */
#include <stdio.h>
#include <ctype.h>
#include "roff.h"
#include "char.h"
#include "code.h"
#include "div.h"
#include "env.h"
#include "esc.h"
#include "hyphen.h"
#include "reg.h"
#include "font.h"
#include "codebug.h"

extern	char *realloc(), *malloc();
#define	MAXTRNB	256			/* Maximum transparent buffer sz. */

#if	(DDEBUG & DBGCALL)
/* This allows tracing back from panic to find where things happened.	*/

/*
 * Make sure there is space to add another command.
 */
#undef	chkcode
chkcode() {
	if (nlinptr >= &linebuf[LINSIZE])
		panic(lbomsg);
}
/*
 * Add a character given the font number and width.
 */
#undef	addchar
addchar(f, w) {
	chkcode();
	nlinptr->c_code = f;
	nlinptr->c_csp = 0;
	nlinptr++->c_move = w;
}
/*
 * Add a directive which takes an integer argument.
 */
#undef	addidir
addidir(d, i) {
	chkcode();
	nlinptr->c_code = d;
	nlinptr++->c_iarg = i;
}

#endif
/*
 * Process input, formatting text.
 */
process()
{
	REG *rp;
	int lastc, n;
	register int c, f, w;
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
			character(asctab[esc]);
			continue;
		case EACA:		/* Acute accent */
			dprintd(DBGPROC, ".acute accent\n");
			character(CACUTE);
			continue;
		case EGRA:		/* Grave accent */
			dprintd(DBGPROC, ".grave accent\n");
			character(CGRAVE);
			continue;
		case EMIN:		/* Minus sign */
			dprintd(DBGPROC, ".minus sign\n");
			character(CMINUS);
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
				char *almsg = "can't allocate";
				char *tb, *tp;
				int i=0;
				int j=MAXTRNB;
				if ((tb = malloc(j)) == NULL) {
				    j /= 2;
				    if ((tb = malloc(j)) == NULL) {
					j /= 2;
					if ((tb = malloc(j)) == NULL)
					    panic(almsg);
				    }
				}
				tp = tb;
				while ((*tp++ = getl(0)) != '\n') {
				    *tp = '\0';
				    if (++i >= MAXTRNB) {
					*tp = '\0';
					addtrab(tb);
					if ((tb = malloc(j)) == NULL)
						panic(almsg);
					tp = tb;
					i = 0;
				    }
				}
				if (*(tp - 2) == '\\') {
					tp -= 2;
				}
				*tp = '\0';
				tb = realloc(tb, i+1);
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
/*	special character indicator	$TO_DO$	*/
			printu("Special char indicator");
			getf();
			getf();
			continue;
		case EBRA:		/* Bracket building function */
			dprintd(DBGPROC, ".bracket building\n");
/*	bracket building		$TO_DO$	*/
			printu("Bracket building");
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
			rp->r_nval = nlinsiz;
			continue;
		case EHLF:		/* Horizontal line drawing function */
			/*
			 * This still does not handle line drawing with
			 * characters other than under-bar.
			 */
			n = 0;
			if (scandel(charbuf, CBFSIZE))
				n = number(charbuf, (long)1, (long)1, 0, 0, 0);
			dprint2(DBGPROC, ".horiz line %d\n", n);
			if (n < 0) {
				hormove(n);
				n = -n;
			}
			f = asctab['_'];
			w = fonwidt[f];
			w = unit(swdmul*w*psz, swddiv);	/* I guess */
			if (n < w) {
				hormove(- ((w-n)/2));
				addchar(f, n + w/2);
				nlindir++;
				nlinsiz += n + w/2;
				continue;
			}
			if (n % w != 0) {
				addchar(f, n%w);
				nlindir++;
				nlinsiz += n%w;
			}
			n /= w;
			while (n-- != 0) {
				addchar(f, w);
				nlindir++;
				nlinsiz += w;
			}
			continue;
		case EVLF:		/* Vertical line drawing function */
			dprintd(DBGPROC, ".vertical line\n");
/* vertical line drawing		$TO_DO$	*/
			printu("Vertical line drawing");
			continue;
		case EOVS:		/* Overstrike */
			dprintd(DBGPROC, ".overstrike\n");
/* overstrike				$TO_DO$	*/
			printu("Overstrike");
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
/* change pointsize			$TO_DO$	*/
			printu("Pointsize change");
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
/* zero width character			$TO_DO$	*/
			printu("Zero width character");
			continue;
		case ECOD:		/* Processed text */
			dprintd(DBGPROC, ".processed text\n");
			c = diverse();
			lastc = '\n';
			goto next;
		case SOH:
			dprintd(DBGPROC, ".SOH\n");
			wordbreak(DHMOV);
			movetab(ldc);
			continue;
		case '\t':
			dprintd(DBGPROC, ".tab\n");
			if ((llinptr->c_code == -7) || (llinptr->c_code == -1))
				if (spcnt <
					((ctabptr + 1)->t_pos - ctabptr->t_pos))
					llinptr->c_csp = spcnt;
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
				llinptr->c_iarg += 2*mws;
				nlinsiz += 2*mws;
				break;
			default:
				llinptr->c_iarg += mws;
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
				llinptr->c_iarg += mws;
				nlinsiz += mws;
				spcnt += mws;
				continue;
			}
		case EDWS:		/* Digit width space */
			dprintd(DBGPROC, ".digit width space\n");
			w = fonwidt[asctab['0']];
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
			if (!isascii(c))
				continue;
			f = asctab[trantab[c]];
			character(f);
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
character(f)
int	f;
{
	int	w;
	if (f == CNULL)
		return;
	if (fontype != newfont || fontype != curfont)
		addidir(DFONT, curfont = fontype = newfont);
	if (psz != newpsz || psz != curpsz)
		addidir(DPSZE, curpsz = psz = newpsz);
	w = fonwidt[f];
	dprint3(DBGCHAR, "** char ='%c', width = %d\n", f, w);
	w = unit(swdmul*w*psz, swddiv);
	if (csz!=0 && csz!=w) {
		w = (csz-w) / 2;
		addidir(DHMOV, w);
		w = csz - w;
	}
	addchar(f, w);
	nlindir++;
	nlinsiz += w;
}

/*
 * Whoever thought up the way diversions work in NROFF/TROFF
 * should have done something else.
 */
int
diverse()
{
	int	c,
		lastcode;

	for (c = ECOD;  c == ECOD;  c = getf(1)) {
		switch (codeval.c_code) {
		case DSPAR:
			if (codeval.c_iarg==0) {
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
				sspace(codeval.c_iarg);
			}
			if (inpltrc && --inpltrc == 0)
				execute(inptrap);
			break;
		case DPADC:
			wordbreak(DPADC);
			if (fil) {
				padspace(lastcode);
			} else {
				llinptr->c_iarg += codeval.c_iarg;
				nlinsiz += codeval.c_iarg;
			}
			break;
		case DHYPC:
			if (fil) {
				register int	c;
	
				addidir(DHYPH, 0);
				while ((c=getf(0))==ECOD
				 && codeval.c_code==DSPAR
				 && codeval.c_iarg==0 )
					;
				if (c!=ECOD || codeval.c_code!=DSPAR)
					panic("Cannot dehyphenate");
			} else {
				nlindir++;
				nlinsiz += codeval.c_iarg;
				addidir(codeval.c_code, codeval.c_iarg);
			}
			break;
		case DHMOV:
			nlinsiz += codeval.c_iarg;
		case DNULL:
		case DVMOV:
		case DHYPH:
			addidir(codeval.c_code, codeval.c_iarg);
			break;
		case DFONT:
			addidir(codeval.c_code, curfont = codeval.c_iarg);
			break;
		case DPSZE:
			addidir(codeval.c_code, curpsz = codeval.c_iarg);
			break;
		default:
			nlindir++;
			nlinsiz += codeval.c_move;
			addchar(codeval.c_code, codeval.c_move);
			break;
		}
		lastcode = codeval.c_code;
	}
	return (c);
}

padspace(lastcode)
int	lastcode;
{
	switch (lastcode) {
	case CEXCLAM:
	case CQUEST:
	case CDOT:
		llinptr->c_iarg += 2*mws;
		nlinsiz += 2*mws;
		break;
	default:
		llinptr->c_iarg += mws;
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
wordbreak(dir)
{
	int n, s, d;

	if (nlindir == llindir)
		return;
	if (llinptr == linebuf)
		setwork();
	else {
		if (fil!=0 && nlinsiz>lln) {
			n = nlinptr - (llinptr+1);
			s = nlinsiz - llinsiz - llinptr->c_iarg;
			d = nlindir - llindir;
			if (hyp==0 || (hypf = fitword(&n, &s, &d))==0)
				copystr(wordbuf, llinptr+1, sizeof (CODE), n);
			if (n > WORSIZE)
				panic("Word buffer overflow");
			linebreak();
			hypf = 0;
			copystr(nlinptr, wordbuf, sizeof (CODE), n);
			nlinptr += n;
			nlinsiz += s;
			nlindir += d;
			setwork();
		}
	}
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
	linebuf[0].c_iarg += tin;
	llinsiz += tin;
	nlinsiz += tin;
	tlinsiz += tin;
}

/*
 * Try to hyphenate and fit the last word in a line.
 * This routine is really part of the routine `wordbreak'.
 * The arguments are pointers to variables in `wordbreak'.
 */
fitword(np, sp, dp)
int *dp;
int *sp;
int *np;
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
		c = cp->c_code;
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
			s += cp->c_move;
			continue;
		}
		if (c==DHMOV || c==DPADC) {
			s += cp->c_iarg;
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
	if (llinsiz > cdivp->d_maxw)
		cdivp->d_maxw = llinsiz;
	sspace(preexls);
	spcnt = 0;
	if (cdivp == mdivp) {
		nrorval = llinsiz - linebuf[0].c_iarg;
		linebuf[0].c_iarg += pof;
		flushl(linebuf, llinptr);
		nsm = 0;
	} else
		flushd(linebuf, llinptr);
	arorval = posexls;
	setline();
	lspace(vls+posexls);
	nrnlreg->r_nval = mdivp->d_rpos;
	sspace((lsp-1)*vls);
}

/*
 * Justify the current line.
 */
justify()
{
	register CODE *cp;
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
		for (cp=linebuf; cp<llinptr; cp++)
			if (cp->c_code == DPADC)
				r++;
		n = lln - llinsiz;
		t = n%r;
		n = 1 + n/r;
		r = t;
		for (cp=linebuf; cp<llinptr; cp++) {
			if (cp->c_code != DPADC)
				continue;
			if (r-- == 0)
				--n;
			cp->c_iarg += n;
		}
		return;
	}
	linebuf[0].c_iarg += n;
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
	int n, f, w, d2;
	register int d, d1;
	register int pos;

	tp = ctabptr;
	pos = tp->t_pos + tin;		/* relative to indent */
	switch (tp->t_jus) {
	case LJUS:
		d = pos - tlinsiz;
		/*
		tlinptr->c_iarg -= d;
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
		llinptr->c_iarg += mws;
		return;
	}
	if ((f=asctab[ltabchr]) == 0) {			/* Blank tabs	*/
		tlinptr->c_iarg += d;
		llinptr->c_iarg -= llinptr->c_csp;
		llinptr->c_csp = 0;
	} else {					/* Leader dots... */
		if ((w = ldrspc) == 0) {
			w = fonwidt[f];
			w = unit(swdmul*w*psz, swddiv);
		}
		if ((n=nlinptr-(tlinptr+1)) > WORSIZE)
			panic("Word buffer overflow");
		copystr(wordbuf, tlinptr+1, sizeof (CODE), n);
		nlinptr = tlinptr + 1;
		if ((d2=(d1=tlinsiz)%w) != 0) {
			d1 += w-d2;
			tlinptr->c_iarg += w-d2;
		}
		d2 = tlinsiz + d;
		while ((d1+=w) <= d2)
			addchar(f, w);
		addidir(DHMOV, d2-(d1-w));
		if (nlinptr+n >= &linebuf[LINSIZE])
			panic("Line buffer overflow");
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
		tlinptr->c_iarg += d = tp->t_pos+tin - tlinsiz;
		nlinsiz = llinsiz += d;
	}
	*/
	ctabptr = tp;
	ltabchr = c;
}

/*
 * Initialise line data for a new line.
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
