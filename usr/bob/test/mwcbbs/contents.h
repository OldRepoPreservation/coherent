#define MASTERFILE "Contents"
#define TRACE printf
#define PATHNAME "/usr/spool/uucppublic/"
#define MAILFILE "Maillist"
#define TEMPFILE "mwcbbs.tmp"
#define HOST "uucp -r mwcbbs!"
#define RECEIVER " ~/"
#define MAXRECORDS 250
#define SCREENMAX 100
#define LENPATH 22
#define NAMELOCATE 2,1
#define DESCLOCATE 4,1
#define DATELOCATE 6,1
#define SIZELOCATE 6,30
#define PARTLOCATE 6,49
#define REQLOCATE 9,1
#define NOTELOCATE 12,1
#define PATHLOCATE 15,1
#define PATHLOCATE2 16,1
#define PATHLOCATE3 17,1
#define NAMEHI 2,12
#define DESCHI 5,1
#define DATEHI 7,5
#define SIZEHI 7,30
#define PARTHI 7,60
#define REQHI 10,2
#define NOTEHI  13,2
#define PATHHI 18,2
#define PATHHI2 18,24
#define FILE0 "Contents.down"
#define FILE1 "Contents.hints"
#define FILE2 "Contents.news"
#define FILE3 "Contents.UPD"
#define FILE4 "Maillist"
#define FILE5 "Net_Maps"
#define FILE6 "QUIT"
#define REC_FORMULA ((sizeof (struct entry) * (screen_num * 100)) + (((row*5) + (col/15)) * sizeof (struct entry)))
#define POS_FORMULA ((screen_num * 100) + ((row*5) + (col/15)))
#define MAIL_FORMULA ((sizeof (struct mail) * (screen_num * 100)) + (((row*5) + (col/15)) * sizeof (struct mail)))
#define MAP_FORMULA ((sizeof (struct map) * (screen_num * 100)) + (((row*5) + (col/15)) * sizeof (struct map)))

void show_files(); /* this should display the filenames on a curses screen */
int lite(); /* inverse/normal video display of a filename */
int rfile(); /* read records from a given file */
void write_win(); /*does the actual work of writing filenames to a window */
void display_form(); /* for for displaying selected filename */
void display_record(); /* display selected filename */
void menu(); /* menu printed at bottom of screen */
void del_rec(); /* this will be used to delete records */
void add_rec(); /* this will be used to add records */
void getstring(); /* this will be called by add_rec to get input */
void build_uucp(); /* this will build multiple uucp requests */
void getfilename(); /* this will run from main, getting the user's choice of*/
					/* file to use. */
void print_states(); /* this will print a screen of states from which a user*/
					 /* can get a list of bbs accounts broken down by state */
void print_mail_states(); /* this will print maillists by given state */
void add_mail();
void add_del_screen();	/* this draws a master mail rec screen */
void display_mail(); 	/* this will display a mail record before deletion */
int get_info();		/* this will get mail info to be added to Mailfile */
void get_net_map();	/* this will get a selection of net map to read */
void map_command();	/* this will generate uucp commands to grab net maps */

char selection[15];
char filenames [MAXRECORDS][15];
char workfile[15];
char workstring[80];
char getfiles[26][115];
int place[MAXRECORDS];
int limit, screen_num;
char open_mode;
char state[52][15]={"Alabama","Alaska","Arizona","Arkansas","California",
		     "Colorado","Connecticut","Delaware","Florida","Georgia",
		     "Hawaii","Idaho","Illinois","Indiana","Iowa","Kansas",
		     "Kentucky","Lousiana","Maine","Maryland","Massachusetts",
		     "Michigan","Minnesota","Mississippi","Missouri","Montana",
		     "Nebraska","Nevada","New Hampshire","New Jersey",
		     "New Mexico","New York","North Carolina","North Dakota",
		     "Ohio","Oklahoma","Oregon","Pennsylvania","Rhode Island",
		     "South Carolina","South Dakota","Tennessee","Texas","Utah",
		     "Vermont","Virginia","Washington","West Virginia",
		     "Wisconsin","Wyoming","NON-US","UNKNOWN"};

char mapfile[4][15]={"Net_Maps.WORLD", "Net_Maps.USA", "Net_Maps.CAN", "QUIT"};

struct mail	{
		char site[8];
		char login[11];
		char state[21];
		char city[25];
		};

struct mail mail_rec;
struct mail new_mail_rec;

struct entry{
		char filename [15];
		char filesize [10];
		char date[7];
		char description [78];
		char requires [60];
		char notes [78];
		char pathname [60];
		int noparts;
	    };

struct entry record;
struct entry new_record;

struct map	{
		char name[15];
		char path[60];
		};

struct map map_rec;
