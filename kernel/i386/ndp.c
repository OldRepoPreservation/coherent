/*
 * File:	ndp.c
 *
 * Purpose:	all ndp-related functions, other than low-level stuff
 *
 * $Log$
 */

/*
 * ----------------------------------------------------------------------
 * Includes.
 */
#include <sys/coherent.h>
#include <errno.h>
#include <sys/seg.h>

/*
 * ----------------------------------------------------------------------
 * Definitions.
 *	Constants.
 *	Macros with argument lists.
 *	Typedefs.
 *	Enums.
 */
/* bit positions in u.u_ndpFlags */
#define NF_NDP_USER	1	/* this process has used the ndp */
#define NF_NDP_SAVED	2	/* ndp status is saved in u area */

/*
 * ----------------------------------------------------------------------
 * Functions.
 *	Import Functions.
 *	Export Functions.
 *	Local Functions.
 */
void ndpConRest();
void ndpNewOwner();
void ndpNewProc();
void ndpEndProc();
int rdNdpUser();
int rdNdpSaved();
void wrNdpUser();
void wrNdpSaved();
void wrNdpSavedU();
void ndpEmTraps();
void ndpDetach();
void ndpMine();

/*
 * ----------------------------------------------------------------------
 * Global Data.
 *	Import Variables.
 *	Export Variables.
 *	Local Variables.
 */

/*
 * ndp control word is 16 bits:
 * 0000 RC:2 PC:2 01 PM:1 UM:1 OM:1 ZM:1 DM:1 IM:1
 * RC - rounding control
 * PC - precision control
 * PM - precision mask
 * UM - underflow mask
 * OM - overflow mask
 * ZM - zero divide mask
 * DM - denormal operand mask
 * IM - invalid operation mask
 *
 * for masks, 1 masks the exception
 * iBCS2 page 3-46 specifies the following:
 *   0000 : 00 10 : 0 1 1 1 : 0 0 1 0
 */
short ndpCW = 0x0272;
short ndpDump = 0;

static int	kerEm = 1;	/* RAM copy of CR0 EM bit */
static int	ndpUseg;	/* system global address of U segment */
static PROC *	ndpOwner;	/* process whose stuff is now in ndp */

/*
 * ----------------------------------------------------------------------
 * Code.
 */

/*
 * Called from trap handler the first time a process executes an ndp
 * instruction.
 */
void
ndpNewOwner()
{
	UPROC *		up;

	/* disable further emulator traps for this process */
	wrNdpUser(1);
	ndpEmTraps(0);

	/* save old ndp status, if any process was using it */
	if (ndpOwner) {
		ptable1_v[WORK0] = sysmem.u.pbase[btocrd(ndpUseg)] | SEG_RW;
		mmuupd();
		up = (UPROC *)(ctob(WORK0) + U_OFFSET);
		ndpSave(&up->u_ndpCon);
		wrNdpSavedU(1, up);
	}

	/* Make current process NDP owner */
	ndpMine();

	/* give process a clean ndp */
	ndpInit(ndpCW);
}

/*
 * NDP initialization for a new process.
 * Called at exec time.
 * Sets defaults, before it is known whether the process uses NDP or not.
 */
void
ndpNewProc()
{
	/* default for a process is to trap on NDP instructions */
	ndpEmTraps(1);
	wrNdpUser(0);
	wrNdpSaved(0);
}

/*
 * Restore some ndp info when doing a regular conrest().
 * Called just after conrest - u area has just been restored.
 */
void
ndpConRest()
{
	UPROC *		up;

	/* make CR0 EM bit match what this process needs */
	ndpEmTraps(rdNdpUser()^1);

	/* if current process uses ndp, may need to fix ndp state */
	if (rdNdpUser() && ndpOwner != SELF) {
		if (ndpOwner) {		/* save old ndp state */
			ptable1_v[WORK0] = sysmem.u.pbase[btocrd(ndpUseg)] | SEG_RW;
			mmuupd();
			up = (UPROC *)(ctob(WORK0) + U_OFFSET);
			ndpSave(&up->u_ndpCon);
			wrNdpSavedU(1, up);
		}

		/* Make current process NDP owner and reload ndp state */
		ndpMine();
		ndpRestore(&u.u_ndpCon);
		wrNdpSaved(0);
	}
}

/*
 * When a process exits, it relinquishes the ndp.
 */
void
ndpEndProc()
{
	if (SELF == ndpOwner)
		ndpDetach();
}

/*
 * fptrap()
 *
 * Entered when NDP generates a CPU error.
 * err is either SIFP or 0x0D40
 */
#define RDUMP() { \
  printf("\neax=%x  ebx=%x  ecx=%x  edx=%x\n", eax, ebx, ecx, edx); \
  printf("esi=%x  edi=%x  ebp=%x  esp=%x\n", esi, edi, ebp, esp); \
  printf("cs=%x  ds=%x  es=%x  ss=%x  fs=%x  gs=%x\n", \
    cs&0xffff, ds&0xffff, es&0xffff, ss&0xffff, fs&0xffff, gs&0xffff); \
  printf("err #%d eip=%x  uesp=%x  cmd=%s\n", err, eip, uesp, u.u_comm); \
  printf("efl=%x  ", efl); }

fptrap(gs, fs, es, ds, edi, esi, ebp, esp, ebx, edx, ecx, eax, trapno, err,
  eip, cs, efl, uesp, ss)
char *eip;
{
	register int	sigcode;
	unsigned short	sw;		/* ndp status word */
	struct _fpstate * fsp = &u.u_ndpCon;

	if (err == SIFP)
		u.u_regl = &gs;	/* hook in register set for consave/conrest */

	sigcode = SIGFPE;
	/*
	 * Send user a signal.
	 */
	ndpSave(fsp);
	wrNdpSaved(1);
	if (ndpDump) {
		RDUMP();
		printf("\nfcs=%x  fip=%x  fos=%x  foo=%x\n",
		  fsp->cssel&0xffff, fsp->ipoff,
		  fsp->datasel&0xffff, fsp->dataoff);
		printf("sigcode=#%d  User Floating Point Trap: ", sigcode);
		sw = fsp->sw;
		if (sw & 1)
			printf("Invalid Operation");
		else if (sw & 2)
			printf("Denormalized Operand");
		else if (sw & 4)
			printf("Divide by Zero");
		else if (sw & 8)
			printf("Overflow");
		else if (sw & 0x10)
			printf("Underflow");
		else if (sw & 0x20)
			printf("Precision");
		else
			printf("???");
	}
}

/*
 * Routines concerned with whether current process has used the ndp.
 */
int
rdNdpUser()
{
	return (u.u_ndpFlags & NF_NDP_USER) ? 1 : 0;
}

void
wrNdpUser(n)
int n;
{
	if (n)
		u.u_ndpFlags |= NF_NDP_USER;
	else
		u.u_ndpFlags &= ~NF_NDP_USER;
}

/*
 * Since saving NDP state is destructive, we need to keep track
 * of where the current NDP state is - u area, or NDP?
 */
int
rdNdpSaved()
{
	return (u.u_ndpFlags & NF_NDP_SAVED) ? 1 : 0;
}

void
wrNdpSaved(n)
int n;
{
	if (n)
		u.u_ndpFlags |= NF_NDP_SAVED;
	else
		u.u_ndpFlags &= ~NF_NDP_SAVED;
}

void
wrNdpSavedU(n, up)
int n;
UPROC * up;
{
	if (n)
		up->u_ndpFlags |= NF_NDP_SAVED;
	else
		up->u_ndpFlags &= ~NF_NDP_SAVED;
}

/*
 * Enable (1) or disable (0) emulator traps.
 */
void
ndpEmTraps(n)
int n;
{
	if (kerEm != n) {
		kerEm = n;
		setEm(n);
	}
}

/*
 * Make ndp owned by no one.
 */
void
ndpDetach()
{
	ndpOwner = 0;
	ndpUseg = 0;
}

/*
 * Make ndp owned by the current process.
 */
void
ndpMine()
{
	SR *		srp = &(u.u_segl[SIUSERP]);
	SEG *		sp = srp->sr_segp;

	ndpOwner = SELF;
	ndpUseg = MAPIO(sp->s_vmem, U_OFFSET);
}
