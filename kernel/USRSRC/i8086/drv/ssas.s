////////
/
/ I/O for Seagate ST01/ST02 SCSI Host Adapters.
/
/ $Log$
/
/	Since these functions are called from the midst of C code in
/	the "ss" driver, they need to preserve the following registers:
/		SI  DI  SP  BP    SS  DS  ES
/	Additionally, surrounding C code is expected to leave the "D"
/	CPU flag clear (string op's increment index registers).
/
////////

////////
/
/	Export functions.
/
////////
	.globl	ss_get_
/	.globl	ss_put_

////////
/
/ Constants
/	These are also defined in /usr/src/sys/i8086/sys/ss.h
/
////////

	SS_CSR	= 0x1A00
	SS_DAT	= 0x1C00

////////
/
/ ss_get(ss_fp, buf_fp, count)
/ faddr_t ss_fp, buf_fp;
/ int count;
/
/ Fetch input bytes from host adapter and store at buffer address.
/ Count must be <= SS_RAM_LEN (0x400).
/
/ Here is the stack after initial "push bp":
/
/	12(bp)	count
/	10(bp)	FP_SEL(buf_fp)
/	8(bp)	FP_OFF(buf_fp)
/	6(bp)	FP_SEL(ss_fp)
/	4(bp)	FP_OFF(ss_fp)
/	2(bp)	return IP
/	0(bp)	old bp
/
////////

ss_get_:
	push	bp
	mov	bp, sp
	push	es
	push	di
	push	ds
	push	si

	lds	si, 4(bp)	/ ss_fp  to DS:SI
	add	si, $SS_DAT	/ ss_dat to DS:SI
	les	di, 8(bp)	/ buf_fp to ES:DI
	mov	cx, 12(bp)	/ count to CX
	rep
	movsb

	pop	si
	pop	ds
	pop	di
	pop	es
	pop	bp
	ret
