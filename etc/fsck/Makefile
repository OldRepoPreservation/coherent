#
#	for Inetco	: CFLAGS = -O -DHPCOHERENT
#	for PDP-11	: CFLAGS = -O
#	others		: CFLAGS =
#

CFLAGS = -OA
#
#	for PDP-11 and IBM PC:	LFLAGS = -i
#	for Z8000	     :	LFLAGS =
#
LFLAGS =
OBJ=main.o init.o phase1.o phase1b.o phase2.o phase3.o phase4.o phase5.o \
	phase6.o cleanup.o util.o link.o virtual.o is_fs.o printf.o

fsck:	$(OBJ)
	$(CC) $(CFLAGS) $(LFLAGS) -s -o fsck $(OBJ)

install: fsck
	su root cp fsck /etc/fsck
	su root chmog 511 bin bin /etc/fsck

#
#	for IBM PC Coherent executable, use:  -DNBLK=1
#	for others, use			   :  -DNBLK=8
#
#	If you are running out of malloc space, try lowering the
#	value of NBLK (no smaller than 1).
#

util.o: util.c
	$(CC) -c $(CFLAGS) $< -DNBLK=8

$(OBJ):	fsck.h
