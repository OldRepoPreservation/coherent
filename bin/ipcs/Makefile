# make ipcs
COMMON = ipcs.h Makefile

SRC = 	coffpatch.c \
	fifo.c \
	getipc.c \
	getkrnl.c \
	ipcs.c \
	misc.c \
	pick_nfile.c \
	printipc.c

OBJ = 	obj/coffpatch.o \
	obj/fifo.o \
	obj/getipc.o \
	obj/getkrnl.o \
	obj/ipcs.o \
	obj/misc.o \
	obj/pick_nfile.o \
	obj/printipc.o

bin/ipcs: $(OBJ) $(COMMON)
	cc $(OBJ) -o bin/ipcs 

obj/coffpatch.o:	coffpatch.c $(COMMON)
	cc -c coffpatch.c -o obj/coffpatch.o

obj/fifo.o:	fifo.c $(COMMON)
	cc -c fifo.c -o obj/fifo.o

obj/pick_nfile.o:	pick_nfile.c $(COMMON)
	cc -c pick_nfile.c -o obj/pick_nfile.o

obj/getipc.o : getipc.c $(COMMON)
	cc -c getipc.c -o obj/getipc.o

obj/getkrnl.o : getkrnl.c $(COMMON)
	cc -c getkrnl.c -o obj/getkrnl.o

obj/ipcs.o : ipcs.c $(COMMON)
	cc -c ipcs.c -o obj/ipcs.o

obj/misc.o : misc.c $(COMMON)
	cc -c misc.c -o obj/misc.o

obj/printipc.o : printipc.c $(COMMON)
	cc -c printipc.c -o obj/printipc.o
