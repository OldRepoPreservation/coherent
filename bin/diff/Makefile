# diff/Makefile 4/13/90

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
	cp diff /bin
	cp diff3.sh /bin/diff3
	cp diffh diff3 /usr/lib
	chown bin /bin/diff /bin/diff3 /usr/lib/diffh /usr/lib/diff3
	chgrp bin /bin/diff /bin/diff3 /usr/lib/diffh /usr/lib/diff3
	chmod 511 /bin/diff /usr/lib/diffh /usr/lib/diff3
	chmod 511 /bin/diff3

clean:	$(OBJ)
	rm -f diff*.o
