/*
 * File:	msig.c
 *
 * Purpose:	signal handler start and return
 *
 * $Log:	msig.c,v $
 * Revision 1.1  93/01/13  15:47:36  root
 * Initial revision
 * 
 */

/*
 * ----------------------------------------------------------------------
 * Includes.
 */
#include <sys/coherent.h>

/*
 * ----------------------------------------------------------------------
 * Definitions.
 *	Constants.
 *	Macros with argument lists.
 *	Typedefs.
 *	Enums.
 */

/*
 * ----------------------------------------------------------------------
 * Functions.
 *	Import Functions.
 *	Export Functions.
 *	Local Functions.
 */
void	msigend();
void	msigstart();

/*
 * ----------------------------------------------------------------------
 * Global Data.
 *	Import Variables.
 *	Export Variables.
 *	Local Variables.
 */
extern void	(*ndpKfsave)();
extern void	(*ndpKfrstor)();

/*
 * ----------------------------------------------------------------------
 * Code.
 */

/*
 * msigstart(signum, func)
 *
 * signum is 1-based signal number
 * func points to signal handler in user text,
 *   or func is magic value (SIG_DFL, etc.)
 *
 * This routine will set up the stack as shown before entering the user
 * signal handler:
 *
 *	ndp/emulator context (struct _fpstate or struct _fpemstate or absent)
 *	ndp/emulator flags
 *	fpstackframe:
 *		wsp (Weitek context pointer - always null, but part of BCS)
 *		fpsp (floating point context pointer, possibly null)
 *		CPU register set (SS+1 long registers)
 *		1-based signal number
 *	u.u_sigreturn (in place of user return address)
 */

/*
 * A special define for signal stack arithmetic:
 * Will copy at least u_sigreturn, _fpstackframe, and ndpFlags.
 */
#define SIG_AREA_BASE	(sizeof(struct _fpstackframe) + 2 * sizeof(long))

void
msigstart(signum, func)
{
	register int uesp;
	int sphi, splo;
	SEG * segp;
	cseg_t * pp;
	int sigArea;	/* number of bytes written to user's stack */
	struct _fpstate * fpsp;

	--signum;	/* convert to zero-based signal number */
	if (SELF->p_dsig & (1<<signum))
		SELF->p_hsig |= 1 << signum;

	/*
	 * Will copy at least u_sigreturn, _fpstackframe, and ndpFlags.
	 * If using ndp, need room for an _fpstate.
	 * If emulating, need room for an _fpemstate.
	 */
	sigArea = SIG_AREA_BASE;
	if (rdNdpUser() || rdEmTrapped())
		sigArea += sizeof(struct _fpstate);
	uesp = u.u_regl[UESP] - sigArea;

	/* Add to user stack if necessary. */
	segp = u.u_segl[SISTACK].sr_segp;
	sphi = (XMODE_286) ? ISP_286 : ISP_386;
	splo = sphi - segp->s_size;

	if (splo > uesp) {
		pp = c_extend(segp->s_vmem, btoc(segp->s_size));
		if (pp==0) {
			printf("Signal failed.  cmd=%s  c_extend(%x,%x)=0 ",
			  u.u_comm, segp->s_vmem, btoc(segp->s_size));
			return;
		}

		segp->s_vmem = pp;
		segp->s_size += NBPC;
		if (sproto(0)==0) {
			printf("Signal failed.  cmd=%s  sproto(0)=0 ",
			  u.u_comm);
			return;
		}

		segload();
	}

	/*
	 * Set the ndp/emulator context pointer fpsp.
	 * Fp context is immediately above SIG_AREA_BASE.
	 */
	if (rdNdpUser() || rdEmTrapped())
		fpsp = (struct _fpstate *)(uesp + SIG_AREA_BASE);
	else
		fpsp = 0;

	/*
	 * Write fpsp and wsp (Weitek state pointer always null).
	 */
	putuwd(uesp + (SS+3) * sizeof(long), fpsp);
	putuwd(uesp + (SS+4) * sizeof(long), 0);

	kucopy(u.u_regl, uesp + 2*sizeof(long), (SS+1) * sizeof(long));
	putuwd(uesp+sizeof(long), signum+1);
	putuwd(uesp, u.u_sigreturn);
	u.u_regl[EFL] &= ~MFTTB;
	u.u_regl[EIP] = func;
	u.u_regl[UESP] = uesp;

	/* Unhook the signal handler. */
	if (signum+1 != SIGTRAP) {
		u.u_sfunc[signum] = SIG_DFL;
		SELF->p_dfsig |= (1 << signum);
	}


	/*
	 * We are about to enter a signal handling function for the process.
	 * If current process is using ndp
	 *   copy ndp state and related flags into signal handler stack
	 *   mark the process as not using ndp
	 *   arm EM traps in case signal handler uses ndp
	 * Else if process is using emulator
	 *   copy emulator state and flags into signal handler stack
	 *   mark the process as not using emulator
	 * Else
	 *   put ndp/emulator flags on stack
	 */
	if (rdNdpUser()) {
		/* if ndp state not saved yet for this process, save it now */
		if (!rdNdpSaved()) {
			ndpSave(&u.u_ndpCon);
			wrNdpSaved(1);
		}

		putuwd(uesp + (SS+5) * sizeof(long), u.u_ndpFlags);

		kucopy(&u.u_ndpCon, fpsp, sizeof(struct _fpstate));
		ndpDetach();

		wrNdpUser(0);
		wrNdpSaved(0);
		ndpEmTraps(1);
	} else if (rdEmTrapped()) {
		putuwd(uesp + (SS+5) * sizeof(long), u.u_ndpFlags);
		if (ndpKfsave) {
			unsigned long sw_old;
			(*ndpKfsave)(&u.u_ndpCon, fpsp);
			sw_old = getuwd(&fpsp->sw);
			putuwd(&fpsp->status, sw_old);
			putuwd(&fpsp->sw, sw_old & 0x7f00);
		}
		wrEmTrapped(0);
	} else {
		putuwd(uesp + (SS+5) * sizeof(long), u.u_ndpFlags);
	}
}

void
msigend(gs, fs, es, ds, edi, esi, ebp, esp, ebx, edx, ecx, eax, trapno, err,
  eip, cs, efl, uesp, ss)
{
	register int signo;
	register PROC *pp = SELF;
	int savedNdpFlags;
	int sigNdpUser;

	u.u_regl = &gs;

	/*
	 * BOGUS - assumes nothing clobbers user stack.
	 * There is a small probability that the u_sigreturn code,
	 * which is
	 *	add	$4,%esp
	 *	lcall	$0xf,$0
	 * might get a signal hit between the first and second instructions.
	 * This will clobber the value being fetched to signo.
	 */
	signo = getuwd(uesp-sizeof(long)) - 1; 

	savedNdpFlags = getuwd(uesp + (SS+3) * sizeof(long));

	sigNdpUser = rdNdpUser();
	u.u_ndpFlags = savedNdpFlags;

	/*
	 * We are about to leave a signal handling function for this process.
	 * If signal function for this process was using ndp
	 * And main process was *not* using ndp
	 *   Detach signal function from ndp
	 *   Restore current EM to its pre-signal value.
	 * If main process *was* using ndp
	 *   restore its ndp state and make it ndp owner again.
	 * If main process was using emulator
	 *   restore emulator state.
	 */
	if (sigNdpUser && !rdNdpUser()) {
		ndpDetach();
		ndpEmTraps(1);
	}

	if (rdNdpUser()) {
		ndpEmTraps(0);
		ukcopy(uesp + (SS+4)*sizeof(long), &u.u_ndpCon,
		  sizeof(struct _fpstate));
		ndpRestore(&u.u_ndpCon);
		wrNdpSaved(0);
		ndpMine();
	} else if (rdEmTrapped()) {
		if (ndpKfrstor)
			(*ndpKfrstor)(uesp + (SS+4)*sizeof(long), &u.u_ndpCon);
	}

	/* Restore process state to pre-signal values. */
	ukcopy(uesp, u.u_regl, (SS+1) * sizeof(long));

	/*
	 * If the signal has been sigset simulate a sigrelse(signal).
	 */
	if (pp->p_hsig & 1<<signo) {
		pp->p_hsig &= ~(1 << signo);
		if (nondsig()) {
			actvsig();
		}
	}
}
