# CFLAGS are not needed for i8086
CFLAGS= -O -DNEWTTYS

CMDS= login passwd su ac ar at bad banner basename c cal calendar cat chgrp \
      chmod chown cmp col comm cp cpdir crypt date dd deroff df du echo egrep \
      factor file fixstack from head help join kill lc ln look ls m4 mail man \
      mesg mkdir msg mv newgrp nm od pr prep prof pwd quot ranlib rev rm rmdir \
      sa scat size sleep sort split strings strip stty sum sync tail tar tee \
      time touch tr tty typo uniq units wall wc who write yes qfind

all:	$(CMDS)
	>all
# include file deftty.h defines the default ERASE, KILL and INTERRUPT 
# characters for ttys.

login:	login.c deftty.h
	$(CC) $(CFLAGS) -o login login.c

passwd:	passwd.c
	$(CC) $(CFLAGS) -o passwd passwd.c

su:	su.c
	$(CC) $(CFLAGS) -o su su.c

ac:	ac.c
	$(CC) $(CFLAGS) -o ac ac.c

ar:	ar.c
	$(CC) $(CFLAGS) -o ar ar.c

at:	at.c
	$(CC) $(CFLAGS) -o at at.c

bad:	bad.c
	$(CC) $(CFLAGS) -o bad bad.c

banner:	banner.c
	$(CC) $(CFLAGS) -o banner banner.c

basename: basename.c
	$(CC) $(CFLAGS) -o basename basename.c

c:	c.c
	$(CC) $(CFLAGS) -o c c.c

cal:	cal.c
	$(CC) $(CFLAGS) -o cal cal.c

calendar: calendar.c
	$(CC) $(CFLAGS) -o calendar calendar.c

cat:	cat.c
	$(CC) $(CFLAGS) -o cat cat.c

chgrp:	chgrp.c
	$(CC) $(CFLAGS) -o chgrp chgrp.c

chmod:	chmod.c
	$(CC) $(CFLAGS) -o chmod chmod.c

chown:	chown.c
	$(CC) $(CFLAGS) -o chown chown.c

cmp:	cmp.c
	$(CC) $(CFLAGS) -o cmp cmp.c

col:	col.c
	$(CC) $(CFLAGS) -o col col.c

comm:	comm.c
	$(CC) $(CFLAGS) -o comm comm.c

cp:	cp.c
	$(CC) $(CFLAGS) -o cp cp.c

cpdir:	cpdir.c
	$(CC) $(CFLAGS) -o cpdir cpdir.c

crypt:	crypt.c
	$(CC) $(CFLAGS) -o crypt crypt.c

date:	date.c
	$(CC) $(CFLAGS) -o date date.c

dd:	dd.c
	$(CC) $(CFLAGS) -o dd dd.c

deroff:	deroff.c
	$(CC) $(CFLAGS) -o deroff deroff.c

df:	df.c
	$(CC) $(CFLAGS) -o df df.c

du:	du.c
	$(CC) $(CFLAGS) -o du du.c

echo:	echo.c
	$(CC) $(CFLAGS) -o echo echo.c

egrep:	egrep.c
	$(CC) $(CFLAGS) -o egrep egrep.c

factor:	factor.c
	$(CC) $(CFLAGS) -o factor factor.c -lm

file:	file.c
	$(CC) $(CFLAGS) -o file file.c

fixstack: fixstack.c
	$(CC) $(CFLAGS) -o fixstack fixstack.c

from:	from.c
	$(CC) $(CFLAGS) -o from from.c

head:	head.c
	$(CC) $(CFLAGS) -o head head.c

help:	help.c
	$(CC) $(CFLAGS) -o help help.c

join:	join.c
	$(CC) $(CFLAGS) -o join join.c

kill:	kill.c
	$(CC) $(CFLAGS) -o kill kill.c

lc:	lc.c
	$(CC) $(CFLAGS) -o lc lc.c

ln:	ln.c
	$(CC) $(CFLAGS) -o ln ln.c

look:	look.c
	$(CC) $(CFLAGS) -o look look.c

ls:	ls.c
	$(CC) $(CFLAGS) -o ls ls.c

m4:	m4.c
	$(CC) $(CFLAGS) -o m4 m4.c

mail:	mail.c
	$(CC) $(CFLAGS) -o mail mail.c

man:	man.c
	$(CC) $(CFLAGS) -o man man.c

mesg:	mesg.c
	$(CC) $(CFLAGS) -o mesg mesg.c

mkdir:	mkdir.c
	$(CC) $(CFLAGS) -o mkdir mkdir.c

msg:	msg.c
	$(CC) $(CFLAGS) -o msg msg.c

mv:	mv.c
	$(CC) $(CFLAGS) -o mv mv.c

newgrp:	newgrp.c
	$(CC) $(CFLAGS) -o newgrp newgrp.c

nm:	nm.c
	$(CC) $(CFLAGS) -o nm nm.c

od:	od.c
	$(CC) $(CFLAGS) -o od od.c

pr:	pr.c
	$(CC) $(CFLAGS) -o pr pr.c

prep:	prep.c
	$(CC) $(CFLAGS) -o prep prep.c

prof:	prof.c
	$(CC) $(CFLAGS) -o prof prof.c

pwd:	pwd.c
	$(CC) $(CFLAGS) -o pwd pwd.c

quot:	quot.c
	$(CC) $(CFLAGS) -o quot quot.c

ranlib:	ranlib
	$(CC) $(CFLAGS) -o ranlib ranlib.c

rev:	rev.c
	$(CC) $(CFLAGS) -o rev rev.c

rm:	rm.c
	$(CC) $(CFLAGS) -o rm rm.c

rmdir:	rmdir.c
	$(CC) $(CFLAGS) -o rmdir rmdir.c

sa:	sa.c
	$(CC) $(CFLAGS) -o sa sa.c

scat:	scat.c
	$(CC) $(CFLAGS) -o scat scat.c

size:	size.c
	$(CC) $(CFLAGS) -o size size.c

sleep:	sleep.c
	$(CC) $(CFLAGS) -o sleep sleep.c

sort:	sort.c
	$(CC) $(CFLAGS) -o sort sort.c

split:	split.c
	$(CC) $(CFLAGS) -o split split.c

strings: strings.c
	$(CC) $(CFLAGS) -o strings strings.c

strip:	strip.c
	$(CC) $(CFLAGS) -o strip strip.c

stty:	stty.c
	$(CC) $(CFLAGS) -o stty stty.c

sum:	sum.c
	$(CC) $(CFLAGS) -o sum sum.c

sync:	sync.c
	$(CC) $(CFLAGS) -o sync sync.c

tail:	tail.c
	$(CC) $(CFLAGS) -o tail tail.c

tar:	tar.c
	$(CC) $(CFLAGS) -o tar tar.c

tee:	tee.c
	$(CC) $(CFLAGS) -o tee tee.c

time:	time.c
	$(CC) $(CFLAGS) -o time time.c

touch:	touch.c
	$(CC) $(CFLAGS) -o touch time.c

tr:	tr.c
	$(CC) $(CFLAGS) -o tr tr.c

tty:	tty.c
	$(CC) $(CFLAGS) -o tty tty.c

typo:	typo.c
	$(CC) $(CFLAGS) -o typo typo.c

uniq:	uniq.c
	$(CC) $(CFLAGS) -o uniq uniq.c

units:	units.c
	$(CC) $(CFLAGS) -o units units.c

wall:	wall.c
	$(CC) $(CFLAGS) -o wall wall.c

wc:	wc.c
	$(CC) $(CFLAGS) -o wc wc.c

who:	who.c
	$(CC) $(CFLAGS) -o who who.c

write:	write.c
	$(CC) $(CFLAGS) -o write write.c

yes:	yes.c
	$(CC) $(CFLAGS) -o yes yes.c

qfind:	qfind.c
	$(CC) $(CFLAGS) -o qfind qfind.c
