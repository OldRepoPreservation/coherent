#
#  12/7/90
#  Makefile for various auxiliary uucp executables
#

IDIR=/newbits/usr/bin/uucpstuff/include
ADIR=/newbits/usr/bin/uucpstuff/aux
DESTDIR2=/interim/usr/lib/uucp/

CFLAGS= -O -I$(IDIR)
LFLAGS= -n

ALIB=	/newbits/usr/bin/uucpstuff/aux/libuuaux.a
LIBX=	/usr/lib/libx.a

all: modemcap uumkdir uutouch uuxqt uucico uumvlog uurmlock
	@

uumvlog:
	(cd ../script; su root co $(DESTDIR2)$@)
	su root chown uucp $(DESTDIR2)$@
	su root chgrp uucp $(DESTDIR2)$@
	su root chmod 555 $(DESTDIR2)$@

uurmlock:
	(cd ../script; su root co $(DESTDIR2)$@)
	su root chown uucp $(DESTDIR2)$@
	su root chgrp uucp $(DESTDIR2)$@
	su root chmod 555 $(DESTDIR2)$@

modemcap:
	(cd /newbits/usr/bin/uucpstuff/modemcap; make)

uumkdir: uumkdir.o
	$(CC) -o $(DESTDIR2)$@ $(LFLAGS) $<
	su root chmog 6511 root root $(DESTDIR2)uumkdir

uutouch: uutouch.o $(ALIB)
	$(CC) -o $(DESTDIR2)$@ $(LFLAGS) $<
	su root chmog 6511 uucp uucp $(DESTDIR2)uutouch

uuxqt:	uuxqt.o $(ALIB) $(LIBX)
	$(CC) -o $(DESTDIR2)$@ $(LFLAGS) $<
	su root chmog 6511 uucp uucp $(DESTDIR2)uuxqt

uucico:
	(cd ../dcp; make)

