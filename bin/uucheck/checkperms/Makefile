# Makefile for checkperms, permissions checking interpreter
# part of uucheck the uucp sanity checker.
#
# Destination directory for installing executable files
BIN=/usr/lib/uucheck/
# Destination directory for installing data files
# BTW if LIB= anything other than /usr/lib/uucheck/, you will have to edit
# uucheck by hand.
LIB=/usr/lib/uucheck/

OBJS=y.tab.o lex.yy.o main.o monitor.o hosts.o do_check.o messages.o \
	chmod_chk.o util.o parse_args.o copyright.o
LIBS=-ly -ll
YACC=yacc
LEX=lex
YACCFLAGS=
LEXFLAGS=
CFLAGS=-O -I..
#
checkperms:	$(OBJS)
	$(CC) -o checkperms $(CFLAGS) $(OBJS) $(LIBS)

y.tab.o:	checkperms.y
	$(YACC) $(YACCFLAGS) checkperms.y
	$(CC) -c $(CFLAGS) y.tab.c

lex.yy.o:	checkperms.lex
	$(LEX) $(LEXFLAGS) checkperms.lex
	$(CC) -c $(CFLAGS) lex.yy.c

main.o:	checkperms.h monitor.h ../uucheck.h

install: checkperms
	cp checkperms $(BIN)uucheckperms
	cp uucp.perms $(LIB)

clean:
	rm -f $(OBJS) y.tab.c y.tab.h lex.yy.c core

