# as/Makefile 2/7/91
# A previous needlessly complicated Makefile made any of several
# machine-dependent versions.
# This is much simpler but may need adjusting for non-i8086 versions.

# SRC defines the root directory of as source, MCH the target machine.
SRC=/usr.src/cmd/as
MCH=i8086

# Add source and mch directories to include path.
CFLAGS=-I$(SRC) -I$(SRC)/$(MCH)

OBJ=\
	asm.o\
	asmexpr.o\
	asmlex.o\
	data.o\
	grindy.o\
	list.o\
	subr.o\
	sym.o

MCHOBJ=\
	addr.o\
	asmout.o\
	machine.o\
	pst.o

# Primary target.
as:	$(OBJ) $(MCHOBJ)
	$(CC) $(LDFLAGS) -o $@ $(OBJ) $(MCHOBJ)

# Secondary targets.
# Machine independent.
asm.o:		$(SRC)/asm.c
asmexpr.o:	$(SRC)/asmexpr.c
asmlex.o:	$(SRC)/asmlex.c
data.o:		$(SRC)/data.c
grindy.o:	$(SRC)/grindy.c
list.o:		$(SRC)/list.c
subr.o:		$(SRC)/subr.c
sym.o:		$(SRC)/sym.c
# Machine dependent.
addr.o:		$(SRC)/$(MCH)/addr.c
asmout.o:	$(SRC)/$(MCH)/asmout.c
machine.o:	$(SRC)/$(MCH)/machine.c
pst.o:		$(SRC)/$(MCH)/pst.c
# Other.
clean:
	rm $(OBJ) $(MCHOBJ)

# end of as/Makefile
