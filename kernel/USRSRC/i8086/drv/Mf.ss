# (lgl-
# 	COHERENT Driver Kit Version 1.1.0
# 	Copyright (c) 1982, 1990 by Mark Williams Company.
# 	All rights reserved. May not be copied without permission.
# -lgl)
#
# Makefile for Seagate ST01/ST02 SCSI driver "ss"
#
AS=exec /bin/as
CC=exec /bin/cc
CPP=exec /lib/icpp
CFLAGS=-I.. -I../sys -I../.. -I../../sys -I/usr/include/sys
AFLAGS=-gx
OBJECTS=objects/ss.o objects/fdisk.o objects/ssqueue.o

# Include directories
USRINC=/usr/include
SYSINC=/usr/include/sys
KERINC=/usr/src/sys/sys
DRVINC=/usr/src/sys/i8086/sys
USRSYS=/usr/sys

ss: $(USRSYS)/lib/ss.a
	:

$(USRSYS)/lib/ss.a: $(OBJECTS)
	rm -f $(USRSYS)/lib/ss.a
	ar rc $(USRSYS)/lib/ss.a $(OBJECTS)

objects/ss.o:				\
		$(KERINC)/coherent.h	$(SYSINC)/types.h $(SYSINC)/timeout.h \
					$(SYSINC)/machine.h $(SYSINC)/param.h \
					$(SYSINC)/fun.h $(DRVINC)/mmu.h \
		$(SYSINC)/io.h		\
		$(SYSINC)/sched.h	\
		$(SYSINC)/uproc.h	\
		$(SYSINC)/proc.h	\
		$(SYSINC)/con.h		\
		$(SYSINC)/stat.h	\
		$(SYSINC)/devices.h	\
		$(USRINC)/errno.h	\
		$(DRVINC)/ss.h		\
		$(SYSINC)/fdisk.h	\
		$(SYSINC)/hdioctl.h	\
		$(SYSINC)/buf.h		\
		$(DRVINC)/scsiwork.h	\
		ss.c
	$(CC) $(CFLAGS) -DVERBOSE=1 -c -o objects/ss.o ss.c

objects/ssqueue.o:			\
		$(KERINC)/coherent.h	$(SYSINC)/types.h $(SYSINC)/timeout.h \
					$(SYSINC)/machine.h $(SYSINC)/param.h \
					$(SYSINC)/fun.h $(DRVINC)/mmu.h \
		$(SYSINC)/buf.h		\
		$(DRVINC)/scsiwork.h	\
		ssqueue.c
	$(CC) $(CFLAGS) -c -o $@ ssqueue.c

objects/fdisk.o:			\
		$(SYSINC)/buf.h		\
		$(KERINC)/coherent.h	$(SYSINC)/types.h $(SYSINC)/timeout.h \
					$(SYSINC)/machine.h $(SYSINC)/param.h \
					$(SYSINC)/fun.h $(DRVINC)/mmu.h \
		$(SYSINC)/con.h \
		$(USRINC)/errno.h	\
		$(SYSINC)/fdisk.h	\
		$(SYSINC)/inode.h	\
		$(SYSINC)/uproc.h	\
		fdisk.c
	$(CC) $(CFLAGS) -c -o $@ fdisk.c
