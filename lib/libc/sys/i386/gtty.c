/* Copyright (c) Bureau d'Etudes Ciaran O'Donnell,1987,1990,1991 */
#include <sgtty.h>
stty(u,v)
{
	return ioctl(u,TIOCSETP,v);
}

gtty(u,v)
{
	return ioctl(u,TIOCGETP,v);
}
