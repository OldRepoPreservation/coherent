# /usr/src/cmd/sed/Makefile 5/26/92
OBJ=\
	sed0.o\
	sed1.o\
	sed2.o\
	sed3.o

# Primary target.
sed:	$(OBJ)
	cc $(LDFLAGS) -so $@ $(OBJ)

install: sed
	su root `srcpath installer`

# Secondary targets.
sed0.o:	sed0.c sed.h
sed1.o:	sed1.c sed.h
sed2.o:	sed2.c sed.h
sed3.o:	sed3.c sed.h
# end of /usr/src/cmd/sed/Makefile
