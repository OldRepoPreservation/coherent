/*
 * Nroff/Troff.
 * Data structure for storing character codes and stream directives.
 */

/*
 * Stream directives.
 */
#define DNULL	0			/* Null */
#define DHMOV	-1			/* Move horizontally */
#define DVMOV	-2			/* Move vertically */
#define DFONT	-3			/* Change font */
#define DPSZE	-4			/* Change pointsize */
#define DSPAR	-5			/* Space down and return */
#define DPADC	-6			/* Paddable character */
#define DHYPH	-7			/* Place to hyphenate */
#define	DHYPC	(-8)			/* Hyphen character */
#define	DTRAN	(-9)			/* Transparent character (dag)*/
#define	DTRAB	(-10)			/* Transparent buffer 	 (dag)*/
#define	DSPEC	(-11)			/* Special character	 (dag)*/
#define	DFPOS	(-12)			/* Fix position		 (dag)*/

/*
 * All characters and commands are stored in this structure.
 * To identify whether an element is a character or a command,
 * call the function ifcchar(element) which returns 1 if the
 * element is a character code and 0 if the element is a command.
 */
typedef	union	{
	struct	{			/* Structure containing character */
		int	c_code;		/* Character code */
		unsigned c_move;	/* Distance to move after char */
		int	c_char;		/* If special character, char */
	} c_arg;
	struct	{			/* Command with one long argument */
		int	c_code;		/* Type of command */
		int	c_iarg;		/* Long parameter */
		int	c_csp;		/* Pre tab space */
	} l_arg;
	struct	{			/* Command with buffer ptr. (dag)*/
		int	c_code;		/* Type of command		*/
		char	*c_bufp;	/* Pointer to buffer		*/
	} b_arg;
} CODE;

/*
 * Functions for determining whether an element is a character code
 * or a stream directive.
 */
#define ifcchar(c)	((c)>0)
#define ifcdirc(c)	((c)<0)

/*
 * Add a character given the font number and width.
 */
#define addchar(f, w) {							\
	chkcode();							\
	nlinptr->c_code = f;						\
	nlinptr->c_csp = 0;						\
	nlinptr++->c_move = w;						\
}

/*
 * Add a directive which takes an integer argument.
 */
#define addidir(d, i) {							\
	chkcode();							\
	nlinptr->c_code = d;						\
	nlinptr++->c_iarg = i;						\
}

/*
 * Add a transparent buffer directive
 */
#define	addtrab(bp) {							\
	chkcode();							\
	nlinptr->c_code = DTRAB;					\
	nlinptr++->c_bufp = (bp);					\
}

/*
 * Make sure there is space to add another command.
 */
#define chkcode() {							\
	if (nlinptr >= &linebuf[LINSIZE])				\
		panic(lbomsg);						\
}

/*
 * Global variables.
 */
extern	CODE	codeval;		/* Diversion struct returned by getl */
extern	char	lbomsg[];		/* "Line buffer overflow" */
