DESTDIR= /f1/bin/

compress : compress.c is_fs.o
	cc -A -DVIRTUAL -DBITS=16 -o $(DESTDIR)compress compress.c is_fs.o
	su root chown bin $(DESTDIR)compress
	su root chgrp bin $(DESTDIR)compress
	su root chmod 511 $(DESTDIR)compress
	su root ln -f $(DESTDIR)compress $(DESTDIR)uncompress
	su root ln -f $(DESTDIR)compress $(DESTDIR)zcat
