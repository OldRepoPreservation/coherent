DESTDIR=/interim/bin/
CFLAGS= -I. -I.. -f -O -VSINU
YFLAGS=  -prods 110 -terms 67 -nterms 30 -states 200
OBJECTS=awk.o awk0.o awk1.o awk2.o awk3.o awk4.o awk5.o awk6.o awk.v.o \
 copyright.o
SRCS=awk.y awk0.c awk1.c awk2.c awk3.c awk4.c awk5.c awk6.c copyright.c

awk:	$(OBJECTS)
	cc $(CFLAGS) -o $(DESTDIR)awk $(OBJECTS) -lm

$(OBJECTS): awk.h

awk.o: awk.y

awk1.o: y.tab.h

