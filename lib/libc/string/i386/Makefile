# /usr/src/libc/string/i386/Makefile 1/10/92

CRTI386=\
	_memxchg.o\
	memccpy.o\
	memchr.o\
	memcmp.o\
	memcpy.o\
	memmove.o\
	memset.o\
	strcat.o\
	strchr.o\
	strcmp.o\
	strcpy.o\
	strcspn.o\
	streq.o\
	strlen.o\
	strpbrk.o\
	strrchr.o\
	strspn.o\
	strstr.o

# Primary target.
STRINGI386:	$(CRTI386)
	$(AR) rkus libc.a $?
	touch $@
	: libc/string/i386 done

# Secondary targets.
_memxchg.o:	_memxchg.s
memccpy.o:	memccpy.s
memchr.o:	memchr.s
memcmp.o:	memcmp.s
memcpy.o:	memcpy.s
memmove.o:	memmove.s
memset.o:	memset.s
strcat.o:	strcat.s
strchr.o:	strchr.s
strcmp.o:	strcmp.s
strcpy.o:	strcpy.s
strcspn.o:	strcspn.s
streq.o:	streq.s
strlen.o:	strlen.s
strpbrk.o:	strpbrk.s
strrchr.o:	strrchr.s
strspn.o:	strspn.s
strstr.o:	strstr.s

# end of /usr/src/libc/string/i386/Makefile
