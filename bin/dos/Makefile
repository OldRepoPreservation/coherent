# dos/Makefile 3/26/90
# Does not include header file dependencies.

CFLAGS= -A
#CFLAGS=-DDEBUG 
OBJ=\
	copyright.o\
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
	copyright.c\
	dos0.c\
	dos1.c\
	dos2.c\
	dos3.c\
	dos4.c\
	dosshrink.c

# Primary targets.
dos:	$(OBJ)
	$(CC) -i -o dos $(OBJ)
	ln -f dos dosdir
	ln -f dos dosls	
	ln -f dos dosmkdir
	ln -f dos doscp
	ln -f dos doscpdir
	ln -f dos dosdel
	ln -f dos doscat
	ln -f dos dosrm
	ln -f dos dosrmdir
	ln -f dos dosformat
	ln -f dos doslabel
dosshrink:	dosshrink.o dos1.o
	$(CC) -i -o dosshrink dosshrink.o dos1.o -f

# Secondary targets.
dos0.o:		dos0.c $(HDR)
dos1.o:		dos1.c $(HDR)
dos2.o:		dos2.c $(HDR)
dos3.o:		dos3.c $(HDR)
dos4.o:		dos4.c $(HDR)
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
