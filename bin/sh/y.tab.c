
#line 8 "/tmp/nsh/sh.y"

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

#line 328 "/tmp/nsh/sh.y"

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
#include <action.h>
unsigned char yypdnt[78] = {
0, 1, 1, 2, 2, 2, 4, 6,
7, 8, 9, 9, 10, 10, 11, 11,
13, 14, 15, 16, 17, 18, 19, 3,
3, 3, 3, 3, 20, 20, 20, 21,
21, 22, 23, 24, 24, 25, 25, 25,
25, 26, 27, 28, 29, 29, 29, 29,
29, 29, 29, 29, 30, 30, 36, 36,
32, 32, 32, 37, 38, 38, 31, 31,
35, 35, 35, 34, 34, 33, 33, 12,
12, 12, 5, 5, 39, 39 
};
unsigned char yypn[78] = {
2, 2, 0, 1, 2, 2, 2, 2,
2, 2, 2, 2, 2, 3, 1, 2,
2, 2, 2, 2, 2, 2, 2, 1,
2, 2, 3, 3, 1, 3, 3, 3,
1, 2, 0, 2, 1, 1, 1, 1,
1, 1, 1, 1, 6, 5, 6, 5,
4, 6, 3, 3, 2, 0, 2, 0,
3, 1, 0, 3, 3, 1, 2, 0,
5, 2, 0, 1, 0, 3, 2, 1,
1, 2, 1, 0, 1, 2 
};
unsigned char yypgo[40] = {
0, 0, 2, 4, 12, 14, 46, 50,
52, 54, 56, 58, 62, 68, 70, 72,
74, 76, 78, 80, 82, 88, 92, 94,
96, 100, 102, 104, 118, 120, 122, 124,
130, 136, 146, 160, 164, 168, 170, 174
};
unsigned int yygo[186] = {
-1000, 1, -1000, 5, 1, 6, 13, 41,
14, 42, -1000, 58, -1000, 31, 16, 46,
19, 49, 26, 53, 27, 54, 28, 55,
29, 56, 30, 57, 58, 73, 75, 94,
77, 97, 96, 115, 102, 117, 104, 119,
110, 123, 111, 124, -1000, 44, 125, 128,
-1000, 76, -1000, 112, -1000, 113, -1000, 32,
-1000, 78, 69, 88, -1000, 68, 66, 82,
72, 91, -1000, 69, -1000, 17, -1000, 18,
-1000, 20, -1000, 33, -1000, 34, -1000, 105,
-1000, 103, 17, 47, 18, 48, -1000, 7,
20, 50, -1000, 8, -1000, 9, -1000, 10,
36, 64, -1000, 35, -1000, 36, -1000, 37,
10, 38, 24, 51, 25, 52, 36, 38,
71, 89, 89, 89, -1000, 84, -1000, 39,
-1000, 40, -1000, 72, 72, 92, 91, 108,
-1000, 79, 88, 106, 103, 118, -1000, 85,
31, 59, 32, 60, 74, 93, 112, 125,
-1000, 61, 34, 63, 76, 95, 78, 98,
105, 120, 113, 126, 128, 129, -1000, 62,
129, 130, -1000, 114, 89, 107, -1000, 90,
-1000, 86, 100, 116, -1000, 87, 51, 70,
58, 74, 66, 70, 67, 83, 72, 70,
-1000, 45 
};
unsigned short yypa[131] = {
0, 2, 28, 32, 36, 38, 40, 44,
50, 56, 60, 82, 84, 86, 108, 130,
130, 134, 134, 130, 136, 138, 140, 142,
144, 144, 130, 130, 130, 130, 130, 136,
136, 148, 152, 156, 158, 180, 182, 184,
186, 188, 190, 192, 194, 196, 200, 202,
204, 206, 208, 210, 218, 222, 224, 226,
228, 230, 130, 232, 236, 240, 242, 246,
250, 252, 254, 260, 264, 268, 272, 276,
280, 288, 290, 130, 314, 322, 328, 332,
336, 338, 340, 342, 346, 350, 354, 358,
264, 276, 362, 236, 364, 368, 370, 372,
130, 378, 380, 382, 384, 388, 130, 264,
130, 390, 396, 400, 402, 406, 130, 130,
136, 408, 412, 416, 418, 420, 422, 424,
426, 428, 430, 432, 434, 232, 436, 438,
314, 440, 446 
};
unsigned int yyact[448] = {
8194, -1000, 2, -1, 4, 10, 8226, 260,
8226, 261, 8226, 262, 8226, 263, 8226, 270,
8226, 271, 8226, 274, 8226, 275, 8226, 276,
8226, 40, 3, -1000, 16384, -1, 24576, -1000,
11, 10, 24576, -1000, 8195, -1000, 8193, -1000,
12, 10, 24576, -1000, 13, 59, 14, 38,
8215, -1000, 15, 258, 16, 259, 8220, -1000,
19, 124, 8224, -1000, 21, 260, 22, 261,
23, 262, 24, 263, 25, 270, 26, 271,
27, 274, 28, 275, 29, 276, 30, 40,
24576, -1000, 8197, -1000, 8196, -1000, 8226, 260,
8226, 261, 8226, 262, 8226, 263, 8226, 270,
8226, 271, 8226, 274, 8226, 275, 8226, 276,
8226, 40, 8217, -1000, 8226, 260, 8226, 261,
8226, 262, 8226, 263, 8226, 270, 8226, 271,
8226, 274, 8226, 275, 8226, 276, 8226, 40,
8216, -1000, 43, 10, 8267, -1000, 8226, -1000,
8226, -1000, 8234, -1000, 8233, -1000, 8235, -1000,
21, 260, 24576, -1000, 8260, 41, 8226, -1000,
8260, 277, 8226, -1000, 8225, -1000, 21, 260,
22, 261, 23, 262, 24, 263, 25, 270,
26, 271, 27, 274, 28, 275, 29, 276,
30, 40, 8228, -1000, 8229, -1000, 8230, -1000,
8231, -1000, 8232, -1000, 8219, -1000, 8218, -1000,
8268, -1000, 8209, -1000, 65, 10, 8266, -1000,
8208, -1000, 8221, -1000, 8222, -1000, 8210, -1000,
8223, -1000, 66, 272, 43, 10, 67, 59,
24576, -1000, 71, 272, 8245, -1000, 8198, -1000,
8203, -1000, 8202, -1000, 8212, -1000, 8211, -1000,
75, 273, 24576, -1000, 77, 264, 8255, -1000,
8259, -1000, 80, 41, 24576, -1000, 81, 277,
24576, -1000, 8227, -1000, 8269, -1000, 43, 10,
67, 59, 8206, -1000, 43, 10, 8264, -1000,
21, 260, 8250, -1000, 66, 272, 24576, -1000,
65, 10, 8263, -1000, 21, 260, 8247, -1000,
77, 264, 43, 10, 67, 59, 8255, -1000,
8262, -1000, 65, 10, 8266, 257, 8266, 264,
8266, 265, 8266, 266, 8266, 267, 8266, 268,
8266, 269, 8266, 273, 8266, 277, 8266, 41,
8226, -1000, 8260, 266, 8260, 267, 8260, 269,
8226, -1000, 43, 10, 96, 59, 8267, -1000,
8260, 265, 8226, -1000, 99, 265, 24576, -1000,
8242, -1000, 8243, -1000, 8207, -1000, 65, 10,
8265, -1000, 100, 124, 8253, -1000, 101, 268,
24576, -1000, 102, 257, 8249, -1000, 104, 41,
24576, -1000, 8244, -1000, 109, 265, 24576, -1000,
8261, -1000, 8199, -1000, 110, 266, 111, 267,
8258, -1000, 8204, -1000, 8254, -1000, 8240, -1000,
21, 260, 24576, -1000, 8239, -1000, 8260, 257,
8260, 268, 8226, -1000, 121, 268, 24576, -1000,
8246, -1000, 122, 265, 24576, -1000, 8237, -1000,
8260, 269, 8226, -1000, 127, 269, 24576, -1000,
8205, -1000, 8252, -1000, 8214, -1000, 8248, -1000,
8213, -1000, 8251, -1000, 8238, -1000, 8236, -1000,
8200, -1000, 8201, -1000, 8257, -1000, 8241, -1000,
110, 266, 111, 267, 8258, -1000, 8256, -1000
};
/* (-lgl
 * 	COHERENT Version 3.2.2
 * 	Copyright (c) 1982, 1992 by Mark Williams Company.
 * 	All rights reserved. May not be copied without permission.
 -lgl) */
/*
 * /lib/yyparse.c
 */

#define	YYNOCHAR	(-1000)
#define	yyerrok		yyerrflag=0
#define	yyclearin	yylval=YYNOCHAR

int	yychar;
short	yyerrflag;
int	*yys;
int	yystack[YYMAXDEPTH];
YYSTYPE	yyvstack[YYMAXDEPTH];
YYSTYPE	*yyv;

#ifdef	YYDEBUG
int	yydebug = 1;	/* No sir, not in the BSS */
#include <stdio.h>
#endif

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

#line 46 "/tmp/nsh/sh.y"

		sesp->s_node = NULL;
		reset(RCMD);
		NOTREACHED;
	}break;

case 4: {

#line 52 "/tmp/nsh/sh.y"

		sesp->s_node = yypvt[-1].yu_node;
		reset(errflag ? RERR : RCMD);
		NOTREACHED;
	}break;

case 5: {

#line 57 "/tmp/nsh/sh.y"

		keyflush();
		keyflag = 1;
		reset(RERR);
		NOTREACHED;
	}break;

case 10: {

#line 73 "/tmp/nsh/sh.y"
	yyval.yu_nval = NWHILE;	}break;

case 11: {

#line 74 "/tmp/nsh/sh.y"
	yyval.yu_nval = NUNTIL;	}break;

case 23: {

#line 96 "/tmp/nsh/sh.y"

		yyval.yu_node = yypvt[0].yu_node;
	}break;

case 24: {

#line 99 "/tmp/nsh/sh.y"

		yyval.yu_node = node(NBACK, yypvt[-1].yu_node, NULL);
	}break;

case 25: {

#line 102 "/tmp/nsh/sh.y"

		yyval.yu_node = yypvt[-1].yu_node;
	}break;

case 26: {

#line 105 "/tmp/nsh/sh.y"

		yyval.yu_node = node(NBACK, yypvt[-2].yu_node, yypvt[0].yu_node);
	}break;

case 27: {

#line 108 "/tmp/nsh/sh.y"

		yyval.yu_node = node(NLIST, yypvt[-2].yu_node, yypvt[0].yu_node);
	}break;

case 28: {

#line 114 "/tmp/nsh/sh.y"

		yyval.yu_node = yypvt[0].yu_node;
	}break;

case 29: {

#line 117 "/tmp/nsh/sh.y"

		yyval.yu_node = node(NORF, yypvt[-2].yu_node, yypvt[0].yu_node);
	}break;

case 30: {

#line 120 "/tmp/nsh/sh.y"

		yyval.yu_node = node(NANDF, yypvt[-2].yu_node, yypvt[0].yu_node);
	}break;

case 31: {

#line 126 "/tmp/nsh/sh.y"

		yyval.yu_node = node(NPIPE, yypvt[-2].yu_node, yypvt[0].yu_node);
	}break;

case 32: {

#line 129 "/tmp/nsh/sh.y"

		yyval.yu_node = yypvt[0].yu_node;
	}break;

case 33: {

#line 135 "/tmp/nsh/sh.y"

		yyval.yu_node = node(NCOMS, yypvt[0].yu_node, NULL);
		keypop();
	}break;

case 34: {

#line 142 "/tmp/nsh/sh.y"

		keypush();
		keyflag = 1;
	}break;

case 35: {

#line 149 "/tmp/nsh/sh.y"

		if ((yypvt[-1].yu_node->n_type == NCTRL && yypvt[0].yu_node->n_type == NARGS)
		 || (yypvt[-1].yu_node->n_type == NARGS && yypvt[0].yu_node->n_type == NCTRL)) {
			YYERROR;
		}
		(yyval.yu_node = yypvt[-1].yu_node)->n_next = yypvt[0].yu_node;
	}break;

case 36: {

#line 156 "/tmp/nsh/sh.y"

		yyval.yu_node = yypvt[0].yu_node;
	}break;

case 37: {

#line 162 "/tmp/nsh/sh.y"

		yyval.yu_node = node(NIORS, yypvt[0].yu_strp, NULL);
	}break;

case 38: {

#line 165 "/tmp/nsh/sh.y"

		yyval.yu_node = node(NARGS, yypvt[0].yu_strp, NULL);
		keyflag = 0;
	}break;

case 39: {

#line 169 "/tmp/nsh/sh.y"

		yyval.yu_node = node(NASSG, yypvt[0].yu_strp, NULL);
	}break;

case 40: {

#line 172 "/tmp/nsh/sh.y"

		if ( ! keyflag) {
			YYERROR;
		}
		yyval.yu_node = node(NCTRL, yypvt[0].yu_node, NULL);
		keyflag = 0;
	}break;

case 41: {

#line 181 "/tmp/nsh/sh.y"

		yyval.yu_strp = duplstr(strt, 0);
	}break;

case 42: {

#line 186 "/tmp/nsh/sh.y"

		yyval.yu_strp = duplstr(strt, 0);
	}break;

case 43: {

#line 191 "/tmp/nsh/sh.y"

		yyval.yu_strp = duplstr(strt, 0);
	}break;

case 44: {

#line 197 "/tmp/nsh/sh.y"

		yyval.yu_node = node(NFOR, yypvt[-4].yu_strp, node(NFOR2, yypvt[-3].yu_node, node(NLIST, yypvt[-1].yu_node, NULL)));
		yyval.yu_node->n_next->n_next->n_next = yyval.yu_node->n_next;
	}break;

case 45: {

#line 201 "/tmp/nsh/sh.y"

		yyval.yu_node = node(NFOR, yypvt[-3].yu_strp, node(NFOR2, yypvt[-2].yu_node, node(NLIST, yypvt[-1].yu_node, NULL)));
		yyval.yu_node->n_next->n_next->n_next = yyval.yu_node->n_next;
	}break;

case 46: {

#line 205 "/tmp/nsh/sh.y"

		yyval.yu_node = node(NCASE, yypvt[-4].yu_strp, yypvt[-1].yu_node);
	}break;

case 47: {

#line 208 "/tmp/nsh/sh.y"

		yyval.yu_node = node(NCASE, yypvt[-3].yu_strp, yypvt[-1].yu_node);
	}break;

case 48: {

#line 211 "/tmp/nsh/sh.y"

		yyval.yu_node = node(yypvt[-3].yu_nval, yypvt[-2].yu_node, node(NLIST, yypvt[-1].yu_node, NULL));
		yyval.yu_node->n_next->n_next = yyval.yu_node;
	}break;

case 49: {

#line 215 "/tmp/nsh/sh.y"

		yyval.yu_node = node(NIF, node(NNULL, yypvt[-4].yu_node, yypvt[-2].yu_node), yypvt[-1].yu_node);
	}break;

case 50: {

#line 218 "/tmp/nsh/sh.y"

		yyval.yu_node = node(NPARN, yypvt[-1].yu_node, NULL);
	}break;

case 51: {

#line 221 "/tmp/nsh/sh.y"

		yyval.yu_node = node(NBRAC, yypvt[-1].yu_node, NULL);
	}break;

case 52: {

#line 227 "/tmp/nsh/sh.y"

		yyval.yu_node = yypvt[0].yu_node;
	}break;

case 53: {

#line 230 "/tmp/nsh/sh.y"

		yyval.yu_node = node(NARGS, "\"$@\"", NULL);
	}break;

case 54: {

#line 236 "/tmp/nsh/sh.y"

		yyval.yu_node = node(NARGS, yypvt[-1].yu_strp, yypvt[0].yu_node);
	}break;

case 55: {

#line 239 "/tmp/nsh/sh.y"

		yyval.yu_node = NULL;
	}break;

case 56: {

#line 245 "/tmp/nsh/sh.y"

		register NODE *np;

		for (np=yypvt[-2].yu_node; np->n_next; np=np->n_next);
		np->n_next = yypvt[0].yu_node;
		yyval.yu_node = yypvt[-2].yu_node;
	}break;

case 57: {

#line 252 "/tmp/nsh/sh.y"

		yyval.yu_node = yypvt[0].yu_node;
	}break;

case 58: {

#line 255 "/tmp/nsh/sh.y"

		yyval.yu_node = NULL;
	}break;

case 59: {

#line 261 "/tmp/nsh/sh.y"

		yyval.yu_node = node(NCASE2, yypvt[0].yu_node, yypvt[-2].yu_node);
	}break;

case 60: {

#line 267 "/tmp/nsh/sh.y"

		yyval.yu_node = node(NCASE3, yypvt[-2].yu_strp, yypvt[0].yu_node);
	}break;

case 61: {

#line 270 "/tmp/nsh/sh.y"

		yyval.yu_node = node(NCASE3, yypvt[0].yu_strp, NULL);
	}break;

case 62: {

#line 276 "/tmp/nsh/sh.y"

		yyval.yu_node = yypvt[0].yu_node;
	}break;

case 63: {

#line 279 "/tmp/nsh/sh.y"

		yyval.yu_node = NULL;
	}break;

case 64: {

#line 285 "/tmp/nsh/sh.y"

		yyval.yu_node = node(NIF, node(NNULL, yypvt[-3].yu_node, yypvt[-1].yu_node), yypvt[0].yu_node);
	}break;

case 65: {

#line 288 "/tmp/nsh/sh.y"

		yyval.yu_node = node(NELSE, yypvt[0].yu_node, NULL);
	}break;

case 66: {

#line 291 "/tmp/nsh/sh.y"

		yyval.yu_node = NULL;
	}break;

case 67: {

#line 297 "/tmp/nsh/sh.y"

		yyval.yu_node = yypvt[0].yu_node;
	}break;

case 68: {

#line 301 "/tmp/nsh/sh.y"

		yyval.yu_node = NULL;
	}break;

case 69: {

#line 307 "/tmp/nsh/sh.y"

		yyval.yu_node = node(NLIST, yypvt[-2].yu_node, yypvt[0].yu_node);
	}break;

case 70: {

#line 310 "/tmp/nsh/sh.y"

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

/* end of /lib/yyparse.c */
