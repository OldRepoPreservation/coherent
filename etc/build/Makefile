# build/Makefile 11/2/90

# Objects.
O=\
	build.o \
	build0.o \
	coh_intro.o \
	cohpatch.o \
	fdisk.o \
	install.o \
	mkdev.o \
	mkserial.o \
	numtab.o \
	serialno.o

# Executables.
E=\
	build \
	coh_intro \
	cohpatch \
	fdisk \
	install \
	mkdev \
	mkserial

# Primary target.
all:	$E
	: all done

# Executables.
build:		build.o numtab.o serialno.o build0.o
	$(CC) $(LFLAGS) -o $@ build.o numtab.o serialno.o build0.o -f
coh_intro:	coh_intro.o build0.o
	$(CC) $(LFLAGS) -o $@ coh_intro.o build0.o
cohpatch:	cohpatch.o build0.o
	$(CC) $(LFLAGS) -o $@ cohpatch.o build0.o
fdisk:		fdisk.o
	$(CC) $(LFLAGS) -o $@ fdisk.o -f
install:	install.o build0.o
	$(CC) $(LFLAGS) -o $@ install.o build0.o
mkdev:	mkdev.o build0.o
	$(CC) $(LFLAGS) -o $@ mkdev.o build0.o
mkserial:	mkserial.o numtab.o
	$(CC) $(LFLAGS) -o $@ mkserial.o numtab.o

# Unlinked objects.
build.o:	build.c build0.h serialno.h
build0.o:	build0.c build0.h
coh_intro.o:	coh_intro.c build0.h
cohpatch.o:	cohpatch.c build0.h
fdisk.o:	fdisk.c fdisk0.h
install.o:	install.c build0.h
mkserial.o:	mkserial.c serialno.h
numtab.o:	numtab.c serialno.h
serialno.o:	serialno.c build0.h serialno.h

# Etc.
strip:	$E
	strip $E
clean:
	rm $O
