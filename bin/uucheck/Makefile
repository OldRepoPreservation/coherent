# Makefile for checkperms, permissions checking interpreter
# part of uucheck the uucp sanity checker.
#
# Destination directory for installing executable files
BIN=/bin/
# Destination directory for installing data files
# BTW if LIB= anything other than /usr/lib/uucheck/, you will have to edit
# uucheck by hand.
LIB=/usr/lib/uucheck/

SUBPROGS=checkperms/checkperms uucheckname/uucheckname
#
uucheck:	$(SUBPROGS)
	touch $@

checkperms/checkperms:
	(cd checkperms; make)

uucheckname/uucheckname:
	(cd uucheckname; make)

install: uucheck
	(cd checkperms; make install)
	(cd uucheckname; make install)
	cp uucheck $(BIN)

clean:
	(cd checkperms; make clean)
	(cd uucheckname; make clean)
	rm -f *.o core
