/ C library - fork	Copyright (c) Ciaran O'Donnell, Bievres (FRANCE), 1991

	.globl	fork
	.globl	.cerror

fork:
	movl	$2,%eax
	lcall	$0x7,$0
	jc	.cerror
	orl	%edx,%edx
	jz	forkret				/ return pid	(parent)
	xorl	%eax,%eax			/ return 0	(child)
forkret:
	orl	%eax,%eax
	ret
