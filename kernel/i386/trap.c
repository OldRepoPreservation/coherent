/* (lgl-
 *	COHERENT Version 4.0
 *	Copyright (c) 1982, 1992.
 *	An unpublished work by Mark Williams Company, Chicago.
 *	All rights reserved.
 -lgl) */
#include <sys/coherent.h>
#include <sys/reg.h>
#include <sys/systab.h>
#include <errno.h>
#include <sys/proc.h>
#include <sys/seg.h>
#include <signal.h>

#define ENTER_OP	0xC8	/* Opcode for 'enter' instruction.  */

/*
 * Macro RDUMP does register dump, followed by final message.
 * If "do_panic" is nonzero, the macro ends with a panic;
 * otherwise, keep going.
 *
 * Callable only from within trap().
 */
#define RDUMP() { \
  printf("\neax=%x\tebx=%x\tecx=%x\tedx=%x\n", eax, ebx, ecx, edx); \
  printf("esi=%x\tedi=%x\tebp=%x\tesp=%x\n", esi, edi, ebp, esp); \
  printf("cs=%x\tds=%x\tes=%x\tss=%x\n", \
    cs&0xffff, ds&0xffff, es&0xffff, ss&0xffff); \
  printf("err #%d eip=%x\tuesp=%x\tcmd=%s\n", err, eip, uesp, u.u_comm); \
  printf("efl=%x\tcr2=%x\tsig=%d\ttrap_ip=%x\n", efl, cr2, sigcode, trapno); \
  printf("trapcode=%x\t", trapcode); }
/* end RDUMP */

/*
 * Debug only - display 64 words of stack traceback.
 */
#define SDUMP() { \
  int *ip = &uesp, i; \
  for (i=0;i < 64;i++) { \
    if ((i % 8)==0) \
      putchar('\n'); \
    printf("%x ", *ip++); \
  } }
/* end SDUMP */

/*
 * Trap handler.
 * The arguments are the registers,
 * saved on the stack by machine code. This call
 * is different from most C calls in that the registers
 * get copied back; if you change a "trap" parameter then
 * the machine register will be altered when the trap is
 * dismissed.
 *
 * Argument "trapno" is the return eip for the code calling tsave().
 */
trap(gs, fs, es, ds, edi, esi, ebp, esp, ebx, edx, ecx, eax, trapno, err,
  eip, cs, efl, uesp, ss)
{
	register struct	systab	*stp;
	register int	callnum;
	register int	sigcode;
	extern int	trapcode;
#ifdef EVENTS
	EVENT *evp;
#endif
	extern	*mmdata[], mminit;
	cseg_t *pp;
	register SEG *segp;
	int	splo, datahi;
	unsigned int	txtlo, txthi;
	unsigned char opcode;	/* Opcode we trapped on.  */
	unsigned short e_arg;	/* Argument to 'enter' opcode.  */
	unsigned long newsp;	/* Anticipated value for stack pointer.  */
	unsigned int cr2 = read_cr2();

	if (ds == (SEG_286_UD | R_USR))
		uesp = (unsigned short)uesp;

	u.u_regl = &gs;
#ifdef EVENTS
	evp = evtrap();
#endif

	if (err==SINMI)
		panic("Parity error: cs=%x ip=%x\n", cs, eip);

	T_HAL(0x40000, goto foo);
	if ((ds&cs&R_USR) != R_USR) {
foo:
		RDUMP();
		T_HAL(0x1000, SDUMP());
		panic("\nSystem Trap");
	}

	/*
	 * Expect this to never happen!
	 */
	if ((SELF->p_flags&PFKERN) != 0) {
		panic("pid%d: kernel process trap: err=%x, ip=%x ax=%d",
			SELF->p_pid, err, eip, eax);
	}

	/*
	 * System call.
	 */
	T_HAL(0x4000, printf("T%d ", err));
	sigcode = 0;
	switch (err) {
	case SIOSYS:
#ifdef EVENTS
		sigcode = oldsys(evp);
#else
		sigcode = oldsys();
#endif
		break;
	case SISYS:
		u.u_error = 0;
		callnum = eax;

		T_PIGGY(4, printf("{%d}", callnum));

		if (callnum >= NMICALL) {
			if (callnum == COHCALL)
				stp = &cohcall;
			else {
				sigcode = SIGSYS;
				goto trapend;
			}
		} else
			stp = sysitab + callnum;
		ukcopy(uesp+sizeof(long),u.u_args, stp->s_nargs*sizeof(long));
		if (u.u_error != 0) {
			sigcode = SIGSYS;
			goto trapend;
		}
#ifdef EVENTS
		for (l=0; l<stp->s_nargs; l++)
			evp->a[l+1] = u.u_args[l];
		evp->a[0] = stp->s_nargs;
		evp->func = stp->s_func;
#endif
		u.u_io.io_seg = IOUSR;
		if (envsave(&u.u_sigenv) != 0)
			u.u_error = EINTR;
		else {
			eax = (*stp->s_func)(u.u_args[0],
			      u.u_args[1],
			      u.u_args[2],
			      u.u_args[3],
			      u.u_args[4],
			      u.u_args[5]);
			edx = u.u_rval2;
		}
#ifdef EVENTS
		evp->err = u.u_error;
		evp->res = eax;
#endif
		efl &= ~MFCBIT;
		if (u.u_error) {
			eax = u.u_error;
			efl |= MFCBIT;
		}
		break;
		/*
		 * Trap.
		 */
	case SIDIV:
		sigcode = SIGFPE;
		break;

	case SISST:
		sigcode = SIGTRAP;
		break;

	case SIBPT:
		sigcode = SIGTRAP;
		break;

	case SIOVF:
		sigcode = SIGEMT;
		break;

	case SIBND:
		/*
		 * Bound
		 */
		sigcode = SIGIOT;
		break;

	case SIOP:
		/*
		 * Invalid opcode
		 */
		sigcode = SIGILL;
		break;

	case SIXNP:
		/*
		 * Processor extension not available
		 */
		sigcode = SIGFPE;
		break;

	case SIDBL:
		/*
		 * Double exception
		 */
		panic("double exception: cs=%x ip=%x", cs, eip);
		sigcode = SIGSEGV;
		break;

	case SIXS:
		/*
		 * Processor extension segment overrun
		 */
		sigcode = SIGSEGV;
		break;

	case SITS:
		/*
		 * Invalid task state segment
		 */
		panic("invalid tss: cs=%x ip=%x", cs, eip);
		sigcode = SIGSEGV;
		break;

	case SINP:
		/*
		 * Segment not present
		 */
		sigcode = SIGSEGV;
		break;

	case SISS:
		/*
		 * Stack segment overrun/not present
		 */
		sigcode = SIGKILL;
		break;

	case SIGP:
		/*
		 * General protection.
		 */
		sigcode = SIGSEGV;
		break;
	case SIPF:
		/*
		 * Page fault
		 * 
		 * check for stack underflow
		 */

		/*
		 * I think 'splo' is being calculated in a bass-ackwards way,
		 * and that 'datahi' is just wrong, but I'm not certain,
		 * so the fixes are #if 0'd out. -piggy
		 *
		 * I'll take out the 0 some day and test these changes.
		 */
		segp = u.u_segl[SISTACK].sr_segp;
#if 0
		splo = u.u_segl[SISTACK].sr_base - segp->s_size;
		datahi = u.u_segl[SIPDATA].sr_base + u.u_segl[SIPDATA].sr_size;
#else
		splo = (XMODE_286) ? ISP_286 : ISP_386;
		splo -= segp->s_size;
		datahi = u.u_segl[SIPDATA].sr_size;
#endif /* 0 */

		/*
		 * Will get double fault if eip is out of valid user text.
		 *
		 * For 286 executables, eip starts at 0, but cs points to
		 * descriptor SEG_286_UII which adds 0x400000 (UII_BASE).
		 */
		txtlo = u.u_segl[SISTEXT].sr_base;
		if (XMODE_286)
			txtlo -= UII_BASE;
		txthi = txtlo + u.u_segl[SISTEXT].sr_size;
		if (eip < txtlo || eip > txthi) {
			T_HAL(0x1000, printf("Bad eip, txtlo=%x txthi=%x\n",
			  txtlo, txthi));
			goto bad;
		}

		/*
		 * Will get double fault accessing data out of range.
		 */
		if (cr2 > splo) {
			T_HAL(0x1000, printf("Bad data, splo=%x datahi=%x\n",
			  splo, datahi));
			goto bad;
		}

		/*
		 *
		 * If we trapped on an 'enter' instruction, the stack
		 * pointer (uesp) has not yet been decremented.  In
		 * order to correctly process such a stack overflow,
		 * we must look at the _expected_ value for uesp.
		 * NB: We COPY uesp, because that arg gets loaded back
		 * into the real esp--when we return from the trap the
		 * enter instruction will decrement the esp.
		 */
		newsp = uesp;
		opcode = (char) selkcopy(cs, eip);
		if (ENTER_OP == opcode) {
			e_arg = (selkcopy(cs, eip+2)<<8) + selkcopy(cs, eip+1);
			newsp -= e_arg;
		}

		if (newsp<=splo && newsp>datahi && btoc(datahi)<btocrd(splo)) {
			pp = c_extend(segp->s_vmem, btoc(segp->s_size));
			if (pp==0) {
				goto bad;
			}

			segp->s_vmem = pp;
			segp->s_size += ctob(1);
			if (sproto(0)==0) {
				goto bad;
			}

			segload();
			return;
		}
	bad:
		sigcode = SIGSEGV;
		break;

	default:
		RDUMP();
		panic("Fatal User Trap");
	}

trapend:
	if (sigcode) {
		RDUMP();
		printf("User Trap\n");
		sendsig(sigcode, SELF);
	}
}

/*
 * Debug-only routine to report every IRQPDth occurrence of all irq's.
 *
 */
#ifdef TRACER
static int irqct[16];
int 	IRQPD=64;

void
irqblab(gs, fs, es, ds, edi, esi, ebp, esp, ebx, edx, ecx, eax,
  trapno, err, eip, cs, efl, uesp, ss)
{
	int irqno = (err >> 8) & 0xff;

	/* flag every IRQPDth interrupt of any kind */
	if (t_hal & 0x20000) {
		if ((err & 0xff) == 0x40) { /* if hardware interrupt */
			if ((irqno & 0xf0) == 0) {
				irqct[irqno]++;
				if ((irqct[irqno] % IRQPD) == 0)
					printf("I%d ", irqno);
			}
		}
	}
}
#endif
