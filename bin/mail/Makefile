O = send.o mail.o util.o
S = send.c mail.c util.c mail.h Makefile

mail : $O
	cc -o mail $O
	su root chown root mail
	su root chgrp root mail
	su root chmod 6511 mail

mail.a : $S
	rm mail.a
	ar q mail.a $S

