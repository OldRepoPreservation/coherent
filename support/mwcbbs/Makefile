LDFLAGS= -lcurses -lterm
CFLAGS= -c -v -A

OBJECTS= main.o disp_rec.o getflname.o lite.o map_cmd.o states.o

mwcbbs:	$(OBJECTS)
	cc -v -A $< $(LDFLAGS) -o $@

main.o:	main.c
	cc $(CFLAGS) $< $(LDFLAGS) -o $@	

states.o: states.c
	cc $(CFLAGS) $< $(LDFLAGS) -o $@

lite.o:	lite.c
	cc $(CFLAGS) $< $(LDFLAGS) -o $@

getflname.o:	getflname.c
	cc $(CFLAGS) $< $(LDFLAGS) -o $@

map_cmd.o:	map_cmd.c
	cc $(CFLAGS) $< $(LDFLAGS) -o $@

disp_rec.o:	disp_rec.c
	cc $(CFLAGS) $< $(LDFLAGS) -o $@
