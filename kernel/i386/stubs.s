/ stubs.s - functions we don't care about for installation kernel.

	.globl	msgpoll
	.globl	umsgctl
	.globl	umsgget
	.globl	umsgrcv
	.globl	umsgsnd
	.globl	usemctl
	.globl	usemget
	.globl	usemop
	.globl	ushmctl
	.globl	ushmget

msgpoll:
umsgctl:
umsgget:
umsgrcv:
umsgsnd:
usemctl:
usemget:
usemop:
ushmctl:
ushmget:
	ret
