# /usr/src/cmd0/sh/Makefile 2/19/91
DESTDIR=/interim/bin/
# Version number compiled into var.o.
VERSION=3.1.1

# For i8086 compile with -VSINU.
# Define VERSION for var.c.
CFLAGS= -O -VSINU -DVERSION='"VERSION=V$(VERSION)"'
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
	sh.h\
	sh.y\
	alloc.c\
	atoi.c\
	eval.c\
	exec1.c\
	exec2.c\
	exec3.c\
	extern.c\
	glob.c\
	lex.c \
	main.c\
	tab.c\
	trap.c\
	var.c

# Primary target.
sh: $(OBJ)
	cc $(LDFLAGS) -o $(DESTDIR)sh $(OBJ)
	su root chown bin $(DESTDIR)$@
	su root chgrp bin $(DESTDIR)$@
	su root chmod 511 $(DESTDIR)$@
	mv *.o objects/

psh: $(OBJ)
	cc -n -o psh -O -DPARANOID $(OBJ)

# Secondary targets.
alloc.o:	sh.h alloc.c
atoi.o:		atoi.c
eval.o:		sh.h eval.c
exec1.o:	sh.h exec1.c
exec2.o:	sh.h exec2.c
exec3.o:	sh.h exec3.c
extern.o:	sh.h extern.c
glob.o:		sh.h glob.c
lex.o:		sh.h lex.c y.tab.o
main.o:		sh.h main.c
tab.o:		sh.h tab.c
trap.o:		sh.h trap.c
var.o:		sh.h var.c
y.tab.c:	sh.y
	yacc sh.y
y.tab.o:	sh.h y.tab.c

# Etc.
clean:
	mv *.o objects/

# end of /usr/src/cmd0/sh/Makefile
