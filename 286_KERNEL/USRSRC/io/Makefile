# (lgl-
# 	COHERENT Driver Kit Version 1.1.0
# 	Copyright (c) 1982, 1990 by Mark Williams Company.
# 	All rights reserved. May not be copied without permission.
# -lgl)
#
# Makefile for AT specific Coherent drivers
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

# Loadable driver directory
LDRV=/usr/sys/ldrv

ARCHIVES=$(USRSYS)/lib/aha154x.a \
	$(USRSYS)/lib/al.a \
	$(USRSYS)/lib/at.a \
	$(USRSYS)/lib/ati.a \
	$(USRSYS)/lib/fl.a \
	$(USRSYS)/lib/gr.a \
	$(USRSYS)/lib/hs.a \
	$(USRSYS)/lib/lp.a \
	$(USRSYS)/lib/mm.a \
	$(USRSYS)/lib/ms.a \
	$(USRSYS)/lib/rm.a \
	$(USRSYS)/lib/rs.a \
	$(USRSYS)/lib/st.a \
	$(USRSYS)/lib/tn.a \

DRVOBJ=	objects/aha.o \
	objects/alx.o \
	objects/at.o \
	objects/atas.o \
	objects/bufq.o \
	objects/ms.o \
	objects/ati.o \
	objects/com1.o objects/com2.o \
	objects/fdisk.o \
	objects/fl.o \
	objects/fontw.o \
	objects/gr.o objects/gras.o \
	objects/hs.o \
	objects/nkb.o objects/mm.o \
	objects/lp.o \
	objects/mmas.o \
	objects/rm.o \
	objects/rs0.o objects/rs1.o objects/rsas.o \
	objects/scsi.o \
	objects/ss.o \
	objects/ssas.o \
	objects/st.o \
	objects/tn.o objects/tnas.o \

DRIVERS=$(LDRV)/aha154x \
	$(LDRV)/al0 \
	$(LDRV)/al1 \
	$(LDRV)/at \
	$(LDRV)/fl \
	$(LDRV)/gr \
	$(LDRV)/hs \
	$(LDRV)/lp \
	$(LDRV)/mm \
	$(LDRV)/ms \
	$(LDRV)/rm \
	$(LDRV)/ss \

install: $(ARCHIVES) $(DRIVERS)
	@exec /bin/sync

all:	$(DRVOBJ)
	@exec /bin/sync

$(USRSYS)/lib/aha154x.a: objects/scsi.o objects/aha.o objects/fdisk.o
	rm -f $@
	ar rc $@ $<
$(USRSYS)/lib/al.a: objects/com1.o objects/com2.o objects/alx.o
	rm -f $@
	ar rc $@ $<
$(USRSYS)/lib/at.a: objects/at.o objects/atas.o objects/fdisk.o
	rm -f $@
	ar rc $@ $<
$(USRSYS)/lib/ati.a: objects/mm.o objects/ati.o objects/nkb.o
	rm -f $@
	ar rc $@ $<
$(USRSYS)/lib/fl.a: objects/fl.o
	rm -f $(USRSYS)/lib/fl.a
	ar rc $(USRSYS)/lib/fl.a objects/fl.o
$(USRSYS)/lib/gr.a: objects/mm.o objects/gr.o objects/gras.o \
				objects/fontw.o objects/nkb.o
	rm -f $@
	ar rc $@ $<
$(USRSYS)/lib/hs.a: objects/hs.o
	rm -f $@
	ar rc $@ $<
$(USRSYS)/lib/lp.a: objects/lp.o
	rm -f $@
	ar rc $@ $<
$(USRSYS)/lib/mm.a: objects/mm.o objects/mmas.o objects/nkb.o
	rm -f $@
	ar rc $@ $<
$(USRSYS)/lib/ms.a: objects/ms.o
	rm -f $@
	ar rc $@ $<
$(USRSYS)/lib/rm.a: objects/rm.o
	rm -f $@
	ar rc $@ $<
$(USRSYS)/lib/rs.a: objects/rs0.o objects/rs1.o objects/rsas.o
	rm -f $@
	ar rc $@ $<
$(USRSYS)/lib/ss.a: objects/ss.o objects/ssas.o objects/bufq.o objects/fdisk.o
	rm -f $@
	ar rc $@ $<
$(USRSYS)/lib/st.a: objects/st.o
	rm -f $@
	ar rc $@ $<
$(USRSYS)/lib/tn.a: objects/tn.o objects/tnas.o
	rm -f $@
	ar rc $@ $<

objects/aha.o:				\
		$(KERINC)/coherent.h	$(SYSINC)/types.h $(SYSINC)/timeout.h \
					$(SYSINC)/machine.h $(SYSINC)/param.h \
					$(SYSINC)/fun.h $(DRVINC)/mmu.h \
		$(SYSINC)/buf.h		\
		$(SYSINC)/sched.h		\
		$(DRVINC)/scsiwork.h	\
		$(DRVINC)/aha154x.h	\
		aha.c
	$(CC) $(CFLAGS) -c -o $@ aha.c

objects/alx.o:				\
		$(KERINC)/clist.h	\
		$(KERINC)/coherent.h	$(SYSINC)/types.h $(SYSINC)/timeout.h \
					$(SYSINC)/machine.h $(SYSINC)/param.h \
					$(SYSINC)/fun.h \
		$(SYSINC)/con.h		\
		$(USRINC)/errno.h	\
		$(DRVINC)/i8086.h	\
		$(DRVINC)/ins8250.h	\
		$(SYSINC)/sched.h	\
		$(SYSINC)/stat.h	\
		$(SYSINC)/timeout.h	\
		$(SYSINC)/tty.h		$(KERINC)/ktty.h \
		$(SYSINC)/uproc.h	\
		alx.c
	$(CC) $(CFLAGS) -c -o objects/alx.o alx.c

objects/at.o: at.c
	$(CC) $(CFLAGS) -DVERBOSE=1 -c -o $@ $<

objects/atas.o: atas.s
	$(AS) -go $@ $<

objects/ati.o: ati.s
	$(CPP) -E -DATI_132=1 ati.s > ati.i
	$(AS) -gxo objects/ati.o ati.i
	exec /bin/rm -f ati.i

objects/bufq.o:			\
		$(KERINC)/coherent.h	$(SYSINC)/types.h $(SYSINC)/timeout.h \
					$(SYSINC)/machine.h $(SYSINC)/param.h \
					$(SYSINC)/fun.h $(DRVINC)/mmu.h \
		$(SYSINC)/buf.h		\
		bufq.c
	$(CC) $(CFLAGS) -DDEBUG=$(DEBUG) -c -o $@ bufq.c

objects/com1.o: 			\
		$(DRVINC)/al.h		\
		$(KERINC)/clist.h	\
		$(KERINC)/coherent.h	$(SYSINC)/types.h $(SYSINC)/timeout.h \
					$(SYSINC)/machine.h $(SYSINC)/param.h \
					$(SYSINC)/fun.h \
		$(SYSINC)/con.h		\
		$(USRINC)/errno.h	\
		$(DRVINC)/i8086.h	\
		$(DRVINC)/ins8250.h	\
		$(SYSINC)/sched.h	\
		$(SYSINC)/stat.h	\
		$(SYSINC)/timeout.h	\
		$(SYSINC)/tty.h		$(KERINC)/ktty.h \
		$(SYSINC)/uproc.h	\
		al.c
	$(CC) $(CFLAGS) -DALCOM1=1 -c -o $@ al.c

objects/com2.o: 			\
		$(DRVINC)/al.h		\
		$(KERINC)/clist.h	\
		$(KERINC)/coherent.h	$(SYSINC)/types.h $(SYSINC)/timeout.h \
					$(SYSINC)/machine.h $(SYSINC)/param.h \
					$(SYSINC)/fun.h \
		$(SYSINC)/con.h		\
		$(USRINC)/errno.h	\
		$(DRVINC)/i8086.h	\
		$(DRVINC)/ins8250.h	\
		$(SYSINC)/sched.h	\
		$(SYSINC)/stat.h	\
		$(SYSINC)/timeout.h	\
		$(SYSINC)/tty.h		$(KERINC)/ktty.h \
		$(SYSINC)/uproc.h	\
		al.c
	$(CC) $(CFLAGS) -DALCOM2=1 -c -o $@ al.c

objects/dmareq.o:			\
		$(SYSINC)/buf.h		\
		$(KERINC)/coherent.h	$(SYSINC)/types.h $(SYSINC)/timeout.h \
					$(SYSINC)/machine.h $(SYSINC)/param.h \
					$(SYSINC)/fun.h \
		$(SYSINC)/con.h		\
		$(DRVINC)/dmac.h	\
		$(USRINC)/errno.h	\
		$(SYSINC)/io.h		\
		$(SYSINC)/proc.h	$(SYSINC)/types.h $(SYSINC)/poll.h \
		$(SYSINC)/sched.h	\
		$(SYSINC)/seg.h		\
		$(SYSINC)/stat.h	\
		$(SYSINC)/uproc.h	\
		dmareq.c
	$(CC) $(CFLAGS) -c -o $@ dmareq.c

objects/fdisk.o:			\
		$(SYSINC)/buf.h		\
		$(KERINC)/coherent.h	$(SYSINC)/types.h $(SYSINC)/timeout.h \
					$(SYSINC)/machine.h $(SYSINC)/param.h \
					$(SYSINC)/fun.h \
		$(SYSINC)/con.h		\
		$(USRINC)/errno.h	\
		$(SYSINC)/fdisk.h	\
		$(SYSINC)/inode.h	\
		$(SYSINC)/uproc.h	\
		fdisk.c
	$(CC) $(CFLAGS) -c -o $@ fdisk.c

objects/fl.o:				\
		$(SYSINC)/buf.h		\
		$(KERINC)/coherent.h	$(SYSINC)/types.h $(SYSINC)/timeout.h \
					$(SYSINC)/machine.h $(SYSINC)/param.h \
					$(SYSINC)/fun.h \
		$(SYSINC)/con.h		\
		$(DRVINC)/dmac.h	\
		$(USRINC)/errno.h	\
		$(SYSINC)/fdioctl.h	\
		$(DRVINC)/i8086.h	\
		$(SYSINC)/sched.h	\
		$(SYSINC)/stat.h	\
		$(SYSINC)/timeout.h	\
		$(SYSINC)/uproc.h	\
		fl.c
	$(CC) $(CFLAGS) -c -o $@ fl.c

objects/fontw.o: tools/fontgen.c
	$(CC) -o tools/fontgen tools/fontgen.c
	exec tools/fontgen > fontw.s
	exec /bin/rm tools/fontgen
	$(AS) -gxo objects/fontw.o fontw.s
	exec /bin/rm fontw.s

objects/gr.o:				\
		$(KERINC)/coherent.h	$(SYSINC)/types.h $(SYSINC)/timeout.h \
					$(SYSINC)/machine.h $(SYSINC)/param.h \
					$(SYSINC)/fun.h \
		$(SYSINC)/con.h		\
		$(USRINC)/errno.h	\
		$(SYSINC)/sched.h	\
		$(SYSINC)/timeout.h	\
		$(SYSINC)/types.h	\
		$(SYSINC)/uproc.h	\
		gr.c
	$(CC) $(CFLAGS) -c -o $@ gr.c

objects/gras.o: gras.s
	$(CPP) -E gras.s > gras.i
	$(AS) -gxo $@ gras.i
	exec /bin/rm -f gras.i

objects/hgas.o: gras.s
	$(CPP) -E -DHERCULES gras.s > hgas.i
	$(AS) -gxo $@ hgas.i
	exec /bin/rm -f hgas.i

objects/hd.o: hd.c
	$(CC) $(CFLAGS) -c -o $@ hd.c

objects/hs.o:				\
		$(KERINC)/coherent.h	$(SYSINC)/types.h $(SYSINC)/timeout.h \
					$(SYSINC)/machine.h $(SYSINC)/param.h \
					$(SYSINC)/fun.h \
		$(SYSINC)/con.h		\
		$(USRINC)/errno.h	\
		$(DRVINC)/ins8250.h	\
		$(SYSINC)/proc.h	$(SYSINC)/types.h $(SYSINC)/poll.h \
		$(SYSINC)/stat.h	\
		$(SYSINC)/tty.h		$(KERINC)/ktty.h \
		$(SYSINC)/uproc.h	\
		hs.c
	$(CC) $(CFLAGS) -c -o $@ hs.c

objects/nkb.o:				\
		$(KERINC)/coherent.h	$(SYSINC)/types.h $(SYSINC)/timeout.h \
					$(SYSINC)/machine.h $(SYSINC)/param.h \
					$(SYSINC)/fun.h \
		$(SYSINC)/con.h		\
		$(USRINC)/errno.h	\
		$(DRVINC)/i8086.h	\
		$(SYSINC)/sched.h	\
		$(USRINC)/signal.h	\
		$(SYSINC)/stat.h	\
		$(SYSINC)/timeout.h	\
		$(SYSINC)/tty.h		$(KERINC)/ktty.h \
		$(SYSINC)/uproc.h	\
		nkb.c
	$(CC) $(CFLAGS) -c -o $@ nkb.c

objects/lp.o:				\
		$(KERINC)/coherent.h	$(SYSINC)/types.h $(SYSINC)/timeout.h \
					$(SYSINC)/machine.h $(SYSINC)/param.h \
					$(SYSINC)/fun.h \
		$(SYSINC)/con.h		\
		$(USRINC)/errno.h	\
		$(DRVINC)/i8086.h	\
		$(SYSINC)/io.h		\
		$(SYSINC)/proc.h	$(SYSINC)/types.h $(SYSINC)/poll.h \
		$(SYSINC)/stat.h	\
		$(SYSINC)/timeout.h	\
		$(SYSINC)/uproc.h	\
		lp.c
	$(CC) $(CFLAGS) -c -o $@ lp.c

objects/mm.o:				\
		$(KERINC)/coherent.h	$(SYSINC)/types.h $(SYSINC)/timeout.h \
					$(SYSINC)/machine.h $(SYSINC)/param.h \
					$(SYSINC)/fun.h \
		$(SYSINC)/sched.h	\
		$(USRINC)/errno.h	\
		$(SYSINC)/stat.h	\
		$(SYSINC)/io.h		\
		$(SYSINC)/tty.h		$(KERINC)/ktty.h \
		$(SYSINC)/uproc.h	\
		$(SYSINC)/timeout.h	\
		mm.c
	$(CC) $(CFLAGS) -c -o $@ mm.c

objects/mmas.o: mmas.s
	-$(CPP) -E mmas.s > mmas.i
	$(AS) -gxo $@ mmas.i
	exec /bin/rm -f mmas.i

objects/ms.o:				\
		$(KERINC)/coherent.h	$(SYSINC)/types.h $(SYSINC)/timeout.h \
					$(SYSINC)/machine.h $(SYSINC)/param.h \
					$(SYSINC)/fun.h \
		$(SYSINC)/uproc.h	\
		$(SYSINC)/con.h		\
		$(SYSINC)/ms.h		\
		$(USRINC)/errno.h	\
		ms.c
	$(CC) $(CFLAGS) -c -o $@ ms.c

objects/rm.o: rm.c
	$(CC) $(CFLAGS) -c -o $@ rm.c

objects/rs0.o:				\
		$(KERINC)/coherent.h	$(SYSINC)/types.h $(SYSINC)/timeout.h \
					$(SYSINC)/machine.h $(SYSINC)/param.h \
					$(SYSINC)/fun.h \
		$(SYSINC)/con.h		\
		$(USRINC)/errno.h	\
		$(DRVINC)/ins8250.h	\
		$(SYSINC)/proc.h	$(SYSINC)/types.h $(SYSINC)/poll.h \
		$(SYSINC)/sched.h	\
		$(SYSINC)/stat.h	\
		$(USRINC)/termio.h	\
		$(SYSINC)/uproc.h	\
		rs.c
	$(CC) $(CFLAGS) -DRS0 -c -o $@ rs.c

objects/rs1.o: 				\
		$(KERINC)/coherent.h	$(SYSINC)/types.h $(SYSINC)/timeout.h \
					$(SYSINC)/machine.h $(SYSINC)/param.h \
					$(SYSINC)/fun.h \
		$(SYSINC)/con.h		\
		$(USRINC)/errno.h	\
		$(DRVINC)/ins8250.h	\
		$(SYSINC)/proc.h	$(SYSINC)/types.h $(SYSINC)/poll.h \
		$(SYSINC)/sched.h	\
		$(SYSINC)/stat.h	\
		$(USRINC)/termio.h	\
		$(SYSINC)/uproc.h	\
		rs.c
	$(CC) $(CFLAGS) -DRS1 -c -o $@ rs.c

objects/rsas.o: rsas.s
	$(AS) -gxo $@ rsas.s

objects/scsi.o:				\
		$(KERINC)/coherent.h	$(SYSINC)/types.h $(SYSINC)/timeout.h \
					$(SYSINC)/machine.h $(SYSINC)/param.h \
					$(SYSINC)/fun.h $(DRVINC)/mmu.h \
		$(SYSINC)/fdisk.h	\
		$(SYSINC)/hdioctl.h	\
		$(SYSINC)/sdioctl.h	\
		$(SYSINC)/buf.h		\
		$(SYSINC)/con.h		\
		$(SYSINC)/stat.h	\
		$(SYSINC)/uproc.h	\
		$(USRINC)/errno.h	\
		$(DRVINC)/scsiwork.h	\
		scsi.c
	$(CC) $(CFLAGS) -c -o objects/scsi.o scsi.c

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
	$(CC) $(CFLAGS) -DDEBUG=$(DEBUG) -c -o objects/ss.o ss.c

objects/ssas.o:				\
		ssas.s
	$(AS) -go $@ $<

objects/st.o:				\
		$(SYSINC)/buf.h		\
		$(KERINC)/coherent.h	$(SYSINC)/types.h $(SYSINC)/timeout.h \
					$(SYSINC)/machine.h $(SYSINC)/param.h \
					$(SYSINC)/fun.h \
		$(SYSINC)/con.h		\
		$(SYSINC)/const.h	\
		$(USRINC)/errno.h	\
		$(SYSINC)/inode.h	\
		$(SYSINC)/mtioctl.h	\
		$(SYSINC)/sched.h	\
		$(SYSINC)/seg.h		\
		$(SYSINC)/stat.h	\
		$(SYSINC)/uproc.h	\
		st.c
	$(CC) $(CFLAGS) -c -o $@ st.c

objects/tn.o:				\
		$(KERINC)/coherent.h	$(SYSINC)/types.h $(SYSINC)/timeout.h \
					$(SYSINC)/machine.h $(SYSINC)/param.h \
					$(SYSINC)/fun.h \
		$(SYSINC)/con.h		\
		$(USRINC)/errno.h	\
		$(SYSINC)/sched.h	\
		$(SYSINC)/timeout.h	\
		$(SYSINC)/types.h	\
		$(SYSINC)/uproc.h	\
		tn.c
	$(CC) $(CFLAGS) -c -o $@ tn.c

objects/tnas.o: tnas.s
	$(AS) -gxo $@ tnas.s

# How to make loadable drivers.

$(LDRV)/aha154x:	$(USRSYS)/lib/aha154x.a
	( cd $(USRSYS); ldconfig aha154x )

$(LDRV)/al0:	$(USRSYS)/lib/al.a
	( cd $(USRSYS); ldconfig al0 )

$(LDRV)/al1:	$(USRSYS)/lib/al.a
	( cd $(USRSYS); ldconfig al1 )

$(LDRV)/at:	$(USRSYS)/lib/at.a
	( cd $(USRSYS); ldconfig at )

$(LDRV)/fl:	$(USRSYS)/lib/fl.a
	( cd $(USRSYS); ldconfig fl )

$(LDRV)/gr:	$(USRSYS)/lib/gr.a
	( cd $(USRSYS); ldconfig gr )

$(LDRV)/hs:	$(USRSYS)/lib/hs.a
	( cd $(USRSYS); ldconfig hs )

$(LDRV)/lp:	$(USRSYS)/lib/lp.a
	( cd $(USRSYS); ldconfig lp )

$(LDRV)/mm:	$(USRSYS)/lib/mm.a
	( cd $(USRSYS); ldconfig mm )

$(LDRV)/ms:	$(USRSYS)/lib/ms.a
	( cd $(USRSYS); ldconfig ms )

$(LDRV)/rm:	$(USRSYS)/lib/rm.a
	( cd $(USRSYS); ldconfig rm )

$(LDRV)/ss:	$(USRSYS)/lib/ss.a
	( cd $(USRSYS); ldconfig ss )
