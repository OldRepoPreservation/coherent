# misc/Makefile 10/11/90

DESTDIR=/interim/usr/lib/

# Objects.
O = \
	yyerror.o\
	yylex.o\
	yymain.o

# Primary target.
liby.a: $O
	su root rm -f $(DESTDIR)liby.a
	ar q $(DESTDIR)liby.a $O
	su root chmog 444 bin bin $(DESTDIR)$@
	su root ranlib $(DESTDIR)$@
	mv *.o objects/	
