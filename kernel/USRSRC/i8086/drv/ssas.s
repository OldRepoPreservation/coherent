////////
/
/ I/O for Seagate ST01/ST02 SCSI Host Adapters.
/
/ $Log:	/usr/src/sys/i8086/drv/RCS/ssas.s,v $
/ Revision 1.1	91/05/16  14:16:21	root
/ Initial version - no code yet for ss_put().
/ 
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
	.globl	ss_put_

////////
/
/ Constants
/	Also defined in /usr/src/sys/i8086/sys/ss.h:
/		SS_CSR  SS_DAT  RS_REQUEST
/
////////

	SS_CSR	= 0x1A00
	SS_DAT	= 0x1C00

	REQ_LIM = 200
	RS_REQUEST = 0x10

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

////////
/
/ int ss_put(ss_fp, buf_fp, count)
/ faddr_t ss_fp, buf_fp;
/ int count;
/
/ Write output bytes to host adapter from buffer address.
/ Count must be <= SS_RAM_LEN (0x400).
/
/ Return 0 if timeout occurred, otherwise nonzero.
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

ss_put_:
	push	bp
	mov	bp, sp
	push	es
	push	di
	push	ds
	push	si 
	lds	si, 8(bp)	/ buf_fp to DS:SI
	les	di, 4(bp)	/ ss_fp  to ES:DI
	mov	bx, di		/ .. and to ES:BX
	add	di, $SS_DAT	/ ss_dat to ES:DI
	add	bx, $SS_CSR	/ ss_csr to ES:BX
	mov	cx, 12(bp)	/ count to CX

P01:				/ start of 2 loops
	mov	ax, $REQ_LIM	/ max # of times to look for REQ
	testb	es:(bx), $RS_REQUEST
	jne	P02
	dec	ax
	jnz	P01
	jmp	P03

P02:				/ got REQ - ok to write a byte
	movsb
	loop	P01
P03:				/ all done - now restore registers
	pop	si
	pop	ds
	pop	di
	pop	es
	pop	bp
	ret
