#
#	Makefile for 'conf' commands
#
#					Angus Telfer (January 1987)
#

CC=exec /bin/cc
CFLAGS=-i -s -O

patch:	patch.c
		$(CC) $(CFLAGS) -o patch patch.c

shrink:
		exec /bin/rm -f *.o patch
