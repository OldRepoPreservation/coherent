#
#	makefile for Coherent 386 - overall system build
#
#	Do not use this makefile directly!  Use a build script.
#	(Unless you know what you are doing.)
#

AR=exec /bin/ar
ARFLAGS=rs
AS=exec /bin/as
CC=exec /bin/cc
CFLAGS=-DTRACER -VPSTR

USRSRC=/usr/src
USRINC=/usr/include
SYSINC=$(USRINC)/sys

IOOBJ =\
	$(K386OBJ)/bufq.o \
	$(K386OBJ)/ct.o \
	$(K386OBJ)/dmareq.o \
	$(K386OBJ)/fdisk.o

COHOBJ =\
	$(K386OBJ)/alloc.o \
	$(K386OBJ)/bio.o \
	$(K386OBJ)/clist.o \
	$(K386OBJ)/clock.o \
	$(K386OBJ)/clocked.o \
	$(K386OBJ)/exec.o \
	$(K386OBJ)/fd.o \
	$(K386OBJ)/fifo.o \
	$(K386OBJ)/fs1.o \
	$(K386OBJ)/fs2.o \
	$(K386OBJ)/fs3.o \
	$(K386OBJ)/main.o \
	$(K386OBJ)/misc.o \
	$(K386OBJ)/null.o \
	$(K386OBJ)/poll.o \
	$(K386OBJ)/printf.o \
	$(K386OBJ)/proc.o \
	$(K386OBJ)/rlock.o \
	$(K386OBJ)/seg.o \
	$(K386OBJ)/sig.o \
	$(K386OBJ)/support.o \
	$(K386OBJ)/sys1.o \
	$(K386OBJ)/sys2.o \
	$(K386OBJ)/sys3.o \
	$(K386OBJ)/sys4.o \
	$(K386OBJ)/sys5.o \
	$(K386OBJ)/syscoh.o \
	$(K386OBJ)/timeout.o \
	$(K386OBJ)/var.o

I386OBJ =\
	$(K386OBJ)/as.o \
	$(K386OBJ)/xdt.o \
	$(K386OBJ)/atcon.o \
	$(K386OBJ)/defer.o \
	$(K386OBJ)/die.o \
	$(K386OBJ)/dmac.o \
	$(K386OBJ)/dmalock.o \
	$(K386OBJ)/fakedma.o \
	$(K386OBJ)/ff.o \
	$(K386OBJ)/md.o \
	$(K386OBJ)/mem_cache.o \
	$(K386OBJ)/memcpy.o \
	$(K386OBJ)/memset.o \
	$(K386OBJ)/mmu.o \
	$(K386OBJ)/sys1632.o \
	$(K386OBJ)/tioc.o \
	$(K386OBJ)/trap.o

OPTS =\
	$(K386OBJ)/aha.o \
	$(K386OBJ)/aha_dsl.o \
	$(K386OBJ)/albaud.o \
	$(K386OBJ)/alx.o \
	$(K386OBJ)/asy.o \
	$(K386OBJ)/at.o \
	$(K386OBJ)/com1.o \
	$(K386OBJ)/com2.o \
	$(K386OBJ)/hs.o \
	$(K386OBJ)/ipc.o \
	$(K386OBJ)/kb.o \
	$(K386OBJ)/kb_d.o \
	$(K386OBJ)/kb_f.o \
	$(K386OBJ)/lp.o \
	$(K386OBJ)/mm.o \
	$(K386OBJ)/mmas.o \
	$(K386OBJ)/msg.o \
	$(K386OBJ)/msgcon.o \
	$(K386OBJ)/nkb.o \
	$(K386OBJ)/pipe.o \
	$(K386OBJ)/pty.o \
	$(K386OBJ)/putchar.o \
	$(K386OBJ)/scsi.o \
	$(K386OBJ)/sem.o \
	$(K386OBJ)/semcon.o \
	$(K386OBJ)/shm.o \
	$(K386OBJ)/shmcon.o \
	$(K386OBJ)/ss.o \
	$(K386OBJ)/tty.o \
	$(K386OBJ)/vtmm.o \
	$(K386OBJ)/vtmmas.o \
	$(K386OBJ)/vtkb.o \
	$(K386OBJ)/vtkb_d.o \
	$(K386OBJ)/vtkb_f.o \
	$(K386OBJ)/vtnkb.o 

# LIBLIST is the .a files needed for 386 installation
LIBLIST= \
	$(K386LIB)/kb.a \
	$(K386LIB)/kb_d.a \
	$(K386LIB)/kb_f.a \
	$(K386LIB)/kl386.a \
	$(K386LIB)/mm.a \
	$(K386LIB)/nkb.a \
	$(K386LIB)/vt.a

$(K386LIB)/kb.a:	$(K386OBJ)/kb.o
	rm -f $@
	$(AR) $(ARFLAGS) $@ $<

$(K386LIB)/kb_d.a:	$(K386OBJ)/kb_d.o
	rm -f $@
	$(AR) $(ARFLAGS) $@ $<

$(K386LIB)/kb_f.a:	$(K386OBJ)/kb_f.o
	rm -f $@
	$(AR) $(ARFLAGS) $@ $<

$(K386LIB)/nkb.a:	$(K386OBJ)/nkb.o
	rm -f $@
	$(AR) $(ARFLAGS) $@ $<

$(K386LIB)/mm.a: \
	$(K386OBJ)/mm.o \
	$(K386OBJ)/mmas.o
	rm -f $@
	$(AR) $(ARFLAGS) $@ $<

$(K386LIB)/vt.a: \
	$(K386OBJ)/vtmm.o \
	$(K386OBJ)/vtmmas.o \
	$(K386OBJ)/vtnkb.o 
	rm -f $@
	$(AR) $(ARFLAGS) $@ $<

# kl386.a is the main library needed for linking customer kernels.
$(K386LIB)/kl386.a: \
	$(K386OBJ)/aha.o \
	$(K386OBJ)/aha_dsl.o \
	$(K386OBJ)/albaud.o \
	$(K386OBJ)/asy.o \
	$(K386OBJ)/at.o \
	$(K386OBJ)/fl.o \
	$(K386OBJ)/ipc.o \
	$(K386OBJ)/lp.o \
	$(K386OBJ)/msg.o \
	$(K386OBJ)/msgcon.o \
	$(K386OBJ)/pipe.o \
	$(K386OBJ)/pty.o \
	$(K386OBJ)/putchar.o \
	$(K386OBJ)/rm.o \
	$(K386OBJ)/scsi.o \
	$(K386OBJ)/sem.o \
	$(K386OBJ)/semcon.o \
	$(K386OBJ)/shm.o \
	$(K386OBJ)/shmcon.o \
	$(K386OBJ)/ss.o \
	$(K386OBJ)/tty.o \
	$(K386OBJ)/memcpy.o \
	$(K386OBJ)/memset.o
	rm -f $@
	$(AR) $(ARFLAGS) $@ $<

$(K386LIB)/ker386.o: $(I386OBJ) coh io
	ld -r -o $@ $(I386OBJ) $(COHOBJ) $(IOOBJ)

coh:
	cd ../coh.386; make "CFLAGS=$(CFLAGS)" "CC=$(CC)"

io:
	cd ../io.386; make "CFLAGS=$(CFLAGS)" "CC=$(CC)"

ver:
	newversion < version

libs:	$(LIBLIST)
	@/bin/echo "Installation libraries up to date."

$(K386OBJ)/as.o: as.s as.inc
	$(AS) -g -o $@ as.s

$(K386OBJ)/xdt.o: xdt.s as.inc
	$(AS) -g -o $@ xdt.s

$(K386OBJ)/defer.o: defer.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(K386OBJ)/die.o: die.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(K386OBJ)/dmac.o:	dmac.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(K386OBJ)/dmalock.o:	dmalock.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(K386OBJ)/fakedma.o:	fakedma.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(K386OBJ)/ff.o: ff.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(K386OBJ)/sys1632.o: sys1632.c \
		$(SYSINC)/coherent.h	$(SYSINC)/types.h $(SYSINC)/timeout.h \
					$(SYSINC)/reg.h $(SYSINC)/param.h \
					$(SYSINC)/fun.h \
		$(USRINC)/acct.h	\
		$(SYSINC)/buf.h		\
		$(SYSINC)/con.h		\
		$(USRINC)/errno.h	\
		$(SYSINC)/filsys.h	\
		$(SYSINC)/ino.h		\
		$(SYSINC)/inode.h	\
		$(USRINC)/l.out.h	\
		$(SYSINC)/proc.h	\
		$(SYSINC)/sched.h	\
		$(SYSINC)/seg.h		\
		$(USRINC)/signal.h	\
		$(SYSINC)/uproc.h	\
		$(SYSINC)/reg.h		\
		$(SYSINC)/systab.h	\
		$(SYSINC)/oldstat.h	\
		$(SYSINC)/oldtimeb.h	\
		$(SYSINC)/fd.h		\
		$(USRINC)/sgtty.h
	$(CC) $(CFLAGS) -c -o $@ sys1632.c

$(K386OBJ)/mmu.o: mmu.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(K386OBJ)/md.o: md.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(K386OBJ)/mem_cache.o: mem_cache.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(K386OBJ)/memcpy.o: memcpy.s
	$(AS) -g -o $@ memcpy.s

$(K386OBJ)/memset.o: memset.s
	$(AS) -g -o $@ memset.s

$(K386OBJ)/tioc.o: tioc.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(K386OBJ)/trap.o: trap.c
	$(CC) $(CFLAGS) -VSUVAR -c -o $@ $<

$(K386OBJ)/atcon.o:			\
		$(SYSINC)/coherent.h	$(SYSINC)/types.h $(SYSINC)/timeout.h \
					$(SYSINC)/reg.h $(SYSINC)/param.h \
					$(SYSINC)/fun.h \
		$(SYSINC)/con.h		\
		$(USRINC)/mtype.h	\
		$(SYSINC)/stat.h	\
		atcon.c
	$(CC) $(CFLAGS) -c -o $@ atcon.c

$(K386OBJ)/fvcon.o:			\
		$(SYSINC)/coherent.h	$(SYSINC)/types.h $(SYSINC)/timeout.h \
					$(SYSINC)/reg.h $(SYSINC)/param.h \
					$(SYSINC)/fun.h \
		$(SYSINC)/con.h		\
		$(USRINC)/mtype.h	\
		$(SYSINC)/stat.h	\
		fvcon.c
	$(CC) $(CFLAGS) -c -o $@ fvcon.c
