# /usr/src/libc/gen/i386/Makefile 4/28/92

OBJ=\
	alloca.o\
	canon.o\
	l3tol.o\
	ldiv.o\
	ltol3.o\
	setjmp.o

# Primary target.
GENI386:	$(OBJ)
	$(AR) rkus libc.a $?
	touch $@
	: libc/gen/i386 done

# Secondary targets.
alloca.o:	alloca.s
canon.o:	canon.s
l3tol.o:	l3tol.c
ldiv.o:		ldiv.s
ltol3.o:	ltol3.c
setjmp.o:	setjmp.s

# end of /usr/src/libc/gen/i386/Makefile
