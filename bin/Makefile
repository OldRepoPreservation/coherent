# CFLAGS are not needed for i8086
CFLAGS= -O -DNEWTTYS
DESTDIR= /f1/bin/
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
	$(CC) $(CFLAGS) -o $(DESTDIR)login login.c

passwd:	passwd.c
	$(CC) $(CFLAGS) -o $(DESTDIR)passwd passwd.c

su:	su.c
	$(CC) $(CFLAGS) -o $(DESTDIR)su su.c

ac:	ac.c
	$(CC) $(CFLAGS) -o $(DESTDIR)ac ac.c

ar:	ar.c
	$(CC) $(CFLAGS) -o $(DESTDIR)ar ar.c

at:	at.c
	$(CC) $(CFLAGS) -o $(DESTDIR)at at.c

bad:	bad.c
	$(CC) $(CFLAGS) -o $(DESTDIR)bad bad.c

banner:	banner.c
	$(CC) $(CFLAGS) -o $(DESTDIR)banner banner.c

basename: basename.c
	$(CC) $(CFLAGS) -o $(DESTDIR)basename basename.c

c:	c.c
	$(CC) $(CFLAGS) -o $(DESTDIR)c c.c

cal:	cal.c
	$(CC) $(CFLAGS) -o $(DESTDIR)cal cal.c

calendar: calendar.c
	$(CC) $(CFLAGS) -o $(DESTDIR)calendar calendar.c

cat:	cat.c
	$(CC) $(CFLAGS) -o $(DESTDIR)cat cat.c

chgrp:	chgrp.c
	$(CC) $(CFLAGS) -o $(DESTDIR)chgrp chgrp.c

chmod:	chmod.c
	$(CC) $(CFLAGS) -o $(DESTDIR)chmod chmod.c

chown:	chown.c
	$(CC) $(CFLAGS) -o $(DESTDIR)chown chown.c

cmp:	cmp.c
	$(CC) $(CFLAGS) -o $(DESTDIR)cmp cmp.c

col:	col.c
	$(CC) $(CFLAGS) -o $(DESTDIR)col col.c

comm:	comm.c
	$(CC) $(CFLAGS) -o $(DESTDIR)comm comm.c

cp:	cp.c
	$(CC) $(CFLAGS) -o $(DESTDIR)cp cp.c

cpdir:	cpdir.c
	$(CC) $(CFLAGS) -o $(DESTDIR)cpdir cpdir.c

crypt:	crypt.c
	$(CC) $(CFLAGS) -o $(DESTDIR)crypt crypt.c

date:	date.c
	$(CC) $(CFLAGS) -o $(DESTDIR)date date.c

dd:	dd.c
	$(CC) $(CFLAGS) -o $(DESTDIR)dd dd.c

deroff:	deroff.c
	$(CC) $(CFLAGS) -o $(DESTDIR)deroff deroff.c

df:	df.c
	$(CC) $(CFLAGS) -o $(DESTDIR)df df.c

du:	du.c
	$(CC) $(CFLAGS) -o $(DESTDIR)du du.c

echo:	echo.c
	$(CC) $(CFLAGS) -o $(DESTDIR)echo echo.c

egrep:	egrep.c
	$(CC) $(CFLAGS) -o $(DESTDIR)egrep egrep.c

factor:	factor.c
	$(CC) $(CFLAGS) -o $(DESTDIR)factor factor.c -lm

file:	file.c
	$(CC) $(CFLAGS) -o $(DESTDIR)file file.c

fixstack: fixstack.c
	$(CC) $(CFLAGS) -o $(DESTDIR)fixstack fixstack.c

from:	from.c
	$(CC) $(CFLAGS) -o $(DESTDIR)from from.c

head:	head.c
	$(CC) $(CFLAGS) -o $(DESTDIR)head head.c

help:	help.c
	$(CC) $(CFLAGS) -o $(DESTDIR)help help.c

join:	join.c
	$(CC) $(CFLAGS) -o $(DESTDIR)join join.c

kill:	kill.c
	$(CC) $(CFLAGS) -o $(DESTDIR)kill kill.c

lc:	lc.c
	$(CC) $(CFLAGS) -o $(DESTDIR)lc lc.c

ln:	ln.c
	$(CC) $(CFLAGS) -o $(DESTDIR)ln ln.c

look:	look.c
	$(CC) $(CFLAGS) -o $(DESTDIR)look look.c

ls:	ls.c
	$(CC) $(CFLAGS) -o $(DESTDIR)ls ls.c

m4:	m4.c
	$(CC) $(CFLAGS) -o $(DESTDIR)m4 m4.c

mail:	mail.c
	$(CC) $(CFLAGS) -o $(DESTDIR)mail mail.c

man:	man.c
	$(CC) $(CFLAGS) -o $(DESTDIR)man man.c

mesg:	mesg.c
	$(CC) $(CFLAGS) -o $(DESTDIR)mesg mesg.c

mkdir:	mkdir.c
	$(CC) $(CFLAGS) -o $(DESTDIR)mkdir mkdir.c

msg:	msg.c
	$(CC) $(CFLAGS) -o $(DESTDIR)msg msg.c

mv:	mv.c
	$(CC) $(CFLAGS) -o $(DESTDIR)mv mv.c

newgrp:	newgrp.c
	$(CC) $(CFLAGS) -o $(DESTDIR)newgrp newgrp.c

nm:	nm.c
	$(CC) $(CFLAGS) -o $(DESTDIR)nm nm.c

od:	od.c
	$(CC) $(CFLAGS) -o $(DESTDIR)od od.c

pr:	pr.c
	$(CC) $(CFLAGS) -o $(DESTDIR)pr pr.c

prep:	prep.c
	$(CC) $(CFLAGS) -o $(DESTDIR)prep prep.c

prof:	prof.c
	$(CC) $(CFLAGS) -o $(DESTDIR)prof prof.c

pwd:	pwd.c
	$(CC) $(CFLAGS) -o $(DESTDIR)pwd pwd.c

quot:	quot.c
	$(CC) $(CFLAGS) -o $(DESTDIR)quot quot.c

ranlib:	ranlib
	$(CC) $(CFLAGS) -o $(DESTDIR)ranlib ranlib.c

rev:	rev.c
	$(CC) $(CFLAGS) -o $(DESTDIR)rev rev.c

rm:	rm.c
	$(CC) $(CFLAGS) -o $(DESTDIR)rm rm.c

rmdir:	rmdir.c
	$(CC) $(CFLAGS) -o $(DESTDIR)rmdir rmdir.c

sa:	sa.c
	$(CC) $(CFLAGS) -o $(DESTDIR)sa sa.c

scat:	scat.c
	$(CC) $(CFLAGS) -o $(DESTDIR)scat scat.c

size:	size.c
	$(CC) $(CFLAGS) -o $(DESTDIR)size size.c

sleep:	sleep.c
	$(CC) $(CFLAGS) -o $(DESTDIR)sleep sleep.c

sort:	sort.c
	$(CC) $(CFLAGS) -o $(DESTDIR)sort sort.c

split:	split.c
	$(CC) $(CFLAGS) -o $(DESTDIR)split split.c

strings: strings.c
	$(CC) $(CFLAGS) -o $(DESTDIR)strings strings.c

strip:	strip.c
	$(CC) $(CFLAGS) -o $(DESTDIR)strip strip.c

stty:	stty.c
	$(CC) $(CFLAGS) -o $(DESTDIR)stty stty.c

sum:	sum.c
	$(CC) $(CFLAGS) -o $(DESTDIR)sum sum.c

sync:	sync.c
	$(CC) $(CFLAGS) -o $(DESTDIR)sync sync.c

tail:	tail.c
	$(CC) $(CFLAGS) -o $(DESTDIR)tail tail.c

tar:	tar.c
	$(CC) $(CFLAGS) -o $(DESTDIR)tar tar.c

tee:	tee.c
	$(CC) $(CFLAGS) -o $(DESTDIR)tee tee.c

time:	time.c
	$(CC) $(CFLAGS) -o $(DESTDIR)time time.c

touch:	touch.c
	$(CC) $(CFLAGS) -o $(DESTDIR)touch time.c

tr:	tr.c
	$(CC) $(CFLAGS) -o $(DESTDIR)tr tr.c

tty:	tty.c
	$(CC) $(CFLAGS) -o $(DESTDIR)tty tty.c

typo:	typo.c
	$(CC) $(CFLAGS) -o $(DESTDIR)typo typo.c

uniq:	uniq.c
	$(CC) $(CFLAGS) -o $(DESTDIR)uniq uniq.c

units:	units.c
	$(CC) $(CFLAGS) -o $(DESTDIR)units units.c

wall:	wall.c
	$(CC) $(CFLAGS) -o $(DESTDIR)wall wall.c

wc:	wc.c
	$(CC) $(CFLAGS) -o $(DESTDIR)wc wc.c

who:	who.c
	$(CC) $(CFLAGS) -o $(DESTDIR)who who.c

write:	write.c
	$(CC) $(CFLAGS) -o $(DESTDIR)write write.c

yes:	yes.c
	$(CC) $(CFLAGS) -o $(DESTDIR)yes yes.c

qfind:	qfind.c
	$(CC) $(CFLAGS) -o $(DESTDIR)qfind qfind.c
