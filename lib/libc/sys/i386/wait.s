//////////
/ libc/sys/i386/wait.s
/ Copyright (c) Ciaran O'Donnell, Bievres (FRANCE), 1991.
//////////

//////////
/ int
/ wait(statusp) int *statusp;
//////////

        .text
	.globl	wait
	.globl	.cerror

wait:
	movl	$7,%eax
	lcall	$0x7,$0
	jc	.cerror

	movl	4(%esp), %ecx
	orl	%ecx,%ecx
	je	?L1
	movl	%edx,(%ecx)
?L1:
	ret

/ end of libc/sys/i386/wait.s
