CFLAGS=	-O -DCM_N -DCM_GT -DCM_B -DCM_D
DESTDIR=/interim/usr/lib/

libterm.a: termcap.o tgoto.o tputs.o
	su root rm -f $(DESTDIR)$@
	ar cr $(DESTDIR)libterm.a termcap.o tgoto.o tputs.o
	su root chmog 444 bin bin $(DESTDIR)$@
