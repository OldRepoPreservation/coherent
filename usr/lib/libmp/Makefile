# /usr/src/libmp/Makefile 2/14/92

DESTDIR=/interim/usr/lib/

OBJ=\
	gcd.o \
	ispos.o \
	itom.o \
	madd.o \
	mcmp.o \
	mconst.o \
	mcopy.o \
	mdiv.o \
	min.o \
	mintfr.o \
	mitom.o \
	mneg.o \
	mout.o \
	mpalloc.o \
	mperr.o \
	msma.o \
	msqrt.o \
	mtoi.o \
	mtos.o \
	mult.o \
	norm.o \
	pow.o \
	rpow.o \
	sdiv.o \
	smult.o \
	spow.o \
	xgcd.o \
	zerop.o

# Primary target.
libmp.a: $(OBJ)
	$(AR) rckus $(DESTDIR)$@ $?

# Secondary targets.
# Does not include header dependencies.
gcd.o:		gcd.c
ispos.o:	ispos.c
itom.o:		itom.c
madd.o:		madd.c
mcmp.o:		mcmp.c
mconst.o:	mconst.c
mcopy.o:	mcopy.c
mdiv.o:		mdiv.c
min.o:		min.c
mintfr.o:	mintfr.c
mitom.o:	mitom.c
mneg.o:		mneg.c
mout.o:		mout.c
mpalloc.o:	mpalloc.c
mperr.o:	mperr.c
msma.o:		msma.c
msqrt.o:	msqrt.c
mtoi.o:		mtoi.c
mtos.o:		mtos.c
mult.o:		mult.c
norm.o:		norm.c
pow.o:		pow.c
rpow.o:		rpow.c
sdiv.o:		sdiv.c
smult.o:	smult.c
spow.o:		spow.c
xgcd.o:		xgcd.c
zerop.o:	zerop.c

# end of /usr/src/libmp/Makefile
