P =lsys1.o uuin.o uunam.o helpscn.o devices.o permis.o \
  comment.o choices.o choices1.o choices2.o getline.o escapes.o
S = Makefile lsys1 uuin uunam helpscn devices permis \
 comment choices choices1 choices2 escapes uuinstall.c getline.c \
 L.sys Permissions domain uucpname uuinstall

DESTDIR=/interim/usr/bin/
CFLAGS = -I. -A
L = -lscn  -lcurses -lterm -lmisc

uuinstall : $P uuinstall.o /usr/lib/libscn.a
	cc -o $(DESTDIR)uuinstall uuinstall.o $P $L
	su root chmog 500 root root /interim/usr/bin/uuinstall

uuinstall.o : uuinstall.c

comment.c : comment
	scn $<

choices.c : choices
	scn $<

choices1.c : choices1
	scn $<

escapes.c : escapes
	scn $<

choices2.c : choices2
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
choices2.o : choices2.c
escapes.o : escapes.c
lsys1.o : lsys1.c
uuin.o : uuin.c
uunam.o : uunam.c
helpscn.o : helpscn.c
getline.o : getline.c
