# build/Makefile 11/2/90
DESTDIR=/interim/etc/
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
	fdisk \
	install \
	mkdev 

# Primary target.
all:	$E
	: all done

# Executables.
build:		build.o numtab.o serialno.o build0.o
	$(CC) $(LFLAGS) -o $(DESTDIR)$@ build.o numtab.o serialno.o build0.o -f
	su root chown root $(DESTDIR)$@
	su root chgrp root $(DESTDIR)$@
	su root chmod 500 $(DESTDIR)$@

coh_intro:	coh_intro.o build0.o
	$(CC) $(LFLAGS) -o $(DESTDIR)$@ coh_intro.o build0.o
	su root chown bin $(DESTDIR)$@
	su root chgrp bin $(DESTDIR)$@
	su root chmod 511 $(DESTDIR)$@

cohpatch:	cohpatch.o build0.o
	$(CC) $(LFLAGS) -o $(DESTDIR)$@ cohpatch.o build0.o

fdisk:		fdisk.o
	$(CC) $(LFLAGS) -o $(DESTDIR)$@ fdisk.o -f
	su root chown root $(DESTDIR)$@
	su root chgrp root $(DESTDIR)$@
	su root chmod 500 $(DESTDIR)$@

install:	install.o build0.o
	$(CC) $(LFLAGS) -o $(DESTDIR)$@ install.o build0.o
	su root chown root $(DESTDIR)$@
	su root chgrp root $(DESTDIR)$@
	su root chmod 500 $(DESTDIR)$@

mkdev:	mkdev.o build0.o
	$(CC) $(LFLAGS) -o $(DESTDIR)$@ mkdev.o build0.o
	su root chown root $(DESTDIR)$@
	su root chgrp root $(DESTDIR)$@
	su root chmod 500 $(DESTDIR)$@

mkserial:	mkserial.o numtab.o
	$(CC) $(LFLAGS) -o $(DESTDIR)$@ mkserial.o numtab.o

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
	strip $(DESTDIR)$E
