# $Header: /usr/src/sys/ldrv/RCS/Makefile,v 1.5 89/06/30 16:31:48 src Exp $
#
#	The  information  contained herein  is a trade secret  of INETCO
#	Systems, Ltd, and is  confidential information.   It is provided
#	under a license agreement,  and may be copied or disclosed  only
#	under  the  terms  of  that  agreement.    Any  reproduction  or
#	disclosure  of  this   material   without  the  express  written
#	authorization of INETCO Systems, Ltd. or persuant to the license
#	agreement is unlawful.
#
#	Copyright (c) 1988
#	An unpublished work by INETCO Systems, Ltd.
#	All rights reserved.

# Loadable Drivers - Makefile
#
# $Log:	/usr/src/sys/ldrv/RCS/Makefile,v $
# Revision 1.5	89/06/30  16:31:48 	src
# Bug:	Multiple simultaneous DMA transfers resulted in data being lost.
# Fix:	Added DMA lock routines to ensure single simultaneous DMA
# 	transfers. (JHB)
# 
# Revision 1.3	88/05/23  17:31:10	src
# Added vtop linkage routine (Jim had filed away in his directory) - ART
# 
# Revision 1.2	88/04/04  16:39:27	src
# defer() function now supported from loadable drivers.
# 
# Revision 1.1	88/03/24  16:30:21	src
# Initial revision
#
# 88/02/16	Jim Belton		/usr/src/sys/ldrv/Makefile
# Added vtop linkage routine to ldlib.a.

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
IBMATOBJ=/usr/src/sys/i8086/ibm_at/objects

CC=exec /bin/cc
CFLAGS=-I$(DRVINC) -I$(KERINC) -I$(SYSINC)

TARGETS=objects/ldrts0.o	\
	objects/ldmain.o	\
	objects/ldswap.o	\
	objects/ldlib.a

all:		$(TARGETS)
	@sync

install:	$(TARGETS)	\
		ldconfig
	cp $(TARGETS) /tmp/usr/sys/lib
	cp ldconfig /tmp/usr/sys

shrink:
	rm -f objects/*

objects/ldrts0.o:	ldrts0.s
	as -x ldrts0.s
	mv l.out $@

objects/ldmain.o:	ldmain.c
	$(CC) $(CFLAGS) -c ldmain.c
	mv ldmain.o $@

#	$(CC) $(CFLAGS) -c ldswap.c
objects/ldswap.o:	ldswap.c
	$(CC) $(CFLAGS) -DNOMONITOR -c ldswap.c
	mv ldswap.o $@

# The following script creates a interface stub to a kernel function.
GEN_DRV=objects/mkstub.sh

# The following script extracts a module from a library.
ARX_LIB=set -e; OBJFIL=`basename $@` ;\
	eval ar x $< $${OBJFIL} ;\
	eval mv $${OBJFIL} $@

# The following modules are supported in loadable drivers.
# NOTE: Most of them are interface stubs to kernel code.
#	Some of them are entirely driver resident.
LIBOBJ1=objects/abs.o		\
	objects/alloc.o		\
	objects/bclaim.o	\
	objects/bdone.o		\
	objects/blkmv.o		\
	objects/boot.o		\
	objects/bread.o		\
	objects/brelease.o	\
	objects/clockedf.o	\
	objects/clrq.o		\
	objects/cs_self.o	\
	objects/dblock.o	\
	objects/dclose.o	\
	objects/defer.o		\
	objects/devmsg.o	\
	objects/dmago.o		\
	objects/dmalock.o	\
	objects/dmaoff.o	\
	objects/dmaon.o		\
	objects/dmareq.o	\
	objects/dopen.o		\
	objects/drvmap.o	\
	objects/dwrite.o	\
	objects/fclear.o	\
	objects/fdisk.o		\
	objects/ffbyte.o	\
	objects/ffword.o	\
	objects/ffmem.o		\
	objects/fkcopy.o	\
	objects/fpxcopy.o	\
	objects/free.o		\
	objects/fucopy.o	\
	objects/getcs.o		\
	objects/getq.o		\
	objects/getubd.o	\
	objects/getuwd.o	\
	objects/inb.o		\
	objects/int11.o		\
	objects/iogetc.o	\
	objects/iomapvp.o	\
	objects/ioputc.o	\
	objects/ioread.o	\
	objects/ioreq.o		\
	objects/iowrite.o	\
	objects/ipcaccess.o	\
	objects/kcall.o		\
	objects/kfcopy.o	\
	objects/kclear.o	\
	objects/kpcopy.o	\
	objects/kucopy.o	\
	objects/lock.o		\
	objects/lxdiv.o		\
	objects/lxmul.o		\
	objects/lxrem.o		\
	objects/lxsgn.o		\
	objects/memset.o	\
	objects/memtest.o	\
	objects/nmidisable.o	\
	objects/nmienable.o	\
	objects/nondsig.o	\
	objects/nonedev.o	\
	objects/nulldev.o	\

LIBOBJ2=objects/outb.o		\
	objects/panic.o		\
	objects/pclear.o	\
	objects/pkcopy.o	\
	objects/plrcopy.o	\
	objects/pollopen.o	\
	objects/pollwake.o	\
	objects/printf.o	\
	objects/prlcopy.o	\
	objects/ptov.o		\
	objects/pucopy.o	\
	objects/putchar.o	\
	objects/putq.o		\
	objects/putubd.o	\
	objects/putuwd.o	\
	objects/rucopy.o	\
	objects/s5_to_sg.o	\
	objects/s5_to_tc.o	\
	objects/salloc.o	\
	objects/sclear.o	\
	objects/sendsig.o	\
	objects/setivec.o	\
	objects/sfree.o		\
	objects/sfbyte.o	\
	objects/sfword.o	\
	objects/sfmem.o		\
	objects/sg_to_s5.o	\
	objects/sleep.o		\
	objects/slrcopy.o	\
	objects/sphi.o		\
	objects/spl.o		\
	objects/splo.o		\
	objects/super.o		\
	objects/swapio.o	\
	objects/tc_to_s5.o	\
	objects/timeout.o	\
	objects/ttclose.o	\
	objects/tthup.o		\
	objects/ttin.o		\
	objects/ttioctl.o	\
	objects/ttopen.o	\
	objects/ttout.o		\
	objects/ttpoll.o	\
	objects/ttread.o	\
	objects/ttsetgrp.o	\
	objects/ttsignal.o	\
	objects/ttstart.o	\
	objects/ttwrite.o	\
	objects/uexit.o		\
	objects/ufcopy.o	\
	objects/ukcopy.o	\
	objects/unlock.o	\
	objects/upcopy.o	\
	objects/urcopy.o	\
	objects/vrelse.o	\
	objects/vremap.o	\
	objects/vtop.o		\
	objects/vxdiv.o		\
	objects/vxmul.o		\
	objects/vxrem.o		\
	objects/waitq.o		\
	objects/wakeup.o	\

objects/ldlib.a:	$(GEN_DRV)	\
			$(LIBOBJ1)	\
			$(LIBOBJ2)
	rm -f  $@
	ar rc  $@ $(LIBOBJ1)
	ar rc  $@ $(LIBOBJ2)
	ranlib $@
	cp $@ /usr/sys/lib

#
# Create the script to create an interface stub to a kernel function.
#
$(GEN_DRV):
	echo ": 'Created by Makefile - do not edit.';\
	set -e; ENTRY=\`basename \$$1\`;(\
	echo \".globl \$${ENTRY}_\";\
	echo \"\$${ENTRY}_: mov ax,\\\\\$$K\$${ENTRY}_\";\
	echo .byte 0x9A;\
	echo .word xcalled;\
	echo .word 0x0060;\
	echo ret;)>/tmp/\$$\$$.s;\
	as -gxo /tmp/\$$\$$.o /tmp/\$$\$$.s;\
	mv /tmp/\$$\$$.o \$$1.o;\
	rm -f /tmp/\$$\$$.s" > $@
	chmod +x $@

objects/abs.o:
	$(GEN_DRV) $*

objects/alloc.o:
	$(GEN_DRV) $*

objects/bclaim.o:
	$(GEN_DRV) $*

objects/bdone.o:
	$(GEN_DRV) $*

objects/blkmv.o:	/lib/libc.a
	$(ARX_LIB)

objects/boot.o:
	$(GEN_DRV) $*

objects/bread.o:
	$(GEN_DRV) $*

objects/brelease.o:
	$(GEN_DRV) $*

objects/clockedf.o:	clockedf.c
	$(CC) $(CFLAGS) -c -o $@ $<

objects/clrivec.o:
	$(GEN_DRV) $*

objects/clrq.o:
	$(GEN_DRV) $*

objects/cs_self.o:	cs_self.s
	as -gxo cs_self.o cs_self.s
	mv cs_self.o $@

objects/dblock.o:
	$(GEN_DRV) $*

objects/dclose.o:
	$(GEN_DRV) $*

objects/defer.o:	defer.s
	as -gxo defer.o defer.s
	mv defer.o $@

objects/devmsg.o:
	$(GEN_DRV) $*

objects/dmago.o:
	$(GEN_DRV) $*

objects/dmalock.o:	dmalock.c
	$(CC) $(CFLAGS) -c dmalock.c
	mv dmalock.o $@

objects/dmaoff.o:
	$(GEN_DRV) $*

objects/dmaon.o:
	$(GEN_DRV) $*

objects/dmareq.o:
	$(GEN_DRV) $*

objects/dopen.o:
	$(GEN_DRV) $*

objects/drvmap.o:
	$(GEN_DRV) $*

objects/dwrite.o:
	$(GEN_DRV) $*

objects/fclear.o:
	$(GEN_DRV) $*

objects/fdisk.o:
	$(GEN_DRV) $*

objects/ffbyte.o:	ffbyte.s
	as -gxo ffbyte.o ffbyte.s
	mv ffbyte.o $@

objects/ffword.o:	ffword.s
	as -gxo ffword.o ffword.s
	mv ffword.o $@

objects/ffmem.o:	ffmem.s
	as -gxo ffmem.o ffmem.s
	mv ffmem.o $@

objects/fkcopy.o:
	$(GEN_DRV) $*

objects/fpxcopy.o:
	$(GEN_DRV) $*

objects/free.o:
	$(GEN_DRV) $*

objects/fucopy.o:
	$(GEN_DRV) $*

objects/getcs.o:	getcs.s
	as -gxo getcs.o getcs.s
	mv getcs.o $@

objects/getq.o:
	$(GEN_DRV) $*

objects/getubd.o:
	$(GEN_DRV) $*

objects/getuwd.o:
	$(GEN_DRV) $*

objects/inb.o:		inb.s
	as -gxo inb.o inb.s
	mv inb.o $@

objects/int11.o:
	$(GEN_DRV) $*

objects/iogetc.o:
	$(GEN_DRV) $*

objects/iomapvp.o:
	$(GEN_DRV) $*

objects/ioputc.o:
	$(GEN_DRV) $*

objects/ioread.o:
	$(GEN_DRV) $*

objects/ioreq.o:
	$(GEN_DRV) $*

objects/iowrite.o:
	$(GEN_DRV) $*

objects/ipcaccess.o:
	$(GEN_DRV) $*

objects/kcall.o:	kcall.s
	as -gxo kcall.o kcall.s
	mv kcall.o $@

objects/kclear.o:
	$(GEN_DRV) $*

objects/kfcopy.o:
	$(GEN_DRV) $*

objects/kpcopy.o:
	$(GEN_DRV) $*

objects/kucopy.o:
	$(GEN_DRV) $*

objects/lock.o:
	$(GEN_DRV) $*

objects/lxdiv.o:	/lib/libc.a
	$(ARX_LIB)

objects/lxmul.o:	/lib/libc.a
	$(ARX_LIB)

objects/lxrem.o:	/lib/libc.a
	$(ARX_LIB)

objects/lxsgn.o:	/lib/libc.a
	$(ARX_LIB)

objects/memset.o:	/lib/libc.a
	$(ARX_LIB)

objects/memtest.o:
	$(GEN_DRV) $*

objects/nmidisable.o:
	$(GEN_DRV) $*

objects/nmienable.o:
	$(GEN_DRV) $*

objects/nondsig.o:
	$(GEN_DRV) $*

objects/nonedev.o:	nonedev.c
	$(CC) $(CFLAGS) -c nonedev.c
	mv nonedev.o $@

objects/nulldev.o:	nulldev.c
	$(CC) $(CFLAGS) -c nulldev.c
	mv nulldev.o $@

objects/outb.o:		outb.s
	as -gxo outb.o outb.s
	mv outb.o $@

objects/panic.o:
	$(GEN_DRV) $*

objects/pclear.o:
	$(GEN_DRV) $*

objects/plrcopy.o:
	$(GEN_DRV) $*

objects/pkcopy.o:
	$(GEN_DRV) $*

objects/pollopen.o:
	$(GEN_DRV) $*

objects/pollwake.o:
	$(GEN_DRV) $*

objects/printf.o:
	$(GEN_DRV) $*

objects/prlcopy.o:
	$(GEN_DRV) $*

objects/ptov.o:
	$(GEN_DRV) $*

objects/pucopy.o:
	$(GEN_DRV) $*

objects/putchar.o:
	$(GEN_DRV) $*

objects/putq.o:
	$(GEN_DRV) $*

objects/putubd.o:
	$(GEN_DRV) $*

objects/putuwd.o:
	$(GEN_DRV) $*

objects/rucopy.o:
	$(GEN_DRV) $*

objects/s5_to_sg.o:
	$(GEN_DRV) $*

objects/s5_to_tc.o:
	$(GEN_DRV) $*

objects/salloc.o:
	$(GEN_DRV) $*

objects/sclear.o:
	$(GEN_DRV) $*

objects/sendsig.o:
	$(GEN_DRV) $*

objects/setivec.o:	setivec.c
	$(CC) $(CFLAGS) -c setivec.c
	mv setivec.o $@

objects/sfree.o:
	$(GEN_DRV) $*

objects/sfbyte.o:
	as -gxo sfbyte.o sfbyte.s
	mv sfbyte.o $@

objects/sfword.o:
	as -gxo sfword.o sfword.s
	mv sfword.o $@

objects/sfmem.o:
	as -gxo sfmem.o sfmem.s
	mv sfmem.o $@

objects/sg_to_s5.o:
	$(GEN_DRV) $*

objects/sleep.o:
	$(GEN_DRV) $*

objects/slrcopy.o:
	$(GEN_DRV) $*

objects/sphi.o:
	as -gxo sphi.o sphi.s
	mv sphi.o $@

objects/spl.o:
	as -gxo spl.o spl.s
	mv spl.o $@

objects/splo.o:
	as -gxo splo.o splo.s
	mv splo.o $@

objects/super.o:
	$(GEN_DRV) $*

objects/swapio.o:
	$(GEN_DRV) $*

objects/tc_to_s5.o:
	$(GEN_DRV) $*

objects/timeout.o:	timeout.c
	$(CC) $(CFLAGS) -c timeout.c
	mv timeout.o $@

objects/ttclose.o:
	$(GEN_DRV) $*

objects/tthup.o:
	$(GEN_DRV) $*

objects/ttin.o:
	$(GEN_DRV) $*

objects/ttioctl.o:
	$(GEN_DRV) $*

objects/ttopen.o:
	$(GEN_DRV) $*

objects/ttout.o:
	$(GEN_DRV) $*

objects/ttpoll.o:
	$(GEN_DRV) $*

objects/ttread.o:
	$(GEN_DRV) $*

objects/ttsetgrp.o:
	$(GEN_DRV) $*

objects/ttsignal.o:
	$(GEN_DRV) $*

objects/ttstart.o:
	$(GEN_DRV) $*

objects/ttwrite.o:
	$(GEN_DRV) $*

objects/uexit.o:
	$(GEN_DRV) $*

objects/ufcopy.o:
	$(GEN_DRV) $*

objects/ukcopy.o:
	$(GEN_DRV) $*

objects/unlock.o:
	$(GEN_DRV) $*

objects/upcopy.o:
	$(GEN_DRV) $*

objects/urcopy.o:
	$(GEN_DRV) $*

objects/vrelse.o:
	$(GEN_DRV) $*

objects/vremap.o:
	$(GEN_DRV) $*

objects/vtop.o:
	$(GEN_DRV) $*

objects/vxdiv.o:	/lib/libc.a
	$(ARX_LIB)

objects/vxmul.o:	/lib/libc.a
	$(ARX_LIB)

objects/vxrem.o:	/lib/libc.a
	$(ARX_LIB)

objects/waitq.o:
	$(GEN_DRV) $*

objects/wakeup.o:
	$(GEN_DRV) $*

