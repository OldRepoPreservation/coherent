# /usr/src/cmd/dos/Makefile 3/26/90
# Does not include header file dependencies.

# CFLAGS=-DDEBUG
DESTDIR= /f1/bin/

OBJ=\
	dos0.o\
	dos1.o\
	dos2.o\
	dos3.o\
	dos4.o
HDR=\
	bpb.h\
	dos0.h\
	dos1.h\
	mdir.h
SRC=\
	dos0.c\
	dos1.c\
	dos2.c\
	dos3.c\
	dos4.c\
	dosshrink.c

# Primary targets.
dos:	$(OBJ)
	$(CC) -i -o $(DESTDIR)dos $(OBJ)
	su root chown bin $(DESTDIR)dos
	su root chgrp bin $(DESTDIR)dos
	su root chmod 511 $(DESTDIR)dos

dosshrink:	dosshrink.o dos1.o
	$(CC) -i -o $(DESTDIR)dosshrink dosshrink.o dos1.o -f

# Secondary targets.
dos0.o:		dos0.c
dos1.o:		dos1.c
dos2.o:		dos2.c
dos3.o:		dos3.c
dos4.o:		dos4.c
dosshrink.o:	dosshrink.c

# Etc.
clean:
	rm $(OBJ) pfm.o
disk:
	su root dd if=/dev/f9a0 of=disk count=32 bs=20b
floppy:
	su root dd of=/dev/f9a0 if=disk count=32 bs=20b
listing:
	prints $(HDR) $(SRC)
dos.tar.Z:
	tar cvf dos.tar dos dosshrink Makefile $(HDR) $(SRC)
	compress dos.tar

# end of /usr/src/cmd/dos/Makefile
