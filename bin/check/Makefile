# /usr/src/cmd/check/Makefile 2/20/91

# CFLAGS for PDP11
CFLAGS= -O

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
	$(CC) $(LDFLAGS) -o $@ $<
clri:	clri.o
	$(CC) $(LDFLAGS) -o $@ $<
dcheck:	dcheck.o
	$(CC) $(LDFLAGS) -o $@ $<
icheck:	icheck.o
	$(CC) $(LDFLAGS) -o $@ $<
ncheck:	ncheck.o
	$(CC) $(LDFLAGS) -o $@ $<

check.o:	check.h check.c
clri.o:		clri.c
dcheck.o:	check.h dcheck.c
icheck.o:	check.h icheck.c
ncheck.o:	ncheck.c

clean:
	rm $(OBJ)

# end of /usr/src/cmd/check/Makefile
