# ld/Makefile 5/14/91
# This is a makefile that creates the loader for the i8086 machine .
#					Angus Telfer (June 11, 1985)
#
# $Log:	/newbits/bin/ld/RCS/Makefile,v $
#Revision 1.1	91/05/15  09:44:17 	bin
#Initial revision

# Revision 1.1	89/07/21  15:52:06 	src
# Initial revision
# 

# set this to maximum size for in memory load (e.g. 16384)
BREADBOX=16384

DESTDIR=/interim/bin/
CFLAGS=-O -DBREADBOX=$(BREADBOX)
LFLAGS=-i -s

OBJ=	main.o\
	data.o\
	message.o\
	pass1.o\
	pass2.o

# Primary target.
ld:	$(OBJ)
	$(CC) $(LFLAGS) -o $(DESTDIR)$@ $<
	su root chmog 511 bin bin $(DESTDIR)$@

# Unlinked objects.
main.o:		main.c data.h
data.o:		data.c 	data.h
message.o:	message.c data.h
pass1.o:	pass1.c	data.h
pass2.o:	pass2.c	data.h

# Etc.
clean:
	rm *.o
install:	ld
	cp -d ld /bin/ld

# end of ld/Makefile
