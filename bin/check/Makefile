# /usr/src/cmd/check/Makefile 2/20/91

# CFLAGS for PDP11
CFLAGS= -O
DESTDIR= /f1/bin/
UTILS=\
	check\
	clri\
	dcheck\
	icheck\
	ncheck
OBJ=\
	check.o\
	clri.o\
	dcheck.o\
	icheck.o\
	ncheck.o

# Primary target.
all:	$(UTILS)
	: all done

check:	check.o
	$(CC) $(LDFLAGS) -o $(DESTDIR)$@ $<
clri:	clri.o
	$(CC) $(LDFLAGS) -o $(DESTDIR)$@ $<
dcheck:	dcheck.o
	$(CC) $(LDFLAGS) -o $(DESTDIR)$@ $<
icheck:	icheck.o
	$(CC) $(LDFLAGS) -o $(DESTDIR)$@ $<
ncheck:	ncheck.o
	$(CC) $(LDFLAGS) -o $(DESTDIR)$@ $<

check.o:	check.h check.c
clri.o:		clri.c
dcheck.o:	check.h dcheck.c
icheck.o:	check.h icheck.c
ncheck.o:	ncheck.c

clean:
	rm $(OBJ)

# end of /usr/src/cmd/check/Makefile
