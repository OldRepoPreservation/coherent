# pacman/Makefile 10/4/90
# This Makefile halfheartedly preserves dal's original braindamaged notion
# of machine independence.
# His sources build either a PDP-11 or an IBM PC version,
# using links to get the right one.
# The pac.c also #includes other C sources, not really kosher,
# but for now this goes with the flow rather than cleaning it all up.

DESTDIR=/interim/usr/games/
OBJ=ibmpac.o ibmout.o

pacman:	$(OBJ)
	cc -o $(DESTDIR)chase $(OBJ)
	su root chmog 511 bin bin $(DESTDIR)$@

ibmpac.o:	ibmpac.c term.c io.c
	cc -c ibmpac.c

ibmout.o:	ibmout.s
	cc -c ibmout.s

term.c:	ibm.c
	ln -f ibm.c term.c

io.c:	ibmio.c
	ln -f ibmio.c io.c

ibmpac.c:	pac.c
	ln -f pac.c ibmpac.c
