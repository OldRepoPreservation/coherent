# Parts of scn
O = y.tab.o lex.o

# Parts of libscn
L = 	yesNo.o \
	longfield.o \
	nonNull.o \
	numeric.o \
	userCtlc.o \
	getStuff.o \
	checkEnd.o \
	clearbak.o \
	cleararea.o \
	noWhite.o \
	noVerify.o \
	showbak.o \
	query.o \
	fatal.o \
	closeup.o \
	setup.o

CFLAGS = -O -A -I. -I..

all : scn libscn.a
	echo all done

install : /usr/lib/libscn.a /usr/bin/scn /usr/include/scn.h
	echo all done

/usr/lib/libscn.a : libscn.a
	su root cp libscn.a /usr/lib

/usr/bin/scn : scn
	su root cp scn /usr/bin

/usr/include/scn.h : scn.h
	su root cp  $< /usr/include

libscn.a : $L
	rm -f libscn.a
	ar q libscn.a $L

longfield.c : longfield scn
	./scn `srcpath longfield`

longfield.o : longfield.c

scn : $O
	cc -o scn $O -lmisc

y.tab.o : y.tab.c

y.tab.c : scn.y
	yacc $<

lex.o : lex.c y.tab.c scn.h
	cc -A -I. -I.. -c -O `srcpath lex.c`
