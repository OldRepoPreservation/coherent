# diff/Makefile 4/13/90
DESTDIR=/interim/bin/
DESTDIR2=/interim/usr/lib/

OBJ=\
	diff1.o \
	diff2.o \
	diffh.o

all:	diff diffh diff3 diff3.script
	: all done

diff3.script:
	co $(DESTDIR)diff3.script
	su root mv $(DESTDIR)diff3.script $(DESTDIR)diff3
	su root chmog 555 bin bin $(DESTDIR)diff3

diff:	diff1.o diff2.o
	$(CC) $(LDFLAGS) -o $(DESTDIR)$@ diff1.o diff2.o
	su root chmog 511 bin bin $(DESTDIR)$@

diff3:	diff3.o
	$(CC) $(LDFLAGS) -o $(DESTDIR2)$@ diff3.o
	su root chmog 511 bin bin $(DESTDIR2)$@
	
diffh:	diffh.o diff2.o
	$(CC) $(LDFLAGS) -o $(DESTDIR2)$@ diffh.o diff2.o
	su root chmog 511 bin bin $(DESTDIR2)$@

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
