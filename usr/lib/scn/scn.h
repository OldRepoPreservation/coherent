/*
 * Header for quick and dirty screen builder.
 * Minamally designed by William G. Lederer
 * Thrown together by Charles Fiterman 2-6-90
 */
#ifndef SCN
#define SCN 1
#include <curses.h>
#include <stdio.h>
typedef struct backGrnd backGrnd;
typedef struct loc loc;

/*
 * screen location.
 */
struct loc {
	char *field;		/* field to fill */
	int len;		/* field length */
	char *Default;		/* field default or NULL */
	int  (*verify)();	/* veryify function or NULL */
	char row;		/* row in display */
	char col;		/* column in display */
	char skipf;		/* skip factor */
	char *help;		/* help message */
};

/*
 * Background data table produced by MWCscreen.
 */
struct backGrnd {
	char *data;	/* data to display */
	char row;
	char col;
};
extern void setUpScreen();	/* setUpScreen(linesForErr, errAtLine); */
extern WINDOW *errWindow;	/* built by setUpScreen() */
extern void closeUp();		/* shut down screen */
extern void showError();	/* showError(fmt, ...); */
extern int  getChr();		/* use instead of getch */
extern int  Query();		/* Query(fmt, ...); one char reply */
extern void showBak();		/* showBak(scn_data); put out background */
extern void clearArea();	/* clearArea(row, col, length); */
extern void clearBak();		/* clearBak(scn_data, scn_locs); */
extern void showDefs();		/* showDefs(scn_data, scn_locs); */
extern void scnDriv();		/* scnDriv(scn_data, scn_locs); */
extern void getAll();		/* getAll(scn_locs); */
extern int  getField();		/* getField(scn_locs, fieldName); */
extern void putField();		/* putField(scn_locs, fieldName); */
#endif
