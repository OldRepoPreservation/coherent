# me/Makefile 3/14/90

SRC=.
CFLAGS=-O -I$(SRC)
LDFLAGS=-n -lterm

OBJS=\
	ansi.o\
	basic.o\
	bracket.o\
	buffer.o\
	comtab.o\
	display.o\
	error.o\
	execute.o\
	file.o\
	fileio.o\
	helplib.o\
	line.o\
	lookup.o\
	main.o\
	random.o\
	region.o\
	search.o\
	spawn.o\
	tcap.o\
	termio.o\
	vt52.o\
	window.o\
	word.o

# Primary target.
me:	$(OBJS)
	$(CC) -o me $(OBJS) $(LDFLAGS)

# Secondary targets.
comtab.c:	ext.c $(SRC)/ed.h
	$(CC) ext.c
	ext

ansi.o:		$(SRC)/ansi.c $(SRC)/ed.h
basic.o:	$(SRC)/basic.c $(SRC)/ed.h
bracket.o:	$(SRC)/bracket.c $(SRC)/ed.h
buffer.o:	$(SRC)/buffer.c $(SRC)/ed.h
comtab.o:	comtab.c $(SRC)/ed.h
display.o:	$(SRC)/display.c $(SRC)/ed.h
error.o:	$(SRC)/error.c $(SRC)/ed.h
execute.o:	execute.c $(SRC)/ed.h
file.o:		$(SRC)/file.c $(SRC)/ed.h
fileio.o:	$(SRC)/fileio.c $(SRC)/ed.h
helplib.o:	$(SRC)/helplib.c
line.o:		$(SRC)/line.c $(SRC)/ed.h
lookup.o:	$(SRC)/lookup.c
main.o:		$(SRC)/main.c $(SRC)/ed.h
random.o:	$(SRC)/random.c $(SRC)/ed.h
region.o:	$(SRC)/region.c $(SRC)/ed.h
search.o:	$(SRC)/search.c $(SRC)/ed.h
spawn.o:	$(SRC)/spawn.c $(SRC)/ed.h
tcap.o:		$(SRC)/tcap.c $(SRC)/ed.h
termio.o:	$(SRC)/termio.c $(SRC)/ed.h
vt52.o:		$(SRC)/vt52.c $(SRC)/ed.h
window.o:	$(SRC)/window.c $(SRC)/ed.h
word.o:		$(SRC)/word.c $(SRC)/ed.h
