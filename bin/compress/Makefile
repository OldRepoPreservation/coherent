DESTDIR= /f1/bin/

compress : compress.c is_fs.o
	cc -A -DVIRTUAL -DBITS=16 -o $(DESTDIR)compress compress.c is_fs.o

