
#Makefile for /usr/games

DESTDIR=/interim/usr/games/

GAMES: fortune guess moo ttt rubik chase lines

all: $(GAMES)

fortune:
	cc -O -o $(DESTDIR)fortune fortune.c
	su root chmog 511 bin bin $(DESTDIR)$@
	cd lib ; co $(DESTDIR)lib/fortunes
	su root chmog 511 bin bin $(DESTDIR)lib/fortunes

guess:
	cc -O -o $(DESTDIR)guess guess.c
	su root chmog 511 bin bin $(DESTDIR)$@

moo:
	cc -O -o $(DESTDIR)moo moo.c
	su root chmog 511 bin bin $(DESTDIR)$@

ttt:
	cc -O -o $(DESTDIR)ttt ttt.c
	su root chmog 511 bin bin $(DESTDIR)$@

rubik:
	co $(DESTDIR)rubik
	su root chmog 511 bin bin $(DESTDIR)$@
	cd lib ; co $(DESTDIR)lib/rubik.m4
	cd $(DESTDIR)lib ; su root chmog 644 bin bin rubik.m4

chase:
	cd pacman ; make

lines:
	cc -O -o $(DESTDIR)$@ lines.c
	su root chmog 511 bin bin $(DESTDIR)$@
