/*
 * show the background for a screen.
 */
#include <scn.h>

void
showBak(back)
register backGrnd *back;
{
	register char *p;

	for (; NULL != (p = back->data); back++)	/* put out background */
		mvaddstr(back->row, back->col, p);
	refresh();
}
