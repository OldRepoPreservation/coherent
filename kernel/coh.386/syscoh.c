/*
 * File:	syscoh.c
 *
 * Purpose:	Functions for the COHERENT-specific system call
 *
 * $Log$
 */

/*
 * ----------------------------------------------------------------------
 * Includes.
 */
#include <sys/coherent.h>
#include <sys/con.h>
#include <errno.h>

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
int ucohcall();

static int devload();
static int setfpe();

/*
 * ----------------------------------------------------------------------
 * Global Data.
 *	Import Variables.
 *	Export Variables.
 *	Local Variables.
 */

/*
 * ----------------------------------------------------------------------
 * Code.
 */

/*
 * Only allow this if running as superuser.
 *
 * a1		call type
 * ----------	----------
 * COH_PRINTF	kernel printf
 * COH_DEVLOAD	call load() routine for device with major number a2
 * COH_SETFPE	a2=0, trap on FP;  a2!=0, allow FP
 * COH_SETBP	a2=bp#,a3=addr,a4=type,a5=len;  set kernel breakpoint
 * COH_CLRBP	a2=bp#;  clear kernel breakpoint
 * COH_REBOOT	reboot
 */
ucohcall(a1,a2,a3,a4,a5,a6)
{
	int ret = 0;

	if (!super()) {
		SET_U_ERROR(EPERM, "cohcall, must be root");
		goto ucc_done;
	}

	switch(a1) {
	case	COH_PRINTF:
		printf(a2);
		break;
	case	COH_DEVLOAD:
		ret = devload(a2);
		break;
	case	COH_SETFPE:
		ret = setfpe(a2);
		break;
	case	COH_SETBP:
		ret = setbp(a2,a3,a4,a5);
		break;
	case	COH_CLRBP:
		ret = clrbp(a2);
		break;
	case	COH_REBOOT:
		ret = boot();
		break;
	default:
		SET_U_ERROR(EINVAL, "bad COH function");
	}
ucc_done:
	return ret;
}

/*
 * Initialize a device.
 */
int
devload(maj_num)
int maj_num;
{
	int ret = -1;
	int mask = 1<<maj_num;

	if (dev_loaded & mask) {
		SET_U_ERROR(EIO, "already loaded");
		goto dldone;
	}

	if (drvl[maj_num].d_conp == 0) {
		SET_U_ERROR(EIO, "no driver");
		goto dldone;
	}

	if (drvl[maj_num].d_conp->c_load) {
		(*drvl[maj_num].d_conp->c_load)();
		dev_loaded |= mask;
		ret = 0;
	}
dldone:
	return ret;
}

unsigned int DR0,DR1,DR2,DR3,DR7;
/*
 * Set a kernel breakpoint.
 */
int
setbp(bp_num, addr, type, len)
unsigned int bp_num, addr, type, len;
{
	/* Range check arguments.
	 * Update RAM images of writeable debug registers.
	 * Call routine (while in RING 1) which will cause GP fault.
	 * GP Fault handler (in RING 0) will copy RAM images to DR's.
	 */
	if (bp_num >= 4 || type >= 4 || len >= 4 || type == 2 || len == 2) {
		SET_U_ERROR(EINVAL, "bad bp setting");
		return -1;
	}
	switch(bp_num) {
	case 0:
		DR0 = addr;
		write_dr0(DR0);
		DR7 |= ((type<<16)|(len<<18)|0x303);
		break;
	case 1:
		DR1 = addr;
		write_dr1(DR1);
		DR7 |= ((type<<20)|(len<<22)|0x30C);
		break;
	case 2:
		DR2 = addr;
		write_dr2(DR2);
		DR7 |= ((type<<24)|(len<<26)|0x330);
		break;
	case 3:
		DR3 = addr;
		write_dr3(DR3);
		DR7 |= ((type<<28)|(len<<30)|0x3C0);
		break;
	}
	write_dr7(DR7);
	return 0;
}

/*
 * Clear a kernel breakpoint.
 */
int
clrbp(bp_num)
unsigned int bp_num;
{
	/* Range check arguments.
	 * Update RAM images of writeable debug registers.
	 * Call routine (while in RING 1) which will cause GP fault.
	 * GP Fault handler (in RING 0) will copy RAM images to DR's.
	 */
	if (bp_num >= 4) {
		SET_U_ERROR(EINVAL, "bad bp # to clear");
		return -1;
	}
	switch(bp_num) {
	case 0:
		DR7 &= ~0x3;
		break;
	case 1:
		DR7 &= ~0xC;
		break;
	case 2:
		DR7 &= ~0x30;
		break;
	case 3:
		DR7 &= ~0xC0;
		break;
	}
	if ((DR7 & 0xFF) == 0)
		DR7 &= ~0x300;
	write_dr7(DR7);
	return 0;
}
