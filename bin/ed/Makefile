# /usr/src/cmd/ed/Makefile 1/23/90

# for i8086 LFLAGS = -i -n
CFLAGS=-O
LFLAGS=-i -n

OBJECTS=ed0.o ed1.o ed2.o ed3.o

ed: $(OBJECTS)
	cc $(LFLAGS) -o ed $(OBJECTS)

ed0.o:	ed.h
ed1.o:	ed.h
ed2.o:	ed.h
ed3.o:	ed.h
