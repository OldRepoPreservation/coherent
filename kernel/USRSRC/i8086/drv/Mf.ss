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

# Include directories
USRINC=/usr/include
SYSINC=/usr/include/sys
KERINC=/usr/src/sys/sys
DRVINC=/usr/src/sys/i8086/sys
USRSYS=/usr/sys

ss: $(USRSYS)/lib/ss.a
	:

$(USRSYS)/lib/ss.a: objects/ss.o
	rm -f $(USRSYS)/lib/ss.a
	ar rc $(USRSYS)/lib/ss.a objects/ss.o

objects/ss.o: ss.c
	$(CC) $(CFLAGS) -DVERBOSE=1 -c -o objects/ss.o ss.c
