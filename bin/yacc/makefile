# yacc/Makefile 1/23/90
# For i8086: LFLAGS=-n.

CFLAGS=-O
LFLAGS=-i -n

all: yacc
	>all

cmp: all
	cmp yacc /bin/yacc
	rm yacc *.o

cp:	all
	cp yacc /bin/yacc
	rm yacc *.o

yacc: y0.o y1.o y2.o y3.o y4.o y5.o y6.o 
	$(CC) $(CFLAGS) $(LFLAGS) -o $@ $<

y0.o y1.o y4.o: yacc.h
y3.o: yacc.h assert.h
y2.o y6.o: yacc.h action.h
y5.o: yacc.h assert.h action.h
