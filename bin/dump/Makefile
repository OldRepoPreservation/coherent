SDIR =.
CFLAGS = -O -I$(SDIR)
CC = exec /bin/cc

all: dumpdate dumpdir dump restor
	echo done

dumpdate: dumpdate.c dumptape.h
	$(CC) $(CFLAGS) -o dumpdate dumpdate.c

dumpdir: dumpdir.c dumptape.h
	$(CC) $(CFLAGS) -o dumpdir dumpdir.c

dump: dump.c dumptape.h discbuf.h discbuf.o
	$(CC) $(CFLAGS) -o dump dump.c discbuf.o

restor: restor.c dumptape.h discbuf.h discbuf.o
	$(CC) $(CFLAGS) -o restor restor.c discbuf.o

discbuf.o: discbuf.h discbuf.c

dumpdate.o: dumptape.h dumpdate.c

dumpdir.o: dumptape.h dumpdir.c

dump.o: dumptape.h discbuf.h dump.c

restor.o: dumptape.h discbuf.h restor.c
