/* build0.h */

#define	NBUF		256		/* buffer size			*/

/* Flags for sys(). */
#define	S_IGNORE	1
#define	S_NONFATAL	2
#define	S_FATAL		3

/* Functions. */
void	cls();
int	exists();
void	fatal();
char	*get_line();
int	is_dir();
void	nonfatal();
int	sys();
void	usage();
int	yes_no();

/* Globals. */
char	*argv0;				/* for error messages	*/
char	buf[NBUF];			/* input buffer		*/
char	cmd[NBUF];			/* command buffer	*/
int	dflag;				/* debug		*/
char	*usagemsg;			/* usage message	*/
int	vflag;				/* verbose		*/

/* end of build0.h */
