# lex/Makefile 2/6/91
# Does not include header file dependencies.

SRC=/usr.src/cmd/lex

OBJ=\
	lex0.o\
	lex1.o\
	lex2.o\
	lex3.o

# Primary target.
lex:	$(OBJ)
	cc -o $@ $(OBJ)

# Secondary targets.
lex0.o:	$(SRC)/lex0.c
lex1.o:	$(SRC)/lex1.c
lex2.o:	$(SRC)/lex2.c
lex3.o:	$(SRC)/lex3.c

clean:
	rm $O

# end of lex/Makefile
