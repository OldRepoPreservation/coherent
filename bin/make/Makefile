DESTDIR=/f1/bin/
# /usr/src/make/Makefile 2/14/91
# make's Makefile for make, what else?

make:	make.o
	$(CC) $(LDFLAGS) -o $(DESTDIR)$@ $?
	su root chown bin $(DESTDIR)make
	su root chgrp bin $(DESTDIR)make
	su root chmod 511 $(DESTDIR)make
make.o: make.c make.h

clean:
	rm make.o

# end of /usr/src/make/Makefile
