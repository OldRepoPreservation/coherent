DESTDIR=/bin386/usr/include/
DESTDIR2=/bin386/usr/include/sys/

INCLUDE=a.out.h access.h acct.h action.h ar.h arcoff.h \
        assert.h canon.h coff.h ctype.h curses.h dirent.h \
        dumptape.h ebcdic.h errno.h fcntl.h float.h fperr.h \
        grp.h l.out.h larges.h limits.h math.h memory.h \
        misc.h mnttab.h mon.h mprec.h mtab.h mtype.h \
        n.out.h path.h poll.h pwd.h setjmp.h sgtty.h \
        signal.h stdarg.h stddef.h stdio.h stdlib.h string.h \
        sys.s term.h terminfo.h termio.h time.h timef.h \
        unctrl.h unistd.h ustat.h utmp.h varargs.h
	
INCLUDE_SYS=alloc.h  ascii.h buf.h chars.h con.h const.h deftty.h \
            devices.h dir.h dirent.h fblk.h fd.h fdioctl.h fdisk.h \
            filsys.h hdioctl.h ino.h inode.h io.h ipc.h kb.h \
            kbscan.h lpioctl.h machine.h malloc.h mdata.h mount.h msg.h \
            msig.h mtioctl.h param.h poll.h proc.h rlock.h sched.h \
            seg.h sem.h shm.h stat.h statfs.h stream.h timeb.h \
            timeout.h times.h tty.h types.h uproc.h utsname.h

op_system: $(INCLUDE) $(INCLUDE_SYS)
		(co $(INCLUDE) ; cp $(INCLUDE) $(DESTDIR) ; rm $(INCLUDE))
		(co $(INCLUDE_SYS) ; cp $(INCLUDE_SYS) $(DESTDIR2) ; rm $(INCLUDE_SYS))
