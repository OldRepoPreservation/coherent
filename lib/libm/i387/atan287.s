//////////
/ /usr/src/libm/i387/atan287.s
/ i387 mathematics library
/ atan2(y, x)
//////////

RASIZE	=	4

	.globl	atan2
	.globl	_cfcc
	.globl	_tstcc

//////////
/ double
/ atan2(y, x)
/ double y, x;
//////////

y	=	RASIZE		/ y arg offset
x	=	RASIZE+8	/ x arg offset

atan2:
	fldl	y(%esp)		/ Load argument y.
	fldl	x(%esp)		/ Load argument x.

	call	_tstcc
	jne	?1		/ Jump if x nonzero.
	fcompp			/ x = 0, compare 0 to y and pop x and y.
	call	_cfcc
	fld1			/ 1
	fchs			/ -1
	fldpi			/ pi, -1
	fscale			/ pi/2, -1
	fstp	%st(1)		/ pi/2
	jbe	?0		/ 0 <= y, return pi/2.
	fchs			/ 0 > y, return -pi/2.

?0:
	ret

?1:
	pushf			/ Save flags with sign of x.
	fld	%st(1)		/ y, x, y
	fxch			/ x, y, y
	fpatan			/ atan(y/x), y
	popf			/ Restore flags.
	jb	?2		/ x < 0, must adjust by pi.
	fstp	%st(1)		/ atan(y/x)
	ret

?2:
	fxch			/ y, atan(y/x)
	call	_tstcc
	fstp	%st		/ atan(y/x)
	fldpi			/ pi, atan(y/x)
	jae	?3		/ y >= 0, add pi.
	fchs			/ y < 0, subtract pi.

?3:
	fadd			/ atan(y/x) + pi
	ret

/ end of atan287.s
