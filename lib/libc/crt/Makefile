# /usr/src/libc/crt/Makefile 7/16/92

OBJ=\
	_prof.o

# Primary target.
all:	CRT crt$(MCH)
	: libc/crt done.

# Pseduotargets.
CRT:	$(OBJ)
	$(AR) rkus libc.a $?
	touch $@
crt$(MCH):
	SRCPATH=$(SRCPATH)/$(MCH) make -e

# Secondary targets.
_prof.o:	_prof.c

# end of /usr/src/libc/crt/Makefile
