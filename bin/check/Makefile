# CFLAGS for PDP11
CFLAGS= -O -I.
UTILS= check dcheck icheck ncheck clri

all:	 $(UTILS)
	echo $(UTILS) are made

check:	check.o check.h
	$(CC) $(CFLAGS) -o check check.o

dcheck:	dcheck.o check.h
	$(CC) $(CFLAGS) -o dcheck dcheck.o

icheck:	icheck.o check.h
	$(CC) $(CFLAGS) -o icheck icheck.o

ncheck:	ncheck.o
	$(CC) $(CFLAGS) -o ncheck ncheck.o

clri:	clri.o
	$(CC) $(CFLAGS) -o clri clri.o

install: $(UTILS)
	chown bin $(UTILS)
	chmod 755 $(UTILS)
	cp $(UTILS) /etc
	rm $(UTILS) *.o
