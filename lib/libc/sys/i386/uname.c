/*
 * libc/sys/ustat.c
 */

ustat(dev, buf)
{
	return _ustn(buf, dev, 2);
}

/* end of libc/sys/ustat.c */
