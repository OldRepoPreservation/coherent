CMDS=kermit vi less cpio modeminit ramdisk me cgrep
DESTDIR=/interim/usr/bin/

all:	$(CMDS)

kermit:	kermit.c
	cc -O -o $(DESTDIR)$@ kermit.c
	su root chown bin $(DESTDIR)$@
	su root chgrp bin $(DESTDIR)$@
	su root chmod 511 $(DESTDIR)$@

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

cgrep: cgrep.c
	cc -O -o $(DESTDIR)$@ cgrep.c -lmisc
	su root chown bin $(DESTDIR)$@
	su root chgrp bin $(DESTDIR)$@
	su root chmod 511 $(DESTDIR)$@
