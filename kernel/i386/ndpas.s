/ ndpas.s - assembler support for COH386 ndp
	.unixorder
	.globl	ndpSave
	.globl	ndpRestore
	.globl	ndpInit

/ void ndpInit(short cw);
ndpInit:
	fninit
	fldcw	4(%esp);
	fwait
	ret

/ void ndpSave(char * bp);
ndpSave:
	mov	4(%esp),%eax
	fnsave	(%eax)
	fwait
	ret

/ void ndpRestore(char * bp);
ndpRestore:
	mov	4(%esp),%eax
	frstor	(%eax)
	ret
