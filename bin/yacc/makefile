# /usr/src/cmd/yacc/Makefile 2/20/91
# For i8086: LFLAGS=-n.

CFLAGS=-O
LDFLAGS=-i -n

OBJ=\
	y0.o\
	y1.o\
	y2.o\
	y3.o\
	y4.o\
	y5.o\
	y6.o

yacc:	$(OBJ)
	$(CC) $(LDFLAGS) -o $@ $(OBJ)

cmp:
	cmp yacc /bin/yacc
	rm yacc *.o

cp:	all
	cp yacc /bin/yacc
	rm yacc *.o

y0.o y1.o y4.o:	yacc.h
y3.o:		yacc.h assert.h
y2.o y6.o:	yacc.h action.h
y5.o:		yacc.h assert.h action.h

# end of /usr/src/cmd/yacc/Makefile
