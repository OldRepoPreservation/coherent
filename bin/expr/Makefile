DESTDIR=/interim/bin/

expr:	expr.o	
	cc -O -o $(DESTDIR)expr expr.o -ly
	su root chown bin $(DESTDIR)$@
	su root chgrp bin $(DESTDIR)$@
	su root chmod 511 $(DESTDIR)$@

expr.o:	expr.y

