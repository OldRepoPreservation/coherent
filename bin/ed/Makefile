# /usr/src/cmd/ed/Makefile 2/20/91

# for i8086 LDFLAGS = -i -n
CFLAGS=-O
LDFLAGS=-i -n

OBJ=ed0.o ed1.o ed2.o ed3.o

ed: $(OBJ)
	cc $(LDFLAGS) -o $@ $(OBJ)

ed0.o:	ed.h ed0.c
ed1.o:	ed.h ed1.c
ed2.o:	ed.h ed2.c
ed3.o:	ed.h ed3.c

clean:
	rm $(OBJ)

# end of /usr/src/cmd/ed/Makefile
