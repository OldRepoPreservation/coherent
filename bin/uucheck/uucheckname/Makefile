# Makefile for uucheckname, permissions checking interpreter
# part of uucheck the uucp sanity checker.
#
# Set this according to where you want this installed.
BIN=/mnt/bin/

LIBS=
OBJS=main.o uucheckname.o gethostname.o parse_args.o lookup.o interact.o copyright.o
CFLAGS=-O -I..
#
uucheckname:	$(OBJS)
	$(CC) -o $@ $(CFLAGS) $(OBJS) $(LIBS)

install: uucheckname
	cp uucheckname $(BIN)

clean:
	rm -f $(OBJS) core
