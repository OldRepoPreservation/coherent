SRC	= .
CFLAGS	=-O -I$(SRC)

OFILES	= asctab.o div.o env.o expr.o extern.o getf.o hyphen.o hyptab.o\
	main.o misc.o process.o reg.o req1.o req2.o reqmisc.o request.o\
	table.o codebug.o req3.o

TFILES	= tty.o

LDFLAGS	= -i

PFILES	= hpljet.o

HFILES	= hhpljet.o

TSRCS	= asctab.c div.c env.c expr.c extern.c getf.c hyphen.c hyptab.c\
	main.c misc.c process.c reg.c req1.c req2.c reqmisc.c request.c\
	table.c hpljet.c codebug.c req3.c

NSRCS	= asctab.c div.c env.c expr.c extern.c getf.c hyphen.c hyptab.c\
	main.c misc.c process.c reg.c req1.c req2.c reqmisc.c request.c\
	table.c tty.c codebug.c req3.c

troff:	$(OFILES) $(PFILES)
	exec $(CC) $(LDFLAGS) -o troff $(OFILES) $(PFILES)

hroff:	$(OFILES) $(HFILES)
	exec $(CC) $(LDFLAGS) -o hroff $(OFILES) $(HFILES)

nroff:	$(OFILES) $(TFILES)
	exec $(CC) $(LDFLAGS) -o nroff $(OFILES) $(TFILES)

hpljet.o: $(SRC)/roff.h $(SRC)/code.h $(SRC)/char.h $(SRC)/env.h \
	$(SRC)/font.h $(SRC)/hpljet.c

hhpljet.o: $(SRC)/roff.h $(SRC)/code.h $(SRC)/char.h $(SRC)/env.h \
	$(SRC)/font.h $(SRC)/hpljet.c
	exec $(CC) -DOLDHP=1 $(CFLAGS) -c -o hhpljet.o $(SRC)/hpljet.c

asctab.o: $(SRC)/roff.h $(SRC)/char.h $(SRC)/asctab.c

div.o: $(SRC)/roff.h $(SRC)/code.h $(SRC)/div.h $(SRC)/reg.h $(SRC)/str.h \
	$(SRC)/div.c
env.o: $(SRC)/roff.h $(SRC)/code.h $(SRC)/env.h $(SRC)/esc.h $(SRC)/env.c

expr.o: $(SRC)/roff.h $(SRC)/code.h $(SRC)/div.h $(SRC)/env.h $(SRC)/expr.c

extern.o: $(SRC)/roff.h $(SRC)/code.h $(SRC)/env.h \
	$(SRC)/div.h $(SRC)/reg.h $(SRC)/str.h $(SRC)/extern.c

getf.o: $(SRC)/roff.h $(SRC)/code.h $(SRC)/env.h \
	$(SRC)/esc.h $(SRC)/div.h $(SRC)/reg.h $(SRC)/str.h $(SRC)/getf.c

hyphen.o: $(SRC)/roff.h $(SRC)/code.h $(SRC)/hyphen.h \
	$(SRC)/char.h $(SRC)/roff.h $(SRC)/hyphen.c

main.o: $(SRC)/roff.h $(SRC)/code.h $(SRC)/env.h $(SRC)/esc.h \
	$(SRC)/div.h $(SRC)/reg.h $(SRC)/str.h $(SRC)/main.c

misc.o: $(SRC)/roff.h $(SRC)/str.h $(SRC)/misc.c

process.o: $(SRC)/roff.h $(SRC)/char.h $(SRC)/code.h $(SRC)/div.h \
	$(SRC)/env.h $(SRC)/esc.h $(SRC)/hyphen.h $(SRC)/reg.h $(SRC)/process.c

reg.o: $(SRC)/roff.h $(SRC)/reg.h $(SRC)/reg.c

req1.o: $(SRC)/roff.h $(SRC)/code.h $(SRC)/div.h $(SRC)/env.h $(SRC)/esc.h \
	$(SRC)/reg.h $(SRC)/str.h $(SRC)/req1.c

req2.o: $(SRC)/roff.h $(SRC)/code.h $(SRC)/env.h $(SRC)/div.h $(SRC)/reg.h \
	$(SRC)/str.h $(SRC)/esc.h $(SRC)/req2.c

reqmisc.o: $(SRC)/roff.h $(SRC)/code.h $(SRC)/env.h $(SRC)/div.h $(SRC)/reg.h \
	$(SRC)/str.h $(SRC)/reqmisc.c

request.o: $(SRC)/roff.h $(SRC)/reg.h $(SRC)/str.h $(SRC)/request.c

table.o: $(SRC)/roff.h $(SRC)/esc.h $(SRC)/table.c

tty.o: $(SRC)/roff.h $(SRC)/code.h $(SRC)/char.h $(SRC)/env.h $(SRC)/tty.c

codebug.o: $(SRC)/codebug.h $(SRC)/codebug.c

req3.o: $(SRC)/code.h $(SRC)/char.h $(SRC)/roff.h $(SRC)/env.h $(SRC)/req3.c
