# /usr/src/cmd0/sh/Makefile 1/23/90
# sh.v.o removed from OBJS, version does not work under Inetco COH.

# for i8086 use CFLAGS=-VSINU
CFLAGS= -O -VSINU -I. -I/usr/include/sys
LDFLAGS= -n -i
# Don't bind shared on the PDP-11
# LDFLAGS= -i

OBJS =  main.o alloc.o eval.o exec1.o exec2.o exec3.o extern.o lex.o \
	tab.o trap.o var.o y.tab.o atoi.o glob.o malloc.o notmem.o

SRCS =  main.c alloc.c eval.c exec1.c exec2.c exec3.c extern.c lex.c \
	tab.c trap.c var.c sh.y atoi.c glob.c sh.h

sh: $(OBJS)
	cc $(LDFLAGS) -o sh $(OBJS)

psh: $(OBJS)
	cc -n -o psh -O -DPARANOID $(OBJS)

alloc.o: sh.h alloc.c

eval.o:	sh.h eval.c

exec1.o: sh.h exec1.c

exec2.o: sh.h exec2.c

exec3.o: sh.h exec3.c

extern.o: sh.h extern.c

lex.o: sh.h y.tab.h lex.c

main.o: sh.h main.c

tab.o: sh.h tab.c

trap.o: sh.h trap.c

#	$(CC) -O -c var.c
var.o: sh.h var.c

y.tab.o: sh.h y.tab.h y.tab.c

y.tab.c: sh.y
	yacc sh.y

atoi.o: atoi.c

glob.o: sh.h glob.c

sh.v.o: $(SRCS)
	version `pwd` sh $(SRCS) > sh.v.s
	as -o sh.v.o sh.v.s
	rm sh.v.s
