# misc/Makefile 10/11/90
CFLAGS=
# Objects.
O = \
	alloc.o \
	approx.o \
	ask.o \
	banner.o \
	basename.o \
	copyd.o \
	crc16.o \
	fatal.o \
	flatten.o \
	getline.o \
	is_fs.o \
	jdtoti.o \
	jdtotm.o \
	lcase.o \
	lock.o \
	match.o \
	metaphone.o \
	newcpy.o \
	pathn.o \
	picture.o \
	randl.o \
	replace.o \
	regexp.o \
	regsub.o \
	regerror.o \
	showflag.o \
	skip.o \
	span.o \
	splitter.o \
	strchrtr.o \
	strcmpl.o \
	strdlen.o \
	titojd.o \
	tmtojd.o \
	tocont.o \
	trim.o \
	ucase.o \
	usage.o \
	virtual.o \
	xdump.o \
	xopen.o \
	yn.o

# Primary target.
libmisc.a: $O
	rm -f libmisc.a
	ar crs libmisc.a $O

install: libmisc.a
	su root cp -d libmisc.a /usr/lib
	su root cp -d `srcpath *.h`  /usr/include

picture: picture.c
	$(CC) -f -DTEST $< -lm
	picture | scat
