//////////
/ db/i386/i386db4.s
/ A debugger.
/ i386 assembly language support.
//////////

//////////
/ double
/ get_fp_reg(struct _fpreg *fpregp)
/
/ Load %st with the 80-bit NDP register to which fpregp points.
//////////

	.intelorder
	.text
	.globl	get_fp_reg
	
get_fp_reg:
	movl	%ecx, 4(%esp)		/ fpregp to ECX
	fldt	(%ecx)			/ fetch 80-bit value
	ret				/ and return it in %st0

/ end of db/i386/i386db.h
