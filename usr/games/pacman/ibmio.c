#include <signal.h>
#include <sgtty.h>
#include <sys/stat.h>
#include <sys/types.h>

#define	bufchr(c)	{sout(caddr++, (c)); sout(caddr++, (a_n));}
#define	bflush()
#define	bufstr(s)	{register char *pc; pc=s; while (*pc) bufchr(*pc++);}
#define	atrb(a)		{a_n=a;}
#define	beep()		write(2, "\7", 1)

int			a_n;
struct	sgttyb	sgttyb, oldsgttyb;
struct	stat	sbuf;
int	pv[3];

cls()
{
	register char *c;

	for (c = 0; c < 4000; ) {
		sout (c++, ' ');
		sout (c++, 7);
	}
}

reset()
{

	ioctl(1, TIOCSETP, &oldsgttyb);
	kill (pv[2], SIGTERM);
	cls();
	exit (0);
}

setup()
{
	int fd;

	fd = open("/dev/console", 2);
	if (fd == -1)
		error("Sorry, you can only play from the console keyboard.\n");
	close(fd);
	if (pipe(pv) < 0)
		error("cannot create pipe\n");
	if ((pv[2]=fork()) < 0)
		error("cannot fork\n");
	if (pv[2] == 0) {
		dup2(pv[1], 1);
		close(pv[0]);
		close(pv[1]);
		chdir("/tmp");
		execl("/bin/cat", "", "-u", 0);
		exit (1);
	}
	signal(SIGINT, reset);
	dup2(pv[0], 0);
	close(pv[0]);
	close(pv[1]);
	ioctl(1, TIOCGETP, &oldsgttyb);
	sgttyb = oldsgttyb;
	sgttyb.sg_flags &= ~ECHO;
	sgttyb.sg_flags |= CBREAK | RAWOUT;
	ioctl(1, TIOCSETP, &sgttyb);
	cls();
}

error(s)
char *s;
{
	register i = 0;
	register char *t = s;

	while (*t++)
		++i;
	write(2, s, i);
	exit (1);
}

input(c)
register char *c;
{
	register n;

	fstat(0, &sbuf);
	n = (int)sbuf.st_size;
	if (n <= 0)
		return (0);
	while (n--)
		read(0, c, 1);
	return (1);
}

inputecho(c)
register char *c;
{
	while (input(c) == 0)
		;
	if (*c == '\n')
		return;
	bufchr(*c);
	bflush();
}

delay(t)
register t;
{
	extern int ndelay;
	register i;

	if (ndelay)
		t *= ndelay;
	while (t--)
		for (i=0; i<4096; ++i)
			;
}
