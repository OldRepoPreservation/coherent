# CFLAGS are not needed for i8086
CFLAGS= -O -DNEWTTYS
UTILS= getty init enable accton cron drvld fdformat \
	mkfs mknod mount umount unmkfs update wall

all:	$(UTILS)
	>all
# include file deftty.h defines the default ERASE, KILL and INTERRUPT 
# characters for ttys.
getty:	getty.c deftty.h
	$(CC) $(CFLAGS) -s -n -i -o getty getty.c

init:	init.c
	$(CC) $(CFLAGS) -s -n -i -o init init.c

enable:	enable.c
	$(CC) $(CFLAGS) -o enable enable.c

ATclock: ATclock.c
	$(CC) $(CFLAGS) -o ATclock ATclock.c

accton:	accton.c
	$(CC) $(CFLAGS) -o accton accton.c

badscan: badscan.c
	$(CC) $(CFLAGS) -o badscan badscan.c

cron:	cron.c
	$(CC) $(CFLAGS) -o cron cron.c

drvld:	drvld.c
	$(CC) $(CFLAGS) -o drvld drvld.c

fdformat: fdformat.c
	$(CC) $(CFLAGS) -o fdformat fdformat.c

mkfs:	mkfs.c
	$(CC) $(CFLAGS) -o mkfs mkfs.c

mknod:	mknod.c
	$(CC) $(CFLAGS) -o mknod mknod.c

mount:	mount.c
	$(CC) $(CFLAGS) -o mount mount.c

reboot:	reboot.c
	$(CC) $(CFLAGS) -o reboot reboot.c

umount:	umount.c
	$(CC) $(CFLAGS) -o umount umount.c

unmkfs:	unmkfs.c
	$(CC) $(CFLAGS) -o unmkfs unmkfs.c

update:	update.c
	$(CC) $(CFLAGS) -o update update.c

wall:	wall.c
	$(CC) $(CFLAGS) -o wall wall.c
