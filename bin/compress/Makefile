compress : compress.o is_fs.o
	cc -o compress $<

compress.o : compress.c
	cc -c -O -DVIRTUAL -DBITS=16 $<

install : compress
	su root cp compress /bin
	su root ln -f /bin/compress /bin/zcat
	su root ln -f /bin/compress /bin/decompress
