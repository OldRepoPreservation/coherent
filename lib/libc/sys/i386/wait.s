/ C library - wait -	Copyright (c) Ciaran O'Donnell, Bievres (FRANCE), 1991
/	pid = wait(&status);

        .text
	.globl	wait
	.globl	.cerror

wait:
	movl	$7,%eax
	lcall	$0x7,$0
	jc	.cerror

	movl	4(%esp), %ecx
	orl	%ecx,%ecx
	je	nostat
	movl	%edx,(%ecx)
nostat:	ret
