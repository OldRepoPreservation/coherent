# CFLAGS are not needed for i8086
CFLAGS= -O 
UTILS= spellin spell

all:	$(UTILS)
	>all
# include file deftty.h defines the default ERASE, KILL and INTERRUPT 
# characters for ttys.

spell2.o: spell2.c
	$(CC) $(CFLAGS) spell2.c

spell:	spell.c spell2.o
	$(CC) $(CFLAGS) -o spell spell.c spell2.o

spellin: spellin.c spell2.o
	$(CC) $(CFLAGS) -o spellin spellin.c spell2.c

