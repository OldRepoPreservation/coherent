DESTDIR=/bin386/bin/

expr:	expr.o	
	cc -O -o $(DESTDIR)expr expr.o -ly
	su root chown bin $(DESTDIR)expr
	su root chgrp bin $(DESTDIR)expr
	su root chmod 511 $(DESTDIR)expr

expr.o:	expr.y

