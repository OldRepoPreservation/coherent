# /usr/src/cmd0/sh/Makefile 2/1/90
# sh.v.o removed from OBJS, version does not work under Inetco COH.

# Version number compiled into var.o.
VERSION=3.1.1

# for i8086 use CFLAGS=-VSINU
CFLAGS= -O -VSINU
# Undefine _copyright_ to pull in libc.a/copyright.o.
LDFLAGS= -n -i -u _copyright_
# Don't bind shared on the PDP-11
# LDFLAGS= -i

OBJS =	\
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
	malloc.o\
	notmem.o\
	tab.o\
	trap.o\
	var.o\
	y.tab.o

SRCS =  sh.h\
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
	malloc.c\
	notmem.c\
	tab.c\
	trap.c\
	var.c\
	sh.y

sh: $(OBJS)
	cc $(LDFLAGS) -o sh $(OBJS)

psh: $(OBJS)
	cc -n -o psh -O -DPARANOID $(OBJS)

clean:
	rm $(OBJS)

alloc.o:	sh.h alloc.c
atoi.o:		atoi.c
eval.o:		sh.h eval.c
exec1.o:	sh.h exec1.c
exec2.o:	sh.h exec2.c
exec3.o:	sh.h exec3.c
extern.o:	sh.h extern.c
glob.o:		sh.h glob.c
lex.o:		sh.h y.tab.h lex.c
main.o:		sh.h main.c
malloc.o:	malloc.c
notmem.o:	notmem.c
tab.o:		sh.h tab.c
trap.o:		sh.h trap.c
var.o:		sh.h var.c
	$(CC) $(CFLAGS) -c -DVERSION='"VERSION=V$(VERSION)"' var.c
y.tab.o:	sh.h y.tab.h y.tab.c
y.tab.c:	sh.y
	yacc sh.y

# Unused currently.
sh.v.o:	$(SRCS)
	version `pwd` sh $(SRCS) > sh.v.s
	as -o sh.v.o sh.v.s
	rm sh.v.s
