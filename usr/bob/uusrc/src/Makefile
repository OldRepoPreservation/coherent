#
#  12/7/90
#  Makefile for various auxiliary uucp executables
#

IDIR=../include
ADIR=../aux
BDIR=../bin

CFLAGS= -O -I$(IDIR)
LFLAGS= -n

ALIB=	$(ADIR)/libuuaux.a
LIBX=	/usr/lib/libx.a

all: support uucp uudecode uuencode uulog uumkdir uuname uutouch uux uuxqt
	@

uucp:	uucp.o $(ALIB)
	$(CC) -o $@ $(LFLAGS) $<
	su root chmog 6511 uucp uucp uucp
	ln -f $@ $(BDIR)

uudecode: uudecode.o
	$(CC) -o $@ $(LFLAGS) $<
	su root chmog  511 bin  bin  uudecode
	ln -f $@ $(BDIR)

uuencode: uuencode.o
	$(CC) -o $@ $(LFLAGS) $<
	su root chmog  511 bin  bin  uuencode
	ln -f $@ $(BDIR)

uulog:	uulog.o $(ALIB)
	$(CC) -o $@ $(LFLAGS) $<
	su root chmog  511 bin  bin  uulog
	ln -f $@ $(BDIR)

uumkdir: uumkdir.o $(ALIB)
	$(CC) -o $@ $(LFLAGS) $<
	su root chmog 6511 root root uumkdir
	ln -f $@ $(BDIR)

uuname:	uuname.o $(ALIB)
	$(CC) -o $@ $(LFLAGS) $<
	su root chmog 6511 uucp uucp uuname
	ln -f $@ $(BDIR)

uutouch: uutouch.o $(ALIB)
	$(CC) -o $@ $(LFLAGS) $<
	su root chmog 6511 uucp uucp uutouch
	ln -f $@ $(BDIR)

uux:	uux.o $(ALIB)
	$(CC) -o $@ $(LFLAGS) $<
	su root chmog 6511 uucp uucp uux
	ln -f $@ $(BDIR)

uuxqt:	uuxqt.o $(ALIB) $(LIBX)
	$(CC) -o $@ $(LFLAGS) $<
	su root chmog 6511 uucp uucp uuxqt
	ln -f $@ $(BDIR)

support:
	(cd ..; make support)

clean:
	[ "`echo *.o`" = "*.o" ] || rm *.o
