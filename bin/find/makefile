DESTDIR=/f1/bin/

find:	find.o	findnode.h
	cc -O -o $(DESTDIR)find find.o -ly
	rm find.o y.tab.h

find.o:	find.y

