# /usr/src/cmd/lpr/Makefile 11/13/90
# Make normal and hp versions of the line printer daemon and spooler.
# This also makes the hp filter,
# for converting nroff output to HP LaserJet printer escape sequences (PCL).

ALL=\
	hp\
	hpd\
	hpr\
	hpskip\
	lpd\
	lpr\
	lpskip\
	prps
OBJ=\
	hp.o\
	hpr.o\
	hpskip.o\
	hpd1.o\
	hpprint.o\
	lpd1.o\
	lpd2.o\
	lpr.o\
	lpskip.o\
	print.o\
	prps.o

# Primary target.
all:	$(ALL)
	: all done

# Secondary targets.
hp:	hp.o
	cc $?
hpd:	hpd1.o lpd2.o hpprint.o
	cc -o $@ hpd1.o lpd2.o hpprint.o
hpr:	hpr.o
	cc $?
hpskip:	hpskip.o
	cc $?
lpr:	lpr.o
	cc $?
lpd:	lpd1.o lpd2.o print.o
	cc -o $@ $?
lpskip:	lpskip.o
	cc $?
prps:	prps.o
	cc $?

# Unlinked objects.
hpr.o:	lpr.c
	cc -c -o $@ -DLASER $?
hpskip.o:	lpskip.c
	cc -c -o $@ -DLASER $?
hpd1.o:	lpd1.c
	cc -c -o $@ -DLASER $?

# Etc.
clean:
	rm $(ALL) $(OBJ)
strip:	$(ALL)
	strip $(ALL)

# end of /usr/src/cmd/lpr/Makefile
