# misc/Makefile 10/11/90

DESTDIR=/interim/usr/lib/

# Objects.
O = \
		auto.o\
		back.o\
		ctxt.o\
		error.o\
		less.o\
		look.o\
		main.o\
		more.o\
		next.o\
		rjct.o\
		wrap.o\
		ctab.o\
		ltab.o


# Primary target.
libl.a: $O
	su root rm -f $(DESTDIR)libl.a
	ar q $(DESTDIR)libl.a $O
	su root chmog 444 bin bin $(DESTDIR)$@
	su root ranlib $(DESTDIR)$@
	mv *.o objects/	
