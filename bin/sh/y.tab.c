
#line 8 "sh.y"

#include "sh.h"

#define YYERROR	{yyerrflag=1; goto YYerract; }

extern	NODE	*node();

#include "y.tab.h"
#define YYCLEARIN yychar = -1000
#define YYERROK yyerrflag = 0
extern int yychar;
extern short yyerrflag;
#ifndef YYMAXDEPTH
#define YYMAXDEPTH 150
#endif
YYSTYPE yyval, yylval;

#line 321 "sh.y"

/*
 * Create a node.
 */
NODE *
node(type, auxp, next)
NODE *auxp, *next;
{
	register NODE *np;

	np = (NODE *) balloc(sizeof (NODE));
	np->n_type = type;
	np->n_auxp = auxp;
	np->n_next = next;
	return (np);
}

#define NBPC 8
#define NKEY 8
static char keys[NKEY] = { 0 };
static int  keyi = NKEY * NBPC;

keyflush()
{
	register char *kp;

	for (kp = keys+NKEY; kp > keys; *--kp = 0);
	keyi = NKEY * NBPC;
}

keypop()
{
	register char	*kp;
	register int	km;

	if ((km = keyi++) >= NKEY * NBPC) {
		panic();
		NOTREACHED;
	}
	kp = keys + (km / NBPC);
	km = 1 << (km %= NBPC);
	keyflag = (*kp & km) ? 1 : 0;
	*kp &= ~km;
}

keypush()
{
	register char	*kp;
	register int	km;

	if ((km = --keyi) < 0) {
		panic();
		NOTREACHED;
	}
	if (keyflag) {
		kp = keys + (km / NBPC);
		km = 1 << (km %= NBPC);
		*kp |= km;
	}
}
/*
 * The following fragments might implement named pipes.
 * The token declaration goes in the header.
 * The nopen production should go with the others of its ilk.
 * The production fragment goes into argument:
%token _NOPEN _NCLOSE
nopen:	_NOPEN optnls ;

|	nopen pipe_command ')' {
		$$ = node(NRPIPE, $2, NULL);
	}
|	oparen pipe_command _NCLOSE {
		$$ = node(NWPIPE, $2, NULL);
	}
 *
 */
#ifdef YYTNAMES
struct yytname yytnames[31] =
{
	"$end", -1, 
	"error", -2, 
	"_NULL", 256, 
	"_DSEMI", 257, 
	"_ANDF", 258, 
	"_ORF", 259, 
	"_NAME", 260, 
	"_IORS", 261, 
	"_ASGN", 262, 
	"_CASE", 263, 
	"_DO", 264, 
	"_DONE", 265, 
	"_ELIF", 266, 
	"_ELSE", 267, 
	"_ESAC", 268, 
	"_FI", 269, 
	"_FOR", 270, 
	"_IF", 271, 
	"_IN", 272, 
	"_THEN", 273, 
	"_UNTIL", 274, 
	"_WHILE", 275, 
	"_OBRAC", 276, 
	"_CBRAC", 277, 
	"'\\n'", 10, 
	"';'", 59, 
	"'|'", 124, 
	"'('", 40, 
	"')'", 41, 
	"'&'", 38, 
	NULL
} ;
#endif
unsigned yypdnt[76] = {
00, 01, 01, 02, 02, 02, 04, 06, 
07, 010, 011, 011, 012, 012, 013, 013, 
015, 016, 017, 020, 021, 022, 023, 03, 
03, 03, 03, 03, 024, 024, 024, 025, 
025, 026, 027, 030, 030, 031, 031, 031, 
031, 032, 033, 034, 035, 035, 035, 035, 
035, 035, 036, 036, 044, 044, 040, 040, 
040, 045, 046, 046, 037, 037, 043, 043, 
043, 042, 042, 041, 041, 014, 014, 014, 
05, 05, 047, 047  
} ;
unsigned yypn[76] = {
02, 02, 00, 01, 02, 02, 02, 02, 
02, 02, 02, 02, 02, 03, 01, 02, 
02, 02, 02, 02, 02, 02, 02, 01, 
02, 02, 03, 03, 01, 03, 03, 03, 
01, 02, 00, 02, 01, 01, 01, 01, 
01, 01, 01, 01, 06, 05, 04, 06, 
03, 03, 02, 00, 02, 00, 03, 01, 
00, 03, 03, 01, 02, 00, 05, 02, 
00, 01, 00, 03, 02, 01, 01, 02, 
01, 00, 01, 02  
} ;
unsigned yypgo[40] = {
00, 00, 02, 04, 014, 016, 056, 062, 
064, 066, 070, 072, 074, 0100, 0102, 0104, 
0106, 0110, 0112, 0114, 0116, 0124, 0130, 0132, 
0134, 0140, 0142, 0144, 0162, 0164, 0166, 0170, 
0174, 0200, 0212, 0230, 0234, 0240, 0242, 0246  
} ;
unsigned yygo[176] = {
0176030, 01, 0176030, 05, 01, 06, 015, 051, 
016, 052, 0176030, 072, 0176030, 037, 020, 056, 
023, 061, 032, 065, 033, 066, 034, 067, 
035, 070, 036, 071, 072, 0106, 0110, 0132, 
0112, 0135, 0134, 0156, 0143, 0160, 0145, 0162, 
0151, 0165, 0152, 0166, 0176030, 054, 0167, 0172, 
0176030, 0111, 0176030, 0153, 0176030, 0154, 0176030, 040, 
0176030, 0113, 0176030, 0103, 0105, 0130, 0176030, 0120, 
0176030, 021, 0176030, 022, 0176030, 024, 0176030, 041, 
0176030, 042, 0176030, 0146, 0176030, 0144, 021, 057, 
022, 060, 0176030, 07, 024, 062, 0176030, 010, 
0176030, 011, 0176030, 012, 044, 0100, 0176030, 043, 
0176030, 044, 0176030, 045, 012, 046, 030, 063, 
031, 064, 044, 046, 0104, 0126, 0126, 0126, 
0176030, 0122, 0176030, 047, 0176030, 050, 0176030, 0105, 
0130, 0150, 0176030, 0114, 0144, 0161, 0176030, 0123, 
037, 073, 040, 074, 0107, 0131, 0153, 0167, 
0176030, 075, 042, 077, 0111, 0133, 0113, 0136, 
0146, 0163, 0154, 0170, 0172, 0173, 0176030, 076, 
0173, 0174, 0176030, 0155, 0126, 0147, 0176030, 0127, 
0176030, 0124, 0141, 0157, 0176030, 0125, 072, 0107, 
0102, 0121, 0105, 0121, 0117, 0140, 0176030, 055  
} ;
unsigned yypa[125] = {
00, 02, 034, 040, 044, 046, 050, 054, 
062, 070, 074, 0122, 0124, 0126, 0154, 0202, 
0202, 0206, 0206, 0202, 0210, 0212, 0214, 0216, 
0220, 0220, 0202, 0202, 0202, 0202, 0202, 0210, 
0210, 0224, 0230, 0234, 0236, 0264, 0266, 0270, 
0272, 0274, 0276, 0300, 0302, 0304, 0310, 0312, 
0314, 0316, 0320, 0322, 0326, 0332, 0334, 0336, 
0340, 0342, 0202, 0344, 0350, 0354, 0356, 0362, 
0366, 0370, 0372, 0400, 0404, 0410, 0416, 0420, 
0202, 0450, 0460, 0466, 0472, 0476, 0500, 0502, 
0506, 0510, 0514, 0520, 0524, 0530, 0404, 0534, 
0350, 0536, 0540, 0542, 0202, 0550, 0552, 0554, 
0556, 0562, 0566, 0202, 0400, 0202, 0570, 0576, 
0600, 0202, 0202, 0210, 0604, 0610, 0614, 0616, 
0620, 0622, 0624, 0626, 0630, 0632, 0634, 0344, 
0636, 0640, 0450, 0642, 0650  
} ;
unsigned yyact[426] = {
020002, 0176030, 02, 0177777, 04, 012, 020042, 0404, 
020042, 0405, 020042, 0406, 020042, 0407, 020042, 0416, 
020042, 0417, 020042, 0422, 020042, 0423, 020042, 0424, 
020042, 050, 03, 0176030, 040000, 0177777, 060000, 0176030, 
013, 012, 060000, 0176030, 020003, 0176030, 020001, 0176030, 
014, 012, 060000, 0176030, 015, 073, 016, 046, 
020027, 0176030, 017, 0402, 020, 0403, 020034, 0176030, 
023, 0174, 020040, 0176030, 025, 0404, 026, 0405, 
027, 0406, 030, 0407, 031, 0416, 032, 0417, 
033, 0422, 034, 0423, 035, 0424, 036, 050, 
060000, 0176030, 020005, 0176030, 020004, 0176030, 020042, 0404, 
020042, 0405, 020042, 0406, 020042, 0407, 020042, 0416, 
020042, 0417, 020042, 0422, 020042, 0423, 020042, 0424, 
020042, 050, 020031, 0176030, 020042, 0404, 020042, 0405, 
020042, 0406, 020042, 0407, 020042, 0416, 020042, 0417, 
020042, 0422, 020042, 0423, 020042, 0424, 020042, 050, 
020030, 0176030, 053, 012, 020111, 0176030, 020042, 0176030, 
020042, 0176030, 020052, 0176030, 020051, 0176030, 020053, 0176030, 
025, 0404, 060000, 0176030, 020102, 051, 020042, 0176030, 
020102, 0425, 020042, 0176030, 020041, 0176030, 025, 0404, 
026, 0405, 027, 0406, 030, 0407, 031, 0416, 
032, 0417, 033, 0422, 034, 0423, 035, 0424, 
036, 050, 020044, 0176030, 020045, 0176030, 020046, 0176030, 
020047, 0176030, 020050, 0176030, 020033, 0176030, 020032, 0176030, 
020112, 0176030, 020021, 0176030, 0101, 012, 020110, 0176030, 
020020, 0176030, 020035, 0176030, 020036, 0176030, 020022, 0176030, 
020037, 0176030, 0102, 0420, 060000, 0176030, 0104, 0420, 
020063, 0176030, 020006, 0176030, 020013, 0176030, 020012, 0176030, 
020024, 0176030, 020023, 0176030, 0110, 0421, 060000, 0176030, 
0112, 0410, 020075, 0176030, 020101, 0176030, 0115, 051, 
060000, 0176030, 0116, 0425, 060000, 0176030, 020043, 0176030, 
020113, 0176030, 053, 012, 0117, 073, 020016, 0176030, 
025, 0404, 020070, 0176030, 025, 0404, 020065, 0176030, 
053, 012, 0117, 073, 060000, 0176030, 020104, 0176030, 
0101, 012, 020110, 0401, 020110, 0410, 020110, 0411, 
020110, 0412, 020110, 0413, 020110, 0414, 020110, 0415, 
020110, 0421, 020110, 0425, 020110, 051, 020042, 0176030, 
020102, 0412, 020102, 0413, 020102, 0415, 020042, 0176030, 
053, 012, 0134, 073, 020111, 0176030, 020102, 0411, 
020042, 0176030, 0137, 0411, 060000, 0176030, 020060, 0176030, 
020061, 0176030, 053, 012, 020106, 0176030, 020017, 0176030, 
0101, 012, 020105, 0176030, 0141, 0174, 020073, 0176030, 
0142, 0414, 060000, 0176030, 0143, 0401, 020067, 0176030, 
0145, 051, 060000, 0176030, 020062, 0176030, 020103, 0176030, 
020007, 0176030, 0151, 0412, 0152, 0413, 020100, 0176030, 
020014, 0176030, 020074, 0176030, 020056, 0176030, 0101, 012, 
020107, 0176030, 025, 0404, 060000, 0176030, 020055, 0176030, 
020102, 0401, 020102, 0414, 020042, 0176030, 020064, 0176030, 
0164, 0411, 060000, 0176030, 020102, 0415, 020042, 0176030, 
0171, 0415, 060000, 0176030, 020015, 0176030, 020072, 0176030, 
020026, 0176030, 020066, 0176030, 020025, 0176030, 020071, 0176030, 
020054, 0176030, 020010, 0176030, 020011, 0176030, 020077, 0176030, 
020057, 0176030, 0151, 0412, 0152, 0413, 020100, 0176030, 
020076, 0176030  
} ;
/* (-lgl
 * 	COHERENT Version 3.0
 * 	Copyright (c) 1982, 1990 by Mark Williams Company.
 * 	All rights reserved. May not be copied without permission.
 -lgl) */
#include "action.h"
#define YYNOCHAR (-1000)
#define	yyerrok	yyerrflag=0
#define	yyclearin	yylval=YYNOCHAR
int yystack[YYMAXDEPTH];
YYSTYPE yyvstack[YYMAXDEPTH], *yyv;
int yychar;

#ifdef YYDEBUG
int yydebug = 1;	/* No sir, not in the BSS */
#include <stdio.h>
#endif

short yyerrflag;
int *yys;

yyparse()
{
	register YYSTYPE *yypvt;
	int act;
	register unsigned *ip, yystate;
	int pno;
	yystate = 0;
	yychar = YYNOCHAR;
	yyv = &yyvstack[-1];
	yys = &yystack[-1];

stack:
	if( ++yys >= &yystack[YYMAXDEPTH] ) {
		write(2, "Stack overflow\n", 15);
		exit(1);
	}
	*yys = yystate;
	*++yyv = yyval;
#ifdef YYDEBUG
	if( yydebug )
		fprintf(stdout, "Stack state %d, char %d\n", yystate, yychar);
#endif

read:
	ip = &yyact[yypa[yystate]];
	if( ip[1] != YYNOCHAR ) {
		if( yychar == YYNOCHAR ) {
			yychar = yylex();
#ifdef YYDEBUG
			if( yydebug )
				fprintf(stdout, "lex read char %d, val %d\n", yychar, yylval);
#endif
		}
		while (ip[1]!=YYNOCHAR) {
			if (ip[1]==yychar)
				break;
			ip += 2;
		}
	}
	act = ip[0];
	switch( act>>YYACTSH ) {
	case YYSHIFTACT:
		if( ip[1]==YYNOCHAR )
			goto YYerract;
		if( yychar != -1 )
			yychar = YYNOCHAR; /* dont throw away EOF */
		yystate = act&YYAMASK;
		yyval = yylval;
#ifdef YYDEBUG
		if( yydebug )
			fprintf(stdout, "shift %d\n", yystate);
#endif
		if( yyerrflag )
			--yyerrflag;
		goto stack;

	case YYACCEPTACT:
#ifdef YYDEBUG
		if( yydebug )
			fprintf(stdout, "accept\n");
#endif
		return(0);

	case YYERRACT:
	YYerract:
		switch (yyerrflag) {
		case 0:
			yyerror("Syntax error");

		case 1:
		case 2:

			yyerrflag = 3;
			while( yys >= & yystack[0] ) {
				ip = &yyact[yypa[*yys]];
				while( ip[1]!=YYNOCHAR )
					ip += 2;
				if( (*ip&~YYAMASK) == (YYSHIFTACT<<YYACTSH) ) {
					yystate = *ip&YYAMASK;
					goto stack;
				}
#ifdef YYDEBUG
				if( yydebug )
					fprintf(stderr, "error recovery leaves state %d, uncovers %d\n", *yys, yys[-1]);
#endif
				yys--;
				yyv--;
			}
#ifdef YYDEBUG
			if( yydebug )
				fprintf(stderr, "no shift on error; abort\n");
#endif
			return(1);

		case 3:
#ifdef YYDEBUG
			if( yydebug )
				fprintf(stderr, "Error recovery clobbers char %o\n", yychar);
#endif
			if( yychar==YYEOFVAL )
				return(1);
			yychar = YYNOCHAR;
			goto read;
		}

	case YYREDACT:
		pno = act&YYAMASK;
#ifdef YYDEBUG
		if( yydebug )
			fprintf(stdout, "reduce %d\n", pno);
#endif
		yypvt = yyv;
		yyv -= yypn[pno];
		yys -= yypn[pno];
		yyval = yyv[1];
		switch(pno) {

case 3: {

#line 46 "sh.y"

		sesp->s_node = NULL;
		reset(RCMD);
		NOTREACHED;
	}break;

case 4: {

#line 52 "sh.y"

		sesp->s_node = yypvt[-1].yu_node;
		reset(errflag ? RERR : RCMD);
		NOTREACHED;
	}break;

case 5: {

#line 57 "sh.y"

		keyflush();
		keyflag = 1;
		reset(RERR);
		NOTREACHED;
	}break;

case 10: {

#line 73 "sh.y"
	yyval.yu_nval = NWHILE;	}break;

case 11: {

#line 74 "sh.y"
	yyval.yu_nval = NUNTIL;	}break;

case 23: {

#line 96 "sh.y"

		yyval.yu_node = yypvt[0].yu_node;
	}break;

case 24: {

#line 99 "sh.y"

		yyval.yu_node = node(NBACK, yypvt[-1].yu_node, NULL);
	}break;

case 25: {

#line 102 "sh.y"

		yyval.yu_node = yypvt[-1].yu_node;
	}break;

case 26: {

#line 105 "sh.y"

		yyval.yu_node = node(NBACK, yypvt[-2].yu_node, yypvt[0].yu_node);
	}break;

case 27: {

#line 108 "sh.y"

		yyval.yu_node = node(NLIST, yypvt[-2].yu_node, yypvt[0].yu_node);
	}break;

case 28: {

#line 114 "sh.y"

		yyval.yu_node = yypvt[0].yu_node;
	}break;

case 29: {

#line 117 "sh.y"

		yyval.yu_node = node(NORF, yypvt[-2].yu_node, yypvt[0].yu_node);
	}break;

case 30: {

#line 120 "sh.y"

		yyval.yu_node = node(NANDF, yypvt[-2].yu_node, yypvt[0].yu_node);
	}break;

case 31: {

#line 126 "sh.y"

		yyval.yu_node = node(NPIPE, yypvt[-2].yu_node, yypvt[0].yu_node);
	}break;

case 32: {

#line 129 "sh.y"

		yyval.yu_node = yypvt[0].yu_node;
	}break;

case 33: {

#line 135 "sh.y"

		yyval.yu_node = node(NCOMS, yypvt[0].yu_node, NULL);
		keypop();
	}break;

case 34: {

#line 142 "sh.y"

		keypush();
		keyflag = 1;
	}break;

case 35: {

#line 149 "sh.y"

		if ((yypvt[-1].yu_node->n_type == NCTRL && yypvt[0].yu_node->n_type == NARGS)
		 || (yypvt[-1].yu_node->n_type == NARGS && yypvt[0].yu_node->n_type == NCTRL)) {
			YYERROR;
		}
		(yyval.yu_node = yypvt[-1].yu_node)->n_next = yypvt[0].yu_node;
	}break;

case 36: {

#line 156 "sh.y"

		yyval.yu_node = yypvt[0].yu_node;
	}break;

case 37: {

#line 162 "sh.y"

		yyval.yu_node = node(NIORS, yypvt[0].yu_strp, NULL);
	}break;

case 38: {

#line 165 "sh.y"

		yyval.yu_node = node(NARGS, yypvt[0].yu_strp, NULL);
		keyflag = 0;
	}break;

case 39: {

#line 169 "sh.y"

		yyval.yu_node = node(NASSG, yypvt[0].yu_strp, NULL);
	}break;

case 40: {

#line 172 "sh.y"

		if ( ! keyflag) {
			YYERROR;
		}
		yyval.yu_node = node(NCTRL, yypvt[0].yu_node, NULL);
		keyflag = 0;
	}break;

case 41: {

#line 181 "sh.y"

		yyval.yu_strp = duplstr(strt, 0);
	}break;

case 42: {

#line 186 "sh.y"

		yyval.yu_strp = duplstr(strt, 0);
	}break;

case 43: {

#line 191 "sh.y"

		yyval.yu_strp = duplstr(strt, 0);
	}break;

case 44: {

#line 197 "sh.y"

		yyval.yu_node = node(NFOR, yypvt[-4].yu_strp, node(NFOR2, yypvt[-3].yu_node, node(NLIST, yypvt[-1].yu_node, NULL)));
		yyval.yu_node->n_next->n_next->n_next = yyval.yu_node->n_next;
	}break;

case 45: {

#line 201 "sh.y"

		yyval.yu_node = node(NCASE, yypvt[-3].yu_strp, yypvt[-1].yu_node);
	}break;

case 46: {

#line 204 "sh.y"

		yyval.yu_node = node(yypvt[-3].yu_nval, yypvt[-2].yu_node, node(NLIST, yypvt[-1].yu_node, NULL));
		yyval.yu_node->n_next->n_next = yyval.yu_node;
	}break;

case 47: {

#line 208 "sh.y"

		yyval.yu_node = node(NIF, node(NNULL, yypvt[-4].yu_node, yypvt[-2].yu_node), yypvt[-1].yu_node);
	}break;

case 48: {

#line 211 "sh.y"

		yyval.yu_node = node(NPARN, yypvt[-1].yu_node, NULL);
	}break;

case 49: {

#line 214 "sh.y"

		yyval.yu_node = node(NBRAC, yypvt[-1].yu_node, NULL);
	}break;

case 50: {

#line 220 "sh.y"

		yyval.yu_node = yypvt[0].yu_node;
	}break;

case 51: {

#line 223 "sh.y"

		yyval.yu_node = node(NARGS, "\"$@\"", NULL);
	}break;

case 52: {

#line 229 "sh.y"

		yyval.yu_node = node(NARGS, yypvt[-1].yu_strp, yypvt[0].yu_node);
	}break;

case 53: {

#line 232 "sh.y"

		yyval.yu_node = NULL;
	}break;

case 54: {

#line 238 "sh.y"

		register NODE *np;

		for (np=yypvt[-2].yu_node; np->n_next; np=np->n_next);
		np->n_next = yypvt[0].yu_node;
		yyval.yu_node = yypvt[-2].yu_node;
	}break;

case 55: {

#line 245 "sh.y"

		yyval.yu_node = yypvt[0].yu_node;
	}break;

case 56: {

#line 248 "sh.y"

		yyval.yu_node = NULL;
	}break;

case 57: {

#line 254 "sh.y"

		yyval.yu_node = node(NCASE2, yypvt[0].yu_node, yypvt[-2].yu_node);
	}break;

case 58: {

#line 260 "sh.y"

		yyval.yu_node = node(NCASE3, yypvt[-2].yu_strp, yypvt[0].yu_node);
	}break;

case 59: {

#line 263 "sh.y"

		yyval.yu_node = node(NCASE3, yypvt[0].yu_strp, NULL);
	}break;

case 60: {

#line 269 "sh.y"

		yyval.yu_node = yypvt[0].yu_node;
	}break;

case 61: {

#line 272 "sh.y"

		yyval.yu_node = NULL;
	}break;

case 62: {

#line 278 "sh.y"

		yyval.yu_node = node(NIF, node(NNULL, yypvt[-3].yu_node, yypvt[-1].yu_node), yypvt[0].yu_node);
	}break;

case 63: {

#line 281 "sh.y"

		yyval.yu_node = node(NELSE, yypvt[0].yu_node, NULL);
	}break;

case 64: {

#line 284 "sh.y"

		yyval.yu_node = NULL;
	}break;

case 65: {

#line 290 "sh.y"

		yyval.yu_node = yypvt[0].yu_node;
	}break;

case 66: {

#line 294 "sh.y"

		yyval.yu_node = NULL;
	}break;

case 67: {

#line 300 "sh.y"

		yyval.yu_node = node(NLIST, yypvt[-2].yu_node, yypvt[0].yu_node);
	}break;

case 68: {

#line 303 "sh.y"

		yyval.yu_node = yypvt[-1].yu_node;
	}break;

		}
		ip = &yygo[ yypgo[yypdnt[pno]] ];
		while( *ip!=*yys && *ip!=YYNOCHAR )
			ip += 2;
		yystate = ip[1];
		goto stack;
	}
}




