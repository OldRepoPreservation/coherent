/ C library - cerror	Copyright (c) Ciaran O'Donnell, Bievres (FRANCE), 1991
	.globl	.cerror
	.globl	errno
	.comm	errno,4
.cerror:
	movl	%eax,errno
	movl	$-1,%eax
	orl	%eax,%eax
	ret
