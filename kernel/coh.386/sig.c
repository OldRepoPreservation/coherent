/* $Header: /y/coh.386/RCS/sig.c,v 1.5 92/11/09 17:10:59 root Exp $ */
/* (lgl-
 *	The information contained herein is a trade secret of Mark Williams
 *	Company, and  is confidential information.  It is provided  under a
 *	license agreement,  and may be  copied or disclosed  only under the
 *	terms of  that agreement.  Any  reproduction or disclosure  of this
 *	material without the express written authorization of Mark Williams
 *	Company or persuant to the license agreement is unlawful.
 *
 *	COHERENT Version 2.3.37
 *	Copyright (c) 1982, 1983, 1984.
 *	An unpublished work by Mark Williams Company, Chicago.
 *	All rights reserved.
 -lgl) */
/*
 * Coherent.
 * Signal handling.
 *
 * $Log:	sig.c,v $
 * Revision 1.5  92/11/09  17:10:59  root
 * Just before adding vio segs.
 * 
 * Revision 1.4  92/10/06  23:48:53  root
 * Ker #64
 * 
 * Revision 1.2  92/01/06  12:00:24  hal
 * Compile with cc.mwc.
 * 
 * Revision 1.1	88/03/24  16:14:24	src
 * Initial revision
 * 
 * 87/11/05	Allan Cornish		/usr/src/sys/coh/sig.c
 * New seg struct now used to allow extended addressing.
 *
 * 86/11/19	Allan Cornish		/usr/src/sys/coh/sig.c
 * sigdump() initializes the (new) (IO).io_flag field to 0.
 */
#include <sys/coherent.h>
#include <errno.h>
#include <sys/ino.h>
#include <sys/inode.h>
#include <sys/io.h>
#include <sys/proc.h>
#include <sys/ptrace.h>
#include <sys/sched.h>
#include <sys/seg.h>
#include <signal.h>

void sendsig();

static struct _fpstate * empack();

/*
 * Set up the action to be taken on a signal.
 */
usigsys(signal, func)
int	signal;
register void (*func)();
{
	register PROC *pp;
	register sig_t s;
	register int (*old_sig)();
	int	sigtype;

	sigtype = signal & ~0xFF;
	signal ^= sigtype;

	pp = SELF;
	if (signal<=0 || signal>NSIG || signal==SIGKILL) {
		u.u_error = EINVAL;
		return;
	}

	/*
	 * In order to avoid runaway, don't allow user to set
	 * handler for SIGSEGV to anything but SIG_DFL.
	 *
	 * We should do something more sophisticated, like detecting
	 * two SEGV's in a row and then killing the process.
	 */
	if (signal == SIGSEGV && func != SIG_DFL) {
		u.u_error = EINVAL;
		return;
	}

	if (sigtype==SIGDEFER || sigtype==0) {
		if (func==SIG_IGN)
			sigtype = SIGIGNORE;
		if (func==SIG_HOLD)
			sigtype = SIGHOLD;
	}

	s = (sig_t)1 << --signal;
	if (pp->p_isig&s)
		old_sig = SIG_IGN;
	else if (pp->p_hsig&s)
		old_sig = SIG_HOLD;
	else
		old_sig = u.u_sfunc[signal];

	switch (sigtype) {
	case SIGHOLD:
		pp->p_hsig |= s;
		break;
	case SIGRELSE:
		pp->p_hsig &= ~s;
		if (nondsig()) {
			T_PIGGY(0x100, printf("a(s)"););
			actvsig();
		}
	case SIGIGNORE:
		pp->p_dfsig &= ~s;	/* No longer defaulted.  */
		pp->p_isig |= s;	/* Mark signal as ignored.  */
		pp->p_ssig &= ~s;	/* Turn off any pending signal.  */
		break;
	case 0:				/* old system entry */
	case SIGDEFER:			/* new system entry */
		u.u_sigreturn = (void (*)())u.u_regl[EDX];
		u.u_sfunc[signal] = func;
		/*
		 * Be sure to mark the signal as defaulted or not.
		 */
		if (SIG_DFL == func) {
			pp->p_dfsig |= s;
		} else {
			pp->p_dfsig &= ~s;
		}
		/*
		 * The signal is no longer ignored or held, and
		 * any pending signal is lost.
		 */
		pp->p_isig &= ~s;
		pp->p_hsig &= ~s;
		pp->p_ssig &= ~s;
		if (sigtype==SIGDEFER)
			pp->p_dsig |= s;
		else
			pp->p_dsig &= ~s;
		break;
	/* SIGPAUSE not done yet */
	default:
		u.u_error = SIGSYS;
		break;
	}
	return old_sig;
}


/*
 * Send a signal to the process `pp'.
 * Return 1 if signal was sent.
 * Return 0 if signal was ignored.
 * The return value is of use to the trap handler.
 */
void
sendsig(sig, pp)
register unsigned sig;
register PROC *pp;
{
	register sig_t f;
	register int s;

	T_PIGGY(0x40000000,
	    printf("<send sig: %d, id: %d, state: %x, flags: %x, event: %x, ",
		   sig, pp->p_pid, pp->p_state, pp->p_flags, pp->p_event);
	); /* T_PIGGY() */

	T_PIGGY(0x40000000, printf("p_isig=%x>", pp->p_isig));

	/*
	 * Convert the signal to a bit position.
	 */
	f = ((sig_t)1) << (sig-1);

	/*
	 * If the signal is ignored, do nothing.
	 */
	if (pp->p_isig & f) {
		goto sendSigDone;
	}

	/*
	 * I do not understand delayed or held signals.
	 */
	if ((pp->p_ssig & f) && (pp->p_hsig|pp->p_dsig) & f)
		goto sendSigDone;
	
	/*
	 * Actually send the signal by flagging the needed bit.
	 */
	pp->p_ssig |= f;

	/*
	 * If the process is sleeping, wake it up so that
	 * it can process this signal.
	 */
	if (pp->p_state == PSSLEEP) {
		s = sphi();
		pp->p_lback->p_lforw = pp->p_lforw;
		pp->p_lforw->p_lback = pp->p_lback;
		addu(pp->p_cval, (utimer-pp->p_lctim)*CVCLOCK);
		setrun(pp);
		spl(s);
	}
sendSigDone:
	return;
}

/*
 * Return signal number if we have a non ignored or delayed signal, else zero.
 */
nondsig()
{
	register PROC *pp;
	register sig_t mask;
	register int signo;

	pp = SELF;
	signo = 0;
	/*
	 * Turn off all ignored signals.
	 */
	pp->p_ssig &= ~pp->p_isig;
	/*
	 * If any signals have arrived, but which are not held,
	 * figure out what they are.
	 */
	if (pp->p_ssig&~pp->p_hsig) {
		/*
		 * There is at least one signal.  Extract its number
		 * from the signal bits.
		 */
		mask = (sig_t) 1;
		signo += 1;
		while (((pp->p_ssig&~pp->p_hsig) & mask) == 0) {
			mask <<= 1;
			signo += 1;
		}
	}
	return (signo);
}

/*
 * If we have a signal that isn't ignored, activate it.
 */
actvsig()
{
	register int signum;
	register PROC *pp;
	register int (*func)();
	int ptval;

	/*
	 * Fetch an unprocessed signal.
	 * Return if there are none.
	 */
	if ((signum = nondsig()) == 0)
		return;

	pp = SELF;

	/*
	 * Reset the signal to indicate that it has been processed.
	 * Bit table p_ssig uses 0-based signals, while signal.h
	 * lists 1-based signals.
	 */
	pp->p_ssig &= ~((sig_t)1<<(signum-1));

	/*
	 * Fetch the user function that goes with this signal.
	 * Function table u_sfunc uses 0-based signals, while signal.h
	 * lists 1-based signals.
	 */
	func = u.u_sfunc[signum-1];

	/*
	 * Store the (1-based) signal number in the u area.
	 * This is how a core dump records the death signal.
	 */
	u.u_signo = signum;

	/*
	 * If the signal is not defaulted, go run the requested
	 * function.
	 */
	if (func != SIG_DFL) {
		if (XMODE_286)
			oldsigstart(signum, func);
		else {
			msigstart(signum, func);
		}
		return;
	}

	/*
	 * ASSERTION:  the signal being processed is SIG_DFL'd.
	 */

	/*
	 * msysgen() is a nop for COHERENT 4.0.  The comment in the
	 * assembly code is "Nothing useful to save".
	 */
	msysgen(u.u_sysgen);

	/*
	 * Do something special for traced processes.  (?)
	 */
	if (pp->p_flags&PFTRAC) {
		pp->p_flags |= PFWAIT;
		ptval = ptret();
		T_HAL(0x10000, printf("ptret()=%x ", ptval));
		pp->p_flags &= ~(PFWAIT|PFSTOP);
		if (ptval == 0)
			return;
	}

	/*
	 * Some signals cause a core file to be written.
	 */
	switch(signum) {
	case SIGQUIT:
	case SIGILL:
	case SIGTRAP:
	case SIGABRT:
	case SIGFPE:
	case SIGSEGV:
	case SIGSYS:
		if (sigdump())
			signum |= 0x80;
		break;
	}
	pexit(signum);
}

/*
 * Create a dump of ourselves onto the file `core'.
 */
sigdump()
{
	register INODE *ip;
	register SR *srp;
	register SEG * sp;
	register int n;
	register paddr_t ssize;
	extern	int	DUMP_LIM;

	if (SELF->p_flags&PFNDMP)
		return (0);
	u.u_io.io_seg  = IOSYS;
	u.u_io.io_flag = 0;
	/* Make the core with the real owners */
	schizo();
	if (ftoi("core", 'c')) {
		schizo();
		return (0);
	}
	if ((ip=u.u_cdiri) == NULL) {
		if ((ip=imake(IFREG|0644, 0)) == NULL) {
			schizo();
			return (0);
		}
	} else {
		if ((ip->i_mode&IFMT)!=IFREG
		 || iaccess(ip, IPW)==0
		 || getment(ip->i_dev, 1)==NULL) {
			idetach(ip);
			schizo();
			return (0);
		}
		iclear(ip);
	}
	schizo();
	u.u_error = 0;
	u.u_io.io_seek = 0;
	for (srp=u.u_segl; u.u_error==0 && srp<&u.u_segl[NUSEG]; srp++) {
		if ((sp = srp->sr_segp)==NULL || (srp->sr_flag&SRFDUMP)==0)
			continue;
		u.u_io.io_seg = IOPHY;
		u.u_io.io.pbase = MAPIO(sp->s_vmem, 0);
		u.u_io.io_flag = 0;
		ssize = sp->s_size;
		if (ssize > DUMP_LIM) {
			printf("seg %d truncated from %d to %d bytes\n",
			  srp-u.u_segl, ssize, DUMP_LIM);
			ssize = DUMP_LIM;
		}
		sp->s_lrefc++;
		while (u.u_error == 0 && ssize != 0) {
			n = ssize > SCHUNK ? SCHUNK : ssize;
			u.u_io.io_ioc = n;
			iwrite(ip, &u.u_io);
			u.u_io.io.pbase += n;
			ssize -= (paddr_t)n;
		}
		sp->s_lrefc--;
	}
	idetach(ip);
	return (u.u_error==0);
}

/*
 * Send a ptrace command to the child.
 */
ptset(req, pid, addr, data)
unsigned req;
int *addr;
{
	register PROC *pp;

	lock(pnxgate);
	for (pp=procq.p_nforw; pp!=&procq; pp=pp->p_nforw)
		if (pp->p_pid == pid)
			break;
	unlock(pnxgate);
	if (pp==&procq || (pp->p_flags&PFSTOP)==0 || pp->p_ppid!=SELF->p_pid){
		u.u_error = ESRCH;
		return;
	}
	lock(pts.pt_gate);
	pts.pt_req = req;
	pts.pt_pid = pid;
	pts.pt_addr = addr;
	pts.pt_data = data;
	pts.pt_errs = 0;
	pts.pt_rval = 0;
	pts.pt_busy = 1;
	wakeup((char *)&pts.pt_req);
	while (pts.pt_busy) {
		v_sleep((char *)&pts.pt_busy, CVPTSET, IVPTSET, SVPTSET, "ptrace");
		/* Send a ptrace command to the child.  */
	}
	u.u_error = pts.pt_errs;
	unlock(pts.pt_gate);
	return (pts.pt_rval);
}

/*
 * This routine is called when a child that is being traced receives a signal
 * that is not caught or ignored.  It follows up on any requests by the parent
 * and returns when done.
 *
 * If the return value is nonzero, the current process (i.e., the traced child)
 * will exit.
 */
ptret()
{
	extern void (*ndpKfrstor)();
	register PROC *pp;
	register PROC *pp1;
	register int sign;
	unsigned off;
	int doEmUnpack = 0;

	struct _fpstate * fstp = empack();

	pp = SELF;
next:
	u.u_error = 0;
	if (pp->p_ppid == 1)
		return (SIGKILL);
	sign = -1;

	/* wake up parent if it is sleeping */
	lock(pnxgate);
	pp1 = &procq;
	for (;;) {
		if ((pp1=pp1->p_nforw) == &procq) {
			sign = SIGKILL;
			break;
		}
		if (pp1->p_pid != pp->p_ppid)
			continue;
		if (pp1->p_state == PSSLEEP)
			wakeup((char *)pp1);
		break;
	}
	unlock(pnxgate);

	while (sign < 0) {
		if (pts.pt_busy==0 || pp->p_pid!=pts.pt_pid) {
			v_sleep((char *)&pts.pt_req,
			  CVPTRET, IVPTRET, SVPTRET, "ptret");
			/* Something about signals to a traced child.  */
			goto next;
		}
		switch (pts.pt_req) {
		case PTRACE_RD_TXT:
			if (XMODE_286) {
				pts.pt_rval = getuwd(NBPS+pts.pt_addr);
				break;
			}
			/* Fall through for 386 mode processes. */
		case PTRACE_RD_DAT:
			pts.pt_rval = getuwd(pts.pt_addr);
			break;
		case PTRACE_RD_USR:
			/* See ptrace.h for valid offsets. */
			off = (unsigned)pts.pt_addr;
			if (off & 3)
				u.u_error = EINVAL;
			else if (off < PTRACE_FP_CW) {
				/* Reading CPU general register state */
				if (off == PTRACE_SIG)
					pts.pt_rval = u.u_signo;
				else
					pts.pt_rval = u.u_regl[off>>2];
			} else if (off < PTRACE_DR0) {
				/*
				 * Reading NDP state.
				 * If NDP state not already saved, save it.
				 * Fetch desired info.
				 * Restore NDP state in case we will resume.
				 */
				if (rdNdpUser()) {
					/* if using coprocessor */
					if (!rdNdpSaved()) {
						ndpSave(&u.u_ndpCon);
						wrNdpSaved(1);
					}
pts.pt_rval = ((int *)&u.u_ndpCon)[(off - PTRACE_FP_CW)>>2];
					ndpRestore(&u.u_ndpCon);
					wrNdpSaved(0);
				} else if (fstp) {
pts.pt_rval = getuwd(((int *)fstp) + ((off - PTRACE_FP_CW)>>2));
					/* if emulating */
				} else /* no ndp state to display */
					pts.pt_rval = 0;
			} else
				u.u_error = EINVAL;
			break;
		case PTRACE_WR_TXT:
			if (XMODE_286) {
				putuwd(NBPS+pts.pt_addr, pts.pt_data);
				break;
			}
			/* Fall through for 386 mode processes. */
		case PTRACE_WR_DAT:
			putuwd(pts.pt_addr, pts.pt_data);
			break;
		case PTRACE_WR_USR:
			/* See ptrace.h for valid offsets. */
			off = (unsigned)pts.pt_addr;

			if (off & 3)
				u.u_error = EINVAL;
			else if (off < PTRACE_FP_CW) {
				/* Writing CPU general register state */
				if (off == PTRACE_SIG)
					u.u_error = EINVAL;
				else
					u.u_regl[off>>2] = pts.pt_data;
			} else if (off < PTRACE_DR0) {
				if (rdNdpUser()) {
					/*
					 * Writing NDP state.
					 * If NDP state not already saved, save it.
					 * Store desired info.
					 * Restore NDP state in case we will resume.
					 */
					if (!rdNdpSaved()) {
						ndpSave(&u.u_ndpCon);
						wrNdpSaved(1);
					}
((int *)&u.u_ndpCon)[(off - PTRACE_FP_CW)>>2] = pts.pt_data;
					ndpRestore(&u.u_ndpCon);
					wrNdpSaved(0);
				} else if (fstp && ndpKfrstor) {
putuwd(((int *)fstp) + ((off - PTRACE_FP_CW)>>2), pts.pt_data);
					doEmUnpack = 1;
				}
			} else
				u.u_error = EINVAL;
			break;
		case PTRACE_RESUME:
			u.u_regl[EFL] &= ~MFTTB;
			goto sig;
		case PTRACE_TERM:
			sign = SIGKILL;
			break;
		case PTRACE_SSTEP:
			u.u_regl[EFL] |= MFTTB;
		sig:
			if (pts.pt_data<0 || pts.pt_data>NSIG) {
				u.u_error = EINVAL;
				break;
			}
			sign = pts.pt_data;
			if (pts.pt_addr != SIG_IGN) {
				u.u_regl[EIP] = pts.pt_addr;
			}
			break;
		default:
			u.u_error = EINVAL;
		}
		if ((pts.pt_errs=u.u_error) == EFAULT)
			pts.pt_errs = EINVAL;
		pts.pt_busy = 0;
		wakeup((char *)&pts.pt_busy);
	}
	if (doEmUnpack)
		(*ndpKfrstor)(fstp, &u.u_ndpCon);
	return (sign);
}

/*
 * Steal space on user stack for packed form of emulator context.
 */
static struct _fpstate *
empack()
{
	int uesp;
	int sphi, splo;
	SEG * segp;
	cseg_t * pp;
	struct _fpstate * ret = 0;
	extern void (*ndpKfsave)();
	unsigned long sw_old;

	/* If not emulating, do nothing */
	if (rdNdpUser() || !rdEmTrapped() || !ndpKfsave)
		return ret;

	/*
	 * Will copy at least u_sigreturn, _fpstackframe, and ndpFlags.
	 * If using ndp, need room for an _fpstate.
	 * If emulating, need room for an _fpemstate.
	 */
	uesp = u.u_regl[UESP] - sizeof(struct _fpstate);

	/* Add to user stack if necessary. */
	segp = u.u_segl[SISTACK].sr_segp;
	sphi = (XMODE_286) ? ISP_286 : ISP_386;
	splo = sphi - segp->s_size;

	if (splo > uesp) {
		pp = c_extend(segp->s_vmem, btoc(segp->s_size));
		if (pp==0) {
			printf("Empack failed.  cmd=%s  c_extend(%x,%x)=0 ",
			  u.u_comm, segp->s_vmem, btoc(segp->s_size));
			return ret;
		}

		segp->s_vmem = pp;
		segp->s_size += NBPC;
		if (sproto(0)==0) {
			printf("Empack failed.  cmd=%s  sproto(0)=0 ",
			  u.u_comm);
			return ret;
		}

		segload();
	}

	ret = (struct _fpstate *)uesp;
	(*ndpKfsave)(&u.u_ndpCon, uesp);
	sw_old = getuwd(&ret->sw);
	putuwd(&ret->status, sw_old);
	putuwd(&ret->sw, sw_old & 0x7f00);

	return ret;
}
