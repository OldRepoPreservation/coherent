#
#	Makefile for the mail system.
#
#	CFLAGS = -O -DHPCOHERENT	: if on Inetco HP Coherent
#	CFLAGS = -O			: if on MWC Coherent
#
CFLAGS=-O
O = send.o lmail.o util.o copyright.o
S = send.c lmail.c util.c mail.h Makefile

lmail : $O
	${CC} -n -o lmail $O
	fixstack 1000 lmail

copyright.o : ../copyright.c
