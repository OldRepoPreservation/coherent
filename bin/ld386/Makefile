O = ld.o pass1.o message.o fake.o
S = Makefile ld.h ld.c pass1.c message.c fake.c script
D = ld.c pass1.c
CFLAGS = -AO

drvld: ld
	ln -f ld drvld
	
ld : $O
	rm -f ld
	cc -so ld $O -lmisc

# save on mwc!
save : $S
	tar cf - $< | compress -c > /tmp/newld.tar.Z
	uucp -r /tmp/newld.tar.Z mwc!/v/cef/

# list source files
list : $S
	ph $?
	touch list

# Build error message list in troff format.
doc : $D
	cgrep -d @ "(message|fatal|filemsg|modmsg|mpmsg|spmsg|spwarn)" $< | \
	 sort | \
	 sed -f `srcpath script` | \
	 fixdoc > doc

# Install new ld
install : ld
	su root `srcpath installer`

# end of ld/Makefile
