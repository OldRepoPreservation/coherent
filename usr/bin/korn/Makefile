# PD Bourne/Korn Shell for COHERENT

BIN = /usr/bin
STD =../std
PRINT = prhp


# Must define one of _V7, _SYSV, _BSD, _POSIX
# may define any of JOBS (have BSD or POSIX job control),
# EDIT (emacs-like command editing), SILLY (see edit.c)
OPTIONS = -D_V7 -DEDIT
CFLAGS = -O $(INC) $(OPTIONS)
LDLIBS = /lib/libc.a -lx
LDFLAGS=-n
DESTDIR=/interim/usr/bin/

HDRS =	sh.h table.h expand.h lex.h tree.h tty.h
SRCS1 =	version.c main.c misc.c trap.c alloc.c io.c \
	syn.c lex.c edit.c history.c tree.c 
SRCS2 =	exec.c jobs.c \
	c_sh.c c_ksh.c c_test.c getopts.c ulimit.c \
	var.c table.c eval.c expr.c 
SRCS3 = unistd.c				# compat lib routines
SRCS =	Makefile $(HDRS) $(SRCS1) $(SRCS2) $(SRCS3)

OBJS =	version.o main.o misc.o \
	syn.o lex.o edit.o tree.o \
	exec.o jobs.o trap.o \
	c_sh.o c_ksh.o c_test.o \
	ulimit.o getopts.o expr.o history.o \
	var.o table.o alloc.o io.o eval.o \
	unistd.o

ksh: $(OBJS) 
	$(CC) $(LDFLAGS) -o $(DESTDIR)$@ $(OBJS) $(LDLIBS)
	su root chown bin $(DESTDIR)$@
	su root chgrp bin $(DESTDIR)$@
	su root chmod 511 $(DESTDIR)$@
	su root fixstack +1000 $(DESTDIR)$@


print: Index $(SRCS)
	$(PRINT) Index $(SRCS)

Index: $(SRCS)
	ctags $(SRCS) >Index

ci:
	touch version.c
	ci -u -q $(SRCS)

tar:	ReadMe ksh.1 $(SRCS)
	tar cf ksh.tar ReadMe ksh.1 $(SRCS)

HP_ksh:	$(OBJS)
	$(CC) $(CFLAGS) -DMWC_HP -c edit.c
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS)
	fixstack +1000 $@
	rm edit.o
