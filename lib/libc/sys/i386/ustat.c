/*
 * libc/sys/uname.c
 */

uname(name)
{
	return _ustn(name, 0, 0);
}

/* end of libc/sys/uname.c */
