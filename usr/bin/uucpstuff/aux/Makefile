#
#  12/2/90
#  Makefile for auxilliary object files used by various
#  uucp binaries.  Builds the library "libuuaux.a"
#

LIBNAME=libuuaux.a

IDIR=../include
CFLAGS= -O -I$(IDIR) -DUUCP

AOBJ=	getopt.o	# Alien Object needed because of bad libc version

LOBJS=	alarm.o checktime.o ckdir.o debuglog.o exec.o fatal.o		\
	getline.o getseq.o knowhost.o ldev.o lock.o log.o lsys.o	\
	permission.o uucpname.o whoami.o

all:	$(LIBNAME) 
	@

$(LIBNAME): $(LOBJS) $(AOBJ)
	ar r $(LIBNAME) $(LOBJS) $(AOBJ)
	ranlib $(LIBNAME)

clean:
	[ "`echo *.o`" = "*.o" ] || rm *.o
