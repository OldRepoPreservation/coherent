////////
/ libc/crt/i386/scount.s
/ i386 C runtime library.
/ Profile call counter.
////////

	.globl	_flst_
	.globl	_scount

////////
/ Profile count routine.
/ Called from the entry sequence of every function
/ compiled with the -VPROF profile option.
/ On entry:
/	%ecx	pointer to 12 byte block in BSS
/ The block looks like this:
/	.long	?	; count
/	.long	?	; link into _flst_
/	.long	?	; pc
////////

_scount:
	incl	(%ecx)			/ bump count
	pop	%edx			/ get ra in EDX
	cmpl	8(%ecx),$0		/ linked already?
	jne	?L0			/ yes, done
	movl	8(%ecx),%edx		/ save function pc
	movl	%eax,_flst_		/ and link the
	movl	4(%ecx),%eax		/ block into the
	movl	_flst_,%ecx		/ chain.

?L0:
	ijmp	%edx			/ return

/ end of libc/crt/i386/scount.s
