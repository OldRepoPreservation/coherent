# ld/Makefile 10/20/91
# i80386 Coff loader Makefile.
CFLAGS=-A -O
LFLAGS=-s

DESTDIR=/interim/bin/

S = Makefile script data.c data.h main.c message.c pass1.c pass2.c
D = main.c pass1.c pass2.c

OBJ=	main.o\
	data.o\
	message.o\
	pass1.o\
	pass2.o

# Primary target.
ld :	$(OBJ)
	rm -f ld
	$(CC) $(LFLAGS) -o ld $<

install : ld
	su root cp ld /bin

# Save on hp
save : $S
	tar cf - $< | compress -c > /tmp/ld.tar.Z
	uucp -r /tmp/ld.tar.Z mwc!/v/cef/

# Unlinked objects.
main.o:		main.c data.h
data.o:		data.c 	data.h
message.o:	message.c data.h
pass1.o:	pass1.c	data.h
pass2.o:	pass2.c	data.h

# Etc.
clean:
	rm *.o


doc : $D
	cgrep -d @ "(message|fatal|filemsg|modmsg|mpmsg|spmsg)" $< | \
	 sort | \
	 sed -f `srcpath script` | \
	 fixdoc > doc

# end of ld/Makefile
