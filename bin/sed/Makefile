# /usr/src/cmd/sed/Makefile 2/13/91

OBJ=\
	sed0.o\
	sed1.o\
	sed2.o\
	sed3.o

# Primary target.
sed:	$(OBJ)
	cc $(LDFLAGS) -o $@ $(OBJ)

# Secondary targets.
sed0.o:	sed0.c sed.h
sed1.o:	sed1.c sed.h
sed2.o:	sed2.c sed.h
sed3.o:	sed3.c sed.h

clean:
	rm $(OBJ)

# end of /usr/src/cmd/sed/Makefile
