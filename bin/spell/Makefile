# CFLAGS are not needed for i8086
DESTDIR=/f1/bin/
DESTDIR2=/f1/usr/dict/
CFLAGS= -O 
UTILS= spellin spell

all:	$(UTILS)
	>all
# include file deftty.h defines the default ERASE, KILL and INTERRUPT 
# characters for ttys.

spell2.o: spell2.c
	$(CC) $(CFLAGS) spell2.c

spell:	spell.c spell2.o
	$(CC) $(CFLAGS) -o $(DESTDIR)spell spell.c spell2.o
	su root chown bin $(DESTDIR)spell
	su root chgrp bin $(DESTDIR)spell
	su root chmod 555 $(DESTDIR)spell

spellin: spellin.c spell2.o
	$(CC) $(CFLAGS) -o $(DESTDIR2)spellin spellin.c spell2.c
	su root chown bin $(DESTDIR2)spellin
	su root chgrp bin $(DESTDIR2)spellin
	su root chmod 511 $(DESTDIR2)spellin
