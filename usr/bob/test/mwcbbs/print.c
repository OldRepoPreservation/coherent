/* print.c
 * With any luck, this program will read the Content file format
 * and print information to a text file which can later be spooled 
 * for printing by the user.
*/

#include <stdio.h>
#include <curses.h>
#include "contents.h"

#define PRINTFILE "mwcbbs.print"

void print(win2)
WINDOW *win2;

{
FILE *infp, *outfp;
char new_date[9];

	if ((infp=(fopen(workfile,"r")))==NULL)
		{
		noraw();
		endwin();
		printf("Could not open file %s for input!\n",workfile);
		exit(1);
		}
	if ((outfp=(fopen(PRINTFILE,"w")))==NULL)
		{
		noraw();
		endwin();
		printf("Could not open file %s for writing!\n",workfile);
		exit(1);
		}

	
	wclear(win2);
	wmove(win2,0,0);
	wprintw(win2,"Writing file %s... please wait.",PRINTFILE);
	wrefresh(win2);

	fprintf(outfp,"Contents file for: %s\n",workfile);
	while ((fread(&record,sizeof(struct entry),1,infp))!=0)
		{
		strcpy(new_date,"");
		strncpy(new_date,record.date,2);
		new_date[2] = '/';
		new_date[3] = record.date[2];
		new_date[4] = record.date[3];
		new_date[5] = '/';
		new_date[6] = record.date[4];
		new_date[7] = record.date[5];
		new_date[8] = '\0';

		fprintf(outfp,"\nFILE: %s\t\tDATE: %s\t\t\tSIZE: %s\n\n",
			record.filename, new_date,record.filesize);
		fprintf(outfp,"Description:\n");
		fprintf(outfp," %s\n",record.description);
		fprintf(outfp,"Other notes:\n");
		fprintf(outfp," %s\n",record.notes);
		fprintf(outfp,"Required files:\n");
		fprintf(outfp," %s\n\n",record.requires);
		}
	fclose(infp);
	fclose(outfp);
	wclear(win2);
	wmove(win2,0,0);
	wprintw(win2,"File written. Press <RETURN> to continue.");
	clrtoeol();
	wrefresh(win2);
	while(13 != wgetch(win2)) ;
}
