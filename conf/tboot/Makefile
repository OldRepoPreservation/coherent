# Makefile for Tertiary boot code for COHERENT
# Targets are stand-alone bootable programs.
#
# La Monte H. Yarroll <piggy@mwc.com>, September 1991

OBJECTS=Startup.o bootlib.o cbootlib.o diskio.o indirect.o intcall.o monitor.o
SOURCES=Startup.s bootlib.s cbootlib.c diskio.c indirect.c intcall.m monitor.c
LIBS=-lc
LD=ld


# NB: OBJECTS must come first in the link line, so the entry
# point in Startup lines up correctly.
tboot: tboot.o builtin.o $(OBJECTS)
	$(LD) -o tboot $(OBJECTS) tboot.o builtin.o $(LIBS)

hello: hello.o $(OBJECTS) 
	$(LD) -o hello $(OBJECTS) hello.o $(LIBS)

dir: dir.o $(OBJECTS) 
	$(LD) -o dir $(OBJECTS) dir.o $(LIBS)

clean:
	rm -f l.out *.o
