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
char new_date[3];
int month, day, year;

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
		month = atoi(new_date);
		strcpy(new_date,"");

		new_date[0] = record.date[2];
		new_date[1] = record.date[3];
		new_date[2] = '/';
		day = atoi(new_date);
		strcpy(new_date,"");

		new_date[0] = record.date[4];
		new_date[1] = record.date[5];
		new_date[2] = '\0';
		year = atoi(new_date);

		fprintf(outfp,"\nFILE: %15s\t\tDATE: %d/%d/%d\t\tSIZE: %s\n\n",
			record.filename, month, day, year ,record.filesize);
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
	wprintw(win2,"Because we have just written the file %s,",PRINTFILE);
	wmove(win2,1,0);
	wprintw(win2,"the program will now end so that you may view the");
	wmove(win2,2,0);
	wprintw(win2,"file and/or print it. This will also prevent the");
	wmove(win2,3,0);
	wprintw(win2,"possibility of immediate accidental erasure of the file.");
	wmove(win2,6,0);
	wprintw(win2,"File written. Press <RETURN> to exit program.");
	clrtoeol();
	wrefresh(win2);
	while(13 != wgetch(win2)) ;
}
