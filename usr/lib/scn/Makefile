CFLAGS = -O -A

O = 	yesNo.o \
	nonNull.o \
	numeric.o \
	userCtlc.o \
	getStuff.o \
	clearbak.o \
	cleararea.o \
	showbak.o \
	tracer.o \
	query.o \
	showerr.o \
	fatal.o \
	closeup.o \
	setup.o

all : libscn.a scn
	echo all done

install : libscn.a scn scn.h
	su root cp libscn.a /usr/lib
	su root cp scn /usr/bin
	su root cp `srcpath scn.h` /usr/include

libscn.a : $O
	rm -f libscn.a
	ar q libscn.a $O

scn : scn.o
	cc scn.o -lmisc
