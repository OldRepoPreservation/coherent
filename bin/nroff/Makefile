# /usr/src/cmd/nroff/Makefile 3/29/91
# Makefile for nroff and troff

# Targets.

DESTDIR=/interim/bin/

ALL=\
	nroff\
	troff\
	fwtable

# Objects included in both nroff and troff images.
OBJ=\
	div.o\
	env.o\
	expr.o\
	extern.o\
	getf.o\
	hyphen.o\
	hyptab.o\
	main.o\
	misc.o\
	process.o\
	reg.o\
	req1.o\
	req2.o\
	request.o\
	table.o

# Objects in nroff only.
NOBJ=	tty.o

# Objects in troff only.
TOBJ=\
	fonts.o\
	output.o

# Too many header files.
HDR=\
	code.h\
	codebug.h\
	div.h\
	env.h\
	escapes.h\
	fonts.h\
	hyphen.h\
	reg.h\
	request.h\
	roff.h\
	str.h

# fwtable objects.
FWTOBJ=\
	fwtable.o\
	fwtableps.o

# Primary target.
all:	$(ALL)
	: Done.

# Secondary targets.
troff:	$(OBJ) $(TOBJ)
	$(CC) $(LDFLAGS) -o $(DESTDIR)troff $(OBJ) $(TOBJ)
	su root chmog 511 bin bin $(DESTDIR)troff

nroff:	$(OBJ) $(NOBJ)
	$(CC) $(LDFLAGS) -o $(DESTDIR)nroff $(OBJ) $(NOBJ)
	su root chmog 511 bin bin $(DESTDIR)nroff

fwtable:	$(FWTOBJ)
	cc -o $(DESTDIR)fwtable $(FWTOBJ) -f
	su root chmog 511 bin bin $(DESTDIR)fwtable

strip:	$(ALL)
	strip $(ALL)

clean:
	rm -f $(OBJ) $(TOBJ) $(NOBJ) fwtable.o fwtableps.o

# Unlinked objects.
div.o:		$(HDR) div.c
env.o:		$(HDR) env.c
expr.o:		$(HDR) expr.c
extern.o:	$(HDR) extern.c
fonts.o:	$(HDR) fonts.c
getf.o:		$(HDR) getf.c
hyphen.o:	$(HDR) hyphen.c
hyptab.o:	$(HDR) hyptab.c
main.o:		$(HDR) main.c
misc.o:		$(HDR) misc.c
output.o:	$(HDR) output.c
process.o:	$(HDR) process.c
reg.o:		$(HDR) reg.c
req1.o:		$(HDR) req1.c
req2.o:		$(HDR) req2.c
request.o:	$(HDR) request.c
table.o:	$(HDR) table.c
tty.o:		$(HDR) tty.c

fwtable.o:	fwtable.c
fwtableps.o:	fwtableps.c

# end of /usr/src/cmd/nroff/Makefile
