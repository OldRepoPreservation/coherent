# /usr/src/cmd/grep/Makefile 2/20/91

OBJ=\
	grep1.o\
	grep2.o

grep:	$(OBJ)
	$(CC) $(LDFLAGS) -o $* $(OBJ)

grep1.o: grep.h grep1.c
grep2.o: grep.h grep2.c

clean:
	rm $(OBJ)

# end of /usr/src/cmd/grep/Makefile
