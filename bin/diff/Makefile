# diff/Makefile 4/13/90
DESTDIR=/f1/bin/
DESTDIR2=/f1/usr/lib/

OBJ=\
	diff1.o \
	diff2.o \
	diffh.o

all:	diff diffh diff3
	: all done

diff:	diff1.o diff2.o
	$(CC) $(LDFLAGS) -o $@ diff1.o diff2.o

diff3:	diff3.o
	$(CC) $(LDFLAGS) -o $@ diff3.o
	
diffh:	diffh.o diff2.o
	$(CC) $(LDFLAGS) -o $@ diffh.o diff2.o

install:	diff diffh diff3.sh diff3
	strip diff diffh diff3
	cp diff $(DESTDIR)
	cp diff3.sh $(DESTDIR)diff3
	cp diffh $(DESTDIR2)
	cp diff3 $(DESTDIR2)
	chown bin $(DESTDIR)diff $(DESTDIR)diff3 $(DESTDIR2)diffh $(DESTDIR2)diff3
	chgrp bin $(DESTDIR)diff $(DESTDIR)diff3 $(DESTDIR2)diffh $(DESTDIR2)diff3
	chmod 511 $(DESTDIR)diff $(DESTDIR2)diffh $(DESTDIR2)diff3
	chmod 511 $(DESTDIR)diff3

clean:	$(OBJ)
	rm -f diff*.o
