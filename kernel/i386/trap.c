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


#define READ_CR0	1
#define WRITE_CR0	2
#define READ_CR2	3
#define READ_CR3	4
#define WRITE_CR3	5
#define HALT		6
#define IRET		7
#define WRITE_DR7	8

#define ENTER_OP	0xC8	/* Opcode for 'enter' instruction.  */
#define IRET_RETRY_LIM	10
#define RESUME_FLAG	0x10000	/* RF bit in PSW */

extern unsigned char selkcopy();
extern unsigned int DR0,DR1,DR2,DR3,DR7;
static int trap_op();

/*
 * Macro RDUMP does register dump, followed by final message.
 * If "do_panic" is nonzero, the macro ends with a panic;
 * otherwise, keep going.
 *
 * Callable only from within trap().
 */
#define RDUMP() { \
  printf("\neax=%x  ebx=%x  ecx=%x  edx=%x\n", eax, ebx, ecx, edx); \
  printf("esi=%x  edi=%x  ebp=%x  esp=%x\n", esi, edi, ebp, esp); \
  printf("cs=%x  ds=%x  es=%x  ss=%x  fs=%x  gs=%x\n", \
    cs&0xffff, ds&0xffff, es&0xffff, ss&0xffff, fs&0xffff, gs&0xffff); \
  printf("err #%d eip=%x  uesp=%x  cmd=%s\n", err, eip, uesp, u.u_comm); \
  printf("efl=%x  ", efl); }
/* end RDUMP */

/*
 * Debug only - display 64 words of stack traceback.
 */
#define SDUMP(frame) { \
  int *ip = frame, i; \
  for (i=0;i < 32;i++) { \
    if ((i % 8)==0) \
      putchar('\n'); \
    printf("%x ", *ip++); \
  } \
  putchar('\n'); \
}
/* end SDUMP */

extern unsigned int	__xtrap_on__;
extern unsigned int	__xtrap_break__;
extern unsigned int	__xtrap_off__;
extern unsigned int	_Idle;

/*
 * iretct is cleared in trap(), incremented and tested in gpfault().
 */
static int iretct;

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
char *eip;
{
	register struct	systab	*stp;
	register int	callnum;
	register int	sigcode;
	extern int	trapcode;
	extern	*mmdata[], mminit;
	cseg_t *pp;
	register SEG *segp;
	int	splo, datahi;
	unsigned int	txtlo, txthi;
	unsigned char opcode;	/* Opcode we trapped on.	*/
	unsigned short e_arg;	/* Argument to 'enter' opcode.  */
	unsigned long newsp;	/* Anticipated value for stack pointer.  */
	unsigned int cr2 = 0;
	unsigned int cpl;
	iretct = 0;

	/*
	 * Avoid sign extension confusion on 286 ds
	 */
	if (ds == (SEG_286_UD | R_USR))
		uesp = (unsigned short)uesp;

	if (err==SINMI)
		panic("Parity error: cs=%x ip=%x\n", cs, eip);

	/*
	 * Expect this to never happen!
	 */
	if ((SELF->p_flags&PFKERN) != 0) {
		panic("pid%d: kernel process trap: err=%x, ip=%x ax=%d",
			SELF->p_pid, err, eip, eax);
	}

	T_HAL(0x4000, printf("T%d ", err));
	sigcode = 0;

	u.u_regl = &gs;	/* hook in register set for consave/conrest */

	switch (err) {
	case SIOSYS:
		/*
		 * 286 System call.
		 */
		sigcode = oldsys();
		break;
	case SISYS:
		/*
		 * 386 System call.
		 */
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

		efl &= ~MFCBIT;		/* clear carry flag in return efl */
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
#if 0
	case SIGP:
		/*
		 * General protection.
		 */
		sigcode = SIGSEGV;
		break;
#endif
	case SIPF:
		cr2 = read_cr2();
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
		 * Catch bad function pointer here - don't want to restart
		 * the user instruction and get runaway segv's.
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
			goto bad_pf;
		}

		/*
		 * Catch bad data pointer here - don't want to restart
		 * the user instruction and get runaway segv's.
		 */
		if (cr2 > splo) {
			T_HAL(0x1000, printf("Bad data, splo=%x datahi=%x\n",
			  splo, datahi));
			goto bad_pf;
		}

		/*
		 * If we trapped on an 'enter' instruction, the stack
		 * pointer (uesp) has not yet been decremented.  In
		 * order to correctly process such a stack overflow,
		 * we must look at the _expected_ value for uesp.
		 * NB: We COPY uesp, because that arg gets loaded back
		 * into the real esp--when we return from the trap the
		 * enter instruction will decrement the esp.
		 */
		newsp = uesp;
		opcode = selkcopy(cs, eip);
		if (ENTER_OP == opcode) {
			e_arg = (selkcopy(cs, eip+2)<<8) + selkcopy(cs, eip+1);
			newsp -= e_arg;
		}

		if (newsp<=splo && newsp>datahi && btoc(datahi)<btocrd(splo)) {
			pp = c_extend(segp->s_vmem, btoc(segp->s_size));
			if (pp==0) {
				T_HAL(0x1000, printf("c_extend(%x,%x)=0 ",
				  segp->s_vmem, btoc(segp->s_size)));
				goto bad_pf;
			}

			segp->s_vmem = pp;
			segp->s_size += NBPC;
			if (sproto(0)==0) {
				T_HAL(0x1000, printf("sproto(0)=0 "));
				goto bad_pf;
			}

			segload();
			goto trapend;
		}
	bad_pf:
		/*
		 * User generated unacceptable page fault.
		 */
		sigcode = SIGSEGV;
		printf("\ncr2=%x  ", cr2);
		break;

	default:
		RDUMP();
		panic("Fatal Trap");
	}

trapend:
	if (sigcode) {
		RDUMP();
		printf("sigcode=#%d  User Trap\n", sigcode);
		sendsig(sigcode, SELF);
	}
	if (efl&0x10000) {
#if 0
		RDUMP();
		panic("Ring 1 V8086");
#endif
		efl &= 0xffff;
	}
}

/*
 * Debug-only routine to report every IRQPDth occurrence of all irq's.
 *
 */
#if 0
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

/*
 * trap_op()
 *
 * Look at the trapped instruction.
 * If it's one of a select few, recognize and return the type.
 * Otherwise, return 0.
 *
 * This could be fancier, but all we want to look for is:
 *	0F 20 C0	READ_CR0
 *	0F 22 C0	WRITE_CR0
 *	0F 20 D0	READ_CR2
 *	0F 20 D8	READ_CR3
 *	0F 22 D8	WRITE_CR3
 *	CF		IRET
 *	F4		HALT
 *	0F 23 F8	WRITE_DR7
 */
static int
trap_op(cs,eip)
unsigned int cs, eip;
{
	int		ret = 0;

	switch (selkcopy(cs,eip)) {
	case 0x0F:
		switch (selkcopy(cs,eip+1)) {
		case 0x20:
			switch (selkcopy(cs,eip+2)) {
			case 0xC0:
				ret = READ_CR0;
				break;
			case 0xD0:
				ret = READ_CR2;
				break;
			case 0xD8:
				ret = READ_CR3;
				break;
			}
			break;
		case 0x22:
			switch (selkcopy(cs,eip+2)) {
			case 0xC0:
				ret = WRITE_CR0;
				break;
			case 0xD8:
				ret = WRITE_CR3;
				break;
			}
			break;
		case 0x23:
			switch (selkcopy(cs,eip+2)) {
			case 0xF8:
				ret = WRITE_DR7;
				break;
			}
			break;
		}
		break;
	case 0xF4:
		ret = HALT;
		break;
	case 0xCF:
		ret = IRET;
		break;
	}
	return ret;
}

/*
 * Kernel debugger.
 *
 * Runs in ring 0.
 */
__debug__(gs, fs, es, ds, edi, esi, ebp, esp, ebx, edx, ecx, eax, trapno,
  err, eip, cs, efl, uesp, ss)
{
	unsigned int dr6 = read_dr6();

	RDUMP();
	printf("Breakpoint  ");
	if (dr6 & 0xf) {	/* report breakpoint exception(s) */
		if (dr6 & 1)
			printf("DR0=%x  ", DR0);
		if (dr6 & 2)
			printf("DR1=%x  ", DR1);
		if (dr6 & 4)
			printf("DR2=%x  ", DR2);
		if (dr6 & 8)
			printf("DR3=%x  ", DR3);
		printf("DR7=%x\n", DR7);
	}
	if (dr6 &  0xf000) {	/* report other debug exception(s) */
		if (dr6 & 0x8000)
			printf("Switch to debugged task\n");
		if (dr6 & 0x4000)
			printf("Single step\n");
		if (dr6 & 0x2000) {
			printf("ICE in use\n");
			eip += 3;
		}
	}
	write_dr6(0);
	efl |= RESUME_FLAG;
}

/*
 * General protection fault handler.
 * Entered via a ring 0 gate.
 */
gpfault(gs, fs, es, ds, edi, esi, ebp, esp, ebx, edx, ecx, eax, trapno, err,
  eip, cs, efl, uesp, ss)
char *eip;
{
	unsigned short cpl;

	/*
	 * Switch on CPL of code that trapped.
	 */
	cpl = cs & SEG_PL;
	switch(cpl) {
	case DPL_0:
		/*
		 * Ring 0 should not gp fault.
		 */
		RDUMP();
		T_HAL(0x1000, SDUMP(&uesp));
		panic("System GP Fault 0");
		break;
	case DPL_1:
		/*
		 * If ring 1 faulted on a valid request, emulate the
		 * request while running in ring 0.
		 */
		switch(trap_op(cs,eip)) {
		case READ_CR0:
			eax = read_cr0();
			eip += 3;
			break;
		case WRITE_CR0:
			if (eax & 4)
				setfpe(0);
			else
				setfpe(1);
			eip += 3;
			break;
		case READ_CR2:
			eax = read_cr2();
			eip += 3;
			break;
		case READ_CR3:
			eax = read_cr3();
			eip += 3;
			break;
		case WRITE_CR3:
			mmuupd();
			eip += 3;
			break;
		case IRET:
			/*
			 * Some CPU's wrongly generate GP faults on IRET
			 * from inner ring to ring 3.
			 * Fix is to retry the instruction a few times.
			 */
			iretct++;
			if (iretct > IRET_RETRY_LIM) {
				RDUMP();
				SDUMP(uesp);
				panic("System GP Fault 1 - iret");
			}
			break;
		case WRITE_DR7:
			/*
			 * Expect breakpoint info in globals DR0-3,DR7.
			 */
printf("Setting DR0=%x  DR1=%x  DR2=%x  DR3=%x  DR7=%x\n",
  DR0, DR1, DR2, DR3, DR7);
			write_dr0(DR0);
			write_dr1(DR1);
			write_dr2(DR2);
			write_dr3(DR3);
			write_dr7(DR7);
			eip += 3;
			break;
		default:
			if (eip >= &__xtrap_on__ && eip < &__xtrap_off__) {
				SET_U_ERROR(EFAULT, "copy service");
				eip = &__xtrap_break__;
			} else {
				RDUMP();
				T_HAL(0x1000, SDUMP(uesp));
				panic("System GP Fault 1");
			}
		}
		goto gpdone;
		break;
	case DPL_2:
		/*
		 * Nothing should be running in Ring 2.
		 */
	case DPL_3:
		/*
		 * Ring 3 gp fault means errant user process.
		 */
		RDUMP();
		printf("User GP Violation\n");
		sendsig(SIGSEGV, SELF);
		break;
	}
gpdone:
	if (efl & 0xffff0000) {
		efl &= 0xffff;
	}
	return;
}
