#bob h Makefile for use on romana

DESTDIR=/interim/usr/lib/

liby.a:
	su root rm -f $(DESTDIR)$@
	cc -O -c *.c
	ar rc $(DESTDIR)liby.a *.o
	su root chmog 444 bin bin $(DESTDIR)$@
	su root ranlib $(DESTDIR)$@

