# /usr/src/libc/sys/Makefile 1/9/92

OBJ=\
	dup2.o\
	execl.o\
	execle.o\
	execlp.o\
	execv.o\
	execvp.o\
	msgop.o\
	semop.o\
	shmop.o\
	time.o

UNUSED=\
	brk.o

# Primary target.
all:	SYS sys$(MCH)
	: libc/sys done.

# Pseudotargets.
SYS:	$(OBJ)
	$(AR) rkus libc.a $?
	touch $@
sys$(MCH):
	SRCPATH=$(SRCPATH)/$(MCH) make -e

# Secondary targets.
dup2.o:		dup2.c
execl.o:	execl.c
execle.o:	execle.c
execlp.o:	execlp.c
execv.o:	execv.c
execvp.o:	execvp.c
msgop.o:	msgop.c
semop.o:	semop.c
shmop.o:	shmop.c
time.o:		time.c

# end of /usr/src/libc/sys/Makefile
