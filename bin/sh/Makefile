# /usr/src/cmd0/sh/Makefile 2/8/91

# Source directory.
SRC=/usr.src/cmd0/sh

# Version number compiled into var.o.
VERSION=3.1.1

# Include source directory to get sh.h for y.tab.c.
# For i8086 use CFLAGS=-VSINU.
CFLAGS= -O -VSINU -I$(SRC)
# Undefine _copyright_ to pull in libc.a/copyright.o.
LDFLAGS= -n -i -u _copyright_
# Don't bind shared on the PDP-11.
# LDFLAGS= -i

OBJ=\
	alloc.o\
	atoi.o\
	eval.o\
	exec1.o\
	exec2.o\
	exec3.o\
	extern.o\
	glob.o\
	lex.o\
	main.o\
	tab.o\
	trap.o\
	var.o\
	y.tab.o

SRCS=\
	$(SRC)/sh.h\
	$(SRC)/sh.y\
	$(SRC)/alloc.c\
	$(SRC)/atoi.c\
	$(SRC)/eval.c\
	$(SRC)/exec1.c\
	$(SRC)/exec2.c\
	$(SRC)/exec3.c\
	$(SRC)/extern.c\
	$(SRC)/glob.c\
	$(SRC)/lex.c \
	$(SRC)/main.c\
	$(SRC)/tab.c\
	$(SRC)/trap.c\
	$(SRC)/var.c

# Primary target.
sh: $(OBJ)
	cc $(LDFLAGS) -o sh $(OBJ)

psh: $(OBJ)
	cc -n -o psh -O -DPARANOID $(OBJ)

# Secondary targets.
alloc.o:	$(SRC)/sh.h $(SRC)/alloc.c
atoi.o:		$(SRC)/atoi.c
eval.o:		$(SRC)/sh.h $(SRC)/eval.c
exec1.o:	$(SRC)/sh.h $(SRC)/exec1.c
exec2.o:	$(SRC)/sh.h $(SRC)/exec2.c
exec3.o:	$(SRC)/sh.h $(SRC)/exec3.c
extern.o:	$(SRC)/sh.h $(SRC)/extern.c
glob.o:		$(SRC)/sh.h $(SRC)/glob.c
lex.o:		$(SRC)/sh.h $(SRC)/lex.c y.tab.o
main.o:		$(SRC)/sh.h $(SRC)/main.c
tab.o:		$(SRC)/sh.h $(SRC)/tab.c
trap.o:		$(SRC)/sh.h $(SRC)/trap.c
var.o:		$(SRC)/sh.h $(SRC)/var.c
	$(CC) $(CFLAGS) -c -DVERSION='"VERSION=V$(VERSION)"' $(SRC)/var.c
y.tab.c:	$(SRC)/sh.y
	yacc $(SRC)/sh.y
y.tab.o:	$(SRC)/sh.h y.tab.c

# Etc.
clean:
	rm $(OBJ)

# Unused currently.
malloc.o:	malloc.c
notmem.o:	notmem.c

# end of /usr/src/cmd0/sh/Makefile
