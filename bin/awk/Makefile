# awk/Makefile 2/6/91

SRC=/usr.src/cmd/awk

CFLAGS=-O -I$(SRC)
LDFLAGS=-f
YFLAGS=  -prods 110 -terms 67 -nterms 30 -states 200

OBJ=\
	awk.o\
	awk0.o\
	awk1.o\
	awk2.o\
	awk3.o\
	awk4.o\
	awk5.o\
	awk6.o

# Primary target.
awk:	$(OBJ)
	cc $(LDFLAGS) -o awk $(OBJ) -lm

# Secondary targets.
awk.o:	$(SRC)/awk.y
awk0.o:	$(SRC)/awk.h $(SRC)/y.tab.h $(SRC)/awk0.c
awk1.o:	$(SRC)/awk.h $(SRC)/y.tab.h $(SRC)/awk1.c
awk2.o:	$(SRC)/awk.h $(SRC)/y.tab.h $(SRC)/awk2.c
awk3.o:	$(SRC)/awk.h $(SRC)/y.tab.h $(SRC)/awk3.c
awk4.o:	$(SRC)/awk.h $(SRC)/y.tab.h $(SRC)/awk4.c
awk5.o:	$(SRC)/awk.h $(SRC)/y.tab.h $(SRC)/awk5.c
awk6.o:	$(SRC)/awk.h $(SRC)/y.tab.h $(SRC)/awk6.c

clean:
	rm $(OBJ)

# end of awk/Makefile
