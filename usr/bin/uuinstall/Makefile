P =lsys1.o uuin.o uunam.o helpscn.o devices.o permis.o \
  comment.o choices.o choices1.o
S = Makefile lsys1 uuin uunam helpscn devices permis \
 comment choices choices1 uuinstall.c \
 L.sys Permissions domain uucpname uuinstall

CFLAGS = -I.
L = -lscn  -lcurses -lterm -lmisc

uuinstall : $P uuinstall.o
	cc -o uuinstall uuinstall.o $P $L

uuinstall.o : uuinstall.c

comment.c : comment
	scn $<

choices.c : choices
	scn $<

choices1.c : choices1
	scn $<

uunam.c : uunam
	scn $<

lsys1.c : lsys1
	scn $<

uuin.c : uuin
	scn $<

helpscn.c : helpscn
	scn $<

devices.c : devices
	scn $<

permis.c : permis
	scn $<

comment.o : comment.c
devices.o : devices.c
permis.o : permis.c
choices.o : choices.c
choices1.o : choices1.c
lsys1.o : lsys1.c
uuin.o : uuin.c
uunam.o : uunam.c
helpscn.o : helpscn.c
