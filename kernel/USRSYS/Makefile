# $Header: /usr/src/sys/i8086/ibm_at/RCS/Makefile,v 1.3 89/06/30 16:26:39 src Exp $
#
# Makefile for ibm specific coherent sources and coherent images.
#
# $Log:	/usr/src/sys/i8086/ibm_at/RCS/Makefile,v $
# Revision 1.3	89/06/30  16:26:39 	src
# Bug:	Lack of DMA lock resulted in failed transfers between SCSI drive
# 	and floppy disk.
# Fix:	Added dma lock routines to insure single DMA transfer. (JHB)
# 
# Revision 1.1	88/03/24  17:33:15	src
# Initial revision
# 

USRSYS=/usr/sys

# Include directories
USRINC=/usr/include
SYSINC=/usr/include/sys
KERINC=/usr/src/sys/sys
DRVINC=/usr/src/sys/i8086/sys
USSINC=/usr/src/sys

# Source directories
COHSRC=/usr/src/sys/coh
DRVSRC=/usr/src/sys/i8086/drv
I86SRC=/usr/src/sys/i8086/src
IBMATSRC=/usr/src/sys/i8086/ibm_at
KERSRC=/usr/src/sys/ker
TTYSRC=/usr/src/sys/ttydrv

# Object directories
I86OBJ=/usr/src/sys/i8086/objects
DRVOBJ=/usr/src/sys/i8086/drv/objects
SRCOBJ=/usr/src/sys/i8086/src/objects
IBMATOBJ=/usr/src/sys/i8086/ibm_at/objects

# Library directory
DRVLIB=/usr/sys/lib

# Compiler stuff
CC=exec /bin/cc
CFLAGS= -I$(DRVINC) -I$(KERINC) -I$(SYSINC)

HERE=	$(IBMATOBJ)/md2.o \
	$(IBMATOBJ)/dmac.o \
	$(DRVOBJ)/pccon.o \
	$(DRVOBJ)/console.o \
	$(DRVOBJ)/dmareq.o \
	$(IBMATOBJ)/mmu.o \
	$(SRCOBJ)/support.o

DOTDOT=	$(I86OBJ)/alloc.o \
	$(I86OBJ)/as1.o \
	$(I86OBJ)/bio.o \
	$(I86OBJ)/clist.o \
	$(I86OBJ)/clock.o \
	$(I86OBJ)/ct.o \
	$(I86OBJ)/defer.o \
	$(I86OBJ)/dmalock.o	\
	$(I86OBJ)/exec.o \
	$(I86OBJ)/fd.o \
	$(I86OBJ)/fs1.o $(I86OBJ)/fs2.o $(I86OBJ)/fs3.o \
	$(I86OBJ)/krunch.o \
	$(I86OBJ)/ld.o $(I86OBJ)/ldas.o \
	$(I86OBJ)/main.o \
	$(I86OBJ)/md1.o \
	$(I86OBJ)/misc.o \
	$(I86OBJ)/mmain.o \
	$(I86OBJ)/null.o \
	$(I86OBJ)/pipe.o \
	$(I86OBJ)/poll.o \
	$(I86OBJ)/printf.o \
	$(I86OBJ)/proc.o \
	$(I86OBJ)/seg.o \
	$(I86OBJ)/sig.o \
	$(I86OBJ)/sys1.o $(I86OBJ)/sys2.o $(I86OBJ)/sys3.o \
	$(I86OBJ)/tab.o \
	$(I86OBJ)/timeout.o \
	$(I86OBJ)/trap.o \
	$(I86OBJ)/var.o

kernel:	$(IBMATOBJ) $(USRSYS)/atkernel.o $(DRVLIB)/support.a $(DRVLIB)/tty.a
	@exec /bin/sync

shrink:
	rm -f $(IBMATOBJ)/* $(I86OBJ)/* $(DRVOBJ)/*

$(IBMATOBJ):
	mkdir $(IBMATOBJ)

$(DRVOBJ)/pccon.o: \
	$(KERINC)/coherent.h	$(SYSINC)/types.h $(SYSINC)/timeout.h \
				$(SYSINC)/machine.h $(SYSINC)/param.h \
				$(SYSINC)/fun.h \
	$(SYSINC)/con.h		\
	$(USRINC)/mtype.h	\
	$(SYSINC)/stat.h	\
	$(DRVSRC)/pccon.c
	$(CC) $(CFLAGS) -c -o $@ $(DRVSRC)/pccon.c

$(DRVOBJ)/console.o:	\
	$(KERINC)/coherent.h	$(SYSINC)/types.h $(SYSINC)/timeout.h \
				$(SYSINC)/machine.h $(SYSINC)/param.h \
				$(SYSINC)/fun.h \
	$(SYSINC)/con.h		\
	$(SYSINC)/inode.h	\
	$(SYSINC)/io.h		\
	$(SYSINC)/stat.h	\
	$(DRVSRC)/console.c
	$(CC) $(CFLAGS) -c -o $@ $(DRVSRC)/console.c

$(DRVOBJ)/dmareq.o:	\
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
	$(DRVSRC)/dmareq.c
	$(CC) $(CFLAGS) -c -o $@ $(DRVSRC)/dmareq.c

$(USRSYS)/atkernel.o: $(IBMATOBJ)/as2.obj $(HERE) $(DOTDOT)
	exec /bin/ld -r -o $@ $<

$(IBMATOBJ)/as2.obj: $(IBMATSRC)/as2.s
	exec as -go $@ $<

$(IBMATOBJ)/dmac.o: $(IBMATSRC)/dmac.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(IBMATOBJ)/dump.o: $(IBMATSRC)/dump.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(IBMATOBJ)/md2.o: $(IBMATSRC)/md2.c
	$(CC) $(CFLAGS) -c -o $@ $<

# Define REAL_MODE if real mode is also required
$(IBMATOBJ)/mmu.o: $(IBMATSRC)/mmu.c
	$(CC) $(CFLAGS) -DREAL_MODE -c -o $@ $<

$(SRCOBJ)/support.o: $(I86SRC)/support.c
	$(CC) $(CFLAGS) -c -o $@ $<

# this stuff was in the /usr/src/sys/i8086 Makefile before 90/08/08
C86FLAGS=-c -DNOMONITOR=1 -I$(DRVINC) -I$(KERINC) -I$(SYSINC) -I$(USSINC)
#
$(I86OBJ):
	mkdir $(I86OBJ)

$(I86OBJ)/alloc.o: $(COHSRC)/alloc.c
	$(CC) $(C86FLAGS) -o $@ $<

$(I86OBJ)/as1.o: $(I86SRC)/as1.s
	$(CC) $(C86FLAGS) -o $@ $<

$(I86OBJ)/bio.o: $(COHSRC)/bio.c
	$(CC) $(C86FLAGS) -DREADAHEAD=0 -o $@ $<

# Clists are assembly source on i8086
$(I86OBJ)/clist.o: $(I86SRC)/clist.s $(SYSINC)/const.h
	/lib/cpp -E -Isys -o clist.i $(I86SRC)/clist.s
	as -go $@ clist.i
	rm clist.i

$(I86OBJ)/clock.o: $(COHSRC)/clock.c
	$(CC) $(C86FLAGS) -o $@ $<

$(I86OBJ)/ct.o: $(TTYSRC)/ct.c
	$(CC) $(C86FLAGS) -o $@ $<

$(I86OBJ)/defer.o: $(I86SRC)/defer.s
	$(CC) $(C86FLAGS) -o $@ $<

$(I86OBJ)/dmalock.o: $(I86SRC)/dmalock.c
	$(CC) $(C86FLAGS) -o $@ $<

$(I86OBJ)/exec.o: $(I86SRC)/exec.c
	$(CC) $(C86FLAGS) -o $@ $<

$(I86OBJ)/fd.o: $(COHSRC)/fd.c
	$(CC) $(C86FLAGS) -o $@ $<

$(I86OBJ)/fs1.o: $(COHSRC)/fs1.c
	$(CC) $(C86FLAGS) -o $@ $<

$(I86OBJ)/fs2.o: $(COHSRC)/fs2.c
	$(CC) $(C86FLAGS) -o $@ $<

$(I86OBJ)/fs3.o: $(COHSRC)/fs3.c
	$(CC) $(C86FLAGS) -DTINY=1 -o $@ $<

$(I86OBJ)/krunch.o: $(I86SRC)/krunch.c
	$(CC) $(C86FLAGS) -o $@ $<

$(I86OBJ)/ld.o: $(I86SRC)/ld.c
	$(CC) $(C86FLAGS) -o $@ $<

$(I86OBJ)/ldas.o: $(I86SRC)/ldas.s
	as -gxo $@ $<

$(I86OBJ)/main.o: $(USRSYS)/version $(COHSRC)/main.c
	$(CC) $(C86FLAGS) `$(USRSYS)/version` -o $@ $(COHSRC)/main.c

$(I86OBJ)/md1.o: $(I86SRC)/md1.c
	$(CC) $(C86FLAGS) -o $@ $<

$(I86OBJ)/misc.o: $(COHSRC)/misc.c
	$(CC) $(C86FLAGS) -o $@ $<

$(I86OBJ)/mmain.o: $(I86SRC)/mmain.c $(SYSINC)/const.h
	$(CC) $(C86FLAGS) -o $@ $(I86SRC)/mmain.c

$(I86OBJ)/null.o: $(COHSRC)/null.c
	$(CC) $(C86FLAGS) -o $@ $<

$(I86OBJ)/pipe.o: $(COHSRC)/pipe.c
	$(CC) $(C86FLAGS) -o $@ $<

$(I86OBJ)/poll.o: $(COHSRC)/poll.c
	$(CC) $(C86FLAGS) -o $@ $<

$(I86OBJ)/printf.o: $(COHSRC)/printf.c
	$(CC) $(C86FLAGS) -o $@ $<

$(I86OBJ)/proc.o: $(COHSRC)/proc.c
	$(CC) $(C86FLAGS) -o $@ $<

$(I86OBJ)/seg.o: $(COHSRC)/seg.c
	$(CC) $(C86FLAGS) -o $@ $<

$(I86OBJ)/sig.o: $(COHSRC)/sig.c
	$(CC) $(C86FLAGS) -o $@ $<

$(I86OBJ)/swap.o: $(KERSRC)/swap.c
	$(CC) $(C86FLAGS) -o $@ $<

$(I86OBJ)/elog.o: $(KERSRC)/elog.c
	$(CC) $(C86FLAGS) -o $@ $<

$(I86OBJ)/sys1.o: $(COHSRC)/sys1.c
	$(CC) $(C86FLAGS) -o $@ $<

$(I86OBJ)/sys2.o: $(COHSRC)/sys2.c
	$(CC) $(C86FLAGS) -o $@ $<

$(I86OBJ)/sys3.o: $(COHSRC)/sys3.c
	$(CC) $(C86FLAGS) -o $@ $<

$(I86OBJ)/tab.o: $(I86SRC)/tab.c
	$(CC) $(C86FLAGS) -o $@ $<

$(I86OBJ)/timeout.o: $(COHSRC)/timeout.c
	$(CC) $(C86FLAGS) -o $@ $<

$(I86OBJ)/trap.o: $(I86SRC)/trap.c
	$(CC) $(C86FLAGS) -o $@ $<

$(I86OBJ)/var.o: $(COHSRC)/var.c
	$(CC) $(C86FLAGS) -o $@ $<

# Additional libraries searched when "config" runs.

$(DRVLIB)/support.a:	$(SRCOBJ)/cs_sel.o $(SRCOBJ)/clocked.o
	rm -f $@
	ar rc $@ $<

$(SRCOBJ)/cs_sel.o: $(I86SRC)/cs_sel.s
	as -gxo $@ $<

$(SRCOBJ)/clocked.o: $(I86SRC)/clocked.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(DRVLIB)/tty.a:	$(DRVOBJ)/tty.o
	rm -f $@
	ar rc $@ $<

$(DRVOBJ)/tty.o: $(TTYSRC)/tty.c
	$(CC) $(CFLAGS) -c -o $@ $<
