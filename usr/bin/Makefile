CMDS=kermit vi less cpio modeminit ramdisk me uuprogs uuins \
     uuclean ksh cut
DESTDIR=/interim/usr/bin/

all:	$(CMDS)

kermit:	kermit.c
	cc -O -o $(DESTDIR)$@ kermit.c
	su root chmog 511 bin bin $(DESTDIR)$@

cut:
	cd cutpaste/objects ; export SRCPATH=.. ; make

vi:
	cd elvis; make

less:
	cd more; make

cpio:
	cd pax/shipping; make

me:
	cd emacs; make

modeminit:
	su root co $(DESTDIR)$@
	su root chown bin $(DESTDIR)$@
	su root chgrp bin $(DESTDIR)$@
	su root chmod 555 $(DESTDIR)$@

ramdisk:
	su root co $(DESTDIR)$@
	su root chgrp root $(DESTDIR)$@
	su root chown root $(DESTDIR)$@
	su root chmod 700 $(DESTDIR)$@

uuins:
	cd uuinstall/objects ; export SRCPATH=.. ; make

uuclean:
	su root co $(DESTDIR)$@
	su root chown uucp $(DESTDIR)$@
	su root chgrp uucp $(DESTDIR)$@
	su root chmod 500 $(DESTDIR)$@

uuprogs:
	cd uucpstuff/src; make

ksh:
	cd korn; make
