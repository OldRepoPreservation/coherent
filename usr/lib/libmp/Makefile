# misc/Makefile 10/11/90

DESTDIR=/interim/usr/lib/

# Objects.
O = \
	rpow.o pow.o msqrt.o spow.o xgcd.o gcd.o mtoi.o	mout.o mtos.o min.o mdiv.o \
	sdiv.o mult.o msma.o smult.o madd.o mneg.o itom.o mitom.o mcopy.o mintfr.o \
	mpalloc.o mperr.o mcmp.o zerop.o mconst.o ispos.o norm.o


# Primary target.
libmp.a: $O
	su root rm -f $(DESTDIR)libmp.a
	ar q $(DESTDIR)libmp.a $O
	su root chmog 444 bin bin $(DESTDIR)$@
	su root ranlib $(DESTDIR)$@
	mv *.o objects/	
